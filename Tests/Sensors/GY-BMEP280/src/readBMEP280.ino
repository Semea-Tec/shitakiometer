#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SDA_PIN 4 // Pico 2 W I2C0 default SDA
#define SCL_PIN 5 // Pico 2 W I2C0 default SCL

#define SEALEVEL_HPA 1013.25 // Adjust to your local sea-level pressure for accurate altitude

Adafruit_BME280 bme;
bool hasHumidity = false;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10);

    Wire.setSDA(SDA_PIN);
    Wire.setSCL(SCL_PIN);
    Wire.begin();

    // GY-BMEP280 boards ship at I2C address 0x76 or 0x77 depending on the SDO pin
    if (!bme.begin(0x76) && !bme.begin(0x77))
    {
        Serial.println("Could not find a BME280/BMP280 sensor. Check wiring and I2C address.");
        while (true)
            delay(1000);
    }

    // The library reports 0x60 for BME280 and 0x58 for BMP280 (no humidity sensor)
    hasHumidity = (bme.sensorID() == 0x60);
    Serial.print("Sensor found: ");
    Serial.println(hasHumidity ? "BME280" : "BMP280");
}

void loop()
{
    Serial.print("Temperature (C): ");
    Serial.println(bme.readTemperature());

    Serial.print("Pressure (hPa): ");
    Serial.println(bme.readPressure() / 100.0F);

    Serial.print("Altitude (m): ");
    Serial.println(bme.readAltitude(SEALEVEL_HPA));

    if (hasHumidity)
    {
        Serial.print("Humidity (%): ");
        Serial.println(bme.readHumidity());
    }

    Serial.println();
    delay(2000);
}
