# Autonomous Robotic Arm — Final Build Specification

**Revision 1.1 · 8 September 2026**
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
| **STM32 NUCLEO-F767ZI** | Six hardware-PWM servo signals, potentiometer ADC, limit switch interrupts, safety logic, trajectory generation, UART command parser |

The Pi thinks slowly and rarely. The STM32 moves fast and constantly. That split is why the arm stays smooth while the Pi is busy with the camera.

### Success criteria

- Speech-command accuracy ≥ 90% in a quiet room
- Colour-object detection ≥ 90%
- Position error < 2 cm
- Grasp success ≥ 80% over 20 tests
- 10 consecutive operations without reset

---

## 2. Budget

| | |
|---|---|
| Parts | **฿3,988** |
| Shipping | ฿250 |
| Contingency | ฿300 |
| **TOTAL** | **฿4,538** |
| Remaining from ฿5,000 | ฿462 |

Already owned at ฿0: STM32 NUCLEO-F767ZI, Raspberry Pi 4 and its adapter, breadboard, RGB-D camera (borrowed).

**Two lines that may free up ฿365** if confirmed: jumper wires (may already be owned) and the B3 balance charger (may be borrowable).

---

## 3. Power architecture

### The governing rule

**Power and signal reach the servos by two separate paths.**

| Path | Carries | Route |
|---|---|---|
| Power | up to 9.75 A | source → buck → terminal block → servo **RED + BLACK** |
| Signal | microamps | Pi → STM32 hardware PWM → SN74AHCT244N → servo **ORANGE** |

The red and black servo wires connect to the 6.0 V distribution block. The orange signal wire connects to one SN74AHCT244N output.

> **Do not send servo power through the STM32 or SN74AHCT244N.** The buffer carries only the six PWM signals. All devices must share one common signal ground.

### Two rails, never shared

| Rail | Voltage | Load | Source |
|---|---|---|---|
| Servo | **6.0 V** | 2.4 A moving, 9.75 A worst-case stall | 20 A buck |
| Logic | **5.1 V** | 2.7 A peak (Pi + camera + STM32) | 5 A buck |

Sharing one rail causes the Pi to brown out on every servo movement — random reboots that look exactly like software crashes.

### Measured load

| Scenario | Servo rail | Logic rail | From battery | Runtime |
|---|---|---|---|---|
| Holding still | 0.99 A / 5.9 W | 1.12 A | 1.21 A | ~99 min |
| All six moving | 2.40 A / 14.4 W | 2.17 A | 2.64 A | ~46 min |
| Worst-case stall | 9.75 A / 58.5 W | 2.72 A | 7.49 A | ~16 min |

At 2.64 A the battery runs at 1.2C against a 35C rating — 3% of capability.

**Tightest constraint is the Pi's USB budget**, not the battery. The Pi supplies 1.2 A total across all ports; camera 0.7 A + Nucleo 0.25 A = 79%. If undervoltage warnings appear, power the Nucleo from its own adapter and keep the ground tied.

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
        ├──▶ Buck 20 A → 6.0 V → terminal block → servos
        └──▶ Buck 5 A  → 5.1 V → Raspberry Pi → STM32 (USB)
```

> **Do not swap the bucks.** 6.0 V into a Raspberry Pi damages it; its maximum is about 5.25 V.

**Only the source changes between phases.** Everything from the terminal block onward is identical, so motion tuned on the bench behaves the same on battery.

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
   ├── 6 × potentiometer    outer legs on 3.3 V (NOT 5 V), wipers to ADC
   ├── 4 × limit switch     PE2/PE3/PE4/PE5 — different pin NUMBERS or EXTI collides
   └── 6 × hardware PWM     TIM3 CH1–CH4 + TIM4 CH1–CH2, final pins checked in CubeMX
        ▼
SN74AHCT244N DIP-20 buffer  VCC = 5 V, GND shared, /1OE and /2OE tied LOW
   └── 100 nF ceramic capacitor directly between VCC and GND
        ▼
6 servos — ORANGE signal wire only
   PWM1 base · PWM2 shoulder · PWM3 elbow · PWM4 wrist pitch · PWM5 wrist rot · PWM6 gripper
```

### Three buses, three purposes

| Bus | Between | Wires |
|---|---|---|
| **UART** | Pi ↔ STM32 | via ST-LINK USB, no jumpers needed |
| **PWM** | STM32 → SN74AHCT244N → servos | Six independent timer-output signals |
| **I²S** | mic → Pi | SCK, WS, SD |

Similar names, unrelated protocols, different pins. No conflict.

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
| Back third | Pi (85 × 56) and Nucleo (133 × 70) side by side |
| Middle strip | Terminal blocks and bucks, close enough that 30 cm servo leads reach |
| Corner | Battery, easy to grab and unplug |

Nothing mounts inside the printed parts — the arm carries only servos.

### The shoulder spring

Links the rotating base to the first arm segment and takes weight off the shoulder servo, which is the marginal joint in every calculation (MG996R gives ~11 kg-cm at 6 V; a fully extended arm asks close to that).

**Spec:** ~38 mm free length, 9 mm OD, 1.0 mm wire. Mount so it sits stretched to 45–50 mm at rest, otherwise it does nothing until the arm is already lowered.

Fit this **before** considering the ฿500 DS3218 upgrade.

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
| **5** | Pi ↔ STM32 UART | `MOVE` command returns `DONE` |
| **6** | Forward and inverse kinematics | Arm reaches a typed XYZ |
| **7** | Camera detection, no motion | Cube located in image reliably |
| **8** | Camera-to-robot calibration | Reported position within 2 cm |
| **9** | Autonomous pick-and-place | 8 of 10 grasps succeed |
| **10** | Speech integration | Fixed command list recognised |
| **11** | Battery conversion, full run-through | Runs untethered |

**Speech is last on purpose.** Integrated early, every failure has three possible causes.

**Stage 8 is the schedule risk.** It looks like one step and typically consumes a week.

---

## 8. Known risks

| Risk | Reality | Mitigation |
|---|---|---|
| **Servo repeatability** | MG996R deadband is 1–2°, which is 5–10 mm at 30 cm reach. Three joints stacked exceeds the 2 cm spec before any code is written | Solve mechanically: 4 cm cubes, rubber gripper pads, always approach from directly above |
| **Shoulder torque** | ~11 kg-cm available, fully extended arm demands close to that | Fit the spring; keep links short; DS3218 only if still drooping |
| **Print accuracy** | 0.3 mm error in servo pockets means play you cannot calibrate out | Test part first |
| **Camera calibration** | Usually the week-eater | Start it early, do not leave to the end |
| **Pi USB budget** | Camera + Nucleo = 79% of 1.2 A | Separate adapter for the Nucleo if warnings appear |
| **Battery runtime** | ~46 min at normal draw | Bench-test on the wall adapter, save the battery for the demo |

---

## 9. Safety

Full procedures in `safety-procedures.html`. Non-negotiables:

**LiPo battery** — 2200 mAh at 35C delivers about 77 A into a short, with no off switch. Never let the output wires touch. Store in a LiPo bag or metal tin. Balance charger only, 1C (2.2 A), never unattended. Retire any pack that swells. The white JST-XH plug goes to the charger and nothing else.

**Capacitor polarity** — the stripe marks negative. Reversed electrolytics vent and burst.

**Emergency stop** — deferred to the deployment batch (฿125 for button + relay). **Until fitted, your only stop is unplugging the adapter or the XT60.** Decide before each session which you will grab and keep it clear. Fit the e-stop before any autonomous operation.

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
- [ ] Potentiometer outer legs on **3.3 V**, not 5 V
- [ ] No stray copper strands at any screw terminal
- [ ] Heat shrink over the XT60 solder split
- [ ] Arm bolted to the base board
- [ ] **One servo only** for first power-on
- [ ] Remaining five added one at a time, watching the rail hold

---

## 11. Open items

1. **Confirm the camera loan** covers the whole semester, in writing. The entire budget depends on it.
2. **Confirm jumper wires** are already owned — ฿120.
3. **Confirm the B3 charger** can be borrowed — ฿245.
4. **Verify the six TIM3/TIM4 PWM pins and `PE2`–`PE5`** in STM32CubeMX before wiring, so PWM, UART, SPI, encoder and EXTI assignments do not conflict.
5. **Buy the e-stop and limit switches** in the deployment batch (฿185 total).

---

## 12. Companion documents

| File | Purpose |
|---|---|
| `Approved-Robotic-Arm-Budget.xlsx` | BOM, vendor options, contingencies |
| `end-to-end-circuit.md` | Wire-by-wire connection reference |
| `full-connection-map.html` | Interactive pin-level diagram |
| `power-budget.html` | Load analysis with scenario switching |
| `safety-procedures.html` | Full safety document with sign-off page |
| `battery-sag-proof.html` | Why AAA cells cannot run servos |

---

*Verify all STM32 pin names against the NUCLEO-F767ZI silkscreen before wiring. Raspberry Pi pin numbers are physical header positions, not BCM.*
