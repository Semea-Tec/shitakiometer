# MH-Z19C CO2 Sensor Test

This folder contains a test sketch (`readCO2.ino`) to read CO2 concentrations (in ppm) using the MH-Z19C NDIR CO2 sensor with an ESP32.

## Sensor Specifications
- **Operating Voltage:** 5.0 ± 0.1V DC (It must be powered from the ESP32's `5V` or `VIN` pin, not `3.3V`).
- **Data Logic Level:** 3.3V compatible (Safe to connect directly to ESP32 RX/TX pins without a logic level converter).
- **Default Measurement Range:** 0 - 5000 ppm.
- **Preheating Time:** ~3 minutes. (The sensor outputs unreliable data or fixed values until the warmup is complete).
- **Communication Interface:** UART (Baud Rate: 9600, Data bits: 8, Stop bits: 1, Parity: None).

## Wiring & Pinout (7-Pin Terminal Version)
Based on the official Winsen datasheet for the terminal/connector version. Assuming the standard rainbow wire sequence:

| Pin | Datasheet Label | Function | Wire Color | ESP32 Connection |
| :---: | :--- | :--- | :--- | :--- |
| **1** | **HD** | Zero Calibration | Brown | *Leave Disconnected* |
| **2** | **Vo** | Analog Output | White | *Leave Disconnected* |
| **3** | **GND** | Ground | **Black** | **GND** |
| **4** | **Vin** | 5V Power Supply | **Red** | **5V / VIN** |
| **5** | **RXD** | UART Receive | **Blue** | **Pin 17 (TX2)** |
| **6** | **TXD** | UART Transmit | **Green** | **Pin 16 (RX2)** |
| **7** | **PWM** | PWM Output | Yellow | *Leave Disconnected* |

> *Note: In UART communication, RX connects to TX, and TX connects to RX. That's why Sensor TXD (Green) goes to ESP32 RX (Pin 16), and Sensor RXD (Blue) goes to ESP32 TX (Pin 17).*

## Code Overview
The provided sketch bypasses a common library bug with fast controllers like the ESP32:
1. **Library Setup:** Uses the `Mhz19` library (by Eduard Malokhvii) for the initialization and configuration commands.
2. **Direct Reading:** In the `loop()`, instead of relying on the library's read function (which checks `Serial.available()` too quickly for the ESP32), it sends the raw read command standard `[0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79]` and waits patiently with a timeout for the 9-byte packet response.
3. **Checksum Validation:** It manually calculates the checksum to guarantee data integrity before parsing the CO2 ppm value.

## Troubleshooting
- **"No response from sensor"**: You most likely have the Green and Blue wires swapped. Try switching pins 16 and 17 in the code or physically swap the wires. Also, confirm the Red wire is receiving a solid 5V.
- **Constant reading of `400 ppm` inside**: Wait at least 3 minutes for pre-heating to finish, and ensure auto-calibration isn't interfering improperly (take it outside for 20 minutes to baseline it to ~400 ppm).
