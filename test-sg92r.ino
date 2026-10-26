#include <Servo.h>
#include <math.h>

Servo joint;
const float LINK_MM = 100.0;

// Three reachable targets: about 90°, 60°, and 120°.
const float targetX[] = {0.0, 50.0, -50.0};
const float targetY[] = {100.0, 86.6025, 86.6025};
int targetIndex = 0;

void setup() {
  Serial.begin(115200);
  joint.attach(9);                 // SG92R signal wire
  joint.writeMicroseconds(1500);   // approximate centre
  delay(2000);
}

void loop() {
  float x = targetX[targetIndex];
  float y = targetY[targetIndex];

  // A one-link arm can reach only points about LINK_MM from its shaft.
  float distance = sqrt(x * x + y * y);
  if (fabs(distance - LINK_MM) > 0.5) {
    Serial.println("Target is not reachable by this one link");
  } else {
    float angle = atan2(y, x) * 180.0 / PI;  // IK result, in degrees

    // Keep this first bench test within a small angle range.
    if (angle >= 60.0 && angle <= 120.0) {
      // Approximate angle-to-pulse mapping for this test.
      int pulseUs = round(1500.0 + (angle - 90.0) * 500.0 / 90.0);
      joint.writeMicroseconds(pulseUs);

      Serial.print("Target x: "); Serial.print(x);
      Serial.print(" mm | y: "); Serial.print(y);
      Serial.print(" mm | IK angle: "); Serial.print(angle, 1);
      Serial.print(" deg | pulse: "); Serial.print(pulseUs);
      Serial.println(" us");
    }
  }

  targetIndex = (targetIndex + 1) % 3;
  delay(3000);
}
