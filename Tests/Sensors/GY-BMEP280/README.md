# GY-BMEP280 (BME280/BMP280) Sensor Test — Raspberry Pi Pico 2 W

This folder contains a test sketch (`readBMEP280.ino`) to read temperature,
pressure and (when available) humidity from a GY-BMEP280 breakout board
using a Raspberry Pi Pico 2 W.

> GY-BMEP280 boards are sold with either the Bosch **BME280** chip
> (temperature + pressure + humidity) or the **BMP280** chip (temperature +
> pressure only) under the same silkscreen. The sketch auto-detects which
> one is populated via the I2C chip-ID register and only prints humidity
> when a BME280 is found.

## Requirements

- Arduino IDE with the **Raspberry Pi Pico/RP2040** board package (Earle
  Philhower core) installed, with **Raspberry Pi Pico 2 W** selected as the
  board.
- Library: **Adafruit BME280 Library** (Library Manager), which pulls in
  **Adafruit Unified Sensor** and **Adafruit BusIO** as dependencies.

## Wiring (I2C)

| GY-BMEP280 Pin | Function | Pico 2 W Pin |
| :---: | :--- | :--- |
| **VCC** | Power | **3V3 (OUT)** |
| **GND** | Ground | **GND** |
| **SCL** | I2C Clock | **GP5** |
| **SDA** | I2C Data | **GP4** |

> The module is 3.3V only — do not power it from 5V/VBUS.

## I2C Address

The sketch tries `0x76` first, then falls back to `0x77`. Which address the
board answers on depends on the state of its `SDO` pin (`SDO` to GND = 0x76,
`SDO` to VCC = 0x77 — most GY-BMEP280 boards default to 0x76 with an
internal pull-down).

## Code Overview

1. **I2C init:** explicitly sets SDA (GP4) and SCL (GP5) before `Wire.begin()`
   so the pins are obvious and easy to change.
2. **Chip detection:** calls `bme.begin()` at `0x76`, then `0x77`, and reads
   `bme.sensorID()` to tell a BME280 (`0x60`) from a BMP280 (`0x58`).
3. **Loop:** prints temperature, pressure and altitude every 2 seconds, and
   humidity too when a BME280 is detected.

## Troubleshooting

- **"Could not find a BME280/BMP280 sensor"**: check the SDA/SCL wiring
  (swap them if unsure), confirm 3.3V power, and try grounding/floating the
  `SDO` pin to hit the other I2C address.
- **Altitude reading looks off**: update `SEALEVEL_HPA` in the sketch to the
  current local sea-level pressure (check a weather report) for an accurate
  reading — otherwise it's only a rough estimate.
