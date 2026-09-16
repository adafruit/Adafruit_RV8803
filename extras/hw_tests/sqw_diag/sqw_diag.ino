// Add 10 kohm from CLOE to switched DUT VIN. D2 only pulls LOW or releases.
#include <Adafruit_RV8803.h>
#include <Adafruit_BusIO_Register.h>

Adafruit_RV8803 rtc;
Adafruit_I2CDevice rtc_device(RV8803_I2C_ADDRESS);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  // Power the RV-8803 via GPIO (VCC wired to A0)
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
  delay(600); // Power-on reset can take 500 ms (manual section 7.4).

  Serial.println(F("=== SQW Simple Diagnostic ==="));

  if (!rtc.begin()) {
    Serial.println(F("RV8803 not found"));
    return;
  }

  if (!rtc_device.begin()) {
    Serial.println(F("Could not open raw register access"));
    return;
  }
  Adafruit_BusIO_Register extension_reg(&rtc_device, RV8803_REG_EXTENSION, 1);

  // Set 32kHz mode
  rtc.writeSqwPinMode(RV8803_SquareWave32kHz);

  // Read back extension register
  uint8_t ext;
  if (!extension_reg.read(&ext)) {
    Serial.println(F("Could not read extension register"));
    return;
  }
  Serial.print(F("Extension reg: 0x"));
  Serial.println(ext, HEX);
  Serial.print(F("FD bits: "));
  Serial.println(rtc.readSqwPinMode());

  // Pull CLOE LOW, then release it to the external VIN pull-up.
  Serial.println(F("CLOE LOW for 5s... (scope SQWAVE now)"));
  digitalWrite(2, LOW);
  pinMode(2, OUTPUT);
  delay(5000);

  Serial.println(F("CLOE released HIGH for 5s... (scope SQWAVE now)"));
  pinMode(2, INPUT);
  delay(5000);

  pinMode(2, OUTPUT); // LOW latch: leave CLKOUT disabled.
  Serial.println(F("Done. CLOE left LOW."));
}

void loop() {}
