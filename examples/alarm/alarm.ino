// Set an alarm for the next minute boundary; it repeats daily at that time.
// Wire INT to an input if you also want a physical wake signal.
#include <Adafruit_RV8803.h>

Adafruit_RV8803 rtc;
uint8_t lastSecond = 0;

void setup() {
  Serial.begin(115200);
  // Wait for Serial Monitor on native USB boards; remove for standalone use.
  while (!Serial) delay(10);
  delay(600); // Allow the RTC's power-on reset to finish.
  Serial.println(F("Adafruit RV8803 next-minute alarm"));
  if (!rtc.begin()) {
    halt(F("RTC not found"));
  }
  if (rtc.lostPower()) {
    if (!rtc.adjust(DateTime(F(__DATE__), F(__TIME__)))) {
      halt(F("Could not set the time"));
    }
  }
  DateTime now = rtc.now();
  if (!now.isValid()) {
    halt(F("Could not read the current time"));
  }
  // The alarm matches minutes, not seconds. Advance to the next minute at :00.
  // DateTime arithmetic also handles hour, day, month, and year rollover.
  DateTime alarmTime = now + TimeSpan(60 - now.second());

  // Disable alarm output while changing the match fields (manual section 4.7.2).
  if (!rtc.disableInterrupt(RV8803_InterruptAlarm)) {
    halt(F("Could not disable the alarm interrupt"));
  }

  // HourMin repeats daily at this time; date/weekday is ignored.
  if (!rtc.setAlarm(alarmTime, RV8803_A_HourMin)) {
    halt(F("Could not set the alarm time"));
  }

  // Clear any pending alarm before enabling the new one.
  if (!rtc.clearAlarm()) {
    halt(F("Could not clear the alarm flag"));
  }

  // Allow the alarm to signal on the INT pin.
  if (!rtc.enableInterrupt(RV8803_InterruptAlarm)) {
    halt(F("Could not enable the alarm interrupt"));
  }

  Serial.print(F("Current RTC time: "));
  Serial.println(now.timestamp(DateTime::TIMESTAMP_TIME));
  Serial.print(F("Alarm set for: "));
  Serial.println(alarmTime.timestamp(DateTime::TIMESTAMP_TIME));
  Serial.println(F("First alarm at the next minute boundary, then daily"));
  lastSecond = now.second();
}

void loop() {
  if (rtc.alarmFired()) {
    Serial.println(F("Alarm fired"));
    if (!rtc.clearAlarm()) Serial.println(F("Could not clear the alarm"));
  }
  // Print whenever the RTC's seconds value changes.
  DateTime now = rtc.now();
  if (now.isValid()) {
    if (now.second() != lastSecond) {
      lastSecond = now.second();
      Serial.print(F("Current RTC time: "));
      Serial.println(now.timestamp(DateTime::TIMESTAMP_TIME));
    }
  } else {
    Serial.println(F("Could not read the current time"));
  }
  delay(100);
}

// Print a setup error and stop here until the board is reset.
void halt(const __FlashStringHelper *message) {
  Serial.println(message);
  while (true) delay(10);
}
