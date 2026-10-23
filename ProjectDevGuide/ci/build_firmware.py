#!/usr/bin/env python3
"""build_firmware.py - build RoboticArmMain the way STM32CubeIDE does, without CubeIDE.

Used by CI (.github/workflows/firmware-ci.yml). Anyone can run it locally too:

    python3 ProjectDevGuide/ci/build_firmware.py                   # Debug configuration
    python3 ProjectDevGuide/ci/build_firmware.py --config Release

Needs arm-none-eabi-gcc on PATH (Ubuntu: apt install gcc-arm-none-eabi libnewlib-arm-none-eabi).

Everything project-specific is READ FROM .cproject: source folders, defines, include paths,
library paths, libraries and the linker script. So if someone breaks .cproject (for example a
wrong include path), this build breaks exactly like CubeIDE does.

Fails (exit 1) on:
  - any compile or link error (this includes FLASH / RAM overflow, the linker reports it)
  - any warning in our own code under App/ (third-party KISS FFT and App/lcd/driver/ are exempt)
Warnings in generated code (Core/, Drivers/, X-CUBE-AI/) are printed but do not fail.
"""
import argparse
import concurrent.futures as cf
import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

CPU_FLAGS = ["-mcpu=cortex-m7", "-mfpu=fpv5-d16", "-mfloat-abi=hard", "-mthumb"]
EXPECTED_MCU = "STM32F767ZITx"
DMA_REGION_START = 0x2007C000   # MPU region 1 + RAM_DMA in the linker script (core-config)
DMA_REGION_SIZE = 16 * 1024

# third-party code in App/ that we do not write ourselves, so its warnings do not fail the build:
# KISS FFT (voice) and the Lab08 ILI9341 driver copied into App/lcd/driver/. Our own code must stay warning-free.
APP_WARNING_EXEMPT = re.compile(r"App/voice/(kiss_fft|kiss_fftr|_kiss_fft_guts|kiss_fft_log)\.[ch]|App/lcd/driver/")

OPT_LEVELS = {  # CubeIDE option value suffix -> gcc flag
    "none": "-O0", "more": "-O2", "most": "-O3", "size": "-Os", "debug": "-Og", "optimize": "-O1",
    "speed": "-Ofast",
}


def fail(msg):
    print(f"::error::{msg}")
    sys.exit(1)


def read_cproject(project: Path, config: str):
    tree = ET.parse(project / ".cproject")
    cfg = None
    for c in tree.iter("cconfiguration"):
        for sm in c.findall("storageModule"):
            if sm.get("moduleId") == "org.eclipse.cdt.core.settings" and sm.get("name") == config:
                cfg = c
    if cfg is None:
        fail(f".cproject has no '{config}' configuration")

    def options(fragment):
        """values of every <option> whose superClass ends with fragment"""
        out = []
        for opt in cfg.iter("option"):
            if opt.get("superClass", "").endswith(fragment):
                vals = [v.get("value") for v in opt.findall("listOptionValue")]
                out.append(vals if vals else opt.get("value"))
        return out

    def one_list(fragment):
        found = options(fragment)
        return found[0] if found and isinstance(found[0], list) else []

    mcu = options("option.target_mcu")
    if not mcu or mcu[0] != EXPECTED_MCU:
        fail(f".cproject MCU is {mcu}, expected {EXPECTED_MCU}")

    opt = options("tool.c.compiler.option.optimization.level")
    opt_flag = "-O0" if config == "Debug" else "-Os"  # CubeIDE defaults when the option is absent
    if opt and isinstance(opt[0], str):
        suffix = opt[0].rsplit(".", 1)[-1]
        opt_flag = OPT_LEVELS.get(suffix, opt_flag)

    script = options("tool.c.linker.option.script")
    if not script:
        fail(".cproject has no linker script")
    script_name = script[0].replace("${workspace_loc:/${ProjName}/", "").rstrip("}")

    sources = []
    for e in cfg.iter("entry"):
        if e.get("kind") == "sourcePath":
            excl = [x for x in (e.get("excluding") or "").split("|") if x]
            sources.append((e.get("name") or "", excl))

    return {
        "c_defines": one_list("tool.c.compiler.option.definedsymbols"),
        "as_defines": one_list("tool.assembler.option.definedsymbols"),
        "includes": one_list("tool.c.compiler.option.includepaths"),
        "lib_dirs": one_list("tool.c.linker.option.directories"),
        "libs": one_list("tool.c.linker.option.libraries"),
        "script": script_name,
        "opt": opt_flag,
        "sources": sources,
    }


def collect_sources(project: Path, sources):
    files = []
    for name, excluded in sources:
        root = project / name if name else project
        for p in sorted(root.rglob("*")):
            if p.suffix not in (".c", ".s", ".S") or not p.is_file():
                continue
            rel = p.relative_to(project).as_posix()
            rel_in_folder = p.relative_to(root).as_posix()
            if any(rel_in_folder == x or rel_in_folder.startswith(x.rstrip("/") + "/") for x in excluded):
                continue
            files.append(rel)
    return files


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("project", nargs="?", default="RoboticArmMain")
    ap.add_argument("--config", default="Debug")
    ap.add_argument("--out", default=None, help="build folder (default build/ci-<project>-<config>)")
    args = ap.parse_args()

    project = Path(args.project).resolve()
    if not (project / ".cproject").is_file():
        fail(f"{project}/.cproject not found")
    out = Path(args.out or f"build/ci-{project.name}-{args.config}").resolve()
    out.mkdir(parents=True, exist_ok=True)

    s = read_cproject(project, args.config)
    gcc = "arm-none-eabi-gcc"
    ver = subprocess.run([gcc, "--version"], capture_output=True, text=True).stdout.splitlines()
    print(f"compiler: {ver[0] if ver else 'not found'}")
    print(f"config  : {args.config} {s['opt']}  defines {s['c_defines']}")
    print(f"sources : {[n for n, _ in s['sources']]}")
    print(f"includes: {s['includes']}")

    # CubeIDE runs gcc from inside <project>/<config>/, so every "../x" in .cproject means <project>/x.
    # We do the same: cwd = a folder one level below the project.
    cwd = project / args.config
    cwd.mkdir(exist_ok=True)

    files = collect_sources(project, s["sources"])
    if not files:
        fail("no source files found")
    print(f"files   : {len(files)}")

    cflags = (CPU_FLAGS + ["-std=gnu11", "-g3", s["opt"], "-ffunction-sections", "-fdata-sections",
                           "-Wall", "--specs=nano.specs", "-c"]
              + [f"-D{d}" for d in s["c_defines"]] + [f"-I{i}" for i in s["includes"]])
    asflags = (CPU_FLAGS + ["-g3", "-x", "assembler-with-cpp", "--specs=nano.specs", "-c"]
               + [f"-D{d}" for d in s["as_defines"]])

    def compile_one(rel):
        obj = out / (rel.replace("/", "__") + ".o")
        flags = asflags if rel.endswith((".s", ".S")) else cflags
        r = subprocess.run([gcc] + flags + [f"../{rel}", "-o", str(obj)],
                           cwd=cwd, capture_output=True, text=True)
        return rel, obj, r.returncode, r.stderr

    objs, errors, warn_lines = [], 0, []
    with cf.ThreadPoolExecutor(max_workers=os.cpu_count() or 2) as ex:
        for rel, obj, rc, err in ex.map(compile_one, files):
            objs.append(str(obj))
            if err.strip():
                print(err.rstrip())
            if rc != 0:
                errors += 1
                print(f"::error file=RoboticArmMain/{rel}::compile failed")
            warn_lines += [l for l in err.splitlines() if ": warning:" in l]
    try:
        cwd.rmdir()  # only removes it if it was empty (we never write there)
    except OSError:
        pass
    if errors:
        fail(f"{errors} file(s) failed to compile")

    elf = out / f"{project.name}.elf"
    mp = out / f"{project.name}.map"
    ldflags = (CPU_FLAGS + [f"-T{project / s['script']}", "--specs=nosys.specs", "--specs=nano.specs",
                            f"-Wl,-Map={mp}", "-Wl,--gc-sections", "-static"]
               + [f"-L{(project / args.config / d).resolve()}" for d in s["lib_dirs"]])
    libs = [f"-l{l}" for l in s["libs"]]
    r = subprocess.run([gcc] + ldflags + ["-o", str(elf)] + objs + libs
                       + ["-Wl,--start-group", "-lc", "-lm", "-Wl,--end-group"],
                       capture_output=True, text=True)
    if r.stderr.strip():
        print(r.stderr.rstrip())
    warn_lines += [l for l in r.stderr.splitlines() if "warning:" in l]
    if r.returncode != 0:
        fail("link failed (see above): 'region ... overflowed' = firmware too big, 'undefined reference' = a source folder or function is missing")

    size = subprocess.run(["arm-none-eabi-size", str(elf)], capture_output=True, text=True).stdout
    print(size.rstrip())

    # every DMA buffer must sit in the non-cacheable SRAM2 window the MPU sets up (dma_buf.h)
    hdr = subprocess.run(["arm-none-eabi-objdump", "-h", str(elf)], capture_output=True, text=True).stdout
    m = re.search(r"\s\.dma_buffer\s+([0-9a-f]+)\s+([0-9a-f]+)", hdr)
    if not m or int(m.group(2), 16) != DMA_REGION_START:
        fail(f".dma_buffer section is not at 0x{DMA_REGION_START:08X} (linker script edit missing?)")
    if int(m.group(1), 16) > DMA_REGION_SIZE:
        fail(f".dma_buffer is 0x{m.group(1)} bytes, more than the 16 KB non-cacheable region")
    print(f"dma     : .dma_buffer 0x{int(m.group(1), 16):X} bytes at 0x{DMA_REGION_START:08X}")

    app_warn = [l for l in warn_lines if "App/" in l and not APP_WARNING_EXEMPT.search(l)]
    other = len(warn_lines) - len(app_warn)
    print(f"warnings: {len(app_warn)} in App/ (must be 0), {other} in generated or third-party code (allowed)")
    if app_warn:
        for l in app_warn:
            print(f"::error::{l}")
        fail("fix the warnings in App/ above")
    print(f"OK: {elf}")


if __name__ == "__main__":
    main()
