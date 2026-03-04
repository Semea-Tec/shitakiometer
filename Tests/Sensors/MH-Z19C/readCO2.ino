#include <Arduino.h>
#include <HardwareSerial.h>
#include <Mhz19.h>                                        

#define RX_PIN 16 // ESP32 RX2 pin - Connect to sensor TX
#define TX_PIN 17 // ESP32 TX2 pin - Connect to sensor RX
#define BAUDRATE 9600

Mhz19 myMHZ19;
HardwareSerial mySerial(2); // Use ESP32 UART hardware serial 2

void setup()
{
  Serial.begin(115200);

  // Initialize UART connection to the sensor
  mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);

  // Initialize MHZ19 object
  myMHZ19.begin(&mySerial);
  myMHZ19.setMeasuringRange(Mhz19MeasuringRange::Ppm_5000); // 5000 is common for C versions
  myMHZ19.enableAutoBaseCalibration(); // Turn auto calibration ON (recommended for ambient air)
  
  Serial.println("Preheating... (This can take up to 3 minutes for accurate readings)");
  while (!myMHZ19.isReady()) {
    delay(50);
  }

  Serial.println("MH-Z19C Ready.");
}

void loop()
{
  // The Mhz19 library sometimes fails on ESP32 because it checks Serial.available() 
  // without waiting for the sensor to reply. We can read directly to ensure it works!
  uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  
  // Clear the buffer
  while(mySerial.available() > 0) mySerial.read();

  // Request reading
  mySerial.write(cmd, 9);
  mySerial.flush();
  
  // Wait up to 1 second for a response
  unsigned long startTime = millis();
  while(mySerial.available() < 9 && (millis() - startTime) < 1000) {
    delay(10);
  }

  if (mySerial.available() >= 9) {
    uint8_t response[9];
    mySerial.readBytes(response, 9);
    
    // Validate response checksum
    uint8_t checksum = 0;
    for (int i = 1; i < 8; i++) {
      checksum += response[i];
    }
    checksum = 255 - checksum + 1;

    if (response[0] == 0xFF && response[1] == 0x86 && response[8] == checksum) {
      int co2 = (response[2] * 256) + response[3];
      Serial.print("CO2 (ppm): ");
      Serial.println(co2);
    } else {
      Serial.println("Checksum failed or invalid response from sensor.");
    }
  } else {
    Serial.println("Failed to read CO2 measurement. No response from sensor.");
    Serial.println(" -> TIP: Try swapping the RX (16) and TX (17) pins if this persists!");
  }

  delay(2000); // Read every 2 seconds
}
