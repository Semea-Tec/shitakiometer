# LoRa Point-to-Point Test (ESP32-S3 Heltec V3)

This project implements a simple point-to-point communication test using two **Heltec WiFi LoRa 32 V3** (ESP32-S3) modules. The Sender transmits a random number, and the Receiver displays it on the integrated OLED screen along with the signal strength (RSSI).

## ⚠️ Hardware Warning
**NEVER power on the modules without the LoRa antenna connected.** Operating without an antenna can permanently damage the SX1262 LoRa radio chip.

## Hardware Specifications (V3)
This setup is specifically optimized for Heltec V3 hardware:
- **MCU:** ESP32-S3
- **LoRa Chip:** SX1262
- **Frequency:** 915 MHz (Brasil)
- **OLED:** 0.96" SSD1306 (I2C)
- **Power Control (Vext):** GPIO 36

## Software Requirements
You will need the following libraries installed in your Arduino IDE:
1. **RadioLib** (by Jan Gisselberg) - For SX1262 communication.
2. **Adafruit SSD1306** - For the OLED display.
3. **Adafruit GFX Library** - Dependency for the OLED.

## Build and Upload Instructions

### 1. Arduino IDE Setup
- Go to **File > Preferences**.
- Add to "Additional Boards Manager URLs": `https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.9/package_heltec_esp32_index.json`
- Go to **Tools > Board > Boards Manager**, search for **Heltec ESP32** and install the latest version.

### 2. Uploading the Code
Follow these steps for each board:

#### Module A: Sender
1. Connect Board A to your computer.
2. Open [Lora/Sender/Sender.ino](Lora/Sender/Sender.ino).
3. Select **Tools > Board > Heltec WiFi LoRa 32 (V3)**.
4. Select the correct **Port**.
5. Click **Upload**.
6. The white onboard LED should light up, and the screen should show "SENDER V3 READY".

#### Module B: Receiver
1. Connect Board B to your computer.
2. Open [Lora/Receiver/Receiver.ino](Lora/Receiver/Receiver.ino).
3. Keep the same board settings.
4. Select the new **Port**.
5. Click **Upload**.
6. The screen should show "RECEIVER V3 READY" and then display values as they are received.

## Troubleshooting
- **Dark/Black Screen:** The code uses GPIO 36 (`Vext`) to power the display. Ensure you are using the V3 board support and the latest code provided.
- **No Data Received:** Check if both boards are on the same frequency (915 MHz) and within range. RSSI values closer to 0 (e.g., -40 dBm) indicate a stronger signal than values further away (e.g., -110 dBm).
