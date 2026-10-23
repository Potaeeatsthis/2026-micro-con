# Servo calibration console

Bench sketch for six **positional** servos: three MG996R and three SG92R. It uses
the Arduino Serial Monitor on **pin 9 at 115200 baud** and connects to **one
physical servo at a time**. The fixed unit labels are `MG996R-1`, `MG996R-2`,
`MG996R-3`, `SG92R-1`, `SG92R-2`, and `SG92R-3`. They identify physical units
only; they do **not** assign a unit to an arm joint.

This sketch does **not**:

- attach or move anything at startup;
- sweep automatically;
- measure the physical position (there is no position feedback);
- decide that any pulse is physically safe for a servo;
- support continuous-rotation MG996R servos.

All records are in RAM. **Reset or power loss loses all six records.** Use
`table` to print CSV and save it before resetting.

## Hardware

- Signal wire to **pin 9**. Connect servo ground to Arduino ground.
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

Open `servo-calibration.ino` in Arduino IDE. Set Serial Monitor to **115200 baud**
and **Newline** (or Both NL & CR). This is an Arduino bench sketch, not STM32
firmware; the verified board target is Arduino Uno.

| Command | Meaning |
|---|---|
| `select 1..6` | Detach the signal, select a unit, and reset the test window to 1400..1600 when switching units. Does not command movement. |
| `off` | Detach the signal. This is not a power cut and does not hold or guarantee the arm position. |
| `arm <pulse_us>` | Attach pin 9 and command the pulse. Warns because this can jump. |
| `pulse <pulse_us>` | Move while armed. Maximum change is 20 us per command. |
| `record <measured_deg>` | Store the measured angle and current pulse for the selected unit. Up to 9 points. |
| `reference` | Store the current pulse as this unit's reference pulse. |
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
Do not paste the session as an automatic sequence. Wait for each movement to
settle and measure the angle before `record`. Enter `limits` only after unloaded
testing, and `installed` / `zero` only after installation and checking clearance.
Save `table` before resetting or powering off the controller. Disconnect servo
power before swapping physical units; keep the controller powered if retaining
RAM records, and select the matching unit before enabling the next servo.

```text
select 1
arm 1500
pulse 1520
record 95.0
reference
off
limits 1460 1580
installed 1480 1560
arm 1500
zero
table
```

In this example `MG996R-1` ends with a reference at 1520 us, unloaded limits
1460..1580 us, installed limits 1480..1560 us, and a joint zero at 1500 us.
All of those values are examples only.

The CSV output has a summary row per servo:

```text
servo,label,reference_us,joint_zero_us,unloaded_min_us,unloaded_max_us,installed_min_us,installed_max_us,points
1,MG996R-1,1520,1500,1460,1580,1480,1560,1
...
```

and one row per recorded point:

```text
servo,label,measured_deg,pulse_us
1,MG996R-1,95.00,1520
...
```

Copy or save this output before resetting the Arduino.

## Host test

The host test compiles this exact `.ino` file with small Arduino/Servo stubs and
checks startup, strict parsing and invalid-byte rejection, recording/printing,
per-servo isolation, range enforcement, the explicit 500/2500 us attach range,
window reset when switching units, and detach behaviour. It needs `python3` and
`g++`, not Arduino.

```sh
python3 code/servo-calibration/test_host.py
```
