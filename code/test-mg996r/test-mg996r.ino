#include <Servo.h>

Servo joint;

const int SERVO_PIN = 9;
const int STOP_US = 1500;       // Adjust slightly if the servo creeps
const int MAX_OFFSET_US = 200;
const float DT = 0.020f;        // 20 ms
const float MAX_CHANGE = 1.0f;  // Speed-command change per second

float speedCommand = 0.0f;      // -1.0 to +1.0; NOT joint speed in deg/s
unsigned long startTime;
unsigned long lastUpdate;
unsigned long lastPrint;

void setup() {
  Serial.begin(115200);

  joint.attach(SERVO_PIN);
  joint.writeMicroseconds(STOP_US);

  startTime = millis();
  lastUpdate = millis();
  lastPrint = millis();

  Serial.println("360-degree servo test on pin 9");
  Serial.println("Keep the horn free. Motion stops after 8 seconds.");
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate < 20) return;
  lastUpdate += 20;

  unsigned long elapsed = now - startTime;

  // Requested direction and approximate speed.
  float targetSpeed = 0.0f;
  if (elapsed >= 2000 && elapsed < 4000) {
    targetSpeed = -0.5f;       // First direction
  } else if (elapsed >= 6000 && elapsed < 8000) {
    targetSpeed = 0.5f;        // Opposite direction
  }

  // Gradually change the command instead of jumping to it.
  float maxStep = MAX_CHANGE * DT;
  float difference = targetSpeed - speedCommand;
  speedCommand += constrain(difference, -maxStep, maxStep);
  speedCommand = constrain(speedCommand, -1.0f, 1.0f);

  int pulseUs = STOP_US + (int)(speedCommand * MAX_OFFSET_US);
  pulseUs = constrain(pulseUs, 1300, 1700);
  joint.writeMicroseconds(pulseUs);

  if (now - lastPrint >= 200) {
    lastPrint = now;

    Serial.print("Time: ");
    Serial.print(elapsed / 1000.0f, 1);
    Serial.print(" s | Requested speed: ");
    Serial.print(targetSpeed, 2);
    Serial.print(" | Smoothed command: ");
    Serial.print(speedCommand, 2);
    Serial.print(" | Pulse: ");
    Serial.print(pulseUs);
    Serial.println(" us");
  }
}
