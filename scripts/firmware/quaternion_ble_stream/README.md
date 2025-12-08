# Quaternion BLE Streamer + BNO085 IMU

High-speed quaternion data streaming from BNO085 IMU over Bluetooth Low Energy (BLE) for the Adafruit LED Glasses Driver (nRF52840).

## Features

- **BNO085 9-DoF IMU** - Hardware quaternion fusion at 200Hz
- **Maximum BLE transmission speed** - Streams as fast as BLE allows
- **LED feedback** - Onboard LED toggles on each transmission
- **Bluefruit Connect compatible** - Standard quaternion packet format
- **Statistics reporting** - Prints IMU reads/sec and packets/sec

## Hardware Requirements

- Adafruit LED Glasses Driver - nRF52840 BLE
- Adafruit BNO085 9-DoF IMU

## I2C Wiring

| BNO085 Pin | nRF52840 Pin |
|------------|--------------|
| VIN | 3.3V |
| GND | GND |
| SDA | SDA |
| SCL | SCL |

Default I2C address: `0x4A` (use `0x4B` if DI pin pulled HIGH)

## Required Libraries

Install via Arduino Library Manager:
1. **Adafruit BNO08x** - IMU driver
2. **Adafruit BusIO** - I2C abstraction (dependency)

## Configuration

```cpp
#define LED_PIN           31        // LED GPIO pin
#define BNO085_I2C_ADDR   0x4A      // I2C address
#define REPORT_INTERVAL_US 5000     // 5ms = 200Hz
#define DEVICE_NAME       "QuatStream"
```

## Packet Format

20 bytes per packet:

| Byte(s) | Content |
|---------|---------|
| 0 | `!` (start) |
| 1 | `Q` (quaternion) |
| 2-5 | w (float) |
| 6-9 | x (float) |
| 10-13 | y (float) |
| 14-17 | z (float) |
| 18 | checksum |
| 19 | `\n` |

## Usage

1. Wire BNO085 to nRF52840 via I2C
2. Upload sketch via Arduino IDE
3. Open Serial Monitor at 115200 baud
4. Connect using Bluefruit Connect app or BLE client
5. Quaternion data streams automatically

## Expected Performance

- **IMU**: 200 quaternion reads/second
- **BLE**: 100-200 packets/second
- **Throughput**: 2-4 KB/sec

## Serial Output

```
IMU reads/sec: 200 | BLE packets/sec: 180 | Quat: w=0.707 x=0.000 y=0.707 z=0.000
```
