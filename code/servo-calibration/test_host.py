#!/usr/bin/env python3
"""Compile the real servo-calibration.ino with tiny Arduino/Servo stubs.

Run from the repository root or from this directory:

    python3 code/servo-calibration/test_host.py

The test needs python3 and g++; it does not need the Arduino toolchain.
"""

from __future__ import annotations

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
SKETCH = HERE / "servo-calibration.ino"

ARDUINO_H = r'''#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

#define F(text) (text)

struct SerialStub {
  std::string input;
  std::size_t inputPos = 0;
  std::string output;

  void begin(long baud) { (void)baud; }
  int available() const {
    return inputPos < input.size() ? static_cast<int>(input.size() - inputPos) : 0;
  }
  int read() {
    if (inputPos >= input.size()) return -1;
    return static_cast<unsigned char>(input[inputPos++]);
  }
  void feed(const std::string &text) { input += text; }

  void print(const char *text) { output += text; }
  void print(char value) { output += value; }
  void print(int value) { output += std::to_string(value); }
  void print(long value) { output += std::to_string(value); }
  void print(unsigned int value) { output += std::to_string(value); }
  void print(unsigned long value) { output += std::to_string(value); }
  void print(float value, int digits = 2) {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.*f", digits, static_cast<double>(value));
    output += buffer;
  }

  void println() { output += '\n'; }
  void println(const char *text) { print(text); println(); }
  void println(char value) { print(value); println(); }
  void println(int value) { print(value); println(); }
  void println(long value) { print(value); println(); }
  void println(unsigned int value) { print(value); println(); }
  void println(unsigned long value) { print(value); println(); }
  void println(float value, int digits = 2) { print(value, digits); println(); }
};

inline SerialStub Serial;

inline bool mockInterruptsEnabled = true;
inline int mockNoInterruptsCount = 0;
inline int mockInterruptsCount = 0;

inline void noInterrupts() {
  mockInterruptsEnabled = false;
  ++mockNoInterruptsCount;
}

inline void interrupts() {
  mockInterruptsEnabled = true;
  ++mockInterruptsCount;
}
'''

SERVO_H = r'''#pragma once
#include "Arduino.h"

#include <vector>

struct MockServoState {
  int attachCount = 0;
  int detachCount = 0;
  int writeCount = 0;
  int lastAttachPin = -1;
  int lastAttachMin = 544;
  int lastAttachMax = 2400;
  int lastPulse = 0;
  bool attached = false;
  bool lastAttachInCritical = false;
  bool lastWriteInCritical = false;
  int attachInCriticalCount = 0;
  int writeInCriticalCount = 0;
  std::vector<int> pulses;

  void reset() {
    attachCount = 0;
    detachCount = 0;
    writeCount = 0;
    lastAttachPin = -1;
    lastAttachMin = 544;
    lastAttachMax = 2400;
    lastPulse = 0;
    attached = false;
    lastAttachInCritical = false;
    lastWriteInCritical = false;
    attachInCriticalCount = 0;
    writeInCriticalCount = 0;
    pulses.clear();
  }
};

inline MockServoState mockServo;

class Servo {
 public:
  void attach(int pin, int minPulse = 544, int maxPulse = 2400) {
    ++mockServo.attachCount;
    mockServo.lastAttachPin = pin;
    mockServo.lastAttachMin = minPulse;
    mockServo.lastAttachMax = maxPulse;
    mockServo.lastAttachInCritical = !mockInterruptsEnabled;
    if (mockServo.lastAttachInCritical) ++mockServo.attachInCriticalCount;
    mockServo.attached = true;
  }
  void detach() {
    ++mockServo.detachCount;
    mockServo.attached = false;
  }
  void writeMicroseconds(int pulseUs) {
    int value = pulseUs;
    if (mockServo.lastAttachMin < mockServo.lastAttachMax) {
      if (value < mockServo.lastAttachMin) value = mockServo.lastAttachMin;
      if (value > mockServo.lastAttachMax) value = mockServo.lastAttachMax;
    }
    ++mockServo.writeCount;
    mockServo.lastPulse = value;
    mockServo.lastWriteInCritical = !mockInterruptsEnabled;
    if (mockServo.lastWriteInCritical) ++mockServo.writeInCriticalCount;
    mockServo.pulses.push_back(value);
  }
  int readMicroseconds() const { return mockServo.lastPulse; }
};
'''

HARNESS = r'''#include "Arduino.h"
#include "Servo.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

void setup();
void loop();

static void send(const std::string &line) {
  Serial.feed(line + "\n");
  int guard = 0;
  while (Serial.available() > 0) {
    loop();
    if (++guard > 100000) {
      std::cerr << "FAIL: loop guard exceeded\n";
      std::exit(1);
    }
  }
}

static std::string run(const std::string &line) {
  Serial.output.clear();
  send(line);
  return Serial.output;
}

static void expect(bool condition, const std::string &message) {
  if (condition) return;
  std::cerr << "FAIL: " << message << "\nLast output:\n" << Serial.output << "\n";
  std::exit(1);
}

static void expectContains(const std::string &text, const std::string &needle,
                           const std::string &message = "expected text") {
  expect(text.find(needle) != std::string::npos, message + " (missing: " + needle + ")");
}

static void expectNoMotion(int attachCount, std::size_t writeCount,
                           const std::string &message) {
  expect(mockServo.attachCount == attachCount, message + ": unexpected attach");
  expect(mockServo.pulses.size() == writeCount, message + ": unexpected write");
}

static std::string withNul(const std::string &prefix, const std::string &suffix) {
  std::string result = prefix;
  result.push_back('\0');
  result += suffix;
  return result;
}

static std::string withByte(const std::string &prefix, unsigned char value) {
  std::string result = prefix;
  result.push_back(static_cast<char>(value));
  return result;
}

static void testStartup() {
  expect(mockServo.attachCount == 0, "setup must not attach a servo");
  expect(mockServo.writeCount == 0, "setup must not command a servo");
  expectContains(Serial.output, "RAM only", "startup must say records are volatile");
  expectContains(Serial.output, "reset loses", "startup must say reset loses records");
  expectContains(Serial.output, "Nothing attached", "startup must say nothing is attached");

  const std::string table = run("table");
  expectContains(table, "4,SG92R-1,,,,,,,0", "unit 4 must be SG92R-1");
  expectContains(table, "5,SG92R-3,,,,,,,0", "unit 5 must keep the SG92R-3 label");
  expectContains(table, "6,SG90,,,,,,,0", "unit 6 must be SG90");
  expect(table.find("SG92R-2") == std::string::npos,
         "retired SG92R-2 must not appear in the active unit list");
}

static void testParser() {
  const char *badLines[] = {
    "select 0",
    "select 7",
    "select x",
    "select 1 extra",
    "help extra",
    "off extra",
    "arm",
    "arm 1500 extra",
    "arm 1e3",
    "arm nan",
    "arm 999999999999999999999",
    "arm -1",
    "arm 499",
    "arm 2501",
    "pulse 1500",
    "record 30",
    "reference",
    "zero",
    "limits 1400",
    "limits 1400 1600 extra",
    "limits 1400 1400",
    "limits 499 1600",
    "limits 1400 2501",
    "installed 1400 1600",
    "window 1400",
    "window 1400 1400",
    "window 499 1600",
    "window 1400 2501",
    "notacommand"
  };

  for (const char *line : badLines) {
    run(line);
    expectNoMotion(0, 0, std::string("invalid line moved hardware: ") + line);
  }

  Serial.output.clear();
  send("select " + std::string(200, 'x'));
  expectContains(Serial.output, "invalid or too-long line", "overlong line must be rejected");
  expectNoMotion(0, 0, "overlong line");

  run("select 1");
  expectNoMotion(0, 0, "select must not move or attach");

  const char *badArm[] = {
    "arm 1500 extra",
    "arm 1e3",
    "arm nan",
    "arm inf",
    "arm 999999999999999999999",
    "arm -1",
    "arm 499",
    "arm 2501",
    "arm 1399",
    "arm 1601"
  };
  for (const char *line : badArm) {
    run(line);
    expectNoMotion(0, 0, std::string("invalid arm moved hardware: ") + line);
  }

  const std::string armOutput = run("arm 1500");
  expectContains(armOutput, "WARNING", "arm must warn about the initial jump");
  expectContains(armOutput, "support", "arm warning must mention support");
  expectContains(armOutput, "No safe pose", "arm must not assume a safe pose");
  expect(mockServo.attachCount == 1, "valid arm must attach once");
  expect(mockServo.lastAttachPin == 9, "arm must use pin 9");
  expect(mockServo.pulses.size() == 1 && mockServo.lastPulse == 1500,
         "valid arm must command its pulse");

  const char *badPulses[] = {
    "pulse 1500 extra",
    "pulse nan",
    "pulse inf",
    "pulse 1e3",
    "pulse 999999999999999999999",
    "pulse -1",
    "pulse 1521",
    "pulse 1399",
    "pulse 1601",
    "pulse 2500"
  };
  const int attaches = mockServo.attachCount;
  const std::size_t writes = mockServo.pulses.size();
  for (const char *line : badPulses) {
    run(line);
    expectNoMotion(attaches, writes, std::string("invalid pulse moved hardware: ") + line);
  }

  const char *badRecords[] = {
    "record",
    "record nan",
    "record inf",
    "record -inf",
    "record 1e999",
    "record 10 extra",
    "record 361",
    "record -361"
  };
  for (const char *line : badRecords) {
    const std::string output = run(line);
    expectContains(output, "ERR", std::string("invalid record must fail: ") + line);
    expectNoMotion(attaches, writes, std::string("invalid record moved hardware: ") + line);
  }

  const std::string table = run("table");
  expectContains(table, "1,MG996R-1,,,,,,,0", "invalid records must not create a point");
}

static void testRecordAndTable() {
  run("select 1");
  run("arm 1500");
  expectContains(run("reference"), "Reference pulse: 1500");
  expectContains(run("record 90.5"), "Recorded 90.50");
  expectContains(run("pulse 1510"), "Pulse 1510");
  expectContains(run("record 95.0"), "Recorded 95.00");
  const std::string offOutput = run("off");
  expectContains(offOutput, "Signal detached", "off must report signal detachment");
  expectContains(offOutput, "position not held/guaranteed",
                 "off must warn that position is not held");

  run("select 1");
  run("arm 1500");
  const int attachesBeforeSelect = mockServo.attachCount;
  const std::size_t writesBeforeSelect = mockServo.pulses.size();
  const int detachesBeforeSelect = mockServo.detachCount;
  const std::string selectOutput = run("select 2");
  expectContains(selectOutput, "signal detached; support arm; position not held/guaranteed",
                 "select must warn that detaching does not hold the arm");
  expect(mockServo.detachCount == detachesBeforeSelect + 1,
         "select while armed must call detach");
  expectNoMotion(attachesBeforeSelect, writesBeforeSelect, "select must not move");

  run("arm 1500");
  expectContains(run("reference"), "Reference pulse: 1500");
  expectContains(run("record 10.25"), "Recorded 10.25");
  run("off");

  const std::string table = run("table");
  expectContains(table, "1,MG996R-1,1500,,,,,,2", "MG996R-1 summary must be isolated");
  expectContains(table, "2,MG996R-2,1500,,,,,,1", "MG996R-2 summary must be isolated");
  expectContains(table, "1,MG996R-1,90.50,1500", "MG996R-1 first point missing");
  expectContains(table, "1,MG996R-1,95.00,1510", "MG996R-1 second point missing");
  expectContains(table, "2,MG996R-2,10.25,1500", "MG996R-2 point missing");
  expect(table.find("2,MG996R-2,90.50") == std::string::npos,
         "MG996R-1 points must not appear under MG996R-2");
  expectContains(table, "# POINTS CSV", "table must print a points CSV");
}

static void testRangeEnforcement() {
  run("select 1");
  expectContains(run("arm 1500"), "WARNING");
  expectContains(run("window 1450 1550"), "detach before changing");
  run("off");

  expectContains(run("window 1450 1550"), "Test window");
  int attaches = mockServo.attachCount;
  std::size_t writes = mockServo.pulses.size();

  expectContains(run("arm 1400"), "outside active guard");
  expectNoMotion(attaches, writes, "arm below test window");
  expectContains(run("arm 1601"), "outside active guard");
  expectNoMotion(attaches, writes, "arm above test window");

  expectContains(run("arm 1450"), "Armed");
  expectContains(run("pulse 1460"), "Pulse 1460");

  writes = mockServo.pulses.size();
  attaches = mockServo.attachCount;
  expectContains(run("pulse 1471"), "step limit");
  expectNoMotion(attaches, writes, "pulse over 10 us step upward");
  expectContains(run("pulse 1470"), "Pulse 1470");
  writes = mockServo.pulses.size();
  attaches = mockServo.attachCount;
  expectContains(run("pulse 1459"), "step limit");
  expectNoMotion(attaches, writes, "pulse over 10 us step downward");
  run("off");

  expectContains(run("limits 1460 1540"), "Unloaded limits");
  attaches = mockServo.attachCount;
  writes = mockServo.pulses.size();
  expectContains(run("arm 1450"), "outside active guard");
  expectNoMotion(attaches, writes, "arm below unloaded limits");
  expectContains(run("arm 1460"), "Armed");
  run("off");

  expectContains(run("installed 1400 1600"), "inside unloaded");
  expectContains(run("installed 1465 1535"), "Installed limits");

  expectContains(run("arm 1465"), "Armed");
  writes = mockServo.pulses.size();
  attaches = mockServo.attachCount;
  expectContains(run("pulse 1464"), "outside active guard");
  expectNoMotion(attaches, writes, "pulse below installed limits");
  run("off");

  expectContains(run("arm 1535"), "Armed");
  writes = mockServo.pulses.size();
  attaches = mockServo.attachCount;
  expectContains(run("pulse 1536"), "outside active guard");
  expectNoMotion(attaches, writes, "pulse above installed limits");
  run("off");

  expectContains(run("installed 1480 1530"), "Installed limits");
  expectContains(run("arm 1500"), "Armed");
  expectContains(run("zero"), "Joint zero pulse: 1500");
  run("off");

  expectContains(run("installed 1501 1530"), "exclude the joint zero");
  expectContains(run("limits 1470 1540"), "Unloaded limits");
  expectContains(run("limits 1501 1540"), "contain the installed range");
  expectContains(run("installed 1490 1520"), "Installed limits");

  expectContains(run("arm 1500"), "Armed");
  expectContains(run("pulse 1510"), "Pulse 1510");
  expectContains(run("pulse 1520"), "Pulse 1520");
  run("off");

  run("select 2");
  run("arm 1500");
  run("reference");
  run("off");
  expectContains(run("limits 1501 1550"), "exclude the reference pulse");
  expectContains(run("limits 1500 1550"), "Unloaded limits");
  expectContains(run("installed 1501 1550"), "Installed limits");
}

static void testMaxPoints() {
  run("select 4");
  run("arm 1400");
  for (int i = 0; i < 9; ++i) {
    if (i > 0) {
      run("pulse " + std::to_string(1400 + i * 10));
    }
    expectContains(run("record " + std::to_string(i * 10)), "Recorded");
  }
  expectContains(run("pulse 1490"), "Pulse 1490");
  expectContains(run("record 90"), "record full");
  const std::string table = run("table");
  expectContains(table, "4,SG92R-1,,,,,,,9", "nine points must be stored");
  expectContains(table, "4,SG92R-1,0.00,1400", "first point missing");
  expectContains(table, "4,SG92R-1,80.00,1480", "ninth point missing");

  run("off");
  expectContains(run("limits 1450 1500"), "WARN 5 recorded point(s)",
                 "limits must report points left outside");
}

static void testAttachEndpoints() {
  run("select 1");
  run("window 500 2500");
  expectContains(run("arm 500"), "Armed");
  expect(mockServo.lastAttachMin == 500, "attach must pass the explicit 500 us minimum");
  expect(mockServo.lastAttachMax == 2500, "attach must pass the explicit 2500 us maximum");
  expect(mockServo.lastPulse == 500, "500 us endpoint must be tracked, not clamped to 544");
  expect(mockServo.lastAttachInCritical, "attach must run with servo interrupts disabled");
  expect(mockServo.lastWriteInCritical, "first write must run with servo interrupts disabled");
  expect(mockInterruptsEnabled, "arm must restore interrupts");
  expect(mockNoInterruptsCount == mockInterruptsCount, "interrupt enable/disable must balance");
  expectContains(run("pulse 505"), "Pulse 505");
  expect(mockServo.lastPulse == 505, "small adjustment near the 500 us endpoint must be allowed");
  run("off");

  expectContains(run("arm 2500"), "Armed");
  expect(mockServo.lastPulse == 2500, "2500 us endpoint must be tracked, not clamped to 2400");
  expect(mockServo.lastAttachInCritical, "second attach must run with interrupts disabled");
  expect(mockServo.lastWriteInCritical, "second first-write must run with interrupts disabled");
  expect(mockInterruptsEnabled, "second arm must restore interrupts");
  expect(mockNoInterruptsCount == mockInterruptsCount, "second arm interrupts must balance");
  expectContains(run("pulse 2495"), "Pulse 2495");
  expect(mockServo.lastPulse == 2495, "small adjustment near the 2500 us endpoint must be allowed");
  run("off");
}

static void testLineRejection() {
  run("select 1");
  int attaches = mockServo.attachCount;
  std::size_t writes = mockServo.pulses.size();

  const std::string nulArm = withNul("arm 1500", "x");
  expectContains(run(nulArm), "invalid or too-long line", "NUL in arm line must reject the line");
  expectNoMotion(attaches, writes, "NUL in arm line");

  const std::string controlLine = withByte("select 1", 0x01);
  expectContains(run(controlLine), "invalid or too-long line", "control byte must reject the line");
  expectNoMotion(attaches, writes, "control byte line");

  const std::string nonAsciiLine = withByte("select 1", 0x80);
  expectContains(run(nonAsciiLine), "invalid or too-long line", "non-ASCII byte must reject the line");
  expectNoMotion(attaches, writes, "non-ASCII line");

  expectContains(run("arm 1500"), "Armed");
  attaches = mockServo.attachCount;
  writes = mockServo.pulses.size();

  const std::string nulPulse = withNul("pulse 1510", "x");
  expectContains(run(nulPulse), "invalid or too-long line", "NUL in pulse line must reject the line");
  expectNoMotion(attaches, writes, "NUL in pulse line");
  expectContains(run("pulse 1510"), "Pulse 1510");
  run("off");

  const std::string nulLimits = withNul("limits 1400 1600", "x");
  expectContains(run(nulLimits), "invalid or too-long line", "NUL in metadata line must reject the line");
  std::string table = run("table");
  expectContains(table, "1,MG996R-1,,,,,,,0", "NUL metadata line must not change state");
  expectContains(run("limits 1400 1600"), "Unloaded limits");
  table = run("table");
  expectContains(table, "1,MG996R-1,,,1400,1600,,,0", "valid metadata after recovery");
}

static void testWindowReset() {
  run("select 1");
  run("window 500 2500");
  expectContains(run("select 1"), "window 500..2500", "selecting the same unit must keep the window");
  expectContains(run("arm 500"), "Armed");
  expectContains(run("record 1.0"), "Recorded 1.00");
  run("off");

  const std::string selectOther = run("select 2");
  expectContains(selectOther, "window 1400..1600", "different unit must reset the test window");
  expectContains(selectOther, "signal detached; support arm; position not held/guaranteed",
                 "select output must not imply the arm is held");

  const int attaches = mockServo.attachCount;
  const std::size_t writes = mockServo.pulses.size();
  expectContains(run("arm 500"), "outside active guard", "widened window must not transfer");
  expectNoMotion(attaches, writes, "window transfer attempt");

  std::string table = run("table");
  expectContains(table, "1,MG996R-1,,,,,,,1", "record must be retained after switching units");
  expectContains(table, "1,MG996R-1,1.00,500", "recorded point must be retained");

  run("select 1");
  table = run("table");
  expectContains(table, "1,MG996R-1,,,,,,,1", "record must survive switching back");
}

int main(int argc, char **argv) {
  setup();
  if (argc != 2) {
    std::cerr << "usage: host_test startup|parser|record_table|range|max_points|endpoints|line_rejection|window_reset\n";
    return 2;
  }

  const std::string scenario = argv[1];
  if (scenario == "startup") {
    testStartup();
  } else if (scenario == "parser") {
    testParser();
  } else if (scenario == "record_table") {
    testRecordAndTable();
  } else if (scenario == "range") {
    testRangeEnforcement();
  } else if (scenario == "max_points") {
    testMaxPoints();
  } else if (scenario == "endpoints") {
    testAttachEndpoints();
  } else if (scenario == "line_rejection") {
    testLineRejection();
  } else if (scenario == "window_reset") {
    testWindowReset();
  } else {
    std::cerr << "unknown scenario: " << scenario << "\n";
    return 2;
  }

  std::cout << "PASS " << scenario << "\n";
  return 0;
}
'''

SCENARIOS = ["startup", "parser", "record_table", "range", "max_points",
             "endpoints", "line_rejection", "window_reset"]


def main() -> int:
    if shutil.which("g++") is None:
        print("FAIL: g++ is not installed", file=sys.stderr)
        return 1
    if not SKETCH.is_file():
        print(f"FAIL: missing sketch {SKETCH}", file=sys.stderr)
        return 1

    with tempfile.TemporaryDirectory(prefix="servo-calibration-test-") as temp:
        temp_path = Path(temp)
        (temp_path / "Arduino.h").write_text(ARDUINO_H, encoding="utf-8")
        (temp_path / "Servo.h").write_text(SERVO_H, encoding="utf-8")
        (temp_path / "harness.cpp").write_text(HARNESS, encoding="utf-8")
        shutil.copyfile(SKETCH, temp_path / "sketch.cpp")

        executable = temp_path / "host_test"
        compile_command = [
            "g++",
            "-std=c++17",
            "-O0",
            "-g",
            "-Wall",
            "-Wextra",
            "-I",
            str(temp_path),
            "-I",
            str(HERE),
            str(temp_path / "harness.cpp"),
            str(temp_path / "sketch.cpp"),
            "-o",
            str(executable),
        ]
        print("+ " + " ".join(compile_command))
        compiled = subprocess.run(compile_command, text=True)
        if compiled.returncode != 0:
            print("FAIL: compilation failed", file=sys.stderr)
            return compiled.returncode

        for scenario in SCENARIOS:
            result = subprocess.run([str(executable), scenario], text=True)
            if result.returncode != 0:
                print(f"FAIL: scenario {scenario}", file=sys.stderr)
                return result.returncode

    print("PASS: all servo calibration host tests")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
