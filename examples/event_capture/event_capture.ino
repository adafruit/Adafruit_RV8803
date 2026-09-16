// Press the breakout's EVI button to capture seconds and hundredths.
// On startup, report any pending event before clearing its flag.
// For battery-only capture, EVI needs a pull-up to battery-backed VDD (Rev B).
// To try it: run this sketch, disconnect the MCU, press EVI, then reconnect.
// Only the latest event is retained, without its minute, hour, or date.
#include <Adafruit_RV8803.h>

Adafruit_RV8803 rtc;

void setup() {
  Serial.begin(115200);
  // Wait for Serial Monitor on native USB boards; remove for standalone use.
  while (!Serial) delay(10);
  delay(600); // Allow the RTC's power-on reset to finish.
  Serial.println(F("Adafruit RV8803 event capture"));
  if (!rtc.begin()) {
    halt(F("RTC not found"));
  }

  // Inspect saved flags before setting the clock or clearing any event.
  uint8_t flags;
  if (!rtc.readFlagRegister(&flags)) {
    halt(F("Could not read the saved event status"));
  }
  if (flags & RV8803_FLAG_TIME_INVALID) {
    Serial.println(F("RTC lost power; previous event data cannot be trusted"));
    if (!rtc.adjust(DateTime(F(__DATE__), F(__TIME__)))) {
      halt(F("Could not set the time"));
    }
  } else if (flags & RV8803_FLAG_EVENT) {
    Serial.println(F("Pending event at startup (latest event):"));
    if (!printCapturedEvent()) {
      halt(F("Could not read the saved event timestamp"));
    }
  } else {
    Serial.println(F("No pending event at startup"));
  }

  // Disable event output while changing its settings.
  if (!rtc.disableInterrupt(RV8803_InterruptEvent)) {
    halt(F("Could not disable the event interrupt"));
  }

  // Falling edge with a 15.6 ms sampling interval filters short button glitches.
  if (!rtc.configureEvent(false, RV8803_EventFilter16ms)) {
    halt(F("Could not configure the event input"));
  }

  // ERST must be off for timestamps: an event reset clears the capture registers.
  if (!rtc.enableEventReset(false)) {
    halt(F("Could not disable event reset"));
  }

  // Store the seconds and hundredths whenever a new event arrives.
  if (!rtc.enableEventCapture(true)) {
    halt(F("Could not enable event capture"));
  }

  // The startup report is complete; clear the old flag and arm event output.
  if (!rtc.clearEvent()) {
    halt(F("Could not clear the event flag"));
  }
  if (!rtc.enableInterrupt(RV8803_InterruptEvent)) {
    halt(F("Could not enable the event interrupt"));
  }

  Serial.println(F("Press EVI to capture a timestamp within the current minute"));
  Serial.println(F("Capture stays armed while the RTC runs on its battery"));
}

void loop() {
  if (rtc.eventFired()) {
    if (printCapturedEvent()) {
      if (!rtc.clearEvent()) Serial.println(F("Could not clear the event flag"));
    }
  }
  delay(10);
}

// Share the same timestamp display between startup and live events.
bool printCapturedEvent() {
  rv8803_timestamp_t captured;
  if (!rtc.getEventTimestamp(&captured)) return false;
  Serial.print(F("\tCaptured second: "));
  Serial.print(captured.seconds);
  Serial.print('.');
  if (captured.hundredths < 10) Serial.print('0');
  Serial.println(captured.hundredths);
  return true;
}

// Print a setup error and stop here until the board is reset.
void halt(const __FlashStringHelper *message) {
  Serial.println(message);
  while (true) delay(10);
}
