# Digital Compass and Inclinometer
Built using ESP32, MPU6050 IMU, and 20x4 LCD display.

## What it does
- Measures pitch, roll and heading in real time
- Displays live values on 20x4 I2C LCD
- Cardinal direction (N/NE/E/SE/S/SW/W/NW) from heading angle
- LED tilt alert when tilt exceeds 30 degrees
- Button toggles between numbers view and bar graph view
- Live telemetry streaming via Serial to Teleplot plotter

## Hardware
- ESP32 Dev Module
- MPU6050 6-axis IMU (I2C)
- 20x4 LCD with I2C backpack
- Push button (GPIO15)
- LED (GPIO14, onboard)

## Libraries
- Adafruit MPU6050
- Adafruit Unified Sensor
- LiquidCrystal I2C

## Architecture
- calibrate() — averages 200 samples at startup to remove sensor bias
- getCardinal() — converts heading degrees to compass direction name
- makeBar() — generates ASCII bar graph for visual tilt display
- loop() — reads sensor, computes angles, updates LCD and serial at 150ms interval
