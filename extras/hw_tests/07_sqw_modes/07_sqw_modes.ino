/*!
 * @file 07_sqw_modes.ino
 * @brief Hardware test 07: SQW Pin Modes
 *
 * Tests CLKOUT frequency by bit-bang counting edges on D5.
 * CLKOE on D2 is active HIGH; add 10 kohm from CLOE to switched DUT VIN.
 * D2 pulls LOW to disable or releases to INPUT to enable; never drive HIGH.
 * Also verifies read-modify-write preserves other Extension register bits.
 *
 * Wiring: SQWAVE -> D5, CLKOE -> D2
 */

#include <Adafruit_RV8803.h>
#include <Adafruit_BusIO_Register.h>

Adafruit_RV8803 rtc;
Adafruit_I2CDevice rtc_device(RV8803_I2C_ADDRESS);

#define SQW_PIN 5
#define CLKOE_PIN 2

/*!
 * @brief Count rising edges on SQW_PIN over a given duration
 * @param ms Duration in milliseconds
 * @return Number of rising edges counted
 */
unsigned long countEdges(unsigned long ms) {
  unsigned long count = 0;
  uint8_t last = digitalRead(SQW_PIN);
  unsigned long start = millis();
  while (millis() - start < ms) {
    uint8_t cur = digitalRead(SQW_PIN);
    if (cur == HIGH && last == LOW) {
      count++;
    }
    last = cur;
  }
  return count;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  // Power the RV-8803 via GPIO (VCC wired to A0)
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
  delay(600); // Power-on reset can take 500 ms (manual section 7.4).

  Serial.println(F("=== HW Test 07: SQW Modes ==="));
  Serial.println();

  if (!rtc.begin()) {
    Serial.println(F("FAIL: RV8803 not found"));
    return;
  }

  if (!rtc_device.begin()) {
    Serial.println(F("Could not open raw register access"));
    return;
  }
  Adafruit_BusIO_Register extension_reg(&rtc_device, RV8803_REG_EXTENSION, 1);

  uint8_t passed = 0;
  uint8_t total = 8;

  pinMode(SQW_PIN, INPUT);
  digitalWrite(CLKOE_PIN, LOW); // Keep the output latch and internal pull-up off.
  pinMode(CLKOE_PIN, INPUT); // External pull-up enables CLKOUT.

  // Test 1: 1Hz — count edges over 3 seconds, expect ~3
  Serial.print(F("Test 1: 1Hz output on D5 ... "));
  rtc.writeSqwPinMode(RV8803_SquareWave1Hz);
  delay(10);
  unsigned long edges1 = countEdges(3000);
  Serial.print(F("edges in 3s = "));
  Serial.print(edges1);
  if (edges1 >= 2 && edges1 <= 4) {
    Serial.println(F(" PASS"));
    passed++;
  } else {
    Serial.println(F(" FAIL"));
  }

  // Test 2: 1kHz — count edges over 200ms, expect ~200
  Serial.print(F("Test 2: 1kHz output on D5 ... "));
  rtc.writeSqwPinMode(RV8803_SquareWave1kHz);
  delay(10);
  unsigned long edges2 = countEdges(200);
  Serial.print(F("edges in 200ms = "));
  Serial.print(edges2);
  // Allow 150-250
  if (edges2 >= 150 && edges2 <= 250) {
    Serial.println(F(" PASS"));
    passed++;
  } else {
    Serial.println(F(" FAIL"));
  }

  // Test 3: 32.768kHz — count edges over 100ms, expect ~3277
  // Note: digitalRead on AVR may not keep up at 32kHz, so we just
  // verify we see significantly more edges than 1kHz
  Serial.print(F("Test 3: 32kHz output on D5 ... "));
  rtc.writeSqwPinMode(RV8803_SquareWave32kHz);
  delay(10);
  unsigned long edges3 = countEdges(100);
  Serial.print(F("edges in 100ms = "));
  Serial.print(edges3);
  // digitalRead tops out ~150kHz on AVR, so we should see most of them
  // Expect at least 2000 edges
  if (edges3 >= 2000) {
    Serial.println(F(" PASS"));
    passed++;
  } else {
    Serial.println(F(" FAIL"));
  }

  // Test 4: Changing SQW frequency must preserve the timer enable field.
  Adafruit_BusIO_RegisterBits timer_enabled(&extension_reg, 1, 4);
  Serial.println(F("Test 4: SQW change preserves timer enable ..."));
  if (!timer_enabled.write(1)) {
    Serial.println(F("FAIL: Could not enable timer"));
    return;
  }
  bool timerBefore = timer_enabled.read();
  rtc.writeSqwPinMode(RV8803_SquareWave1Hz);
  bool timerAfter = timer_enabled.read();
  if (timerBefore && timerAfter) {
    Serial.println(F("  PASS - timer enable preserved"));
    passed++;
  } else {
    Serial.println(F("  FAIL - timer enable changed"));
  }
  if (!timer_enabled.write(0)) {
    Serial.println(F("FAIL: Could not disable timer"));
    return;
  }

  // Test 5: SQW changes must preserve alarm and periodic-update settings.
  Adafruit_BusIO_RegisterBits date_mode(&extension_reg, 1, 6);
  Adafruit_BusIO_RegisterBits minute_update(&extension_reg, 1, 5);
  Serial.println(F("Test 5: SQW change preserves alarm and update modes ..."));
  if (!date_mode.write(1)) {
    Serial.println(F("FAIL: Could not select date alarm mode"));
    return;
  }
  if (!minute_update.write(1)) {
    Serial.println(F("FAIL: Could not select minute updates"));
    return;
  }
  bool dateModeBefore = date_mode.read();
  bool minuteUpdateBefore = minute_update.read();
  rtc.writeSqwPinMode(RV8803_SquareWave32kHz);
  bool dateModeAfter = date_mode.read();
  bool minuteUpdateAfter = minute_update.read();
  if (dateModeBefore && dateModeAfter && minuteUpdateBefore && minuteUpdateAfter) {
    Serial.println(F("  PASS - alarm and update modes preserved"));
    passed++;
  } else {
    Serial.println(F("  FAIL - alarm or update mode changed"));
  }

  // Test 6: SQW changes must preserve the timer clock selection.
  Adafruit_BusIO_RegisterBits timer_clock(&extension_reg, 2, 0);
  Serial.println(F("Test 6: SQW change preserves timer clock ..."));
  if (!timer_clock.write(RV8803_Timer1Hz)) {
    Serial.println(F("FAIL: Could not select timer clock"));
    return;
  }
  uint8_t clockBefore = timer_clock.read();
  rtc.writeSqwPinMode(RV8803_SquareWave1Hz);
  rtc.writeSqwPinMode(RV8803_SquareWave1kHz);
  rtc.writeSqwPinMode(RV8803_SquareWave32kHz);
  uint8_t clockAfter = timer_clock.read();
  if (clockBefore == RV8803_Timer1Hz && clockAfter == RV8803_Timer1Hz) {
    Serial.println(F("  PASS - timer clock preserved"));
    passed++;
  } else {
    Serial.println(F("  FAIL - timer clock changed"));
  }

  // Restore the extension register after the preservation checks.
  if (!extension_reg.write(0)) {
    Serial.println(F("FAIL: Could not restore extension register"));
    return;
  }

  // Test 7: CLKOE LOW disables CLKOUT (active HIGH)
  Serial.print(F("Test 7: CLKOE LOW disables CLKOUT ... "));
  rtc.writeSqwPinMode(RV8803_SquareWave1kHz);
  delay(10);
  pinMode(CLKOE_PIN, OUTPUT); // The LOW latch disables CLKOUT.
  delay(10);
  unsigned long edgesOff = countEdges(200);
  Serial.print(F("edges in 200ms = "));
  Serial.print(edgesOff);
  if (edgesOff == 0) {
    Serial.println(F(" PASS (no pulses)"));
    passed++;
  } else {
    Serial.println(F(" FAIL (still pulsing)"));
  }

  // Test 8: CLKOE HIGH re-enables CLKOUT
  Serial.print(F("Test 8: CLKOE HIGH re-enables CLKOUT ... "));
  pinMode(CLKOE_PIN, INPUT); // Release to the external pull-up.
  delay(10);
  unsigned long edgesOn = countEdges(200);
  Serial.print(F("edges in 200ms = "));
  Serial.print(edgesOn);
  if (edgesOn >= 150 && edgesOn <= 250) {
    Serial.println(F(" PASS (1kHz restored)"));
    passed++;
  } else {
    Serial.println(F(" FAIL"));
  }

  // Clean up: leave CLKOUT disabled.
  rtc.writeSqwPinMode(RV8803_SquareWave32kHz);
  pinMode(CLKOE_PIN, OUTPUT);

  Serial.println();
  Serial.print(passed);
  Serial.print(F("/"));
  Serial.print(total);
  Serial.println(F(" tests passed"));
}

void loop() {
  // Nothing to do
}
