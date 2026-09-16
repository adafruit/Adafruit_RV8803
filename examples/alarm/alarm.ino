// Set an alarm for the next minute boundary; it repeats daily at that time.
// Wire INT to an input if you also want a physical wake signal.
#include <Adafruit_RV8803.h>

Adafruit_RV8803 rtc;
unsigned long lastTimePrint = 0;

void setup() {
  Serial.begin(115200);
  // Wait for Serial Monitor on native USB boards; remove for standalone use.
  while (!Serial) delay(10);
  delay(600); // Allow the RTC's power-on reset to finish.
  Serial.println(F("Adafruit RV8803 next-minute alarm"));
  if (!rtc.begin()) {
    Serial.println(F("RTC not found"));
    while (true) delay(10);
  }
  if (rtc.lostPower() && !rtc.adjust(DateTime(F(__DATE__), F(__TIME__)))) {
    Serial.println(F("Could not set the time"));
    while (true) delay(10);
  }
  DateTime now = rtc.now();
  if (!now.isValid()) {
    Serial.println(F("Could not read the current time"));
    while (true) delay(10);
  }
  // The alarm matches minutes, not seconds. Advance to the next minute at :00.
  // DateTime arithmetic also handles hour, day, month, and year rollover.
  DateTime alarmTime = now + TimeSpan(60 - now.second());

  // Disable alarm output while changing the match fields (manual section 4.7.2).
  // HourMin repeats daily at this time; date/weekday is ignored.
  if (!rtc.disableInterrupt(RV8803_InterruptAlarm) ||
      !rtc.setAlarm(alarmTime, RV8803_A_HourMin) ||
      !rtc.clearAlarm() || !rtc.enableInterrupt(RV8803_InterruptAlarm)) {
    Serial.println(F("Could not configure the alarm"));
    while (true) delay(10);
  }
  Serial.print(F("Current RTC time: "));
  Serial.println(now.timestamp(DateTime::TIMESTAMP_TIME));
  Serial.print(F("Alarm set for: "));
  Serial.println(alarmTime.timestamp(DateTime::TIMESTAMP_TIME));
  Serial.println(F("First alarm at the next minute boundary, then daily"));
  lastTimePrint = millis();
}

void loop() {
  if (rtc.alarmFired()) {
    Serial.println(F("Alarm fired"));
    if (!rtc.clearAlarm()) Serial.println(F("Could not clear the alarm"));
  }
  // Print once per second without slowing down the alarm checks.
  unsigned long currentMillis = millis();
  if (currentMillis - lastTimePrint >= 1000) {
    lastTimePrint = currentMillis;
    DateTime now = rtc.now();
    if (now.isValid()) {
      Serial.print(F("Current RTC time: "));
      Serial.println(now.timestamp(DateTime::TIMESTAMP_TIME));
    } else {
      Serial.println(F("Could not read the current time"));
    }
  }
  delay(100);
}
