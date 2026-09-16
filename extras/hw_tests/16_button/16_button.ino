// Manual test of the breakout's EVI pushbutton.
// Metro Mini: VIN=A0, INT=D3, EVI=D4, SDA=A4, SCL=A5.
// D4 stays an input: press the physical button when prompted.
#include <Adafruit_RV8803.h>
#include <Adafruit_BusIO_Register.h>

Adafruit_RV8803 rtc;
Adafruit_I2CDevice rtc_device(RV8803_I2C_ADDRESS);
Adafruit_BusIO_Register control_reg(&rtc_device, RV8803_REG_CONTROL, 1);
const uint16_t buttonPin = 4;
const uint16_t interruptPin = 3;
const unsigned long buttonTimeoutMs = 30000;
const unsigned long debounceMs = 50;

void setup() {
  Serial.begin(115200);
  // Wait for Serial Monitor on native USB boards; remove for standalone use.
  while (!Serial) delay(10);
  delay(250);
  Serial.println(F("Adafruit RV8803 physical EVI button test"));
  // Use an output to supply VIN; an input HIGH would first enable a weak pull-up.
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
  // The breakout pulls EVI and INT up. Do not drive either pin from the MCU.
  pinMode(buttonPin, INPUT);
  pinMode(interruptPin, INPUT);
  delay(600); // Power-on reset can take 500 ms (manual section 7.4).
  check(rtc.begin(), F("Begin succeeded"));
  check(rtc_device.begin(), F("Raw register access ready"));
  // Isolate this test from timer, alarm, or update interrupts left by a sketch.
  check(control_reg.write(0), F("Other interrupt sources disabled"));
  check(rtc.enableEventReset(false), F("Event reset disabled"));
  check(rtc.enableEventCapture(false), F("Timestamp capture disabled"));
  check(rtc.configureEvent(false, RV8803_EventFilter16ms),
        F("Falling-edge button detection configured"));

  Serial.println();
  Serial.println(F("Release the EVI button (30 seconds allowed)."));
  check(waitForButton(HIGH), F("Released button reads HIGH on D4"));
  check(rtc.clearEvent(), F("Old event flag cleared"));
  check(rtc.enableInterrupt(RV8803_InterruptEvent), F("Event interrupt enabled"));
  uint8_t flags = rtc.readFlagRegister();
  check(flags != RV8803_READ_ERROR && !(flags & RV8803_FLAG_EVENT),
        F("Event flag initially clear"));
  check(digitalRead(interruptPin) == HIGH, F("INT initially released"));

  Serial.println();
  Serial.println(F("Press and HOLD the EVI button (30 seconds allowed)."));
  check(waitForButton(LOW), F("Pressed button reads LOW on D4"));
  // The stable-input wait exceeds two periods of the RTC's 15.6 ms filter.
  flags = rtc.readFlagRegister();
  check(flags != RV8803_READ_ERROR && (flags & RV8803_FLAG_EVENT),
        F("Button press set the event flag"));
  check(digitalRead(interruptPin) == LOW, F("Button press asserted INT"));

  Serial.println();
  Serial.println(F("Release the EVI button now (30 seconds allowed)."));
  check(waitForButton(HIGH), F("Button returned HIGH after release"));
  flags = rtc.readFlagRegister();
  check(flags != RV8803_READ_ERROR && (flags & RV8803_FLAG_EVENT),
        F("Event remained latched after button release"));
  check(rtc.clearEvent(), F("Button event acknowledged"));
  flags = rtc.readFlagRegister();
  check(flags != RV8803_READ_ERROR && !(flags & RV8803_FLAG_EVENT),
        F("Event flag cleared"));
  check(digitalRead(interruptPin) == HIGH, F("INT released after acknowledgement"));
  check(rtc.disableInterrupt(RV8803_InterruptEvent), F("Event interrupt disabled"));
  Serial.println(F("ALL PASSED - press Metro reset to test the button again"));
}

void loop() {}

bool waitForButton(uint8_t level) {
  unsigned long start = millis();
  unsigned long stableSince = start;
  while (millis() - start < buttonTimeoutMs) {
    if (digitalRead(buttonPin) != level) stableSince = millis();
    else if (millis() - stableSince >= debounceMs) return true;
    delay(1);
  }
  return false;
}

void clearOutputsAndHalt(const __FlashStringHelper* message) {
  Serial.print(F("FAIL: "));
  Serial.println(message);
  rtc.disableInterrupt(RV8803_InterruptEvent);
  rtc.clearEvent();
  pinMode(buttonPin, INPUT);
  pinMode(interruptPin, INPUT);
  while (true) delay(10);
}

void check(bool ok, const __FlashStringHelper* message) {
  if (!ok) clearOutputsAndHalt(message);
  Serial.println(message);
}
