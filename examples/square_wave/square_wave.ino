// Generate a continuous 1 Hz square wave on the RTC's SQW / CLKOUT pin.
// Connect VIN, GND, SDA, and SCL as usual, then connect CLKOE (CLOE) to VIN.
// Measure SQW with an oscilloscope or logic analyzer sharing the RTC's GND.
// CLKOE must stay HIGH; pulling it LOW disables the output.
// The RTC supports only 1 Hz, 1024 Hz, and 32.768 kHz, not 10 Hz.
#include <Adafruit_RV8803.h>

Adafruit_RV8803 rtc;

void setup() {
  Serial.begin(115200);
  // Wait for Serial Monitor on native USB boards; remove for standalone use.
  while (!Serial) delay(10);
  delay(600); // Allow the RTC's power-on reset to finish.

  Serial.println(F("Adafruit RV8803 square wave"));
  if (!rtc.begin()) {
    Serial.println(F("RTC not found"));
    while (true) delay(10);
  }

  // Other choices: RV8803_SquareWave1kHz (1024 Hz) or
  // RV8803_SquareWave32kHz (32768 Hz).
  if (!rtc.writeSqwPinMode(RV8803_SquareWave1Hz)) {
    Serial.println(F("Could not configure the square wave"));
    while (true) delay(10);
  }
  Serial.println(F("SQW configured for 1 Hz; keep CLKOE connected to VIN"));
}

void loop() {
  // The RTC generates the waveform in hardware; no polling is needed.
}
