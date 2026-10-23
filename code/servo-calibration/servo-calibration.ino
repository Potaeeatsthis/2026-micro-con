#include <Arduino.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// The console is shared by the Arduino Uno sketch and the STM32 PlatformIO
// project. Each entry point includes its own small hardware backend first; the
// console itself only calls attach/write/detach and pin-description functions.
#if defined(ARDUINO_ARCH_STM32)
#include "servo_backend_stm32.h"
#else
#include "servo_backend_uno.h"
#endif

// Bench calibration for six positional servos (3 x MG996R, 3 x SG92R).
// One physical servo is connected at a time. Nothing is attached at startup.
// All records live in RAM and are lost on reset. There is no auto-sweep and
// no physical position feedback.

static const uint8_t SERVO_COUNT = 6;
static const uint8_t MAX_POINTS = 9;

static const int16_t HARD_MIN_US = 500;
static const int16_t HARD_MAX_US = 2500;
static const int16_t STEP_LIMIT_US = 20;

// Conservative bench-test guard. This is not a claim that any servo is safe
// over this range; the user must find and mark each unit's real limits.
static const int16_t DEFAULT_WINDOW_MIN_US = 1400;
static const int16_t DEFAULT_WINDOW_MAX_US = 1600;
static int16_t windowMinUs = DEFAULT_WINDOW_MIN_US;
static int16_t windowMaxUs = DEFAULT_WINDOW_MAX_US;

struct Point {
  float angleDeg;
  int16_t pulseUs;
};

struct ServoRecord {
  int16_t referenceUs;    // 0 means not set
  int16_t zeroUs;         // 0 means not set
  int16_t unloadedMinUs;  // 0/0 means not set
  int16_t unloadedMaxUs;
  int16_t installedMinUs; // 0/0 means not set
  int16_t installedMaxUs;
  uint8_t pointCount;
  Point points[MAX_POINTS];
};

static ServoRecord records[SERVO_COUNT];
// Unit labels only; this sketch does not assign units to arm joints.
static const char *const LABELS[SERVO_COUNT] = {
  "MG996R-1", "MG996R-2", "MG996R-3", "SG92R-1", "SG92R-2", "SG92R-3"
};

static int8_t selectedIndex = -1;
static int8_t attachedIndex = -1;
static int16_t currentPulseUs = 0;

static char lineBuffer[64];
static uint8_t lineLength = 0;
static bool lineRejected = false;

#define PRINT_ERR(msg) \
  do { Serial.print(F("ERR ")); Serial.println(F(msg)); } while (0)

static bool parseLongStrict(const char *token, long &value) {
  if (token == NULL || token[0] == '\0') return false;
  errno = 0;
  char *end = NULL;
  long parsed = strtol(token, &end, 10);
  if (errno == ERANGE || end == token || *end != '\0') return false;
  value = parsed;
  return true;
}

static bool parsePulseStrict(const char *token, int16_t &pulseUs) {
  long value;
  if (!parseLongStrict(token, value)) return false;
  if (value < INT16_MIN || value > INT16_MAX) return false;
  pulseUs = (int16_t)value;
  return true;
}

static bool parseAngleStrict(const char *token, float &angleDeg) {
  if (token == NULL || token[0] == '\0') return false;
  errno = 0;
  char *end = NULL;
  double value = strtod(token, &end);
  if (errno == ERANGE || end == token || *end != '\0') return false;
  if (value != value || value < -360.0 || value > 360.0) return false;
  angleDeg = (float)value;
  return true;
}

static char *nextToken(char *&cursor) {
  while (*cursor == ' ' || *cursor == '\t') ++cursor;
  if (*cursor == '\0') return NULL;
  char *token = cursor;
  while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') ++cursor;
  if (*cursor != '\0') {
    *cursor = '\0';
    ++cursor;
  }
  return token;
}

static bool effectiveBounds(const ServoRecord &record, int &lowUs, int &highUs) {
  lowUs = HARD_MIN_US;
  highUs = HARD_MAX_US;

  if (windowMinUs > lowUs) lowUs = windowMinUs;
  if (windowMaxUs < highUs) highUs = windowMaxUs;

  if (record.unloadedMinUs != 0) {
    if (record.unloadedMinUs > lowUs) lowUs = record.unloadedMinUs;
    if (record.unloadedMaxUs < highUs) highUs = record.unloadedMaxUs;
  }
  if (record.installedMinUs != 0) {
    if (record.installedMinUs > lowUs) lowUs = record.installedMinUs;
    if (record.installedMaxUs < highUs) highUs = record.installedMaxUs;
  }

  return lowUs <= highUs;
}

static void printBounds(const ServoRecord &record) {
  int lowUs;
  int highUs;
  if (!effectiveBounds(record, lowUs, highUs)) {
    Serial.print(F("none"));
    return;
  }
  Serial.print(lowUs);
  Serial.print(F(".."));
  Serial.print(highUs);
  Serial.print(F(" us"));
}

static void detachCurrent() {
  if (attachedIndex >= 0) {
    servoBackendDetach(attachedIndex);
    attachedIndex = -1;
    currentPulseUs = 0;
  }
}

static void printUsField(int16_t value) {
  if (value != 0) Serial.print(value);
}

static uint8_t countPointsOutside(const ServoRecord &record, int16_t minUs,
                                  int16_t maxUs) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < record.pointCount; ++i) {
    if (record.points[i].pulseUs < minUs ||
        record.points[i].pulseUs > maxUs) {
      ++count;
    }
  }
  return count;
}

static void printTable() {
  Serial.print(F("# test_window_us,"));
  Serial.print(windowMinUs);
  Serial.print(',');
  Serial.println(windowMaxUs);
  Serial.println(F("# SUMMARY CSV"));
  Serial.println(F("servo,label,reference_us,joint_zero_us,unloaded_min_us,unloaded_max_us,installed_min_us,installed_max_us,points"));

  for (uint8_t i = 0; i < SERVO_COUNT; ++i) {
    const ServoRecord &record = records[i];
    Serial.print(i + 1);
    Serial.print(',');
    Serial.print(LABELS[i]);
    Serial.print(',');
    printUsField(record.referenceUs);
    Serial.print(',');
    printUsField(record.zeroUs);
    Serial.print(',');
    printUsField(record.unloadedMinUs);
    Serial.print(',');
    printUsField(record.unloadedMaxUs);
    Serial.print(',');
    printUsField(record.installedMinUs);
    Serial.print(',');
    printUsField(record.installedMaxUs);
    Serial.print(',');
    Serial.println(record.pointCount);
  }

  Serial.println(F("# POINTS CSV"));
  Serial.println(F("servo,label,measured_deg,pulse_us"));
  for (uint8_t i = 0; i < SERVO_COUNT; ++i) {
    const ServoRecord &record = records[i];
    for (uint8_t p = 0; p < record.pointCount; ++p) {
      Serial.print(i + 1);
      Serial.print(',');
      Serial.print(LABELS[i]);
      Serial.print(',');
      Serial.print(record.points[p].angleDeg, 2);
      Serial.print(',');
      Serial.println(record.points[p].pulseUs);
    }
  }
  Serial.println(F("# End. RAM only; reset loses all records."));
}

static void printHelp() {
  Serial.print(F("Output map: "));
  Serial.println(servoBackendPinSummary());
  Serial.println(F("Commands (lowercase, one line at a time):"));
  Serial.println(F("  select 1..6          detach signal, select unit, reset test window"));
  Serial.println(F("  off                  detach signal; not a power cut or brake"));
  Serial.println(F("  arm <pulse_us>       attach and jump to a pulse; warns first"));
  Serial.println(F("  pulse <pulse_us>     move while armed; max step 20 us"));
  Serial.println(F("  record <measured_deg> record angle at the current pulse"));
  Serial.println(F("  reference            save current pulse as unit reference"));
  Serial.println(F("  zero                 save current pulse as installed joint zero"));
  Serial.println(F("  limits <min> <max>   unloaded safe range; must be detached"));
  Serial.println(F("  installed <min> <max> installed safe range; subset of limits"));
  Serial.println(F("  window <min> <max>   test guard inside 500..2500; detached only"));
  Serial.println(F("  table                print CSV summary and points"));
  Serial.println(F("  help                 show this list"));
  Serial.println(F("All motion is checked against window AND unloaded AND installed limits."));
  Serial.println(F("No auto-sweep, no physical feedback, no automatic safety decision."));
}

static void processCommand() {
  char *cursor = lineBuffer;
  char *command = nextToken(cursor);
  if (command == NULL) return;

  if (strcmp(command, "help") == 0) {
    if (nextToken(cursor) != NULL) {
      PRINT_ERR("help takes no arguments");
      return;
    }
    printHelp();
    return;
  }

  if (strcmp(command, "select") == 0) {
    char *arg = nextToken(cursor);
    if (arg == NULL || nextToken(cursor) != NULL) {
      PRINT_ERR("usage: select 1..6");
      return;
    }
    long value;
    if (!parseLongStrict(arg, value) || value < 1 || value > 6) {
      PRINT_ERR("select must be 1..6");
      return;
    }
    int8_t previousIndex = selectedIndex;
    detachCurrent();
    selectedIndex = (int8_t)(value - 1);
    if (previousIndex >= 0 && previousIndex != selectedIndex) {
      windowMinUs = DEFAULT_WINDOW_MIN_US;
      windowMaxUs = DEFAULT_WINDOW_MAX_US;
    }
    Serial.print(F("Selected "));
    Serial.print(value);
    Serial.print(F(" ("));
    Serial.print(LABELS[selectedIndex]);
    Serial.print(F("); window "));
    Serial.print(windowMinUs);
    Serial.print(F(".."));
    Serial.print(windowMaxUs);
    Serial.println(F(" us; signal detached; support arm; position not held/guaranteed."));
    return;
  }

  if (strcmp(command, "off") == 0) {
    if (nextToken(cursor) != NULL) {
      PRINT_ERR("off takes no arguments");
      return;
    }
    bool wasAttached = attachedIndex >= 0;
    detachCurrent();
    if (wasAttached) {
      Serial.println(F("Signal detached; support arm; position not held/guaranteed."));
    } else {
      Serial.println(F("Signal already detached; support arm; position not held/guaranteed."));
    }
    return;
  }

  if (strcmp(command, "arm") == 0) {
    char *arg = nextToken(cursor);
    if (arg == NULL || nextToken(cursor) != NULL) {
      PRINT_ERR("usage: arm <pulse_us>");
      return;
    }
    if (selectedIndex < 0) {
      PRINT_ERR("select a servo first");
      return;
    }
    if (attachedIndex >= 0) {
      PRINT_ERR("already armed; use pulse or off");
      return;
    }
    int16_t targetUs;
    if (!parsePulseStrict(arg, targetUs)) {
      PRINT_ERR("arm pulse must be an integer");
      return;
    }
    const ServoRecord &record = records[selectedIndex];
    int lowUs;
    int highUs;
    if (!effectiveBounds(record, lowUs, highUs) ||
        targetUs < lowUs || targetUs > highUs) {
      Serial.print(F("ERR pulse outside active guard; allowed "));
      printBounds(record);
      Serial.println();
      return;
    }

    Serial.print(F("WARNING: attach "));
    Serial.print(servoBackendPinName(selectedIndex));
    Serial.print(F(" and jump to "));
    Serial.print(targetUs);
    Serial.println(F(" us; support the unloaded servo/arm. No safe pose is assumed."));
    // The AVR backend keeps attach + first write between noInterrupts() and
    // interrupts(). The STM32 backend ignores these hooks and sets the compare
    // value before starting the channel.
    servoBackendEnterCritical();
    servoBackendAttach(selectedIndex, HARD_MIN_US, HARD_MAX_US);
    servoBackendWrite(selectedIndex, targetUs);
    servoBackendExitCritical();
    attachedIndex = selectedIndex;
    currentPulseUs = targetUs;
    Serial.print(F("Armed "));
    Serial.print(LABELS[selectedIndex]);
    Serial.print(F(" at "));
    Serial.print(targetUs);
    Serial.println(F(" us."));
    return;
  }

  if (strcmp(command, "pulse") == 0) {
    char *arg = nextToken(cursor);
    if (arg == NULL || nextToken(cursor) != NULL) {
      PRINT_ERR("usage: pulse <pulse_us>");
      return;
    }
    if (attachedIndex < 0) {
      PRINT_ERR("not armed; use arm <pulse_us>");
      return;
    }
    int16_t targetUs;
    if (!parsePulseStrict(arg, targetUs)) {
      PRINT_ERR("pulse must be an integer");
      return;
    }
    const ServoRecord &record = records[attachedIndex];
    int lowUs;
    int highUs;
    if (!effectiveBounds(record, lowUs, highUs) ||
        targetUs < lowUs || targetUs > highUs) {
      Serial.print(F("ERR pulse outside active guard; allowed "));
      printBounds(record);
      Serial.println();
      return;
    }
    int stepUs = targetUs - currentPulseUs;
    if (stepUs < 0) stepUs = -stepUs;
    if (stepUs > STEP_LIMIT_US) {
      Serial.print(F("ERR step limit is "));
      Serial.print(STEP_LIMIT_US);
      Serial.println(F(" us while armed"));
      return;
    }
    servoBackendWrite(attachedIndex, targetUs);
    currentPulseUs = targetUs;
    Serial.print(F("Pulse "));
    Serial.print(targetUs);
    Serial.println(F(" us."));
    return;
  }

  if (strcmp(command, "record") == 0) {
    char *arg = nextToken(cursor);
    if (arg == NULL || nextToken(cursor) != NULL) {
      PRINT_ERR("usage: record <measured_deg>");
      return;
    }
    if (attachedIndex < 0) {
      PRINT_ERR("arm the servo before recording");
      return;
    }
    float angleDeg;
    if (!parseAngleStrict(arg, angleDeg)) {
      PRINT_ERR("measured angle must be finite and -360..360");
      return;
    }
    ServoRecord &record = records[attachedIndex];
    if (record.pointCount >= MAX_POINTS) {
      Serial.print(F("ERR record full ("));
      Serial.print(MAX_POINTS);
      Serial.println(F(" points)"));
      return;
    }
    for (uint8_t i = 0; i < record.pointCount; ++i) {
      if (record.points[i].pulseUs == currentPulseUs) {
        PRINT_ERR("a point already exists at this pulse");
        return;
      }
    }
    record.points[record.pointCount].angleDeg = angleDeg;
    record.points[record.pointCount].pulseUs = currentPulseUs;
    ++record.pointCount;
    Serial.print(F("Recorded "));
    Serial.print(angleDeg, 2);
    Serial.print(F(" deg at "));
    Serial.print(currentPulseUs);
    Serial.println(F(" us."));
    return;
  }

  if (strcmp(command, "reference") == 0) {
    if (nextToken(cursor) != NULL) {
      PRINT_ERR("reference takes no arguments");
      return;
    }
    if (attachedIndex < 0) {
      PRINT_ERR("arm the servo before saving a reference");
      return;
    }
    records[attachedIndex].referenceUs = currentPulseUs;
    Serial.print(F("Reference pulse: "));
    Serial.print(currentPulseUs);
    Serial.println(F(" us."));
    return;
  }

  if (strcmp(command, "zero") == 0) {
    if (nextToken(cursor) != NULL) {
      PRINT_ERR("zero takes no arguments");
      return;
    }
    if (attachedIndex < 0) {
      PRINT_ERR("arm the servo before saving a joint zero");
      return;
    }
    ServoRecord &record = records[attachedIndex];
    if (record.installedMinUs == 0) {
      PRINT_ERR("set installed limits before saving a joint zero");
      return;
    }
    if (currentPulseUs < record.installedMinUs ||
        currentPulseUs > record.installedMaxUs) {
      PRINT_ERR("joint zero must be inside installed limits");
      return;
    }
    record.zeroUs = currentPulseUs;
    Serial.print(F("Joint zero pulse: "));
    Serial.print(currentPulseUs);
    Serial.println(F(" us."));
    return;
  }

  if (strcmp(command, "limits") == 0) {
    char *argMin = nextToken(cursor);
    char *argMax = nextToken(cursor);
    if (argMin == NULL || argMax == NULL || nextToken(cursor) != NULL) {
      PRINT_ERR("usage: limits <min_us> <max_us>");
      return;
    }
    if (attachedIndex >= 0) {
      PRINT_ERR("detach before changing limits");
      return;
    }
    if (selectedIndex < 0) {
      PRINT_ERR("select a servo first");
      return;
    }
    int16_t minUs;
    int16_t maxUs;
    if (!parsePulseStrict(argMin, minUs) || !parsePulseStrict(argMax, maxUs)) {
      PRINT_ERR("limits must be integer pulses");
      return;
    }
    if (minUs < HARD_MIN_US || maxUs > HARD_MAX_US || minUs >= maxUs) {
      PRINT_ERR("limits must be inside 500..2500 and min < max");
      return;
    }
    ServoRecord &record = records[selectedIndex];
    if (record.installedMinUs != 0 &&
        (minUs > record.installedMinUs || maxUs < record.installedMaxUs)) {
      PRINT_ERR("limits must contain the installed range");
      return;
    }
    if (record.referenceUs != 0 &&
        (record.referenceUs < minUs || record.referenceUs > maxUs)) {
      PRINT_ERR("limits would exclude the reference pulse");
      return;
    }
    if (record.zeroUs != 0 &&
        (record.zeroUs < minUs || record.zeroUs > maxUs)) {
      PRINT_ERR("limits would exclude the joint zero");
      return;
    }
    uint8_t stalePoints = countPointsOutside(record, minUs, maxUs);
    record.unloadedMinUs = minUs;
    record.unloadedMaxUs = maxUs;
    Serial.print(F("Unloaded limits: "));
    Serial.print(minUs);
    Serial.print(F(".."));
    Serial.print(maxUs);
    Serial.println(F(" us."));
    if (stalePoints > 0) {
      Serial.print(F("WARN "));
      Serial.print(stalePoints);
      Serial.println(F(" recorded point(s) are outside these limits; kept as raw measurements."));
    }
    return;
  }

  if (strcmp(command, "installed") == 0) {
    char *argMin = nextToken(cursor);
    char *argMax = nextToken(cursor);
    if (argMin == NULL || argMax == NULL || nextToken(cursor) != NULL) {
      PRINT_ERR("usage: installed <min_us> <max_us>");
      return;
    }
    if (attachedIndex >= 0) {
      PRINT_ERR("detach before changing installed limits");
      return;
    }
    if (selectedIndex < 0) {
      PRINT_ERR("select a servo first");
      return;
    }
    int16_t minUs;
    int16_t maxUs;
    if (!parsePulseStrict(argMin, minUs) || !parsePulseStrict(argMax, maxUs)) {
      PRINT_ERR("installed limits must be integer pulses");
      return;
    }
    if (minUs < HARD_MIN_US || maxUs > HARD_MAX_US || minUs >= maxUs) {
      PRINT_ERR("installed limits must be inside 500..2500 and min < max");
      return;
    }
    ServoRecord &record = records[selectedIndex];
    if (record.unloadedMinUs == 0) {
      PRINT_ERR("set unloaded limits first");
      return;
    }
    if (minUs < record.unloadedMinUs || maxUs > record.unloadedMaxUs) {
      PRINT_ERR("installed range must be inside unloaded limits");
      return;
    }
    if (record.zeroUs != 0 &&
        (record.zeroUs < minUs || record.zeroUs > maxUs)) {
      PRINT_ERR("installed range would exclude the joint zero");
      return;
    }
    record.installedMinUs = minUs;
    record.installedMaxUs = maxUs;
    Serial.print(F("Installed limits: "));
    Serial.print(minUs);
    Serial.print(F(".."));
    Serial.print(maxUs);
    Serial.println(F(" us."));
    return;
  }

  if (strcmp(command, "window") == 0) {
    char *argMin = nextToken(cursor);
    char *argMax = nextToken(cursor);
    if (argMin == NULL || argMax == NULL || nextToken(cursor) != NULL) {
      PRINT_ERR("usage: window <min_us> <max_us>");
      return;
    }
    if (attachedIndex >= 0) {
      PRINT_ERR("detach before changing the test window");
      return;
    }
    int16_t minUs;
    int16_t maxUs;
    if (!parsePulseStrict(argMin, minUs) || !parsePulseStrict(argMax, maxUs)) {
      PRINT_ERR("window must be integer pulses");
      return;
    }
    if (minUs < HARD_MIN_US || maxUs > HARD_MAX_US || minUs >= maxUs) {
      PRINT_ERR("window must be inside 500..2500 and min < max");
      return;
    }
    windowMinUs = minUs;
    windowMaxUs = maxUs;
    Serial.print(F("Test window: "));
    Serial.print(minUs);
    Serial.print(F(".."));
    Serial.print(maxUs);
    Serial.println(F(" us."));
    return;
  }

  if (strcmp(command, "table") == 0) {
    if (nextToken(cursor) != NULL) {
      PRINT_ERR("table takes no arguments");
      return;
    }
    printTable();
    return;
  }

  PRINT_ERR("unknown command; type help");
}

static bool lineByteAllowed(int value) {
  if (value == '\t') return true;
  return value >= 0x20 && value <= 0x7E;
}

static void processLine() {
  if (lineRejected) {
    lineRejected = false;
    lineLength = 0;
    PRINT_ERR("invalid or too-long line; discarded");
    return;
  }

  lineBuffer[lineLength] = '\0';
  if (lineLength > 0) processCommand();
  lineLength = 0;
}

void setup() {
  servoBackendBegin();
  Serial.begin(115200);
  Serial.print(F("Servo calibration console ("));
  Serial.print(servoBackendBoardName());
  Serial.println(F(", 115200)."));
  Serial.print(F("Output map: "));
  Serial.println(servoBackendPinSummary());
  Serial.println(F("RAM only: reset loses all six records. Nothing attached or moved at startup."));
  Serial.println(F("Type 'help'."));
}

void loop() {
  while (Serial.available() > 0) {
    int value = Serial.read();
    if (value < 0) break;

    if (value == '\n' || value == '\r') {
      processLine();
      return;
    }

    if (lineRejected) continue;

    if (!lineByteAllowed(value)) {
      lineRejected = true;
      continue;
    }

    if (lineLength >= sizeof(lineBuffer) - 1) {
      lineRejected = true;
      continue;
    }

    lineBuffer[lineLength] = (char)value;
    ++lineLength;
  }
}
