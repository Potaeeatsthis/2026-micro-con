# STM32 servo calibration console (NUCLEO-F767ZI)

PlatformIO build of the **same six-unit calibration console** used on the
Arduino Uno bench. It targets the STM32 NUCLEO-F767ZI with the Arduino core and
uses STM32 hardware timers for the servo PWM.

The console is not duplicated. This project compiles
[`../servo-calibration/servo-calibration.ino`](../servo-calibration/servo-calibration.ino);
the only platform code is the small output backend in
[`include/servo_backend_stm32.h`](include/servo_backend_stm32.h). Commands,
RAM-only records, CSV tables, guards, and the no-auto-sweep behaviour are the
same as the Arduino version.

Read the shared command and calibration documentation in
[`../servo-calibration/README.md`](../servo-calibration/README.md). This page
covers only the STM32 build, wiring, and upload.

## Board and outputs

- Board: STM32 NUCLEO-F767ZI (`nucleo_f767zi`)
- Framework: Arduino (`framework = arduino`)
- Upload/debug: on-board ST-LINK (`upload_protocol = stlink`)
- Serial: **USART3 through the ST-LINK virtual COM port**, 115200 baud. The
  Arduino core variant selects `SERIAL_UART_INSTANCE 3` ("Connected to
  ST-Link"); this build does **not** use USB CDC for `Serial`.

| Unit ID | Bench label | MCU pin | Timer channel | Header label / connection ID |
|---|---|---|---|---|
| 1 | `MG996R-1` | PC6 | TIM3 CH1 | `PC6` |
| 2 | `MG996R-2` | PC7 | TIM3 CH2 | `PC7` |
| 3 | `MG996R-3` | PD12 | TIM4 CH1 | `PD12` |
| 4 | `SG92R-1` | PD13 | TIM4 CH2 | `PD13` |
| 5 | `SG92R-3` | PD14 | TIM4 CH3 | `PD14` |
| 6 | `SG90` | PD15 | TIM4 CH4 | `PD15` |

Use the **MCU pin names printed on the NUCLEO-144 headers** (`PC6`, `PD12`,
etc.), not Arduino-style pin numbers such as `9` or `D9`. The six unit IDs and
labels identify physical bench servos. The confirmed current mapping is
J1..J6 = units 1..6 (J5 is `SG92R-3`, J6 is `SG90`); installed zeros, limits
and home are still measured after assembly, not inferred from the labels. See
[`ProjectDevGuide/docs/servo-calibration.md`](../../ProjectDevGuide/docs/servo-calibration.md).

Both timers run at **50 Hz** (20 ms period) with a 1 us timer tick, so a compare
value is the pulse width in microseconds. Only **one output is armed at a
time**. Selecting another unit detaches and stops the previous channel. A
detach pauses the channel, stops the shared timer, writes a zero compare, and
drives the pin low. At startup all six pins are driven low **before** the
timers are configured, no channel is enabled, and the timers do not start until
a valid `arm` command. The first compare value is loaded before the first
channel start.

The hardware mapping follows the confirmed J1-J6 PWM plan in
[`../../ProjectDevGuide/docs/core-config.md`](../../ProjectDevGuide/docs/core-config.md):
J1-J6 are PC6, PC7, PD12-PD15 for TIM3/TIM4. The unit-to-joint mapping is
confirmed for the current build, but the installed safe ranges and home pose
remain uncalibrated until measured on the assembled arm.

## Install PlatformIO

### Option A: VS Code PlatformIO

1. Install the **PlatformIO IDE** extension in VS Code.
2. Open the folder `code/stm32-servos` (not the repository root) with
   **File > Open Folder**.
3. Let PlatformIO install the `ststm32` platform and ARM toolchain on the first
   build. PlatformIO Home shows the packages if you want to watch progress.
4. Use the PlatformIO status-bar buttons, or the PlatformIO sidebar:
   - **Build** (check mark)
   - **Upload** (right arrow)
   - **Monitor** (plug icon)
5. `platformio.ini` already selects `nucleo_f767zi`, `stlink`, and
   `monitor_speed = 115200`.

### Option B: command line with uv (no global PlatformIO install)

From the repository root (requires `uv`; alternatively use Option A):

```sh
uv tool run --from platformio pio --version
uv tool run --from platformio pio run -d code/stm32-servos
```

The first build downloads the STM32 platform and toolchain. `uv tool run`
caches the tool, so later commands are quicker. If you prefer a persistent CLI
entry point, `uv tool install platformio` then use `pio` in place of
`uv tool run --from platformio pio`; the commands below use the uv fallback.

Build, upload, and monitor, from the repository root:

```sh
uv tool run --from platformio pio run -d code/stm32-servos
uv tool run --from platformio pio run -d code/stm32-servos -t upload
uv tool run --from platformio pio device monitor -d code/stm32-servos -b 115200
```

`pio run -t upload` writes over the on-board ST-LINK. The monitor uses the
ST-LINK virtual COM port. To list ports:

```sh
uv tool run --from platformio pio device list
```

On Linux the port is normally `/dev/ttyACM0` (or the next ACM number); the
user must be in the `dialout` group. On Windows it is a `COMx` port; on macOS
it is normally `/dev/cu.usbmodem*`.

## USB and serial

Connect the **ST-LINK USB** port (the USB connector next to the ST-LINK
section, not the separate USB OTG connector) to the computer. That one cable
provides:

- ST-LINK SWD programming/upload;
- the USART3 virtual COM port for the 115200 console.

The board does not need a second USB cable for this bench build. Do not select
a USB CDC build option: `Serial` is the ST-LINK VCP on USART3 by default in
this PlatformIO environment, and the sketch does not enable `USBD_USE_CDC`.

## Bench wiring and safety

**One physical servo at a time.** The console records six units but commands
one output. Disconnect servo power before swapping the signal lead.

Servo signal path through the **SN74AHCT244N** buffer (3.3 V STM32 PWM to 5 V
servo logic):

| Buffer pin | Signal | Connection |
|---|---|---|
| 20 | VCC | Regulated **5.1 V logic rail** |
| 10 | GND | Common system ground |
| 1 | `/1OE` | Ground |
| 19 | `/2OE` | Ground |
| 2 -> 18 | `1A1 -> 1Y1` | PC6 -> unit 1 signal |
| 4 -> 16 | `1A2 -> 1Y2` | PC7 -> unit 2 signal |
| 6 -> 14 | `1A3 -> 1Y3` | PD12 -> unit 3 signal |
| 8 -> 12 | `1A4 -> 1Y4` | PD13 -> unit 4 signal |
| 11 -> 9 | `2A1 -> 2Y1` | PD14 -> unit 5 signal |
| 13 -> 7 | `2A2 -> 2Y2` | PD15 -> unit 6 signal |
| 15, 17 | Unused inputs | Ground |
| 5, 3 | Unused outputs | Leave unconnected |

Add a **100 nF ceramic capacitor** between buffer pin 20 and pin 10, as close
as possible. Connect the Nucleo ground, buffer ground, and servo power-supply
ground together. The buffer only passes the signal; **never** route servo
current through the buffer or through the Nucleo board.

Add a **10 kΩ pulldown to ground on each of the six used buffer inputs**
(pins 2, 4, 6, 8, 11, 13). MCU pins can float during reset and before firmware
initialization; the pulldowns establish a low signal with the buffer enabled.
Firmware drives the pins low only after initialization, not throughout every
power-up/reset sequence. Verify power-up, reset, and first-pulse waveforms with
servo power disconnected before connecting a servo.

Power rules:

- Use a separate regulated **servo supply** for the servo rail; do not power a
  servo from the Nucleo 5 V pin or from the buffer's 5.1 V logic rail.
- **Check the exact SG92R you have** before choosing the voltage. Do not assume
  every SG92R accepts 6 V; use its actual rated voltage and start
  conservatively. An MG996R has a different stall-current requirement.
- Test **one servo on its own supply rail** first, with a current limit or fuse
  appropriate to that servo, and with the arm/horn clear.
- Support the servo or arm before `arm`, `select`, or `off`. Detaching the
  signal is **not** a brake and does not guarantee a held position.
- Provide a **hardware emergency stop** that physically disconnects servo
  power (switch, relay, or supply output). The `off` command only stops the
  signal; it is not an emergency stop.

The console's `500..2500 us` envelope is only a command sanity limit. The
default test window `1400..1600 us` is only a guard. Neither is proof that a
servo is mechanically safe. Follow the calibration procedure in the shared
README.

## Verify the build and shared behaviour

From the repository root:

```sh
# Shared console logic and the Uno Servo backend (host stubs)
python3 code/servo-calibration/test_host.py

# STM32 target compile
uv tool run --from platformio pio run -d code/stm32-servos
```

The host suite does not emulate STM32 timers. It tests the shared parser,
records, guards, and Uno backend. The PlatformIO target build verifies that the
STM32 backend's fixed API, pin map, and `HardwareTimer` calls compile against
the real STM32 Arduino core. **No physical PWM, voltage, or servo motion is
verified by these tests**; check the pins with a scope or logic analyser and
test one servo on a current-limited supply before connecting an arm.

The example console session in the shared README is illustrative only. Firmware
does not start PWM until an explicit valid `arm` command; this is not a guarantee
against physical movement during power-up. There is no automatic sweep.
