# Autonomous Robotic Arm - Updated System Architecture

**Project:** Microcontroller Interfacing, Semester 1/2026  
**Main controller:** STM32 NUCLEO-F767ZI  
**High-level computer:** Raspberry Pi 4  
**Control method:** Direct STM32 hardware PWM  

## 1. Project Goal

The project is a six-axis robotic arm that can find and move a coloured object. It supports automatic voice control and three manual control methods.

The system has four operating modes:

1. Auto/Voice Mode
2. Cartesian Mode
3. Joint Group Mode
4. Single-Joint Service Mode

The Raspberry Pi handles tasks that require more processing power. The STM32 handles real-time movement, manual controls, safety, the display, and servo PWM.

## 2. Final Architecture

```text
Raspberry Pi 4
|- RGB-D camera
|- INMP441 microphone
|- Speech recognition
|- Object and colour detection
|- Depth calculation
|- Inverse kinematics
`- Autonomous task planning
              |
              | USB UART at 115200 8N1
              v
STM32 NUCLEO-F767ZI - Main Controller
|- UART interrupt
|  `- Pi commands and system status
|
|- ADC with DMA
|  |- Joystick X
|  `- Joystick Y
|
|- Timer Encoder Mode
|  `- Rotary encoder rotation
|
|- EXTI interrupts
|  |- MODE button
|  |- Encoder push button
|  |- Hold-to-Enable button
|  |- Emergency-stop status
|  `- Limit switches
|
|- SPI
|  `- Graphic LCD, ILI9341 or ST7789
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
     SN74AHCT244N buffer
              |
              v
          Six servos
```

The planned PWM assignment is TIM3 channels 1-4 and TIM4 channels 1-2. The final pins must be checked in STM32CubeMX because the PWM pins must not conflict with UART, ADC, SPI, EXTI, or the rotary encoder timer.

## 3. Responsibilities

| Device | Main responsibilities |
|---|---|
| Raspberry Pi 4 | Speech recognition, camera processing, depth measurement, inverse kinematics, and automatic task planning |
| STM32F767ZI | Manual inputs, operating modes, safety checks, smooth trajectories, LCD, communication, and six servo PWM signals |
| SN74AHCT244N | Changes the six STM32 PWM signals from 3.3 V logic to reliable 5 V logic |
| Servos | Move the six robot joints to the requested positions |

The STM32 is the main physical controller. The Raspberry Pi may request a movement, but the STM32 checks the request before moving the arm.

## 4. Control Panel

The control panel contains:

- One two-axis analogue joystick
- One rotary encoder with a push button
- One MODE button
- One Hold-to-Enable button
- One separate emergency-stop button
- One SPI graphic LCD

The joystick replaces the earlier six-potentiometer design. Its X and Y outputs give the STM32 two ADC inputs.

## 5. Operating Modes

### 5.1 Normal mode cycle

A short press of the MODE button changes between the three normal modes:

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
AUTO/VOICE
```

### 5.2 Single-Joint Service Mode

Single-Joint Test is a protected service mode. It is not part of the normal short-press cycle.

```text
Hold MODE for 2 seconds
          |
          v
Enter SINGLE-JOINT SERVICE

Hold MODE for 2 seconds again
          |
          v
Exit SINGLE-JOINT SERVICE
```

The STM32 must not use `HAL_Delay(2000)` to detect the long press. It should record the button press and release times with a hardware timer. This allows the safety system and PWM outputs to continue working.

## 6. Controls in Each Mode

| Mode | Joystick | Rotate encoder | Press encoder | Hold-to-Enable |
|---|---|---|---|---|
| Auto/Voice | Movement control disabled | Select display information | Cancel the active task | Not required |
| Cartesian | Move target X and Y | Move target Z | Open or close the gripper | Must be held |
| Joint Group | Control two joints | Select Group 1-3 | Change Fine/Fast speed | Must be held |
| Single-Joint Service | Move the selected joint | Select J1-J6 | Reset the selected target | Must be held |

### 6.1 Joint groups

| Group | Joystick X | Joystick Y |
|---|---|---|
| Group 1 | J1 Base | J2 Shoulder |
| Group 2 | J3 Elbow | J4 Wrist pitch |
| Group 3 | J5 Wrist rotation | J6 Gripper |

### 6.2 Cartesian control

```text
Joystick X       -> Change target X
Joystick Y       -> Change target Y
Encoder rotation -> Change target Z
Encoder press    -> Open or close gripper
```

Cartesian Mode sends the XYZ target to the Raspberry Pi. The Pi calculates inverse kinematics and returns six target joint angles. Therefore, Cartesian Mode needs the Pi.

Joint Group Mode and Single-Joint Service Mode can operate without the Pi. They provide a manual fallback if speech, camera, or Pi software fails.

## 7. Shared Motion Pipeline

Every mode must use the same safety and movement functions. A mode only creates a new target. It must not control PWM directly.

```text
Auto joint target ---------+
Cartesian joint target ----+
Joint Group target --------+--> Joint-limit checks
Single-Joint target -------+            |
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
2. Cancel the active Raspberry Pi task when leaving Auto/Voice Mode.
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
Raspberry Pi recognises the command
      |
      v
Camera finds the coloured object
      |
      v
Depth camera calculates the XYZ position
      |
      v
Pi calculates inverse kinematics
      |
      v
Pi sends six target angles to STM32
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

An example UART command is:

```text
MOVE 90 65 110 45 90 30 2500
```

The command contains six target joint angles and the planned movement time in milliseconds.

Because normal hobby servos do not report their positions, `DONE` means that the planned movement time has ended. It does not confirm the exact physical position.

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

- A latching emergency-stop button
- A fuse close to the battery or source
- A suitable relay or DC power switch
- A transistor or MOSFET relay driver
- A flyback diode across the relay coil
- An EXTI status connection to the STM32

Software stopping alone is not enough because software may freeze.

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
SERVICE: SINGLE JOINT

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

## 16. Course Requirement Check

| Course requirement | Project implementation | Status |
|---|---|---|
| STM32, HAL, and STM32CubeIDE | STM32F767ZI is the main controller | Satisfied |
| Interrupts from two modules | UART interrupt and EXTI; timer interrupts are also used | Satisfied |
| ADC | Joystick X and Y | Satisfied |
| PWM | Six STM32 hardware PWM outputs | Satisfied |
| Graphic LCD | SPI graphic LCD | Satisfied |
| At least 4-6 features | Four modes, voice, vision, LCD, and safety functions | Satisfied |

This architecture uses all three requested modules: ADC, PWM, and graphic LCD.

## 17. Main Features

The project can present these six main features:

1. Voice-controlled automatic pick-and-place
2. RGB-D object position detection
3. Cartesian XYZ manual control
4. Two-joint group control
5. Protected single-joint testing
6. LCD status and safety monitoring

## 18. Development Priority

The recommended development order is:

1. Verify the complete STM32CubeMX pin map.
2. Test one servo with STM32 hardware PWM.
3. Test all six PWM outputs without attaching the arm links.
4. Read and filter the joystick.
5. Implement Joint Group Mode.
6. Implement Single-Joint Service Mode.
7. Add the MODE and Hold-to-Enable behaviour.
8. Add the graphic LCD.
9. Test Pi-to-STM32 UART communication.
10. Implement Cartesian Mode through the Pi.
11. Add the camera and depth calculation.
12. Add fixed voice commands.
13. Install and test the hardware emergency stop.
14. Test the complete autonomous sequence.
15. Add battery operation only if enough time remains.

The Motion 2350 Pro and PCA9685 are not part of the final architecture. They may be kept only as backup hardware if direct STM32 PWM cannot be completed.
