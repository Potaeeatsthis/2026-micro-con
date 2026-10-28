# Servo calibration console

Bench console for six **positional** servos: three MG996R and the units
`SG92R-1`, `SG92R-3`, and `SG90`. On the **Arduino Uno** it uses the `Servo`
library on **pin 9 at 115200 baud** and connects to **one physical servo at a
time**. The same console source is also built for the **STM32 NUCLEO-F767ZI**
with hardware-timer PWM on PC6, PC7, and PD12-PD15; see
[`../stm32-servos/README.md`](../stm32-servos/README.md). The fixed unit labels
are `MG996R-1`, `MG996R-2`, `MG996R-3`, `SG92R-1`, `SG92R-3`, and `SG90`. They
identify physical units only; they do **not** by themselves define an arm joint.
The current confirmed unit-to-joint mapping is in
[`ProjectDevGuide/docs/servo-calibration.md`](../../ProjectDevGuide/docs/servo-calibration.md).
`SG92R-2` is retired and is not an active bench unit.

This sketch does **not**:

- attach or move anything at startup;
- sweep automatically;
- measure the physical position (there is no position feedback);
- decide that any pulse is physically safe for a servo;
- support continuous-rotation MG996R servos.

All records are in RAM. **Reset or power loss loses all six records.** Use
`table` to print CSV and save it before resetting. The canonical measured table
in `ProjectDevGuide/docs/servo-calibration.md` is documentation only; it is
**not loaded automatically** into the console RAM.

## Hardware

- **Arduino Uno:** signal wire to **pin 9**; connect servo ground to Arduino
  ground. For the STM32 NUCLEO-F767ZI, use the physical pins PC6, PC7, PD12,
  PD13, PD14, and PD15 through the SN74AHCT244N buffer described in the
  [STM32 README](../stm32-servos/README.md). Do not use Arduino pin 9 on the
  STM32.
- Use a suitable external regulated servo supply, not the Arduino 5 V pin.
- **Do not assume a blanket 6 V is acceptable for every SG92R.** Check the
  voltage rating of the exact SG92R you have and start conservatively.
- Support the servo or arm before `arm`. The first `arm` can jump to the
  requested pulse. No safe pose is assumed.
- `select` and `off` only detach the signal. They do **not** cut servo power
  and do not hold the arm; the servo can drop or hold unpredictably when the
  signal stops. Support the arm before using either command.
- Keep the arm clear while testing. Use a hardware power disconnect for an
  emergency stop; detaching the signal is not an emergency stop.

## Commands

Commands are lowercase, one full line each. Malformed lines, extra tokens,
overflowing numbers, non-finite angles, invalid control/non-ASCII bytes, and
overlong lines are rejected without moving the servo. Tabs are allowed as
separators.

Open `servo-calibration.ino` in Arduino IDE for the Arduino Uno build. Set
Serial Monitor to **115200 baud** and **Newline** (or Both NL & CR). The
verified Uno target uses the `Servo` library on pin 9. For the STM32
NUCLEO-F767ZI, do not open the `.ino` in Arduino IDE: build the PlatformIO
project in [`../stm32-servos`](../stm32-servos/README.md), which reuses this
console with the `HardwareTimer` backend.

| Command | Meaning |
|---|---|
| `select 1..6` | Detach the signal, select a unit, and reset the test window to 1400..1600 when switching units. Does not command movement. |
| `off` | Detach the signal. This is not a power cut and does not hold or guarantee the arm position. |
| `arm <pulse_us>` | Attach the selected unit's output (pin 9 on Uno; PC6/PC7/PD12-PD15 on STM32) and command the pulse. Warns because this can jump. |
| `pulse <pulse_us>` | Move while armed. Maximum change is 10 us per command; smaller steps are allowed. |
| `record <measured_deg>` | Store the measured angle and current pulse for the selected unit. Up to 9 points. |
| `reference` | Store the current pulse as this unit's reference pulse. Use the pulse at measured 90 deg. |
| `zero` | Store the current pulse as the installed joint-zero pulse. Installed limits must already exist. |
| `limits <min> <max>` | Store the unloaded safe pulse range. Detach first. |
| `installed <min> <max>` | Store the narrower installed safe range. Detach first. Must be inside `limits`. |
| `window <min> <max>` | Change the conservative test guard. Detach first. Must be inside 500..2500. |
| `table` | Print the summary and point tables as CSV. |
| `help` | Print the command list on the Serial Monitor. |

The default test `window` is **1400..1600 us**. It is only a guard to reduce the
chance of an accidental large command while testing; it is **not** a claim that
the servo is safe over that range. The 500..2500 us envelope is only the hard
command sanity limit.

Selecting a **different** unit resets the window to 1400..1600 us, so a window
widened for one unit is not silently reused on another. Selecting the same unit
again keeps the window. Records for other units are retained.

`limits` is the range you mark after testing a unit unloaded. `installed` is the
narrower range after fitting it to the arm. Every `arm` and `pulse` must be
inside the intersection of the test window, unloaded limits, and installed
limits that have been set. Installed limits must be a subset of unloaded
limits.

The reference pulse is the pulse where the **measured output angle is 90 deg**,
not the arithmetic midpoint of the 0/180 deg pulses. Save it with `reference`
while the unit is armed and positioned at that measured 90 deg point.

Range changes are only accepted while the signal is detached. Unloaded
`limits` must contain the saved reference and any installed range. Installed
limits must be inside the unloaded range and contain the saved joint zero. The
narrower installed range **may exclude the unloaded reference**.
Calibration points are raw measurements; changing a range does not delete or
edit them, and the sketch warns if a new `limits` range leaves any stored points
outside it. The `window` is only a guard on future commands; changing it does
not rewrite any stored value.

## Example session

The numbers below are **made-up examples, not measurements** from real servos.
The measured unloaded values are in
[`ProjectDevGuide/docs/servo-calibration.md`](../../ProjectDevGuide/docs/servo-calibration.md).
Do not paste the session as an automatic sequence. Wait for each movement to
settle and measure the angle before `record`. Enter `limits` only after unloaded
testing, and `installed` / `zero` only after installation and checking clearance.
Save `table` before resetting or powering off the controller. Disconnect servo
power before swapping physical units; keep the controller powered if retaining
RAM records, and select the matching unit before enabling the next servo.

Unloaded example. Every `pulse` change is at most 10 us, and the reference is
saved at the pulse where the measured angle is 90 deg:

```text
select 1
arm 1500
record 90.0
reference
pulse 1510
record 93.0
pulse 1505
record 92.0
off
limits 1400 1600
table
```

In this example `MG996R-1` ends with a reference at 1500 us and unloaded limits
1400..1600 us. All values are examples only.

After installation, the narrower installed range and zero are recorded
separately. If the controller reset, the RAM records are gone; restore only the
saved unloaded `limits` while detached. Keep historical measurement points in
the saved CSV; do not replay unloaded endpoints on the assembled arm to restore
records.

```text
select 1
limits 1400 1600
window 1480 1520
arm 1500
pulse 1510
pulse 1505
off
installed 1480 1520
arm 1500
zero
table
```

Again, all numbers are examples. `installed` must be inside the unloaded range,
and `zero` must be inside the installed range. Do not copy these installed
values to a real joint; measure them on the assembled arm.

Without a reset between the examples, the CSV output has a summary row per servo:

```text
servo,label,reference_us,joint_zero_us,unloaded_min_us,unloaded_max_us,installed_min_us,installed_max_us,points
1,MG996R-1,1500,1500,1400,1600,1480,1520,3
...
```

and one row per recorded point:

```text
servo,label,measured_deg,pulse_us
1,MG996R-1,90.00,1500
1,MG996R-1,93.00,1510
1,MG996R-1,92.00,1505
...
```

Copy or save this output before resetting or powering off the board.

## Arrow-key front end (Windows)

[`servo-cal-keys.ps1`](servo-cal-keys.ps1) drives this console from the keyboard: arrow keys send `pulse`
commands in 10 us steps (the console's own limit), Enter sends `record` / `reference`, and the five steps
(90 deg, 0 deg, 180 deg, low and high limit) end with `limits` and `table`. It asks for the servo's own name
(e.g. `SG92R-4`), so a new unit can use any slot (`-Slot`, default 4). Results go to `results/` (not in git).

```text
powershell -ExecutionPolicy Bypass -File servo-cal-keys.ps1 -Port COM7
```

Tested on an Arduino Nano (same ATmega328 as the Uno; upload with `--fqbn arduino:avr:nano`).

## Host test

The host test compiles this exact `.ino` file with the real Uno backend header
and small Arduino/Servo stubs. It checks startup, strict parsing and
invalid-byte rejection, recording/printing, per-servo isolation, range
enforcement, the explicit 500/2500 us attach range, window reset when switching
units, and detach behaviour. It needs `python3` and `g++`, not Arduino.

```sh
python3 code/servo-calibration/test_host.py
```
