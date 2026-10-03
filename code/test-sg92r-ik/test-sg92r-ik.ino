#include <Servo.h>
#include <math.h>

Servo joint;

const int SERVO_PIN = 9;
const float LINK_MM = 100.0f;  // Imagined one-link length

// Targets measured from the servo shaft (0, 0).
// They point at approximately 90°, 60°, and 120°.
const float targetX[] = {0.0f, 50.0f, -50.0f};
const float targetY[] = {100.0f, 86.6025f, 86.6025f};
const int TARGET_COUNT = 3;

int targetIndex = 0;

// Convert a physical angle into a pulse using YOUR measurements.
int angleToPulse(float angle) {
  if (angle <= 90.0f) {
    // 0° -> 616 µs; 90° -> 1523 µs
    return roundf(616.0f
                + angle * (1523.0f - 616.0f) / 90.0f);
  }

  // 90° -> 1523 µs; 180° -> 2400 µs
  return roundf(1523.0f
              + (angle - 90.0f) * (2400.0f - 1523.0f) / 90.0f);
}

void setup() {
  Serial.begin(115200);

  joint.attach(SERVO_PIN);
  joint.writeMicroseconds(1523);  // Your measured physical 90°

  Serial.println("One-joint IK test started.");
  delay(2000);
}

void loop() {
  // Select one target position.
  float x = targetX[targetIndex];
  float y = targetY[targetIndex];

  // Check that the target is about 100 mm from the shaft.
  float distance = sqrtf(x * x + y * y);

  if (fabsf(distance - LINK_MM) > 0.5f) {
    Serial.println("Target is outside the one-link reach.");
  } else {
    // IK: calculate which direction the link must point.
    // atan2 returns radians, so convert the result to degrees.
    float ikAngle = atan2f(y, x) * 180.0f / PI;

    // A small tolerance avoids rejecting 60° or 120°
    // because of floating-point rounding.
    if (ikAngle < 59.9f || ikAngle > 120.1f) {
      Serial.println("IK angle is outside the 60-120 deg test range.");
    } else {
      // Keep the command strictly inside the selected test range.
      ikAngle = constrain(ikAngle, 60.0f, 120.0f);

      int pulseUs = angleToPulse(ikAngle);
      joint.writeMicroseconds(pulseUs);

      Serial.print("Target: x=");
      Serial.print(x, 1);
      Serial.print(" mm, y=");
      Serial.print(y, 1);
      Serial.print(" mm | IK angle=");
      Serial.print(ikAngle, 1);
      Serial.print(" deg | Commanded pulse=");
      Serial.print(pulseUs);
      Serial.println(" us");
    }
  }

  // Select the next target after three seconds.
  targetIndex = (targetIndex + 1) % TARGET_COUNT;
  delay(3000);
}
