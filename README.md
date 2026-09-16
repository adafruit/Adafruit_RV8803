# Adafruit RV8803 [![Arduino Library CI](https://github.com/adafruit/Adafruit_RV8803/actions/workflows/githubci.yml/badge.svg)](https://github.com/adafruit/Adafruit_RV8803/actions/workflows/githubci.yml) [![Documentation](https://img.shields.io/badge/documentation-doxygen-blue.svg)](https://adafruit.github.io/Adafruit_RV8803/html/index.html)

Arduino library for the Micro Crystal RV-8803-C7 real-time clock, using
RTClib's `DateTime` class. Supports alarms, countdown timers, periodic updates,
external-event timestamps, clock output, offset calibration, and RAM.

Install **Adafruit RV8803** and its **RTClib** and **Adafruit BusIO** dependencies
through Arduino Library Manager once the library is released. Before release,
download this repository as a ZIP and use **Sketch > Include Library > Add .ZIP
Library**, then install the dependencies through Library Manager.

## Examples

- `simpletest`: set and read the date and time.
- `alarm`: trigger at the next minute boundary, then daily at that time.
- `timer`: repeat a five-second countdown. The first interval takes 5–6 seconds.
- `event_capture`: report a pending event at startup, then capture seconds and
  hundredths with the EVI input. Only the latest event is retained.
- `square_wave`: output a continuous 1 Hz square wave on SQW, with CLKOE tied HIGH.

CLOE must be HIGH for the SQW output. The clock provides 32.768 kHz, 1024 Hz,
or 1 Hz. Event timestamps contain only seconds and hundredths within a minute.

MIT licensed; see [LICENSE](LICENSE).
