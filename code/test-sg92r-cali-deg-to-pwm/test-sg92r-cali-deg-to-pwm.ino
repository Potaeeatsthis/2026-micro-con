#include <Servo.h>

Servo joint;

void setup() {
  Serial.begin(115200);
  joint.attach(9);

  joint.write(96);  // Start at your measured physical 90°
  Serial.println("Type a Servo.write value, then press Enter.");
  Serial.println("Example: 96");
}

void loop() {
  if (Serial.available() == 0) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  int requested = input.toInt();
  if (requested < 0 || requested > 190) {
    Serial.println("Enter a value from 0 to 190.");
    return;
  }

  joint.write(requested);

  Serial.print("Requested write value: ");
  Serial.print(requested);
  Serial.print(" | Value used by library: ");
  Serial.print(constrain(requested, 0, 180));
  Serial.print(" | Commanded pulse: ");
  Serial.print(joint.readMicroseconds());
  Serial.println(" us");
}
