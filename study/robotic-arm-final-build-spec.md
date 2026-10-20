# Autonomous Robotic Arm — Final Build Specification

**Revision 1.2 · 20 October 2026**
Microcontroller Project · Third Year Computer Engineering

---

## 1. What we are building

A 6-DOF 3D-printed robotic arm that takes a spoken command, finds a coloured cube with an RGB-D camera, picks it up, and places it in a target box.

**Example:** *"Pick up the red block and place it in the blue box."*

Mechanical design is based on the [OmArTronics DIY 6-DOF robotic arm](https://omartronics.com/diy-6-dof-robotic-arm-build/), with the Arduino replaced by our STM32 and the imperial screws replaced by metric.

### Division of labour

| Board | Responsibility |
|---|---|
| **Raspberry Pi 4** | Speech-to-text, object detection, depth, inverse kinematics, task state machine |
| **STM32 NUCLEO-F767ZI** | Four control modes, KY-023 joystick ADC, KY-040 encoder, ILI9341 SPI LCD, six hardware-PWM servo signals, limit switch interrupts, safety logic, trajectory generation and UART command parser |

The Pi handles camera, speech and inverse kinematics. The STM32 keeps movement smooth and checks safety even while the Pi is busy.

### Four operating modes

Short MODE presses cycle **Auto/Voice → Cartesian → Joint Group → Single-Joint Service → Auto/Voice**.

| Mode | What it does | Pi required? |
|---|---|---|
| Auto/Voice | A fixed spoken command starts camera-guided pick-and-place | Yes |
| Cartesian | KY-023 changes target X/Y; KY-040 changes Z and its push button opens or closes the gripper | Yes, for inverse kinematics |
| Joint Group | KY-023 moves two joints; KY-040 selects one of three joint pairs and its push button changes Fine/Fast speed | No |
| Single-Joint Service | KY-040 selects J1–J6; KY-023 moves the selected joint for testing | No |

Manual movement requires a separate Hold-to-Enable button. Releasing it stops the manual trajectory and holds the current command. Changing modes must stop the old movement, wait for Hold-to-Enable to be released and the joystick to be centred, then start the new mode from the current joint targets. All four modes use the same joint-limit and trajectory checks.

### Success criteria

- Speech-command accuracy ≥ 90% in a quiet room
- Colour-object detection ≥ 90%
- Position error < 2 cm
- Grasp success ≥ 80% over 20 tests
- 10 consecutive operations without reset

---

## 2. Equipment

| Equipment | Quantity | Purpose |
|---|---:|---|
| STM32 NUCLEO-F767ZI, Raspberry Pi 4 and its adapter | 1 each | Main controller and high-level computer |
| RGB-D camera and INMP441 microphone | 1 each | Vision and voice input |
| KY-023 joystick and KY-040 rotary encoder | 1 each | Manual movement and selection |
| MG996R and SG90 servos | 3 of each fitted | Six arm joints |
| ILI9341 2.4-inch SPI TFT LCD | 1 | Mode, target and safety display |
| 12 mm normally-open momentary button | 2 | MODE and Hold-to-Enable |
| 22 mm latching emergency stop, 1NO + 1NC | 1 | Hardware stop and status input |
| Relay or suitable DC switch, fuse, driver and flyback diode | 1 set | Disconnect servo power when e-stop is pressed |
| Limit switches | 4 | Detect travel limits |
| SN74AHCT244N DIP-20 and 100 nF capacitor | 1 each | Buffer and decouple six PWM signals |
| 2200 µF, 16 V capacitors | 3 | Support the 6.0 V servo rail during current changes |
| PETG, 6806ZZ bearing, metric fasteners and base board | As required | Printed arm and support |
| Breadboard, 12 V adapter, 20 A and 5 A buck converters, terminal blocks and wiring | 1 set | Bench testing and two power rails |
| 3S LiPo battery, XT60 connector and balance charger | 1 each | Untethered operation |

---

## 3. Power architecture

### The governing rule

**Power and signal reach the servos by two separate paths.**

| Path | Carries | Route |
|---|---|---|
| Power | up to 9.75 A | source → fuse → e-stop relay → buck → terminal block → servo **RED + BLACK** |
| Signal | microamps | Pi → STM32 hardware PWM → SN74AHCT244N → servo **ORANGE** |

The red and black servo wires connect to the 6.0 V distribution block. The orange signal wire connects to one SN74AHCT244N output.

> **Do not send servo power through the STM32 or SN74AHCT244N.** The buffer carries only the six PWM signals. All devices must share one common signal ground.

### Two rails, never shared

| Rail | Voltage | Load | Source |
|---|---|---|---|
| Servo | **6.0 V** | 2.4 A moving, 9.75 A worst-case stall | 20 A buck |
| Logic | **5.1 V** | Earlier 2.7 A peak estimate for Pi + camera + STM32; recheck with LCD and controls | 5 A buck |

Sharing one rail causes the Pi to brown out on every servo movement — random reboots that look exactly like software crashes.

### Earlier load estimate

These figures came from the previous parts list. They are planning estimates, **not measurements of the updated build**. Recheck logic-rail current and battery runtime after adding the LCD and controls.

| Scenario | Servo rail | Logic rail | From battery | Runtime |
|---|---|---|---|---|
| Holding still | 0.99 A / 5.9 W | 1.12 A | 1.21 A | ~99 min |
| All six moving | 2.40 A / 14.4 W | 2.17 A | 2.64 A | ~46 min |
| Worst-case stall | 9.75 A / 58.5 W | 2.72 A | 7.49 A | ~16 min |

The earlier 2.64 A moving estimate is about 1.2C for a 2200 mAh battery, below its 35C rating. The updated runtime still needs a load test.

Check the Pi's USB current limit after adding the LCD. The earlier estimate put the camera at 0.7 A and Nucleo at 0.25 A, or 79% of the Pi's 1.2 A total USB-port limit. If undervoltage warnings appear, power the Nucleo from its own adapter and keep the ground tied.

---

## 4. Two operating phases

### Experiment phase (≈10 weeks)

```
12 V 10 A sealed adapter
        │  barrel jack
        ▼
Barrel-to-screw-terminal adapter
        │  14 AWG
        ▼
Fuse + e-stop relay  ← install before autonomous motion
        │
        ▼
Buck 20 A → set to 6.0 V   ← measure BEFORE connecting servos
        │  14 AWG
        ▼
Terminal block (PLUS bus / MINUS bus)
        ├──▶ 6 × servo RED
        ├──▶ 6 × servo BLACK
        ├──▶ 3 × capacitor 2200 µF
        └──▶ 1 × wire to Pi / STM32 ground

Raspberry Pi runs from its own USB-C adapter.
```

Mains stays sealed inside the adapter brick. No exposed 220 V on the bench.

### Deployment phase (final 1–2 weeks)

```
LiPo 3S 11.1 V 2200 mAh  (XT60 female on pack)
        ▼
XT60 male + 100 mm pigtail
        ▼
SOLDER SPLIT (heat shrink over the joint)
        ├──▶ Fuse → e-stop relay → Buck 20 A → 6.0 V → terminal block → servos
        └──▶ Buck 5 A  → 5.1 V → Raspberry Pi → STM32 (USB)
```

> **Do not swap the bucks.** 6.0 V into a Raspberry Pi damages it; its maximum is about 5.25 V.

The 6.0 V servo distribution stays the same between phases. On the bench the Pi uses its own adapter; on battery it uses the separate 5.1 V buck. Check both rails again after switching sources.

---

## 5. Signal chain

```
Raspberry Pi 4
   ├── INMP441 microphone   I²S on GPIO18 / 19 / 20, VDD = 3.3 V ONLY
   ├── RGB-D camera         USB 3.0 (blue port)
   └── USB-A → micro-USB into Nucleo CN1
              (power + programming + UART on one cable)
        ▼
STM32 NUCLEO-F767ZI
   ├── KY-023 joystick      X/Y to ADC; power from 3.3 V, not 5 V
   ├── KY-040 encoder       CLK/DT to timer encoder inputs; SW to digital input
   ├── ILI9341 2.4-inch LCD SPI plus control pins; check module voltage ratings
   ├── MODE button          12 mm momentary, normally open
   ├── Hold-to-Enable       separate 12 mm momentary, normally open
   ├── E-stop status        22 mm latching button NO contact to input
   ├── 4 × limit switch     PE2/PE3/PE4/PE5 proposed; verify in CubeMX
   └── 6 × hardware PWM     TIM3 CH1–CH4 + TIM4 CH1–CH2, final pins checked in CubeMX
        ▼
SN74AHCT244N DIP-20 buffer  VCC = 5 V, GND shared, /1OE and /2OE tied LOW
   └── 100 nF ceramic capacitor directly between VCC and GND
        ▼
6 servos — ORANGE signal wire only
   PWM1 base · PWM2 shoulder · PWM3 elbow · PWM4 wrist pitch · PWM5 wrist rot · PWM6 gripper
```

### Signal links

| Bus | Between | Wires |
|---|---|---|
| **UART** | Pi ↔ STM32 | via ST-LINK USB, no jumpers needed |
| **PWM** | STM32 → SN74AHCT244N → servos | Six independent timer-output signals |
| **I²S** | mic → Pi | SCK, WS, SD |
| **SPI** | STM32 → ILI9341 LCD | Clock, data and display-control signals |

The KY-023 adds two analogue ADC signals. The KY-040 adds two timer-encoder signals and one push-button input. Verify the complete pin map in STM32CubeMX before wiring; the timer, SPI, UART and interrupt pins must not overlap.

Power the KY-040 from 3.3 V so its digital outputs match STM32 inputs. Filter its mechanical CLK/DT signals and debounce its push button and the MODE button. The KY-040 is a user input; it does **not** measure a joint's actual position. Use the ILI9341 module's specified supply and logic voltages, which depend on the exact breakout board.

### UART detail

`USART3` on `PD8`/`PD9` is routed internally to the ST-LINK virtual COM port. One USB cable from the Pi carries power, flashing and serial. Appears on the Pi as `/dev/ttyACM0` at **115200 8N1**. No CP2102 module required.

**Protocol:**
```
Pi   → STM32:  MOVE 90 65 110 45 90 30 2500
STM32 → Pi:    ACK
STM32 → Pi:    DONE
```
Six joint angles plus duration in milliseconds.

---

## 6. Mechanical

### Printing

- **PETG, not PLA.** PLA slowly bends under sustained load and our joints hold torque for long periods
- Layer height **0.2 mm**, infill **40%+** on base and arm links
- One 1 kg spool covers the whole arm
- **Print one test part first** and check the servo pocket and screw holes before committing

### Fasteners — measured from our STL files

| Hole | Count | Fastener |
|---|---|---|
| 1.5 mm | 6 | M1.6 thread-forming |
| 2.0 mm | 12 | M2 thread-forming |
| 2.5 mm | 12 | M2 clearance + nut |
| 3.0 mm | 2 | M3 thread-forming |
| 3.3 mm | 5 | M3 clearance + nut |
| 3.5 mm | 7 | M3 loose / gripper pivots |
| **42.0 mm bore** | 1 | 6806ZZ bearing seat — confirmed |

**Thread-forming holes** are smaller than the screw; the screw cuts its own thread, no nut. **Clearance holes** are larger; the screw passes through and needs a nyloc nut on the far side.

The model is already dimensioned for metric — the original's imperial No.2-32 warning does not apply here.

**M3 hardware is required** and the assorted DIN912 box covers it.

**Gripper pivots (3.5 mm)** rotate under load. Use a shoulder bolt or unthreaded rod, or do not fully tighten a normal bolt — threads chew through PETG when a joint moves repeatedly.

**Tip:** run each screw into a thread-forming hole once while the part is still warm from the printer, then back it out. Forcing a cold part cracks it.

### Base board

**50 × 40 cm minimum.** The arm needs a clear 30 cm reach radius; the electronics need roughly 25 × 20 cm.

| Zone | Contents |
|---|---|
| Front third | Arm bolted down, nothing else in reach |
| Back third | Pi (85 × 56), Nucleo (133 × 70) and the LCD/control panel, outside the arm's reach |
| Middle strip | Terminal blocks and bucks, close enough that 30 cm servo leads reach |
| Corner | Battery, easy to grab and unplug |

Nothing mounts inside the printed parts — the arm carries only servos.

### The shoulder spring

Links the rotating base to the first arm segment and takes weight off the shoulder servo, which is the marginal joint in every calculation (MG996R gives ~11 kg-cm at 6 V; a fully extended arm asks close to that).

**Spec:** ~38 mm free length, 9 mm OD, 1.0 mm wire. Mount so it sits stretched to 45–50 mm at rest, otherwise it does nothing until the arm is already lowered.

Fit this **before** considering a DS3218 shoulder-servo upgrade.

### Shoulder-servo replacement fit

| Servo | Body size | Fit conclusion |
|---|---|---|
| MG996R | 40.7 × 19.7 × 42.9 mm | Original standard-size shoulder servo |
| DS3218 | 40 × 20 × 40.5 mm | Close to MG996R size, but test the pocket, mounting holes and horn before final assembly |
| SG90 | 23 × 12.2 × 29 mm | Micro servo; not the same size and cannot directly replace either standard-size servo |

The DS3218 is a practical shoulder upgrade, but it is not guaranteed to be a perfect drop-in replacement. Print the shoulder servo pocket first and test-fit the exact servo before printing the full arm.

---

## 7. Build order

| Stage | Work | Gate before moving on |
|---|---|---|
| **1** | Print one test part | Servo pocket and screw holes fit |
| **2** | Print and assemble the arm | Joints move freely by hand |
| **3** | Wire the 6.0 V rail, one servo | Rail holds 6.0 V under load |
| **4** | STM32 → SN74AHCT244N → all six servos | Each joint sweeps its safe calibrated range |
| **5** | Read/filter KY-023 and KY-040; implement Single-Joint Service with Hold-to-Enable | One selected joint moves only while enabled; release stops the command |
| **6** | Implement Joint Group | Each joystick axis moves only its selected joint, inside safe limits |
| **7** | Add MODE cycle and ILI9341 LCD | Four modes cycle correctly; switches start from current targets and LCD shows status |
| **8** | Pi ↔ STM32 UART | `MOVE` command returns `DONE` |
| **9** | Forward/inverse kinematics and Cartesian mode | Arm reaches a typed or joystick-selected XYZ within safe limits |
| **10** | Camera detection, no motion | Cube located in image reliably |
| **11** | Camera-to-robot calibration | Reported position within 2 cm |
| **12** | Install/test limit switches and hardware emergency stop | E-stop removes servo power; STM32 reports status; switches stop unsafe travel |
| **13** | Autonomous pick-and-place | 8 of 10 grasps succeed |
| **14** | Speech integration | Fixed command list recognised |
| **15** | Battery conversion, full run-through | Runs untethered with both rails checked |

**Speech is last on purpose.** Integrated early, every failure has three possible causes.

**Camera-to-robot calibration at stage 11 is the schedule risk.** It looks like one step and typically consumes a week.

---

## 8. Known risks

| Risk | Reality | Mitigation |
|---|---|---|
| **Servo repeatability** | MG996R deadband is 1–2°, which is 5–10 mm at 30 cm reach. Three joints stacked exceeds the 2 cm spec before any code is written | Solve mechanically: 4 cm cubes, rubber gripper pads, always approach from directly above |
| **Shoulder torque** | ~11 kg-cm available, fully extended arm demands close to that | Fit the spring; keep links short; DS3218 only if still drooping |
| **Print accuracy** | 0.3 mm error in servo pockets means play you cannot calibrate out | Test part first |
| **Camera calibration** | Usually the week-eater | Start it early, do not leave to the end |
| **Pi USB current limit** | Earlier camera + Nucleo estimate used 79% of 1.2 A; the LCD may add load | Measure the updated setup; use a separate Nucleo adapter if undervoltage appears |
| **Battery runtime** | Earlier estimate was ~46 min at normal draw; updated build is unmeasured | Bench-test on the wall adapter, then measure battery runtime |

---

## 9. Safety

Non-negotiable safety rules:

**LiPo battery** — 2200 mAh at 35C delivers about 77 A into a short, with no off switch. Never let the output wires touch. Store in a LiPo bag or metal tin. Balance charger only, 1C (2.2 A), never unattended. Retire any pack that swells. The white JST-XH plug goes to the charger and nothing else.

**Capacitor polarity** — the stripe marks negative. Reversed electrolytics vent and burst.

**Emergency stop** — fit the 22 mm latching 1NO + 1NC button, fuse, relay or suitable DC switch, driver and flyback diode **before autonomous motion**. The NC contact controls the low-current relay circuit; the relay contacts disconnect servo power. The NO contact reports status to the STM32. Do not run servo current through the button. During early bench tests before it is fitted, keep the adapter plug within reach so power can be disconnected immediately.

**The arm** — hands physically outside the reach envelope whenever the servo rail is live. First run of any new motion: reduced speed, reduced range, one hand on the disconnect.

**Common ground is mandatory** — both bucks, Pi, STM32, SN74AHCT244N and all servos connect to one common ground point. A PWM signal is a voltage relative to ground; without a shared reference the servos move unpredictably, and it looks exactly like a software bug.

---

## 10. Pre-power checklist

- [ ] Buck output measured at **6.0 V** with no servo connected
- [ ] All three capacitors checked against the stripe
- [ ] SN74AHCT244N pin 20 on 5 V and pin 10 on common ground
- [ ] SN74AHCT244N `/1OE` and `/2OE` held LOW
- [ ] 100 nF capacitor fitted directly between buffer VCC and GND
- [ ] No servo red power wire connected to the STM32 or logic buffer
- [ ] Ground continuity: terminal block minus → Pi GND → STM32 GND
- [ ] INMP441 `VDD` measured at **3.3 V**, not 5 V
- [ ] KY-023 and KY-040 powered from **3.3 V**, not 5 V
- [ ] Exact ILI9341 breakout supply and signal-voltage ratings checked before connection
- [ ] No stray copper strands at any screw terminal
- [ ] Heat shrink over the XT60 solder split
- [ ] Arm bolted to the base board
- [ ] **One servo only** for first power-on
- [ ] Remaining five added one at a time, watching the rail hold
- [ ] Before autonomous motion: fuse, limit switches and latching e-stop tested; pressing e-stop removes servo power

---

## 11. Open items

1. **Confirm the camera loan** covers the whole semester.
2. **Confirm all equipment is available:** ILI9341, two 12 mm buttons, e-stop circuit, four limit switches, jumper wires and LiPo balance charger.
3. **Verify the complete STM32CubeMX pin map:** six TIM3/TIM4 PWM outputs, two ADC inputs, KY-040 timer inputs and switch, ILI9341 SPI/control pins, UART and EXTI inputs. Check the proposed `PE2`–`PE5` limit-switch pins.
4. **Confirm the exact ILI9341 module's voltage ratings** and test the emergency-stop circuit before autonomous motion.

---

## 12. Companion documents

| File | Purpose |
|---|---|
| [robotic-arm-guide.md](robotic-arm-guide.md) | Four-mode controls, signal buffer, LCD and safety behaviour |
| [stm32_servo_pwm_guide.pdf](stm32_servo_pwm_guide.pdf) | STM32 servo PWM reference |

---

*Verify all STM32 pin names against the NUCLEO-F767ZI silkscreen before wiring. Raspberry Pi `GPIO18`, `GPIO19` and `GPIO20` above are BCM numbers; their physical header pins are 12, 35 and 38.*
