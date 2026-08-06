# Autonomous Robotic Arm with Speech Command and RGB-D Camera

## 1. Project Title

**Voice-Controlled Autonomous Pick-and-Place Robotic Arm Using STM32, Raspberry Pi 4, and an RGB-D Camera**

---

## 2. Project Goal

The robot arm will receive a speech command, detect an object with a camera, calculate the object's 3D position, pick it up, and place it at a target location.

Example command:

> **"Pick up the red block and place it in the blue box."**

The complete operation is:

1. The microphone receives the speech command.
2. Raspberry Pi converts speech into text.
3. The camera detects the requested object.
4. The depth camera measures the object's distance.
5. Raspberry Pi calculates the object's 3D position.
6. Raspberry Pi calculates the required joint angles.
7. STM32 moves the motors smoothly and safely.
8. The gripper picks up the object.
9. The arm moves the object to the destination.
10. The camera checks whether the task succeeded.

---

## 3. Recommended Project Scope

Use a **5-DOF robotic arm plus one gripper**.

| Motor | Joint |
|---|---|
| Motor 1 | Base rotation |
| Motor 2 | Shoulder |
| Motor 3 | Elbow |
| Motor 4 | Wrist pitch |
| Motor 5 | Wrist rotation |
| Motor 6 | Gripper |

This configuration uses **six motors in total**.

> A real 6-DOF arm normally has six movement joints plus a gripper, so it may require seven actuators. For this project, 5-DOF plus a gripper is more realistic and less expensive.

### Initial limitations

The first version should use:

- Small colored cubes
- Payload below 50–100 g
- Fixed camera above the table
- Fixed destination boxes
- Quiet indoor environment
- Slow arm movement
- English speech commands
- Simple pick-and-place tasks

Do not begin with random household objects or unrestricted natural-language commands.

---

## 4. System Architecture

```text
                    RASPBERRY PI 4
             ┌─────────────────────────┐
Microphone ─>│ Speech recognition      │
             │ Command interpretation  │
RGB-D Camera>│ Object detection         │
             │ 3D position calculation │
             │ Inverse kinematics      │
             │ Task state machine      │
             └───────────┬─────────────┘
                         │ USB UART
                         │ Joint targets
                         ▼
                    STM32 BOARD
             ┌─────────────────────────┐
E-Stop ─────>│ Safety control          │
Limit switch>│ Joint limit checking    │
             │ Smooth interpolation    │
             │ PWM generation          │
             │ Communication watchdog  │
             └───────────┬─────────────┘
                         │
                         ▼
                  PCA9685 / Servos
                         │
                         ▼
                   ROBOTIC ARM
```

---

## 5. Responsibilities of Each Computer

### Raspberry Pi 4

The Raspberry Pi performs high-level processing:

- Speech recognition
- Command interpretation
- RGB and depth processing
- Object detection
- Object-position calculation
- Camera-to-robot coordinate transformation
- Inverse kinematics
- Task planning
- Success verification

### STM32

The STM32 performs real-time control:

- Generate PWM signals
- Control six motors
- Apply joint-angle limits
- Move joints smoothly
- Read limit switches
- Read the emergency-stop signal
- Monitor Raspberry Pi communication
- Stop movement when communication is lost
- Report `ACK`, `DONE`, or error messages

This separation makes STM32 an important part of the project instead of using it only as a simple receiver.

---

# 6. Equipment and Estimated Prices

## Important price note

The prices below are example Thailand prices checked in **August 2026**. Marketplace prices, stock, promotions, and delivery fees can change.

You already own:

| Equipment | Cost |
|---|---:|
| STM32 board | ฿0 |
| Raspberry Pi 4 | ฿0 |

---

## 6.1 Required Hardware

| Item | Quantity | Estimated unit price | Estimated total | Notes |
|---|---:|---:|---:|---|
| Metal 6-axis arm frame without motors | 1 | ฿1,550 | ฿1,550 | Compatible with standard-size servos |
| MG996R 180° metal-gear servo | 6 | ฿160 | ฿960 | Low-cost prototype motors |
| PCA9685 16-channel servo driver | 1 | ฿170–195 | ฿170 | STM32 controls it through I2C |
| 5 V 20 A switching power supply | 1 | ฿280 | ฿280 | Separate power for servos |
| USB microphone or microphone HAT | 1 | ฿200–560 | ฿560 | Better microphone improves speech recognition |
| Emergency-stop switch | 1 | ฿40–80 | ฿50 | Must disconnect or disable servo power |
| Small limit switches | 2–4 | ฿20–40 | ฿100 | Use at shoulder and elbow first |
| Fuse, fuse holder, wires, connectors | 1 set | ฿150–250 | ฿250 | Use thick wires for servo power |
| 1,000–2,200 µF capacitor | 1–2 | ฿20–50 | Included above | Helps reduce voltage drops |
| USB-to-UART module or USB cable | 1 | ฿80–150 | ฿120 | Raspberry Pi to STM32 communication |
| Mounting plate, screws, cable ties | 1 set | ฿150–300 | ฿250 | Mechanical installation |
| Colored cubes and destination boxes | 1 set | ฿100–200 | ฿150 | Initial test objects |

### Estimated subtotal without camera

\[
1550 + 960 + 170 + 280 + 560 + 50 + 100 + 250 + 120 + 250 + 150
= \textbf{฿4,440}
\]

Add approximately **฿300–600** for shipping and unexpected parts.

### Practical starter budget

\[
\boxed{\textbf{Approximately ฿4,700–5,000}}
\]

This fits your earlier ฿5,000 budget only when you **borrow the depth camera** or temporarily use an existing normal camera.

---

## 6.2 Camera Options

| Camera option | Estimated price | Recommendation |
|---|---:|---|
| Existing USB webcam | ฿0 | Good for the first motor and color-detection tests |
| Basic USB webcam | ฿300–800 | No real depth; use known table height |
| Borrowed RGB-D camera | ฿0 | Best choice for keeping the project near ฿5,000 |
| Luxonis OAK-D Lite | About ฿9,850 | Good with Raspberry Pi; performs stereo depth and on-device AI |
| RealSense D435 | About ฿13,132 | Good for tabletop robotics |
| RealSense D435i | About ฿11,555–15,900 | Includes an IMU, but the IMU is not necessary for a fixed camera |

### Full project total with OAK-D Lite

\[
4440 + 9850 = \boxed{\textbf{฿14,290}}
\]

With shipping and spare parts:

\[
\boxed{\textbf{Approximately ฿14,500–15,500}}
\]

### Full project total with RealSense D435

\[
4440 + 13132 = \boxed{\textbf{฿17,572}}
\]

With shipping and spare parts:

\[
\boxed{\textbf{Approximately ฿18,000–19,000}}
\]

---

## 6.3 Alternative Complete Arm Kit

A complete metal arm kit containing six MG996R servos, a PCA9685 board, and an Arduino-compatible controller has been listed at approximately:

\[
\boxed{\textbf{฿4,990}}
\]

This is easier to purchase, but after adding the power supply, microphone, safety components, wiring, and camera, it will exceed the ฿5,000 budget.

| Extra item | Estimated price |
|---|---:|
| Complete arm kit | ฿4,990 |
| Servo power supply | ฿280 |
| Microphone | ฿200–560 |
| Emergency stop and safety parts | ฿200–400 |
| Wiring and mounting parts | ฿250–500 |
| OAK-D Lite | ฿9,850 |
| **Estimated complete total** | **฿15,770–16,580** |

---

# 7. Recommended Buying Plan

## Plan A: Stay Near ฿5,000

Buy:

- Metal arm frame
- Six MG996R servos
- PCA9685
- 5 V 20 A power supply
- USB microphone
- Emergency-stop switch
- Limit switches
- Fuse, wires, and connectors

Use:

- Your existing STM32
- Your existing Raspberry Pi 4
- An existing USB camera, or
- A depth camera borrowed from your university or laboratory

### Estimated total

\[
\boxed{\textbf{฿4,700–5,000}}
\]

This version can demonstrate:

- STM32 motor control
- Speech commands
- Color-object detection
- Pick and place on a known flat table
- Camera-based result checking

It will not have true depth unless you borrow an RGB-D camera.

---

## Plan B: Full RGB-D Version

Use all parts from Plan A and add an OAK-D Lite.

### Estimated total

\[
\boxed{\textbf{฿14,500–15,500}}
\]

This is the recommended complete version because the OAK-D Lite reduces some computer-vision workload on the Raspberry Pi.

---

## Plan C: Better Motors

MG996R motors are acceptable for a low-cost prototype, but the shoulder may be weak because it carries the weight of the complete arm.

A 35 kg-cm servo can cost approximately **฿1,680 each** from a local electronics supplier.

A useful upgrade order is:

1. Upgrade the shoulder servo.
2. Upgrade the elbow servo.
3. Keep cheaper servos for the wrist and gripper.

Replacing one ฿160 MG996R with one ฿1,680 high-torque servo adds:

\[
1680 - 160 = \boxed{\textbf{฿1,520}}
\]

### Starter system with one shoulder upgrade

\[
4440 + 1520 = \boxed{\textbf{฿5,960}}
\]

This is above the original budget, but it can improve reliability.

> Do not buy an expensive depth camera while using an arm that cannot lift its own wrist and gripper. Mechanical reliability should come first.

---

# 8. Power Design

Do not power the servos from the STM32 or Raspberry Pi.

```text
AC power
   |
   v
5 V 20 A servo power supply
   |
   +---- Emergency-stop switch or power cut circuit
   |
   +---- Fuse
   |
   +---- PCA9685 servo-power input
            |
            +---- Servo 1
            +---- Servo 2
            +---- Servo 3
            +---- Servo 4
            +---- Servo 5
            +---- Servo 6
```

The signal ground must be shared:

```text
STM32 GND
Raspberry Pi GND
PCA9685 GND
Servo power-supply GND
```

### Important

- Use short and thick power wires.
- Do not carry servo current through a breadboard.
- Put a fuse between the supply and servo system.
- Put a large capacitor near the PCA9685 power input.
- Test one servo before connecting all six.
- Set the power supply to the voltage supported by the selected servos.

---

# 9. Servo Torque Check

The shoulder requires the highest torque.

Use:

\[
\tau = \sum m_i g r_i
\]

Where:

- \(\tau\) is torque in N·m
- \(m_i\) is supported mass in kg
- \(g = 9.81\ \text{m/s}^2\)
- \(r_i\) is distance from the joint in metres

Example:

- Forearm: 0.25 kg at 0.15 m
- Wrist and gripper: 0.20 kg at 0.25 m
- Payload: 0.10 kg at 0.35 m

\[
\tau =
(0.25 \times 9.81 \times 0.15)
+
(0.20 \times 9.81 \times 0.25)
+
(0.10 \times 9.81 \times 0.35)
\]

\[
\tau \approx 1.20\ \text{N·m}
\]

This is approximately 12.2 kg-cm. With a safety factor of two:

\[
12.2 \times 2 = 24.4\ \text{kg-cm}
\]

Therefore, a shoulder servo near **25–35 kg-cm** is safer for this example.

The cheap MG996R is useful for learning and light tests, but it may not be reliable for a long metal arm.

---

# 10. Camera Position

For the first version, use a fixed camera above the table.

```text
          RGB-D camera
                |
                v
     ┌─────────────────────┐
     │                     │
     │   Robot workspace   │
     │                     │
     │ Arm          Cubes  │
     │                     │
     └─────────────────────┘
```

Recommended position:

- 50–80 cm above the workspace
- Camera points down toward the table
- The camera must not move after calibration
- Avoid direct sunlight
- Avoid glass and reflective objects
- Keep all objects inside the camera view

Do not mount the camera on the wrist in the first version. A wrist camera makes calibration, cable management, and motion planning more difficult.

---

# 11. Software

## Raspberry Pi

Recommended software:

- Ubuntu 24.04 64-bit
- ROS 2 Jazzy, optional for the first prototype
- Python 3
- OpenCV
- NumPy
- Vosk for offline English speech recognition
- DepthAI for OAK-D Lite
- RealSense SDK for D435/D435i
- PySerial for communication with STM32

## STM32

Recommended firmware modules:

```text
Core/
├── servo_control.c
├── trajectory.c
├── joint_limits.c
├── serial_protocol.c
├── safety.c
├── limit_switch.c
├── watchdog.c
└── main.c
```

---

# 12. Speech Commands

Start with a small command set:

```text
"Robot home"
"Pick up the red block"
"Pick up the blue block"
"Place it in box one"
"Open gripper"
"Close gripper"
"Stop"
```

Convert the recognized sentence into a structured command:

```json
{
  "action": "pick_and_place",
  "object": "red_block",
  "destination": "blue_box"
}
```

The speech-recognition program must not control motors directly. It should send a safe task request to the planning program.

---

# 13. Object Detection

Start with colored objects instead of YOLO.

```text
RGB image
    |
    v
Convert BGR to HSV
    |
    v
Apply color threshold
    |
    v
Find the largest contour
    |
    v
Find the center pixel
    |
    v
Read depth at that pixel
    |
    v
Convert pixel and depth to X, Y, Z
```

For a detected pixel \((u,v)\) and depth \(Z\):

\[
X_c = \frac{(u-c_x)Z}{f_x}
\]

\[
Y_c = \frac{(v-c_y)Z}{f_y}
\]

\[
Z_c = Z
\]

Then convert the camera coordinate to the robot-base coordinate:

\[
P_{base} = T_{base}^{camera} P_{camera}
\]

The transformation matrix is found during camera-to-robot calibration.

---

# 14. Pick-and-Place State Machine

```text
IDLE
  |
  v
LISTENING
  |
  v
DETECTING
  |
  v
CALCULATING POSITION
  |
  v
PLANNING
  |
  v
MOVING TO OBJECT
  |
  v
GRASPING
  |
  v
LIFTING
  |
  v
MOVING TO DESTINATION
  |
  v
RELEASING
  |
  v
VERIFYING
  |
  v
COMPLETE
```

For an error:

```text
ERROR -> STOP -> SAFE POSITION
```

---

# 15. STM32 Communication Protocol

Example Raspberry Pi command:

```text
<MOVE,90,65,110,45,90,30,2500,CRC>
```

Meaning:

| Value | Meaning |
|---|---|
| 90 | Base angle |
| 65 | Shoulder angle |
| 110 | Elbow angle |
| 45 | Wrist-pitch angle |
| 90 | Wrist-rotation angle |
| 30 | Gripper angle |
| 2500 | Movement duration in milliseconds |

STM32 responses:

```text
<ACK>
<DONE>
<ERR,LIMIT>
<ERR,TIMEOUT>
<ERR,CHECKSUM>
<STOPPED>
```

The Raspberry Pi should wait for `<DONE>` before sending the next movement.

---

# 16. Smooth Motor Movement

Do not change a joint immediately from 20° to 150°.

Bad method:

```text
20° -> 150°
```

Better method:

```text
20°, 22°, 24°, 26° ... 150°
```

A more advanced version uses:

```text
Accelerate -> Constant speed -> Decelerate
```

Smooth movement reduces:

- Mechanical vibration
- Current spikes
- Gear damage
- Dropped objects
- Raspberry Pi or STM32 resets

---

# 17. Safety Requirements

The minimum safety system should include:

1. **Physical emergency-stop button**
   - It should disable servo power.
   - Do not depend only on software.

2. **Joint-angle limits**
   - Store safe minimum and maximum values in STM32.

3. **Limit switches**
   - Begin with shoulder and elbow.

4. **Communication watchdog**
   - Raspberry Pi sends a heartbeat every 200–500 ms.
   - STM32 stops motion when the heartbeat disappears.

5. **Workspace limits**
   - Reject positions outside the arm reach.
   - Reject positions below the table.
   - Reject positions too close to the base.

6. **Slow test mode**
   - Use 20–30% speed during early tests.

7. **Light objects**
   - Begin with foam cubes.

8. **Temperature checks**
   - Stop if a servo becomes very hot or continuously vibrates.

---

# 18. Development Process

## Phase 1: Define the Robot

Decide:

```text
Configuration: 5-DOF plus gripper
Number of motors: 6
Initial payload: 50–100 g
Objects: colored cubes
Camera: fixed above the table
Commands: fixed English commands
```

## Phase 2: Assemble and Test One Motor

- Assemble the arm frame.
- Test one motor at a time.
- Find safe minimum and maximum angles.
- Record the angle limits.
- Check whether the shoulder can hold the arm.

## Phase 3: STM32 Motor Control

Implement:

- PWM or PCA9685 control
- Joint limits
- Smooth interpolation
- Emergency stop
- Limit-switch input
- UART command parser
- Watchdog

At the end of this phase, control the arm from a serial terminal:

```text
HOME
OPEN
CLOSE
MOVE 90 60 110 45 90 30
```

## Phase 4: Raspberry Pi Communication

Create a Python program:

```python
arm.send_joint_target([
    90,
    65,
    110,
    45,
    90,
    30
])
```

Wait for `DONE` before sending the next target.

## Phase 5: Forward and Inverse Kinematics

Implement:

- Joint angles to gripper position
- Requested position to joint angles
- Reachability check
- Joint-limit check

Test with typed coordinates:

```python
move_to_xyz(x=0.20, y=0.05, z=0.12)
```

## Phase 6: Camera

- Display the RGB image.
- Display the depth image.
- Detect one colored cube.
- Print its center pixel.
- Print its depth.
- Convert it to camera X, Y, Z.

Do not move the arm automatically yet.

## Phase 7: Camera-to-Robot Calibration

Place a marker at known arm positions.

Example:

```text
Point 1: X=100, Y=100, Z=0 mm
Point 2: X=200, Y=100, Z=0 mm
Point 3: X=100, Y=200, Z=0 mm
Point 4: X=200, Y=200, Z=0 mm
```

Calculate the transformation between the camera and robot coordinate systems.

## Phase 8: Autonomous Pick and Place

Use fixed movement stages:

```text
HOME
  |
  v
PRE-GRASP
  |
  v
GRASP
  |
  v
LIFT
  |
  v
PRE-PLACE
  |
  v
PLACE
  |
  v
HOME
```

## Phase 9: Speech Integration

First make this work:

```text
Keyboard command -> Camera -> Arm
```

Then replace the keyboard command with:

```text
Speech command -> Camera -> Arm
```

Speech should be added last because it is easier to debug the robot without speech-recognition errors.

---

# 19. Recommended Final Demonstration

User says:

> **"Pick up the red block and place it in the blue box."**

The robot:

1. Recognizes the words.
2. Detects the red block.
3. Reads its depth.
4. Calculates its robot coordinates.
5. Checks that the position is safe.
6. Calculates joint angles.
7. Moves above the block.
8. Opens the gripper.
9. Moves down slowly.
10. Closes the gripper.
11. Lifts the block.
12. Moves above the blue box.
13. Places the block.
14. Opens the gripper.
15. Returns home.
16. Uses the camera to check the result.

---

# 20. Success Metrics

| Metric | Initial target |
|---|---:|
| Speech-command accuracy | At least 90% in a quiet room |
| Color-object detection | At least 90% |
| Position error | Below 2 cm |
| Successful grasp rate | At least 80% over 20 tests |
| Successful placement rate | At least 80% |
| Emergency-stop operation | Stops power immediately |
| Initial payload | 50–100 g |
| Consecutive operations | 10 operations without reset |

---

# 21. Final Recommendation

With the original **฿5,000 budget**, use this approach:

1. Buy the frame, motors, motor power supply, controller board, microphone, and safety parts.
2. Use your existing STM32 and Raspberry Pi 4.
3. Borrow an OAK-D Lite or RealSense camera from your university.
4. Start with a normal webcam when developing motor control and object detection.
5. Keep the initial payload below 100 g.
6. Upgrade the shoulder servo before increasing payload.
7. Add speech only after typed pick-and-place commands work.

The correct development order is:

```text
Mechanical arm
    ->
STM32 manual motor control
    ->
Raspberry Pi serial control
    ->
Inverse kinematics
    ->
Camera detection
    ->
Autonomous pick and place
    ->
Speech commands
```

> **Make the arm work from typed joint angles and XYZ coordinates before adding camera autonomy and speech recognition.**

---

# 22. Example Price Sources

Prices were checked in August 2026 and may change.

- [6-DOF metal arm frame without motors — Arduitronics](https://www.arduitronics.com/product/5791/)
- [MG996R 180-degree servo — Arduitronics](https://www.arduitronics.com/product/440/)
- [PCA9685 servo driver listings — Arduitronics](https://www.arduitronics.com/product/tag/servo-motor)
- [5 V 20 A switching power supply — Shopee Thailand](https://shopee.co.th/Switching-Power-Supply-%E0%B8%AA%E0%B8%A7%E0%B8%B4%E0%B8%95%E0%B8%8A%E0%B8%B4%E0%B9%88%E0%B8%87%E0%B9%80%E0%B8%9E%E0%B8%B2%E0%B9%80%E0%B8%A7%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B8%8B%E0%B8%B1%E0%B8%9E%E0%B8%9E%E0%B8%A5%E0%B8%B2%E0%B8%A2-5V-20A-100W-%28%E0%B8%AA%E0%B8%B5%E0%B9%80%E0%B8%87%E0%B8%B4%E0%B8%99%29-i.130672716.2806780813)
- [ReSpeaker 2-microphone Raspberry Pi HAT — Cytron Thailand](https://th.cytron.io/p-respeaker-2-microphone-raspberry-pi-hat)
- [OAK-D Lite autofocus — Arduitronics](https://www.arduitronics.com/product/5252/)
- [OAK-D Lite fixed focus — Arduitronics](https://www.arduitronics.com/product/6727/)
- [RealSense D435 — DigiKey Thailand](https://www.digikey.co.th/en/products/detail/realsense/962305/9926003)
- [RealSense D435i — DigiKey Thailand](https://www.digikey.co.th/th/products/detail/intel-realsense/82635D435IDK5P/9926004)
- [35 kg-cm high-torque servo — Arduitronics](https://www.arduitronics.com/product/5342/)
- [Complete six-servo metal arm kit — Shopee Thailand](https://shopee.co.th/%E0%B8%8A%E0%B8%B8%E0%B8%94%E0%B8%9B%E0%B8%A3%E0%B8%B0%E0%B8%81%E0%B8%AD%E0%B8%9A%E0%B9%81%E0%B8%82%E0%B8%99%E0%B8%81%E0%B8%A5%E0%B9%82%E0%B8%A5%E0%B8%AB%E0%B8%B0%E0%B9%81%E0%B8%9A%E0%B8%9A-6-%E0%B8%88%E0%B8%B8%E0%B8%94%E0%B8%AB%E0%B8%A1%E0%B8%B8%E0%B8%99%E0%B8%AD%E0%B8%B4%E0%B8%AA%E0%B8%A3%E0%B8%B0-%E0%B8%9E%E0%B8%A3%E0%B9%89%E0%B8%AD%E0%B8%A1%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B9%82%E0%B8%A7%E0%B8%A1%E0%B8%AD%E0%B9%80%E0%B8%95%E0%B8%AD%E0%B8%A3%E0%B9%8C-6-%E0%B8%95%E0%B8%B1%E0%B8%A7-%28-6-DOF-Arm-Robotic-Nail-Clamp-Mount-and-servo-motor-Kit%29-i.264166634.3657731949)
