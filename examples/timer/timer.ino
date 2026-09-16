// A repeating five-second countdown. INT is an active-low, pulsed output.
#include <Adafruit_RV8803.h>

Adafruit_RV8803 rtc;

void setup() {
  Serial.begin(115200);
  // Wait for Serial Monitor on native USB boards; remove for standalone use.
  while (!Serial) delay(10);
  delay(250);

  Serial.println(F("Adafruit RV8803 countdown timer"));

  if (!rtc.begin()) {
    halt(F("RTC not found"));
  }

  // Disable timer output while changing its settings.
  if (!rtc.disableInterrupt(RV8803_InterruptTimer)) {
    halt(F("Could not disable the timer interrupt"));
  }

  // Clear any pending timer flag before starting the countdown.
  if (!rtc.clearTimer()) {
    halt(F("Could not clear the timer flag"));
  }

  // Select 1 tick per second, 5 ticks. Valid counts are 1 through 4095.
  // Manual section 4.5.3: the first period is 5-6 s; later periods are 5 s.
  // Each enableCountdownTimer() call restarts this startup interval.
  // For less startup uncertainty, 64 Hz with 320 ticks gives 5-5.015625 s.
  if (!rtc.enableCountdownTimer(RV8803_Timer1Hz, 5)) {
    halt(F("Could not start the countdown timer"));
  }

  // Allow the timer to signal on the INT pin.
  if (!rtc.enableInterrupt(RV8803_InterruptTimer)) {
    halt(F("Could not enable the timer interrupt"));
  }

  Serial.println(F("First countdown takes 5-6 seconds, then repeats every 5 seconds"));
}

void loop() {
  // The flag stays set after the short INT pulse ends, so polling is possible.
  if (rtc.timerFired()) {
    Serial.println(F("Countdown finished"));
    if (!rtc.clearTimer()) Serial.println(F("Could not clear the timer flag"));
  }
  delay(100);
}

// Print a setup error and stop here until the board is reset.
void halt(const __FlashStringHelper *message) {
  Serial.println(message);
  while (true) delay(10);
}
