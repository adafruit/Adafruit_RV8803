# Jumperless RV8803 fixture

Run `jumperless_setup.py` on the Jumperless to replace the hand-wired Metro Mini
fixture with a **5 V ATmega328P Arduino Nano** in the Nano socket. The attached
Jumperless reports V5 firmware 5.7.11.0; this script uses its MicroPython API.
It has not been tested on the older V3 hardware.

Insert the RV8803 QT rev A header into consecutive rows as follows:

| Breadboard row | RV8803 pin | Nano connection |
| --- | --- | --- |
| 1 | VIN | A0 |
| 2 | GND | GND |
| 3 | SCL | A5 |
| 4 | SDA | A4 |
| 5 | CLOE / CLKOE | D2 |
| 6 | INT | D3 |
| 7 | EVI | D4 |
| 8 | SQW | D5 |

These assignments match the existing sketches without code changes. VIN is
powered only by A0 so the sketches can turn it off. Leave STEMMA QT and other
external power connections unplugged. The Nano socket supplies the Nano's own
power and ground. By default, upload and monitor through the Nano's own USB
connector. `CONNECT_NANO_UART = False` leaves Jumperless UART disconnected so its
TX cannot contend with the Nano's USB serial adapter on D0. To use Jumperless
UART passthrough instead, unplug Nano USB and set `CONNECT_NANO_UART = True`.

## Run the setup

Open `jumperless_setup.py` in [JumperIDE](https://ide.jumperless.org/), connect to
the Jumperless **third USB serial interface (MicroPython)**, and click Run.
On the development fixture this is **COM11**. Direct Nano USB is **COM16** and
uses the newer ATmega328P bootloader. Jumperless UART passthrough is **COM9**;
COM10 is the Jumperless menu and COM12 is its machine interface. Port
numbers can change after reconnecting, so identify the interface rather than
assuming COM numbers on another computer.

The script prints each replaced connection, disconnects existing routes touching
rows 1–8 or the Nano/UART pins it needs, then installs and checks all eight RV
routes (plus two UART routes when enabled).
Unrelated routes remain in place. It can be rerun to restore the fixture wiring.
The Jumperless OLED routing is disabled because its firmware otherwise restores
its own I2C connections on Nano D2/D3 when leaving the REPL. This fixture needs
those pins for CLOE and INT. The script also rejects unexpected bridges touching
the fixture pins, so a requested connection cannot pass while still tied to an
unwanted GPIO or supply.
It selects global connection context so the wiring remains after the script
exits; rerun it after changing slots or restarting the Jumperless. It does not
save over a slot file or change programmable supply voltages.

After setup, upload a sketch from this directory to the Nano and open its serial
interface at **115200 baud**. Select Arduino Nano / ATmega328P, using the old
bootloader option only if that is what another Nano requires. Direct USB uploads
were verified on this fixture; Jumperless passthrough uploads did not respond.
Route checks
verify the Jumperless configuration, not electrical communication or test results.

The sketches configure A0 as an output before driving VIN high. Writing HIGH
while A0 is still an input first enables its weak pull-up. The timer and event
sketches originally used that order and failed the initial address probe on
this fixture, although subsequent status reads worked. Use the same output-first
power sequence as the other sketches, then allow the existing 600 ms startup wait.

## Test-specific handling

- Tests `00`–`14` and `sqw_diag` use this same wiring. For `06_ram`, set
  `batteryInstalled` to match the actual coin cell; it currently defaults to true.
- `15_no_battery` requires physically removing the coin cell. The same routes
  remain valid: the sketch disables pull-ups and auxiliary outputs before its
  five-minute VIN-off interval. Jumperless cannot disconnect the onboard cell.
- `16_button` requires manually pressing and releasing the RV8803 button when
  prompted. Nano D4 stays an input in that sketch.
- Do not change routes, switch slots, or run another Jumperless script while a
  test is running.

API and transport references: [Jumperless MicroPython API](https://docs.jumperless.org/09.5-micropythonAPIreference/)
and [automation interfaces](https://docs.jumperless.org/07.5-automation/).
