# Autonomous Robotic Arm - Updated System Architecture

**Project:** Microcontroller Interfacing, Semester 1/2026  
**Main controller:** STM32 NUCLEO-F767ZI  
**Vision computer:** Raspberry Pi 4
**Control method:** Direct STM32 hardware PWM  

## 1. Project Goal

The project is a six-axis robotic arm that can find and move a coloured object. It supports automatic voice control and three manual control methods.

The system has four operating modes:

1. Auto/Voice Mode
2. Cartesian Mode
3. Joint Group Mode
4. Single-Joint Service Mode

The Raspberry Pi processes the RGB-D camera and sends object positions to the STM32. The STM32 handles voice recognition, inverse kinematics (IK), movement, manual controls, safety, the display, and servo PWM. The STM32 voice model and complete IK solver are planned; neither is implemented in this repository yet.

## 2. Final Architecture

```text
Raspberry Pi 4
|- RGB-D camera
|- Object and colour detection
`- Depth and object XYZ in robot-base coordinates
              |
              | 3.3 V GPIO UART, TX/RX/GND, 115200 8N1
              v
STM32 NUCLEO-F767ZI - Main Controller
|- SAI/I²S audio input
|  `- 2 × INMP441 microphones, left and right channels
|- Voice recognition and task state machine
|- Inverse kinematics and reachability checks
|- UART receive
|  `- Pi object positions and status
|
|- ADC with DMA
|  |- KY-023 joystick X
|  `- KY-023 joystick Y
|
|- Timer Encoder Mode
|  `- KY-040 rotary encoder rotation
|
|- EXTI interrupts
|  |- MODE button
|  |- KY-040 encoder push button
|  |- Hold-to-Enable button
|  |- Emergency-stop status
|  `- 4 × physical limit switches
|
|- SPI
|  `- ILI9341 2.4-inch graphic TFT LCD
|
|- Mode state machine
|  |- Auto/Voice
|  |- Cartesian
|  |- Joint Group
|  `- Single-Joint Service
|
|- Safety and trajectory controller
|  |- Input filtering
|  |- Joint-angle limits
|  |- Speed and acceleration limits
|  |- Communication timeout
|  `- Smooth movement generation
|
`- STM32 hardware PWM
   |- Servo 1: base
   |- Servo 2: shoulder
   |- Servo 3: elbow
   |- Servo 4: wrist pitch
   |- Servo 5: wrist rotation
   `- Servo 6: gripper
              |
              v
     SN74AHCT244N DIP-20 buffer
              |
              v
          Six servos
```

Six hardware-PWM outputs are planned on TIM3 and TIM4. TIM3 channels 1-2 and TIM4 channels 1-4 are a candidate, not a verified pin map. Check PWM, audio, UART, ADC, SPI, EXTI, the encoder, and all four limit switches together in STM32CubeMX. In particular, do not reuse an audio pin for a limit switch.

## 3. Responsibilities

| Device | Main responsibilities |
|---|---|
| Raspberry Pi 4 | Camera processing, depth measurement, and object XYZ in the robot-base frame |
| STM32F767ZI | Two-microphone voice recognition, task state machine, inverse kinematics, manual inputs, operating modes, safety checks, smooth trajectories, LCD, UART, and six servo PWM signals |
| SN74AHCT244N | Changes the six STM32 PWM signals from 3.3 V logic to reliable 5 V logic |
| Servos | Move the six robot joints to the requested positions |

The STM32 is the main physical controller. The Pi reports what the camera sees; the STM32 decides whether and how to move. Camera-to-robot calibration must convert the reported XYZ position into the robot-base frame.

## 4. Control Panel

The control panel and servo-signal hardware contain:

| Equipment | Quantity | Purpose |
|---|---:|---|
| STM32 NUCLEO-F767ZI | 1 | Main controller, safety checks and six PWM outputs |
| Raspberry Pi 4 | 1 | RGB-D camera processing |
| RGB-D camera | 1 | Object location and depth |
| INMP441 microphone | 2 | Left and right voice input to the STM32 |
| KY-023 joystick module | 1 | Two analogue axes for manual movement |
| KY-040 rotary encoder module | 1 | Rotation and push-button input |
| ILI9341 2.4-inch SPI TFT LCD | 1 | Mode, target and safety status |
| 12 mm normally-open momentary push button | 2 | One MODE button and one separate Hold-to-Enable button |
| 22 mm latching emergency-stop button, 1NO + 1NC | 1 | NC contact controls the servo-power disconnect; NO contact reports status to the STM32 |
| SN74AHCT244N DIP-20 | 1 | Buffers six STM32 PWM signals for the servos |

The KY-023 replaces the earlier six-potentiometer design. Power the KY-023, KY-040 and both INMP441 microphones from 3.3 V so their signals stay within STM32 input limits. The two microphones share audio clock/data wiring and use separate left/right channel selection; verify the exact SAI wiring in CubeMX. The KY-040 push button is separate from the MODE and Hold-to-Enable buttons.

## 5. Operating Modes

### 5.1 Mode cycle

A short press of the MODE button changes between all four modes:

```text
AUTO/VOICE
    |
    v
CARTESIAN
    |
    v
JOINT GROUP
    |
    v
SINGLE-JOINT SERVICE
    |
    v
AUTO/VOICE
```

### 5.2 Single-Joint Service Mode

This mode is in the short-press cycle, but moving a joint still requires the Hold-to-Enable button. The STM32 must debounce MODE presses without blocking the safety system or PWM outputs.

## 6. Controls in Each Mode

| Mode | Joystick | Rotate encoder | Press encoder | Hold-to-Enable |
|---|---|---|---|---|
| Auto/Voice | Movement control disabled | Select display information | Cancel the active task | Not required |
| Cartesian | Move target X and Y | Move target Z | Open or close the gripper | Must be held |
| Joint Group | Control two joints | Select Group 1-3 | Change Fine/Fast speed | Must be held |
| Single-Joint Service | Move the selected joint | Select J1-J6 | Reset the selected target to the current commanded position | Must be held for movement |

### 6.1 Cartesian control

```text
Joystick X       -> Change target X
Joystick Y       -> Change target Y
Encoder rotation -> Change target Z
Encoder press    -> Open or close gripper
```

The STM32 calculates inverse kinematics from the XYZ target and rejects positions outside the arm's reachable workspace. Cartesian Mode therefore works without the Pi.

All three manual modes work without the Pi. Joint Group and Single-Joint Service also remain available if the STM32 voice or IK functions fail.

### 6.2 Joint Group control

| Group | Joystick X | Joystick Y |
|---|---|---|
| Group 1 | J1 Base | J2 Shoulder |
| Group 2 | J3 Elbow | J4 Wrist pitch |
| Group 3 | J5 Wrist rotation | J6 Gripper |

Rotate the KY-040 to select a group. Press it to change between Fine and Fast speed. Joint limits and the Hold-to-Enable button still apply.

## 7. Shared Motion Pipeline

Every mode must use the same safety and movement functions. A mode only creates a new target. It must not control PWM directly.

```text
Auto target ------+
Cartesian target -+--> STM32 IK --+
Joint Group target ---------------+--> Joint-limit and limit-switch checks
Single-Joint target --------------+                  |
                                                   v
                                         Trajectory generator
                                                   |
                                                   v
                                           STM32 hardware PWM
                                                   |
                                                   v
                                                 Servos
```

This design prevents duplicated movement code. It also makes sure that every mode uses the same speed limits, joint limits, and emergency behaviour.

## 8. Safe Mode Switching

Before the STM32 changes modes, it must:

1. Stop accepting movement input from the current mode.
2. Cancel the active voice task and any pending camera request when leaving Auto/Voice Mode.
3. Smoothly reduce the current movement speed.
4. Wait until the planned trajectory state becomes `IDLE`.
5. Require the Hold-to-Enable button to be released.
6. Check that the joystick is near its centre position.
7. Copy the current joint targets into the new mode.
8. Clear old joystick movement commands.
9. Change the mode.
10. Update the graphic LCD.

The old design sent `STOP` to the Motion 2350 Pro. The new design does not use that board. The STM32 stops its own trajectory and holds the current servo targets.

The arm has no external joint encoders. Therefore, the STM32 can confirm that its command has stopped changing, but it cannot prove that every physical joint has reached the exact angle.

## 9. Hold-to-Enable Button

The Hold-to-Enable button is required for movement in all manual modes.

```text
Button held     -> Manual movement is allowed
Button released -> Stop the manual trajectory and hold position
```

Releasing this button should create an EXTI interrupt so the STM32 reacts immediately.

The Hold-to-Enable button is not the same as the emergency stop:

- Hold-to-Enable stops a manual movement command.
- The emergency stop physically disconnects servo power.

## 10. Auto/Voice Mode

```text
Voice command
      |
      v
STM32 recognises the command from two INMP441 microphones
      |
      v
STM32 asks the Pi to find the coloured object
      |
      v
Pi camera reports the object's XYZ position in the robot-base frame
      |
      v
STM32 calculates inverse kinematics and checks reachability
      |
      v
STM32 checks safety and joint limits
      |
      v
STM32 creates a smooth trajectory
      |
      v
STM32 hardware timers generate PWM
      |
      v
Robot arm moves the object
```

The Pi link carries camera results, not six servo angles. A proposed message exchange is:

```text
STM32 → Pi: FIND red_block
Pi → STM32: OBJ red_block 182 -45 30
Pi → STM32: NOTFOUND red_block
```

Coordinates are millimetres in the robot-base frame. The exact message format, timeout, and calibration method still need agreement and testing. Connect Pi GPIO14 (TX, physical pin 8) to an STM32 UART RX pin, STM32 UART TX to Pi GPIO15 (RX, physical pin 10), and the grounds together. Both sides use 3.3 V signals. USART6 on PG9/PG14 is a candidate; verify the board routing and full pin map before wiring. The ST-LINK USB cable may still provide programming, debugging, or power, but it is not the Pi command link.

Because normal hobby servos do not report their positions, a completed trajectory means that the commanded motion time ended. It does not confirm the exact physical position.

## 11. PWM System

The STM32 uses hardware timers to generate stable servo PWM. Software delays must not generate the PWM signal.

A normal servo control period is approximately 20 ms, or 50 Hz. The pulse width normally changes between about 1,000 and 2,000 microseconds. Each servo must be calibrated because its safe limits may be different.

The trajectory controller updates target pulse widths smoothly. It should not immediately jump from one large angle to another.

```text
Unsafe command:
20 degrees -> 120 degrees immediately

Smooth command:
20 -> 22 -> 24 -> ... -> 120 degrees
```

The SN74AHCT244N is an eight-channel DIP-20 buffer, so one chip can support all six PWM signals. Place a 100 nF decoupling capacitor close to its power pins.

### 11.1 SN74AHCT244N Installation

The SN74AHCT244N is not built into the STM32 NUCLEO-F767ZI. It is a separate
20-pin DIP integrated circuit. The STM32 GPIO pins generate 3.3 V PWM signals,
while this buffer changes them into reliable 5 V logic signals for the servos.

For early testing, place the DIP-20 chip across the centre gap of a breadboard.
This does not require soldering. For the final robot, solder a 20-pin DIP socket
onto perfboard and insert the chip into the socket after soldering. The socket
makes the chip easier to replace and prevents soldering heat from reaching it.

```text
STM32 PWM outputs at 3.3 V
              |
              v
      SN74AHCT244N buffer
              |
              v
       5 V PWM logic signals
              |
              v
       Six servo signal wires
```

### 11.2 SN74AHCT244N Wiring

| Chip pin | Signal | Connection |
|---|---|---|
| 20 | VCC | Regulated 5.1 V logic rail |
| 10 | GND | Common system ground |
| 1 | `/1OE` | Ground to enable outputs 1-4 |
| 19 | `/2OE` | Ground to enable outputs 5-8 |
| 2 -> 18 | `1A1 -> 1Y1` | STM32 PWM 1 -> Servo 1 signal |
| 4 -> 16 | `1A2 -> 1Y2` | STM32 PWM 2 -> Servo 2 signal |
| 6 -> 14 | `1A3 -> 1Y3` | STM32 PWM 3 -> Servo 3 signal |
| 8 -> 12 | `1A4 -> 1Y4` | STM32 PWM 4 -> Servo 4 signal |
| 11 -> 9 | `2A1 -> 2Y1` | STM32 PWM 5 -> Servo 5 signal |
| 13 -> 7 | `2A2 -> 2Y2` | STM32 PWM 6 -> Servo 6 signal |
| 15 and 17 | Unused inputs | Connect to ground so they do not float |
| 5 and 3 | Unused outputs | Leave unconnected |

Connect a 100 nF ceramic capacitor directly between pin 20 and pin 10, as close
to the chip as possible. Connect the STM32 ground, buffer ground, and servo
ground together so that every PWM signal has the same voltage reference.

> **Important:** Power the SN74AHCT244N from the regulated 5.1 V logic rail.
> Never connect its VCC pin to the 6.0 V servo rail. The servo current must also
> never pass through the buffer; only the servo signal wires connect to its
> outputs.

## 12. Power Architecture

Servo power and logic power use separate regulated paths, but all devices share a common signal ground.

```text
12 V adapter or 3S LiPo
          |
          +--> Fuse --> E-stop relay --> 20 A buck at 6.0 V
          |                                  |
          |                                  v
          |                         Servo distribution block
          |                                  |
          |                                  v
          |                         Six servo power inputs
          |
          `--> 5 A buck at 5.1 V --> Raspberry Pi and logic
```

For every servo:

```text
STM32 PWM through buffer -> Signal wire
6.0 V distribution      -> Red power wire
Common ground            -> Black or brown ground wire
```

Do not send servo current through the STM32 board or the logic buffer.

For the first development tests, use the 12 V wall adapter. Add battery operation only after the complete arm works reliably on the bench.

## 13. Emergency Stop

The emergency stop must work through both hardware and software:

```text
E-stop pressed
   |- Hardware: relay disconnects the 6 V servo rail
   `- Software: EXTI informs the STM32
```

The power system should include:

- A 22 mm latching emergency-stop button with one normally-closed (NC) and one normally-open (NO) contact
- A fuse close to the battery or source
- A suitable relay or DC power switch
- A transistor or MOSFET relay driver
- A flyback diode across the relay coil
- An EXTI status connection to the STM32

Software stopping alone is not enough because software may freeze.
Use the NC contact in the low-current relay-control circuit so pressing the button releases the relay and removes servo power. Use the NO contact for the STM32 status input. Do not put the servo current through the button contacts; the relay or DC power switch carries it.

Keep four **physical limit switches** at the arm's most important travel boundaries. If one is triggered, the STM32 must block motion farther into that boundary while allowing a safe move away. Their mounting points and GPIO pins require mechanical testing and CubeMX verification. Limit switches are a backup to software angle limits, not a replacement for the hardware emergency stop.

## 14. Graphic LCD

The LCD should show only useful operating information.

Example for Joint Group Mode:

```text
MODE: JOINT GROUP
Group: 2

X: J3 ELBOW
Y: J4 WRIST PITCH
Speed: FINE
```

Example for Cartesian Mode:

```text
MODE: CARTESIAN

X: 120 mm
Y: 80 mm
Z: 150 mm
Gripper: OPEN
```

Example for Single-Joint Service Mode:

```text
MODE: SINGLE-JOINT SERVICE

Joint: J2 SHOULDER
Target: 65 deg
Enable: RELEASED
```

The LCD should also show:

- Pi connection status
- E-stop status
- Limit-switch status
- Current task
- Error messages

## 15. Input Filtering

The joystick ADC values will contain small changes even when the joystick is not moving. The STM32 should use:

- ADC scanning with DMA
- An average or exponential filter
- A centre dead zone
- A small movement deadband

Example:

```text
Raw joystick value changes slightly
2045 -> 2049 -> 2042 -> 2047

Filtered result
2046 -> no movement
```

The dead zone prevents the arm from slowly moving when the joystick is released.
The KY-040's mechanical contacts can also bounce. Filter its CLK/DT inputs and debounce its push button so one physical detent or press produces one action.

## 16. Course Requirement Check

| Course requirement | Project implementation | Status |
|---|---|---|
| STM32, HAL, and STM32CubeIDE | STM32F767ZI is the main controller | Satisfied |
| Interrupts from two modules | UART and EXTI; audio DMA and timers are also planned | Planned |
| ADC | Joystick X and Y | Satisfied |
| PWM | Six STM32 hardware PWM outputs | Satisfied |
| Graphic LCD | ILI9341 2.4-inch SPI TFT LCD | Satisfied |
| At least 4-6 features | Four control modes, with voice, vision, LCD and safety functions | Satisfied |

This architecture uses all three requested modules: ADC, PWM, and graphic LCD.

## 17. Main Features

The project has four main control features:

1. **Auto/Voice:** Say a fixed command, such as "pick up the red block." The STM32 recognises it using two microphones and asks the Pi camera for the object's position. The STM32 calculates the joint angles and moves the arm.
2. **Cartesian:** Use the KY-023 joystick to move the target in X and Y, and turn the KY-040 to change Z. The STM32 calculates the joint angles, so this mode works without the Pi.
3. **Joint Group:** Move two selected joints with the KY-023 joystick. Turn the KY-040 to select one of three joint groups.
4. **Single-Joint Service:** Test one joint at a time. Turn the KY-040 to select J1-J6, then move that joint with the joystick.

The 2.4-inch ILI9341 LCD shows the active mode and status. Manual movement requires Hold-to-Enable; four physical limit switches protect travel boundaries, and the emergency stop disconnects servo power.

## 18. Development Priority

The recommended development order is:

1. Verify the complete STM32CubeMX pin map.
2. Test one servo with STM32 hardware PWM.
3. Test all six PWM outputs without attaching the arm links.
4. Read and filter the KY-023 joystick and KY-040 encoder.
5. Implement Joint Group Mode with Hold-to-Enable.
6. Implement Single-Joint Service Mode.
7. Add the short-press MODE cycle.
8. Add the ILI9341 graphic LCD.
9. Implement and test STM32 inverse kinematics and Cartesian Mode with measured arm geometry.
10. Test Pi GPIO UART communication and camera-to-robot calibration.
11. Add the camera object search and depth calculation on the Pi.
12. Capture audio from both STM32 microphones and validate the planned voice model on the board.
13. Install and test all four limit switches and the hardware emergency stop before autonomous motion.
14. Test the complete autonomous sequence.
15. Add battery operation only if enough time remains.

The Motion 2350 Pro and PCA9685 are not part of the final architecture. They may be kept only as backup hardware if direct STM32 PWM cannot be completed.
