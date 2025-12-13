# Quaternion + Magnetometer + Linear Acceleration BLE Streamer + BNO085 IMU

High-speed quaternion, magnetometer, and linear acceleration data streaming from BNO085 IMU over Bluetooth Low Energy (BLE) for the Adafruit LED Glasses Driver (nRF52840).

## Features

- **BNO085 9-DoF IMU** - Hardware quaternion fusion at 200Hz
- **Calibrated Magnetometer** - 3-axis magnetic field in micro Tesla (µT)
- **Linear Acceleration** - 3-axis acceleration with gravity removed (m/s²)
- **Maximum BLE transmission speed** - Streams as fast as BLE allows
- **LED feedback** - Onboard LED toggles on each transmission
- **Bluefruit Connect compatible** - Standard packet format
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

## Packet Formats

### Quaternion Packet (20 bytes)

| Byte(s) | Content | Description |
|---------|---------|-------------|
| 0 | `!` | Start marker |
| 1 | `Q` | Quaternion identifier |
| 2-5 | w | float (real component) |
| 6-9 | x | float (i component) |
| 10-13 | y | float (j component) |
| 14-17 | z | float (k component) |
| 18 | checksum | ~(sum of bytes 0-17) |
| 19 | `\n` | Newline terminator |

### Magnetometer Packet (16 bytes)

| Byte(s) | Content | Description |
|---------|---------|-------------|
| 0 | `!` | Start marker |
| 1 | `M` | Magnetometer identifier |
| 2-5 | mag_x | float, micro Tesla (µT) |
| 6-9 | mag_y | float, micro Tesla (µT) |
| 10-13 | mag_z | float, micro Tesla (µT) |
| 14 | checksum | ~(sum of bytes 0-13) |
| 15 | `\n` | Newline terminator |

### Linear Acceleration Packet (16 bytes)

| Byte(s) | Content | Description |
|---------|---------|-------------|
| 0 | `!` | Start marker |
| 1 | `A` | Linear acceleration identifier |
| 2-5 | accel_x | float, m/s² |
| 6-9 | accel_y | float, m/s² |
| 10-13 | accel_z | float, m/s² |
| 14 | checksum | ~(sum of bytes 0-13) |
| 15 | `\n` | Newline terminator |

## Sensor Units and Citations

### Magnetometer Units (µT)

The magnetometer values are **already in physical units** (micro Tesla, µT). The BNO085's `SH2_MAGNETIC_FIELD_CALIBRATED` report provides calibrated readings directly - no LSB conversion is required.

**Citation:** Adafruit BNO085 datasheet, Page 31:
> "Magnetic Field Strength Vector / Magnetometer: Three axes of magnetic field sensing in micro Teslas (uT)"

Typical Earth's magnetic field ranges from ~25 µT to ~65 µT depending on location.

### Linear Acceleration Units (m/s²)

The linear acceleration values are in **meters per second squared** (m/s²). The BNO085's `SH2_LINEAR_ACCELERATION` report provides acceleration data with the gravity vector removed through sensor fusion.

**Citation:** Adafruit BNO085 datasheet, Page 6 & 31:
> "Linear Acceleration Vector: Three axes of linear acceleration data (acceleration minus gravity) in m/s^2"

When the device is stationary, linear acceleration should read approximately (0, 0, 0). Motion in any direction will produce non-zero values.

## Usage

1. Wire BNO085 to nRF52840 via I2C
2. Upload sketch via Arduino IDE
3. Open Serial Monitor at 115200 baud
4. Connect using Bluefruit Connect app or BLE client
5. Quaternion, magnetometer, and linear acceleration data streams automatically

## Expected Performance

- **IMU Quaternion**: 200 reads/second
- **Magnetometer**: 200 reads/second
- **Linear Acceleration**: 200 reads/second
- **BLE Quaternion**: 100-200 packets/second
- **BLE Magnetometer**: 100-200 packets/second
- **BLE Linear Accel**: 100-200 packets/second
- **Throughput**: 5-10 KB/sec

## Serial Output

```
Quat reads/sec: 200 | Mag reads/sec: 200 | LinAccel reads/sec: 200 | BLE Quat: 180 | BLE Mag: 180 | BLE Accel: 180 (9360 bytes/sec)
  Quat: w=0.707 x=0.000 y=0.707 z=0.000
  Mag (uT): x=25.50 y=-12.30 z=42.10
  LinAccel (m/s2): x=0.05 y=-0.02 z=0.01
```

## Parsing Example (JavaScript)

```javascript
function parsePacket(data) {
  if (data[0] !== 0x21) return null; // '!' start marker
  
  if (data[1] === 0x51) { // 'Q' quaternion
    const view = new DataView(data.buffer);
    return {
      type: 'quaternion',
      w: view.getFloat32(2, true),
      x: view.getFloat32(6, true),
      y: view.getFloat32(10, true),
      z: view.getFloat32(14, true)
    };
  } else if (data[1] === 0x4D) { // 'M' magnetometer
    const view = new DataView(data.buffer);
    return {
      type: 'magnetometer',
      x: view.getFloat32(2, true),  // µT
      y: view.getFloat32(6, true),  // µT
      z: view.getFloat32(10, true)  // µT
    };
  } else if (data[1] === 0x41) { // 'A' linear acceleration
    const view = new DataView(data.buffer);
    return {
      type: 'linearAccel',
      x: view.getFloat32(2, true),  // m/s²
      y: view.getFloat32(6, true),  // m/s²
      z: view.getFloat32(10, true)  // m/s²
    };
  }
  return null;
}
```
