#!/usr/bin/env python3
"""check_rules.py - project rules a PR must not break (things a build alone does not catch).

Used by CI (.github/workflows/firmware-ci.yml). Run it locally from the repo root:

    python3 tools/ci/check_rules.py

Every rule prints PASS or FAIL with the reason. Exit code 1 if any rule fails.
To change a rule, open a PR that edits this file (core owner reviews it).
"""
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

PROJECT = "RoboticArmMain"
MAX_FILE_BYTES = 3 * 1024 * 1024        # biggest legit file today: the CMSIS device header, ~1.5 MB

# files that must never be committed (build output, per-user IDE state, local overrides)
FORBIDDEN = [
    (r"(^|/)Debug/", "CubeIDE build output"),
    (r"(^|/)Release/", "CubeIDE build output"),
    (r"(^|/)\.settings/", "per-user IDE settings"),
    (r"\.launch$", "per-user debug launch file"),
    (r"(^|/)\.metadata/", "CubeIDE workspace folder (keep the workspace outside the repo)"),
    (r"(^|/)MXTmpFiles/", "CubeMX temp files"),
    (r"(^|/)\.ai/", "X-CUBE-AI analyze cache"),
    (r"App/common/app_config_local\.h$", "your local feature flags (gitignored on purpose)"),
    (r"\.(elf|hex|bin|map|o|su|cyclo)$", "build output"),
]

SOURCE_FOLDERS = {"App", "Core", "Drivers", "X-CUBE-AI"}
AI_RUNTIME_LIB = ":NetworkRuntime1020_CM7_GCC.a"
LINKER_SCRIPT = "STM32F767ZITX_FLASH.ld"

# generated files that hold our USER CODE (CubeMX keeps only what is between the markers)
GENERATED_WITH_USER_CODE = ["Core/Src", "Core/Inc", "X-CUBE-AI/App"]

# where the single definitions live (two definitions = link error, a copy elsewhere = confusion)
SINGLE_HOME = {
    "__io_putchar": "App/common/app_log.c",
    "HAL_*Callback": "App/app_callbacks.c",
}

results = []


def rule(name, ok, why=""):
    results.append((name, ok, why))


def tracked_files(root: Path):
    try:
        out = subprocess.run(["git", "ls-files"], cwd=root, capture_output=True, text=True, check=True).stdout
        return [l for l in out.splitlines() if l]
    except Exception:
        return [p.relative_to(root).as_posix() for p in root.rglob("*") if p.is_file() and ".git/" not in p.as_posix()]


def user_block(text, tag):
    """text between /* USER CODE BEGIN tag */ and /* USER CODE END tag */, or None"""
    m = re.search(r"/\*\s*USER CODE BEGIN " + re.escape(tag) + r"\s*\*/(.*?)/\*\s*USER CODE END "
                  + re.escape(tag) + r"\s*\*/", text, re.S)
    return m.group(1) if m else None


def strip_comments(code):
    code = re.sub(r"/\*.*?\*/", "", code, flags=re.S)
    return re.sub(r"//[^\n]*", "", code)


def main():
    root = Path(__file__).resolve().parents[2]
    proj = root / PROJECT
    files = tracked_files(root)

    # 1. nothing that must stay local is committed
    bad = []
    for f in files:
        if not f.startswith(PROJECT + "/"):
            continue
        for pat, why in FORBIDDEN:
            if re.search(pat, f):
                bad.append(f"{f} ({why})")
                break
    rule("no build output / IDE state / local config committed", not bad,
         "remove with 'git rm --cached <file>': " + ", ".join(bad[:10]))

    # 2. no huge files
    big = [f"{f} ({(root / f).stat().st_size // 1024} KB)" for f in files
           if (root / f).is_file() and (root / f).stat().st_size > MAX_FILE_BYTES]
    rule(f"no file over {MAX_FILE_BYTES // (1024 * 1024)} MB", not big, ", ".join(big))

    # 3. .cproject: both configurations compile App/ and find its headers
    try:
        tree = ET.parse(proj / ".cproject")
        configs = {}
        for c in tree.iter("cconfiguration"):
            for sm in c.findall("storageModule"):
                if sm.get("moduleId") == "org.eclipse.cdt.core.settings":
                    configs[sm.get("name")] = c
        for want in ("Debug", "Release"):
            c = configs.get(want)
            if c is None:
                rule(f".cproject {want}: exists", False, "configuration missing")
                continue
            srcs = {e.get("name") for e in c.iter("entry") if e.get("kind") == "sourcePath"}
            missing = SOURCE_FOLDERS - srcs
            rule(f".cproject {want}: source folders {sorted(SOURCE_FOLDERS)}", not missing,
                 f"missing {sorted(missing)}: Properties > C/C++ General > Paths and Symbols > "
                 f"Configuration {want} > Source Location > Add Folder")
            inc, libs, script = [], [], ""
            for o in c.iter("option"):
                sc = o.get("superClass", "")
                vals = [v.get("value") for v in o.findall("listOptionValue")]
                if sc.endswith("tool.c.compiler.option.includepaths"):
                    inc = vals
                elif sc.endswith("tool.c.linker.option.libraries"):
                    libs = vals
                elif sc.endswith("tool.c.linker.option.script"):
                    script = o.get("value") or ""
            rule(f".cproject {want}: include path ../App", "../App" in inc and "../../App" not in inc,
                 f"include paths are {inc}. In Paths and Symbols > Includes type 'App' (the dialog adds '../'); "
                 f"'../App' typed there becomes '../../App'")
            rule(f".cproject {want}: links the X-CUBE-AI runtime", AI_RUNTIME_LIB in libs, f"libraries are {libs}")
            rule(f".cproject {want}: linker script {LINKER_SCRIPT}", script.endswith(LINKER_SCRIPT + "}"),
                 f"script is '{script}'")
    except (ET.ParseError, FileNotFoundError) as e:
        rule(".cproject readable", False, str(e))

    # 4. main.c hands over to App/
    main_c = (proj / "Core/Src/main.c").read_text(errors="replace")
    inc_b, b2, b3 = (user_block(main_c, t) for t in ("Includes", "2", "3"))
    rule("main.c: #include \"app.h\" in USER CODE Includes", inc_b is not None and '#include "app.h"' in inc_b)
    rule("main.c: app_init(); in USER CODE 2", b2 is not None and "app_init();" in strip_comments(b2))
    rule("main.c: app_loop(); in USER CODE 3", b3 is not None and "app_loop();" in strip_comments(b3))

    # 5. X-CUBE-AI glue: create the network only, never loop
    ai_c = (proj / "X-CUBE-AI/App/app_x-cube-ai.c").read_text(errors="replace")
    b5, b6, bi = (user_block(ai_c, t) for t in ("5", "6", "includes"))
    rule("app_x-cube-ai.c: kws_model.h in USER CODE includes", bi is not None and "voice/kws_model.h" in bi)
    rule("app_x-cube-ai.c: ai_boostrap() in USER CODE 5", b5 is not None and "ai_boostrap(" in b5)
    rule("app_x-cube-ai.c: USER CODE 6 is empty (the template body never returns)",
         b6 is not None and strip_comments(b6).strip() == "", "code found in USER CODE 6")

    # 6. USER CODE markers intact in generated files (a lost marker = code deleted on next Generate)
    broken = []
    for d in GENERATED_WITH_USER_CODE:
        for p in sorted((proj / d).glob("*.[ch]")):
            t = p.read_text(errors="replace")
            begins = re.findall(r"USER CODE BEGIN (\S+)", t)
            ends = re.findall(r"USER CODE END (\S+)", t)
            if sorted(begins) != sorted(ends):
                broken.append(p.relative_to(proj).as_posix())
    rule("USER CODE BEGIN/END markers paired in generated files", not broken, ", ".join(broken))

    # 7. callbacks and putchar defined in one place only
    pat = re.compile(r"^\s*(?:void|int)\s+(HAL_\w*Callback|__io_putchar)\s*\([^;]*$", re.M)
    wrong = []
    for d in ("Core", "X-CUBE-AI", "App"):
        for p in sorted((proj / d).rglob("*.c")):
            rel = p.relative_to(proj).as_posix()
            for name in pat.findall(p.read_text(errors="replace")):
                home = SINGLE_HOME["__io_putchar" if name == "__io_putchar" else "HAL_*Callback"]
                if rel != home:
                    wrong.append(f"{name} in {rel} (belongs in {home})")
    rule("HAL callbacks only in App/app_callbacks.c, __io_putchar only in App/common/app_log.c",
         not wrong, "; ".join(wrong))

    # 8. the model the .ioc points at is committed
    ioc = (proj / f"{PROJECT}.ioc").read_text(errors="replace")
    models = re.findall(r"ModelStructureFile-[0-9a-f]+=(.+)", ioc)
    missing = [m.strip() for m in models if not (proj / m.strip()).is_file()]
    rule(".ioc model file is committed (relative path)", bool(models) and not missing,
         f"model paths {models}" if models else "no model in .ioc")

    w = max(len(n) for n, _, _ in results)
    for name, ok, why in results:
        print((f"{'PASS' if ok else 'FAIL'}  {name.ljust(w)}").rstrip() + ("" if ok or not why else f"\n      -> {why}"))
    failed = [r for r in results if not r[1]]
    for name, _, why in failed:
        print(f"::error::{name}: {why}")
    print(f"\n{len(results) - len(failed)}/{len(results)} rules pass")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
