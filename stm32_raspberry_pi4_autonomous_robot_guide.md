# Autonomous Mobile Robot Navigation Using STM32, Raspberry Pi 4, and LiDAR

## 1. Project Overview

This project builds a real autonomous mobile robot that can enter an unknown indoor area and navigate without using a prepared map.

The robot will:

1. Scan the environment with a 2D LiDAR.
2. create a map while it is moving.
3. estimate its position inside the map.
4. detect free and unknown areas.
5. choose a place to explore.
6. calculate a safe path.
7. follow the path.
8. avoid obstacles.
9. stop safely if a fault happens.

The project uses two computers:

- **Raspberry Pi 4** for SLAM, path planning, exploration, and ROS 2.
- **STM32** for motor control, encoders, PID control, and safety.

This is called a **two-level robot control system**.

---

## 2. Recommended Project Name

**STM32 and Raspberry Pi Based Autonomous Mobile Robot with Online LiDAR SLAM and Navigation**

A shorter project name is:

**LiDAR-Based Autonomous Exploration Robot**

---

## 3. Main Project Goal

The main goal is:

> Build a small indoor robot that can create a map of an unknown environment and autonomously explore it without receiving a map before deployment.

The robot does not need to see the map before starting.

When the robot is powered on, it will:

```text
Start
  ↓
Check sensors and motors
  ↓
Start LiDAR scanning
  ↓
Create an empty map
  ↓
Rotate slowly to scan the area
  ↓
Find an unexplored area
  ↓
Plan a path
  ↓
Move toward the target
  ↓
Avoid obstacles
  ↓
Update the map
  ↓
Repeat until exploration is complete
```

---

## 4. Important Design Decision

Do not run every algorithm on the STM32.

The STM32 is good at fast and reliable low-level control. However, it does not have enough memory and processing power for full LiDAR SLAM and ROS 2 navigation.

Use the devices as follows.

### Raspberry Pi 4

The Raspberry Pi runs:

- Ubuntu Linux
- ROS 2
- LiDAR driver
- SLAM Toolbox
- Nav2
- Theta* path planning
- Regulated Pure Pursuit
- Local obstacle detection
- Frontier exploration
- Mission management

### STM32

The STM32 runs:

- Motor PWM control
- Motor direction control
- Encoder reading
- Wheel speed calculation
- Left and right wheel PID
- Communication watchdog
- Emergency stop logic
- Battery monitoring, if available

---

## 5. System Architecture

```text
                    2D LiDAR
                        │ USB
                        ▼
              ┌───────────────────┐
              │ Raspberry Pi 4    │
              │ Ubuntu + ROS 2    │
              │                   │
              │ LiDAR Driver      │
              │ SLAM Toolbox      │
              │ Nav2              │
              │ Theta*            │
              │ Pure Pursuit      │
              │ Exploration       │
              └─────────┬─────────┘
                        │ USB Serial
                        │
              ┌─────────▼─────────┐
              │ STM32             │
              │                   │
              │ Encoder Reading   │
              │ Wheel PID         │
              │ Motor Safety      │
              └──────┬─────┬──────┘
                     │     │
                 Driver   Driver
                     │     │
               Left Motor Right Motor
                 + Encoder + Encoder
```

The LiDAR should connect directly to the Raspberry Pi.

Do not send all LiDAR data through the STM32.

---

# 6. Recommended Robot Type

Use a **two-wheel differential-drive robot**.

It has:

- one left motor,
- one right motor,
- two normal wheels,
- and one caster wheel.

```text
Top View

             Front
        ┌─────────────┐
        │    LiDAR    │
        │             │
 Left   │ Raspberry   │   Right
 Wheel  │ Pi + STM32  │   Wheel
        │             │
        └──────┬──────┘
               │
          Caster Wheel
              Rear
```

## Why Use Two Wheels First?

A two-wheel robot is:

- cheaper,
- easier to build,
- easier to control,
- easier to calculate odometry,
- easier to tune,
- less affected by wheel slipping,
- and well supported by ROS 2 Nav2.

A four-wheel skid-steer robot can be added later. It causes more wheel slip when turning, which can reduce SLAM accuracy.

---

# 7. Equipment List

You already have:

| Equipment | Status |
|---|---|
| Raspberry Pi 4 | Already available |
| STM32 development board | Already available |
| Laptop or desktop computer | Assumed available |

## 7.1 Essential Equipment to Buy

| Equipment | Quantity | Purpose | Target Price |
|---|---:|---|---:|
| YDLIDAR X2 or similar 360-degree 2D LiDAR | 1 | Mapping and obstacle detection | ฿2,000–฿2,900 |
| JGB37-520 12 V geared motor with AB encoder | 2 | Robot movement and odometry | About ฿740 total |
| BTS7960 motor driver | 2 | Drive the two DC motors | About ฿280 total |
| Rubber wheels for 6 mm motor shaft | 2 | Main robot wheels | ฿150–฿250 |
| Caster wheel | 1 | Support the rear or front | ฿40–฿100 |
| Motor brackets | 2 | Attach motors to chassis | ฿100–฿200 |
| 3S lithium battery with BMS | 1 | Power the robot | ฿300–฿500 |
| 12.6 V charger for 3S battery | 1 | Charge the battery safely | Sometimes included |
| 5 V, at least 3 A DC-DC buck converter | 1 | Power Raspberry Pi 4 | ฿100–฿200 |
| Fuse and fuse holder | 1 | Electrical safety | ฿30–฿80 |
| Main power switch | 1 | Turn robot power on and off | ฿20–฿50 |
| Emergency stop switch | 1 | Stop motors immediately | ฿80–฿200 |
| Wires, connectors, terminals, and screws | Set | Electrical and mechanical assembly | ฿100–฿250 |
| Chassis material | 1 | Robot structure | ฿0–฿300 |

## 7.2 Optional Equipment

| Equipment | Purpose |
|---|---|
| IMU such as MPU6050 or BNO055 | Improve turning and orientation estimation |
| Bumper switches | Detect physical contact |
| Cliff sensors | Detect stairs or floor edges |
| Current sensor | Detect motor overload |
| Battery voltage sensor | Monitor battery level |
| Cooling fan or heatsink for Raspberry Pi | Prevent high temperature |
| USB hub with external power | Useful if USB power is not stable |

## 7.3 Tools

You may also need:

- soldering iron,
- solder,
- multimeter,
- wire cutter,
- wire stripper,
- screwdrivers,
- heat-shrink tube,
- electrical tape,
- cable ties,
- drill,
- and a ruler or vernier caliper.

---

# 8. Budget Plan

The ฿5,000 budget is possible, but it is tight because the LiDAR is the most expensive part.

## Example Budget

| Item | Estimated Cost |
|---|---:|
| YDLIDAR X2 | ฿2,000–฿2,895 |
| Two encoder motors | ฿740 |
| Two BTS7960 drivers | ฿280 |
| Wheels, caster, and brackets | ฿300–฿500 |
| Battery and charger | ฿350–฿550 |
| Raspberry Pi buck converter | ฿100–฿200 |
| Chassis, wires, fuse, and switches | ฿200–฿500 |
| **Estimated total** | **฿3,970–฿5,665** |

To remain close to ฿5,000:

1. Use plywood, acrylic scrap, or an old robot chassis.
2. Find a LiDAR sale or a second-hand unit.
3. Confirm that the LiDAR includes its USB adapter.
4. Use two motors, not four.
5. Do not buy a camera during the first version.
6. Delay the IMU if the budget is too tight.
7. Reuse a battery only if it is safe and suitable.

A Thai retailer listed the YDLIDAR X2 at ฿2,895 and the JGB37-520 encoder motor at about ฿370 each when this guide was prepared. Prices can change.

---

# 9. Software Stack

Install the following software on the Raspberry Pi 4:

```text
Ubuntu Server 24.04 ARM64
ROS 2 Jazzy
Nav2
SLAM Toolbox
robot_localization
YDLIDAR ROS 2 driver
Custom STM32 bridge node
Frontier exploration package
```

ROS 2 Jazzy supports Ubuntu 24.04 on 64-bit ARM systems.

Use **Ubuntu Server** instead of the full desktop version because it uses less RAM and CPU.

Run RViz on your laptop through Wi-Fi.

```text
Robot Raspberry Pi 4 ───── Wi-Fi ───── Laptop
ROS 2 navigation                         RViz
```

---

# 10. Main Algorithms

## 10.1 Online SLAM

Use **SLAM Toolbox**.

SLAM means:

- **Simultaneous Localization and Mapping**.

The robot builds a map and estimates its position at the same time.

SLAM Toolbox receives:

```text
/scan
/odom
/tf
```

It produces:

```text
/map
map → odom transform
```

Use online asynchronous mode:

```text
slam_toolbox online_async
```

---

## 10.2 Theta* Path Planning

Theta* is the global path planner.

Its job is to find a short path from the robot to the target.

Theta* can create paths with fewer unnecessary grid turns than basic A*.

Example:

```text
Robot Position
      │
      ▼
Theta* reads the map
      │
      ▼
Theta* avoids occupied cells
      │
      ▼
Theta* creates a path to the target
```

Use the Nav2 Theta Star planner plugin instead of writing the full algorithm first.

---

## 10.3 Regulated Pure Pursuit

Pure Pursuit follows the planned path.

The controller selects a point in front of the robot and turns toward it.

Use **Regulated Pure Pursuit** because it can:

- reduce speed during sharp turns,
- reduce speed near obstacles,
- check possible collisions,
- and follow paths more smoothly.

The Raspberry Pi calculates the required robot velocity:

```text
linear velocity  = v
angular velocity = w
```

It sends this velocity to the STM32.

---

## 10.4 Local Obstacle Avoidance

For the first version, do not use Artificial Potential Field as the main safety system.

APF can:

- become stuck between obstacles,
- oscillate,
- fail inside U-shaped areas,
- and conflict with the path controller.

Start with:

```text
Nav2 Local Costmap
        +
Regulated Pure Pursuit
        +
Nav2 Collision Monitor
```

APF can be added later as an experiment or custom controller.

---

## 10.5 Frontier Exploration

SLAM creates the map, but it does not decide where the robot should go.

Frontier exploration finds the boundary between:

- known free space,
- and unknown space.

```text
Known Area | Frontier | Unknown Area
```

The exploration process is:

```text
Read the map
    ↓
Find frontiers
    ↓
Remove unsafe or unreachable frontiers
    ↓
Choose the best frontier
    ↓
Send it as a Nav2 goal
    ↓
Move to the goal
    ↓
Update the map
    ↓
Repeat
```

This is the part that lets the robot explore without receiving a map first.

---

# 11. STM32 Motor Control

## 11.1 Input from Raspberry Pi

The Raspberry Pi sends:

- linear velocity `v`,
- angular velocity `w`,
- motor enable command,
- heartbeat message.

Example:

```text
CMD,0.20,0.30
```

Meaning:

```text
Move forward at 0.20 m/s
Turn left at 0.30 rad/s
```

## 11.2 Differential-Drive Calculation

The STM32 calculates the left and right wheel speed.

\[
v_L = v - \frac{\omega L}{2}
\]

\[
v_R = v + \frac{\omega L}{2}
\]

Where:

- \(v_L\) is left wheel linear speed,
- \(v_R\) is right wheel linear speed,
- \(v\) is robot linear speed,
- \(\omega\) is robot angular speed,
- \(L\) is the distance between the two wheels.

Convert wheel linear speed to wheel angular speed:

\[
\omega_{wheel} = \frac{v_{wheel}}{r}
\]

Where \(r\) is the wheel radius.

---

## 11.3 Encoder Reading

Each motor must have an AB quadrature encoder.

Connect encoder A and B signals to STM32 timer encoder inputs.

The STM32 uses encoder pulses to calculate:

- motor direction,
- motor speed,
- travel distance,
- and robot odometry.

Do not use motors without encoders for this project.

---

## 11.4 Wheel PID

Each wheel needs its own PID controller.

```text
Target Wheel Speed
        │
        ▼
Compare with Measured Speed
        │
        ▼
PID Controller
        │
        ▼
PWM Output
        │
        ▼
Motor Driver
        │
        ▼
Motor
```

The controller should run at approximately 100–500 Hz for the first version.

A higher rate can be used after the system becomes stable.

---

## 11.5 Communication Watchdog

The STM32 must stop the robot if it does not receive a command.

Example rule:

```text
If no valid command is received for 500 ms:
    Set left motor PWM to zero
    Set right motor PWM to zero
    Disable motor drivers
```

This protection must work even if ROS 2 crashes.

---

# 12. Communication Between Raspberry Pi and STM32

Use USB serial for the first version.

```text
Raspberry Pi USB
      │
      ▼
STM32 USB or USB-to-UART
```

A simple protocol is easier than micro-ROS for the first prototype.

## Raspberry Pi to STM32

```text
CMD,<linear_velocity>,<angular_velocity>
```

Example:

```text
CMD,0.20,0.00
```

## STM32 to Raspberry Pi

```text
ODOM,<left_count>,<right_count>,<left_speed>,<right_speed>
```

Example:

```text
ODOM,12550,12490,0.198,0.196
```

Add a checksum or CRC after the basic communication works.

Example improved packet:

```text
$CMD,0.20,0.00,157A
```

---

# 13. Power System

Use one battery with separate power paths.

```text
                  3S Battery
                      │
                  Main Fuse
                      │
              Emergency Stop
                      │
          ┌───────────┴───────────┐
          │                       │
  Motor Driver Power       5 V Buck Converter
          │                       │
       Motors                Raspberry Pi 4
                                  │ USB
                                STM32
```

## Important Power Rules

1. Do not power motors from the Raspberry Pi.
2. Do not power motors from the STM32.
3. Use a common electrical ground.
4. Place a fuse close to the battery.
5. Use a stable 5 V supply for the Raspberry Pi.
6. Raspberry Pi 4 normally requires a good 5 V, 3 A supply.
7. The LiDAR also uses power, so the converter needs enough current.
8. Use thick wires for battery and motor connections.
9. Keep encoder wires away from motor power wires when possible.
10. Add capacitors near the motor driver if electrical noise is high.

---

# 14. Development Process

Do not build every function at the same time.

Use the following stages.

---

## Stage 1: Define the First Version

The first robot should work in:

- an indoor environment,
- a flat floor,
- a small room or corridor,
- low speed,
- no stairs,
- and normal lighting.

Recommended limits:

```text
Maximum speed: 0.20–0.30 m/s
Robot width: below 45 cm
Robot length: below 50 cm
LiDAR height: about 15–30 cm above floor
```

---

## Stage 2: Build the Chassis

Install:

1. two encoder motors,
2. two main wheels,
3. one caster wheel,
4. battery,
5. motor drivers,
6. STM32,
7. Raspberry Pi,
8. LiDAR.

Keep the LiDAR:

- level,
- stable,
- near the centre,
- higher than the robot body,
- and free from cable blockage.

Keep heavy parts such as the battery low to improve stability.

---

## Stage 3: Test One Motor

Before moving the robot:

1. Lift the wheels from the floor.
2. Connect one motor driver.
3. Send a low PWM value.
4. Test forward movement.
5. Test reverse movement.
6. Check the motor current.
7. Check if the driver becomes hot.
8. Press the emergency stop.

Do not test at full power first.

---

## Stage 4: Read Encoders

For each wheel:

1. Rotate the wheel by hand.
2. Check encoder count.
3. Rotate it forward.
4. Confirm the count increases.
5. Rotate it backward.
6. Confirm the count decreases.

If the direction is wrong, swap encoder A and B in software or hardware.

---

## Stage 5: Implement Wheel PID

Test one wheel before using two wheels.

Test these target speeds:

```text
10 RPM
20 RPM
30 RPM
40 RPM
```

Record:

- target speed,
- measured speed,
- PWM,
- overshoot,
- settling time.

After one wheel works, create a separate PID controller for the second wheel.

---

## Stage 6: Test Differential Drive

Test these movements:

1. forward,
2. backward,
3. rotate left,
4. rotate right,
5. move in an arc,
6. stop,
7. communication timeout.

At this stage, control the robot manually from a laptop.

---

## Stage 7: Connect Raspberry Pi and STM32

Create a ROS 2 bridge node on the Raspberry Pi.

The bridge node:

```text
Subscribes to /cmd_vel
        ↓
Converts velocity into a serial command
        ↓
Sends command to STM32
        ↓
Reads encoder data from STM32
        ↓
Publishes wheel odometry
```

Main ROS 2 topics:

```text
/cmd_vel
/wheel/odom_raw
/odom
/joint_states
/diagnostics
```

---

## Stage 8: Calibrate Wheel Odometry

### Straight-Line Test

1. Mark a 2 m straight line.
2. Command the robot to travel 2 m.
3. Measure the real distance.
4. Compare it with ROS odometry.
5. Adjust wheel radius.

### Rotation Test

1. Mark the starting direction.
2. Command a 360-degree turn.
3. Check the final direction.
4. Adjust the wheel separation value.

Repeat each test several times.

Do not continue to SLAM until odometry is reasonably stable.

---

## Stage 9: Install and Test LiDAR

Connect the LiDAR directly to the Raspberry Pi.

Check the device:

```bash
ls /dev/ttyUSB*
```

Start the LiDAR ROS 2 driver.

Check the scan topic:

```bash
ros2 topic echo /scan
```

Open RViz on the laptop.

Add:

```text
LaserScan
Topic: /scan
```

Rotate the robot slowly.

The walls in RViz should appear in the correct direction.

---

## Stage 10: Create the Robot TF Tree

The robot needs coordinate frames.

```text
map
 └── odom
      └── base_link
           ├── laser_frame
           ├── left_wheel_link
           └── right_wheel_link
```

Important rules:

- SLAM publishes `map → odom`.
- Odometry publishes `odom → base_link`.
- The robot model publishes `base_link → laser_frame`.
- Do not publish the same transform from two nodes.

---

## Stage 11: Test Online SLAM

Start SLAM Toolbox in online asynchronous mode.

First, control the robot manually.

Drive slowly around one room.

Check that:

- walls remain straight,
- the map does not rotate incorrectly,
- the robot position is stable,
- loop closure works,
- and old walls match new scans.

If the map is poor, check:

1. wheel radius,
2. wheel separation,
3. encoder direction,
4. LiDAR mounting,
5. TF frames,
6. timestamps,
7. wheel slip,
8. robot speed.

---

## Stage 12: Add Nav2

Configure:

- global costmap,
- local costmap,
- obstacle layer,
- inflation layer,
- robot footprint,
- Theta* planner,
- Regulated Pure Pursuit,
- velocity smoother,
- collision monitor.

Use the real robot shape.

Example rectangular footprint:

```yaml
footprint: "[[0.22, 0.18],
             [0.22,-0.18],
             [-0.22,-0.18],
             [-0.22,0.18]]"
```

Replace the numbers with real measurements.

Start at a low speed:

```yaml
desired_linear_vel: 0.15
max_linear_velocity: 0.25
max_angular_velocity: 0.8
```

---

## Stage 13: Test Goal Navigation

Before autonomous exploration:

1. Build a map manually.
2. click a nearby goal in RViz.
3. Check global path generation.
4. Check path following.
5. Put a box in front of the robot.
6. Confirm that the robot slows down, stops, or replans.
7. Test recovery after a blocked path.

Do not continue until normal goal navigation is reliable.

---

## Stage 14: Add Frontier Exploration

After Nav2 works:

1. Start SLAM.
2. Start Nav2.
3. Start the frontier exploration node.
4. Let the robot choose a nearby frontier.
5. Observe the selected goal.
6. Stop the robot if the target is unsafe.
7. Tune frontier distance and cost.
8. Add a blacklist for failed frontiers.
9. Add return-to-start after exploration.

The robot can now explore an unknown area without receiving a prepared map.

---

## Stage 15: Automatic Startup

Create one ROS 2 launch file.

Example:

```text
amr_system.launch.py
```

It should start:

```text
robot_state_publisher
STM32 serial bridge
LiDAR driver
wheel odometry
SLAM Toolbox
Nav2
Collision Monitor
frontier exploration
diagnostics
mission manager
```

Recommended mission states:

```text
BOOT
  ↓
SELF_TEST
  ↓
START_LIDAR
  ↓
START_SLAM
  ↓
INITIAL_ROTATION
  ↓
EXPLORE
  ↓
SAVE_MAP
  ↓
RETURN_HOME
  ↓
IDLE
```

---

# 15. Suggested ROS 2 Workspace

```text
amr_ws/
└── src/
    ├── amr_description/
    │   ├── urdf/
    │   ├── meshes/
    │   └── launch/
    │
    ├── amr_stm32_bridge/
    │   ├── src/
    │   ├── include/
    │   └── launch/
    │
    ├── amr_odometry/
    │   ├── src/
    │   └── config/
    │
    ├── amr_navigation/
    │   ├── config/
    │   ├── maps/
    │   └── launch/
    │
    └── amr_bringup/
        └── launch/
```

---

# 16. Suggested STM32 Firmware Structure

```text
Core/
├── Inc/
│   ├── motor.h
│   ├── encoder.h
│   ├── pid.h
│   ├── communication.h
│   ├── odometry.h
│   └── safety.h
│
└── Src/
    ├── motor.c
    ├── encoder.c
    ├── pid.c
    ├── communication.c
    ├── odometry.c
    ├── safety.c
    └── main.c
```

Suggested tasks:

```text
Motor PID:          100–500 Hz
Encoder update:     100–500 Hz
Serial reception:   interrupt or DMA
Odometry message:   30–50 Hz
Safety check:       every control loop
```

---

# 17. Safety Requirements

The robot must stop when:

- the emergency stop is pressed,
- the Raspberry Pi communication is lost,
- no velocity command is received for 500 ms,
- a motor driver reports a fault,
- battery voltage becomes too low,
- encoder data becomes invalid,
- or the software enters an unknown state.

## Physical Safety

A single 2D LiDAR may not detect:

- table surfaces above the laser,
- very low objects,
- stairs,
- glass walls,
- thin chair legs,
- hanging objects,
- or objects outside the scan height.

For better safety, add:

- bumper switches,
- cliff sensors,
- low speed limits,
- a large emergency stop,
- and a human operator during testing.

Never test the first version near stairs.

---

# 18. Testing Checklist

## Motor System

- [ ] Left motor moves forward correctly.
- [ ] Right motor moves forward correctly.
- [ ] Encoder direction is correct.
- [ ] Wheel speed PID works.
- [ ] Motors stop after communication timeout.
- [ ] Emergency stop works.

## Raspberry Pi and Communication

- [ ] Raspberry Pi has stable 5 V power.
- [ ] STM32 connects by USB serial.
- [ ] `/cmd_vel` reaches the STM32.
- [ ] Encoder data reaches ROS 2.
- [ ] Odometry updates correctly.

## LiDAR and SLAM

- [ ] LiDAR publishes `/scan`.
- [ ] LiDAR frame is correct.
- [ ] Map is created.
- [ ] Walls remain stable while turning.
- [ ] Loop closure works.

## Navigation

- [ ] Theta* creates a path.
- [ ] Pure Pursuit follows the path.
- [ ] Local costmap detects obstacles.
- [ ] Robot stops near obstacles.
- [ ] Robot replans when blocked.
- [ ] Frontier exploration selects new areas.

---

# 19. Recommended Milestones

## Milestone 1: Motor Base

The STM32 controls two motors with encoder PID.

Success condition:

> The robot can move forward, rotate, stop, and maintain similar left and right wheel speeds.

## Milestone 2: ROS 2 Control

The Raspberry Pi sends `/cmd_vel` to the STM32.

Success condition:

> The robot can be controlled by ROS 2 teleoperation.

## Milestone 3: Odometry

The Raspberry Pi receives encoder data and publishes `/odom`.

Success condition:

> The robot movement in RViz matches the real movement.

## Milestone 4: LiDAR SLAM

The robot creates a map while being manually controlled.

Success condition:

> The robot can map one room without major map distortion.

## Milestone 5: Autonomous Navigation

The robot moves to a selected goal.

Success condition:

> The robot follows the path and avoids normal obstacles.

## Milestone 6: Autonomous Exploration

The robot chooses unexplored areas automatically.

Success condition:

> The robot explores a small unknown room without a prepared map.

---

# 20. What Not to Do at the Beginning

Do not begin with:

- four-wheel skid steering,
- mecanum wheels,
- APF as the only obstacle avoidance method,
- high robot speed,
- outdoor navigation,
- camera object detection,
- deep learning,
- automatic charging,
- multi-floor navigation,
- or fully autonomous testing near people.

Build a reliable simple robot first.

---

# 21. Future Improvements

After the first version works, you can add:

1. IMU and EKF sensor fusion.
2. Four-wheel skid-steer chassis.
3. Artificial Potential Field controller.
4. Depth camera.
5. YOLO object detection.
6. Dynamic obstacle tracking.
7. Automatic charging station.
8. Map saving and reuse.
9. Voice or web control.
10. Multi-robot exploration.
11. Better LiDAR.
12. ROS 2 micro-ROS communication.
13. Battery percentage estimation.
14. Return-to-home behavior.

---

# 22. Recommended Final Scope

For a student project, the recommended scope is:

> Design and develop a two-wheel autonomous mobile robot using an STM32 and Raspberry Pi 4. The STM32 controls the motors with encoder feedback and PID. The Raspberry Pi processes 2D LiDAR data, performs online SLAM, plans paths using Theta*, follows paths using Regulated Pure Pursuit, avoids obstacles using Nav2 costmaps and Collision Monitor, and explores unknown areas using frontier exploration.

This scope is realistic, technically strong, and suitable for a budget prototype.

---

# 23. Final Equipment Summary

## Already Available

- Raspberry Pi 4
- STM32 board
- Computer for programming and RViz

## Must Buy

- 360-degree 2D LiDAR
- two geared motors with AB encoders
- two motor drivers
- two wheels
- one caster wheel
- motor brackets
- battery
- battery charger
- 5 V buck converter
- fuse
- power switch
- emergency stop
- wires and connectors
- chassis material

## Recommended Later

- IMU
- bumper switches
- cliff sensors
- current sensor
- better battery monitor
- camera
- stronger LiDAR

---

# 24. References

- [ROS 2 Jazzy Installation for Ubuntu](https://docs.ros.org/en/jazzy/Installation/Alternatives/Ubuntu-Install-Binary.html)
- [Nav2: Navigating While Mapping](https://docs.nav2.org/tutorials/docs/navigation2_with_slam.html)
- [Nav2 Documentation](https://docs.nav2.org/)
- [Raspberry Pi 4 Specifications](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/specifications/)
- [YDLIDAR X2 Thai Retail Listing and Specifications](https://www.arduitronics.com/product/4403/ydlidar-x2-lidar-360-degree-laser-range-scanner-8m-supports-ros)
- [JGB37-520 Encoder Motor Listing](https://www.modulemore.com/en/product/2626/dc-gear-motor-12v-60rpm-ab-encoder-1-169-mecanum-wheel)

---

**Document version:** 1.0  
**Updated:** 6 August 2026  
**Budget target:** Approximately ฿5,000, excluding the Raspberry Pi 4 and STM32 already owned
