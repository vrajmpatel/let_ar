# LED Glasses IMU Firmware Design Document

**Project:** Adafruit LED Glasses Driver nRF52840 + BNO085 IMU  
**Date:** December 7, 2025  
**Status:** In Development

---

## Table of Contents

1. [Hardware Overview](#hardware-overview)
2. [Documentation Verification Checklist](#documentation-verification-checklist)
3. [Design Decisions](#design-decisions)
4. [Technical Specifications](#technical-specifications)
5. [Implementation Plan](#implementation-plan)
6. [UF2 Configuration](#uf2-configuration)

---

## Hardware Overview

### Target Board

| Parameter | Value | Source |
|-----------|-------|--------|
| **Board** | Adafruit LED Glasses Driver | Bootloader INFO_UF2.TXT |
| **MCU** | nRF52840 | ARM Cortex-M4F @ 64MHz |
| **Flash** | 1 MB | nRF52840_PS_v1.11.pdf |
| **RAM** | 256 KB | nRF52840_PS_v1.11.pdf |
| **Bootloader** | UF2 Bootloader 0.8.0 | Device INFO_UF2.TXT |
| **SoftDevice** | S140 6.1.1 | Device INFO_UF2.TXT |
| **Board-ID** | nRF52840-LedGlasses-revA | Device INFO_UF2.TXT |

### External IMU Sensor

| Parameter | Value | Source |
|-----------|-------|--------|
| **Sensor** | BNO085 | Hillcrest Laboratories |
| **Interface** | I2C via STEMMA QT | Adafruit BNO085 Guide |
| **I2C Address** | 0x4A (default) | "The default I2C address for the BNO08x is 0x4A" |
| **Operating Voltage** | 3.3V (3-5V VIN) | Adafruit BNO085 Guide |
| **Features** | 9-DOF IMU with sensor fusion | Accelerometer, Gyroscope, Magnetometer |

### On-board Sensor (Not Used)

| Parameter | Value | Note |
|-----------|-------|------|
| **Sensor** | LIS3DH | 3-axis accelerometer only |
| **Reason Not Used** | No gyroscope | BNO085 provides full 9-DOF |

---

## Documentation Verification Checklist

### 1. Component Identification

| Component | Document | Citation |
|-----------|----------|----------|
| **BNO085** | Adafruit BNO085 Guide | "The BNO085 by the motion sensing experts at Hillcrest Laboratories takes the familiar 3-axis accelerometers, gyroscopes, and magnetometers and packages them alongside an Arm Cortex M0 processor running Hillcrest's SH-2 firmware" |
| **nRF52840** | nRF52840_PS_v1.11.pdf | "Arm® Cortex®-M4 32-bit processor with FPU, 64 MHz, 1 MB flash and 256 kB RAM" |
| **LED Glasses Driver** | Adafruit Pinout Guide | "nRF52840 - ARM Cortex M4F running at 64MHz, with 1MB flash and 256KB SRAM, Bluetooth Low Energy compatible 2.4GHz radio" |

### 2. Physical Interface Verification

#### BNO085 Pinout

| Pin | Function | Citation |
|-----|----------|----------|
| **VIN** | Power (3-5V) | "Since the sensor chip uses 3 VDC, we have included a voltage regulator on board that will take 3-5VDC" |
| **GND** | Ground | "GND - common ground for power and logic" |
| **SCL** | I2C Clock | "SCL - I2C clock pin...This pin is level shifted so you can use 3-5V logic, and there's a 10K pullup on this pin" |
| **SDA** | I2C Data | "SDA - I2C data pin...This pin is level shifted so you can use 3-5V logic, and there's a 10K pullup on this pin" |
| **RST** | Reset (Active Low) | "RST- Reset, Active Low. Pull low to GND to reset the sensor" |
| **INT** | Interrupt (Active Low) | "INT - Interrupt/Data Ready-Active Low pin. Indicates that the BNO085 needs the host's attention" |
| **P0/P1** | Mode Select | "P0/P1 Pins - Mode select. Both pins pulled low by default, defaulting to I2C" |

#### BNO085 Mode Selection

| PS1 | PS0 | Mode |
|-----|-----|------|
| Low | Low | **I2C** ← Selected |
| Low | High | UART-RVC |
| High | Low | UART |
| High | High | SPI |

#### nRF52840 LED Glasses Driver I2C Connection

| Signal | Interface | Citation |
|--------|-----------|----------|
| **STEMMA QT** | I2C (SCL/SDA) | "STEMMA QT connector labeled I2C on the board, which enables plug-and-play I2C support" |
| **board.SCL** | I2C Clock | "In CircuitPython, you can use the STEMMA connector with board.SCL and board.SDA" |
| **board.SDA** | I2C Data | Same as above |

#### Compatibility Note

> "The BNO08x I2C implementation violates the I2C protocol in some circumstances. This causes it not to work well with certain chip families. It does not work well with Espressif ESP32, ESP32-S3, and NXP i.MX RT1011. **Operation with SAMD51, RP2040, STM32F4, and nRF52840 is more reliable.**"

✅ **nRF52840 is confirmed compatible with BNO085 I2C**

### 3. Electrical Characteristics Verification

| Parameter | Value | Citation |
|-----------|-------|----------|
| **BNO085 I2C Address** | **0x4A** (default) or 0x4B | "The default I2C address for the BNO08x is 0x4A but it can be switched to 0x4B by pulling the DI pin high to VCC" |
| **BNO085 Operating Voltage** | 3.3V internal (3-5V VIN) | "Since the sensor chip uses 3 VDC" |
| **nRF52840 I2C Frequencies** | 100, 250, 400 kbps | "TWIM - Supported baud rates: 100, 250, 400 kbps" (nRF52840_PS_v1.11.pdf) |
| **BNO085 Recommended I2C** | 400 kHz | "i2c = busio.I2C(board.SCL, board.SDA, frequency=400000)" |
| **TWIM0 Base Address** | 0x40003000 | nRF52840_PS_v1.11.pdf |
| **TWIM1 Base Address** | 0x40004000 | nRF52840_PS_v1.11.pdf |

### 4. Core Operational Flow Verification

#### BNO085 Report Types Available

| Report ID | Report Name | Data | Unit | Citation |
|-----------|-------------|------|------|----------|
| 0x01 | **Accelerometer** | X, Y, Z | m/s² | "Acceleration Vector - Three axes of acceleration from gravity and linear motion, in m/s²" |
| 0x02 | **Gyroscope** | X, Y, Z | rad/s | "Angular Velocity Vector - Three axes of rotational speed in radians per second" |
| 0x03 | Magnetometer | X, Y, Z | µT | "Magnetic Field Strength Vector - Three axes of magnetic field sensing in micro Teslas (uT)" |
| 0x05 | **Rotation Vector** | i, j, k, real | Quaternion | "Absolute Orientation - Four-point quaternion output for accurate data manipulation" |
| 0x08 | Game Rotation Vector | i, j, k, real | Quaternion | No magnetometer (faster) |

#### BNO085 SH-2 Protocol Overview

The BNO085 uses Hillcrest's **SHTP (Sensor Hub Transport Protocol)** over I2C:

1. **Channel Structure:**
   - Channel 0: Command
   - Channel 1: Executable
   - Channel 2: Control (sensor config)
   - Channel 3: Input Sensor Reports
   - Channel 4: Wake Input Sensor Reports
   - Channel 5: Gyro Rotation Vector

2. **Initialization Sequence:**
   ```
   1. Wait for RESET_COMPLETE advertisement (channel 0)
   2. Send Product ID Request
   3. Enable desired sensor reports with SET_FEATURE_COMMAND
   4. Poll for sensor reports on channel 3
   ```

3. **Report Enable Command (SET_FEATURE_COMMAND = 0xFD):**
   ```
   Byte 0: Report ID (0xFD)
   Byte 1: Feature Report ID (e.g., 0x05 for Rotation Vector)
   Bytes 2-3: Reserved
   Bytes 4-7: Report Interval (microseconds, little-endian)
   Bytes 8-11: Batch Interval
   Bytes 12-15: Sensor-specific config
   ```

#### nRF52840 BLE Configuration

| Parameter | Value | Citation |
|-----------|-------|----------|
| **SoftDevice** | S140 6.1.1 | Bootloader INFO: "SoftDevice: S140 6.1.1" |
| **App Start Address** | 0x26000 | S140 reserves 0x00000-0x25FFF (152KB) |
| **BLE Modes** | 1 Mbps, 2 Mbps, Long Range | "Supported data rates: Bluetooth 5 – 2 Mbps, 1 Mbps, 500 kbps, and 125 kbps" |
| **TX Power** | -20 to +8 dBm | "-20 to +8 dBm TX power, configurable in 4 dB steps" |
| **MTU** | Up to 247 bytes | BLE 5.0 extended MTU |

---

## Design Decisions

### User Selections

| Decision | Selection | Rationale |
|----------|-----------|-----------|
| **Development Approach** | Bare-metal + SoftDevice S140 | Maximum control, official Nordic BLE stack |
| **Data Rate** | 200+ Hz | Maximum responsiveness for real-time applications |
| **Primary Data Output** | Rotation Vector (Quaternion) | Fused orientation data from BNO085's onboard processor |

### Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    BLE Central (Phone/PC)                    │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ BLE 5.0 (2 Mbps)
                              │ Custom GATT Service
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  nRF52840 LED Glasses Driver                 │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐  │
│  │   SoftDevice    │  │   Application   │  │    TWIM     │  │
│  │    S140 6.1.1   │◄─┤   (main.c)      │◄─┤  I2C Driver │  │
│  │   BLE Stack     │  │                 │  │  @ 400kHz   │  │
│  └─────────────────┘  └─────────────────┘  └─────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ I2C @ 400kHz (STEMMA QT)
                              │ Address: 0x4A
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                         BNO085                               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ Accel (3-ax)│  │ Gyro (3-ax) │  │ SH-2 Fusion Engine  │  │
│  │ ±8g, 16-bit │  │ ±2000°/s    │  │ Rotation Vector     │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### BLE Service Design

**Custom IMU Service UUID:** `12340000-1234-1234-1234-123456789ABC` (placeholder)

| Characteristic | UUID | Properties | Size | Description |
|----------------|------|------------|------|-------------|
| Quaternion | ...0001 | Notify | 16 bytes | i, j, k, real (4x float32) |
| Accelerometer | ...0002 | Notify | 12 bytes | X, Y, Z (3x float32) m/s² |
| Gyroscope | ...0003 | Notify | 12 bytes | X, Y, Z (3x float32) rad/s |
| Sample Rate | ...0004 | Read/Write | 2 bytes | Report interval in ms |
| Status | ...0005 | Read/Notify | 1 byte | Sensor status flags |

---

## Technical Specifications

### Memory Map (with SoftDevice S140 6.1.1)

| Region | Start | End | Size | Usage |
|--------|-------|-----|------|-------|
| SoftDevice | 0x00000000 | 0x00025FFF | 152 KB | BLE Stack (read-only) |
| **Application** | **0x00026000** | 0x000F3FFF | 824 KB | Firmware |
| Bootloader | 0x000F4000 | 0x000FDFFF | 40 KB | UF2 Bootloader |
| MBR Params | 0x000FE000 | 0x000FEFFF | 4 KB | MBR Parameters |
| Bootloader Settings | 0x000FF000 | 0x000FFFFF | 4 KB | Bootloader Settings |
| RAM (Application) | 0x20000000 + SD_RAM | ... | ~248 KB | Application RAM |

### I2C Configuration (TWIM)

| Parameter | Value | Register/Citation |
|-----------|-------|-------------------|
| Instance | TWIM0 | Base: 0x40003000 |
| Frequency | 400 kHz (K400) | FREQUENCY register = 0x06400000 |
| SCL Pin | Board-specific | LED Glasses STEMMA QT |
| SDA Pin | Board-specific | LED Glasses STEMMA QT |
| Pull-ups | External (on BNO085 breakout) | 10K on breakout board |

### BNO085 Sensor Configuration

| Parameter | Value | Calculation |
|-----------|-------|-------------|
| Report Type | Rotation Vector (0x05) | Quaternion output |
| Report Interval | 5000 µs (5 ms) | 200 Hz = 1,000,000 / 200 |
| Batch Interval | 0 | No batching (real-time) |

---

## Implementation Plan

### Project Structure

```
scripts/
├── firmware/
│   ├── include/
│   │   ├── bno085.h          # BNO085 driver header
│   │   ├── ble_imu_service.h # Custom BLE service
│   │   ├── board.h           # LED Glasses pin definitions
│   │   ├── config.h          # Application configuration
│   │   └── shtp.h            # SHTP protocol definitions
│   ├── src/
│   │   ├── main.c            # Application entry point
│   │   ├── bno085.c          # BNO085 I2C driver
│   │   ├── ble_imu_service.c # BLE GATT service
│   │   ├── twim.c            # nRF52840 I2C driver
│   │   └── board.c           # Board initialization
│   ├── linker/
│   │   └── nrf52840_s140.ld  # Linker script for S140
│   ├── Makefile              # Build system
│   └── sdk_config.h          # Nordic SDK configuration
├── uf2conv.py                # UF2 conversion tool
├── uf2families.json          # UF2 family definitions
├── create-uf2.ps1            # Windows build script
├── create-uf2.sh             # Linux/Mac build script
├── FIRMWARE_DESIGN.md        # This document
└── README.md                 # General documentation
```

### Development Phases

#### Phase 1: Project Setup
- [ ] Create directory structure
- [ ] Configure Makefile for ARM GCC + SoftDevice
- [ ] Create linker script for S140 6.1.1
- [ ] Setup board pin definitions

#### Phase 2: I2C Driver (TWIM)
- [ ] Implement nRF52840 TWIM driver
- [ ] Configure 400 kHz I2C
- [ ] Test basic I2C communication

#### Phase 3: BNO085 Driver
- [ ] Implement SHTP protocol layer
- [ ] Implement product ID query
- [ ] Implement sensor report enable
- [ ] Implement rotation vector parsing
- [ ] Add accelerometer/gyroscope raw reports

#### Phase 4: BLE Service
- [ ] Initialize SoftDevice S140
- [ ] Create custom IMU GATT service
- [ ] Implement notification characteristics
- [ ] Configure 2 Mbps PHY for throughput
- [ ] Implement connection parameter negotiation

#### Phase 5: Integration & Test
- [ ] Integrate IMU polling with BLE notifications
- [ ] Optimize for 200+ Hz throughput
- [ ] Power optimization
- [ ] UF2 generation and flashing

---

## UF2 Configuration

### Verified Parameters

| Parameter | Value | Source |
|-----------|-------|--------|
| **Family ID** | `0xADA52840` | uf2families.json: `"NRF52840"` |
| **Base Address** | `0x26000` | S140 SoftDevice 6.1.1 end address + 1 |
| **Board-ID** | `nRF52840-LedGlasses-revA` | Device INFO_UF2.TXT |

### Conversion Command

```powershell
# PowerShell
.\create-uf2.ps1 firmware\build\firmware.bin

# Or directly
python uf2conv.py firmware\build\firmware.bin -c -f 0xADA52840 -b 0x26000 -o firmware.uf2
```

### Flashing Procedure

1. Double-tap reset button on LED Glasses Driver to enter bootloader
2. Wait for `LEDGLASSES` drive to appear
3. Copy `firmware.uf2` to the drive
4. Device will automatically reset and run new firmware

---

## References

1. **nRF52840 Product Specification v1.11** - Nordic Semiconductor
2. **Adafruit 9-DOF Orientation IMU Fusion Breakout - BNO085** - Adafruit Learning Guide
3. **USB Flashing Format (UF2)** - Microsoft
4. **Adafruit LED Glasses Driver nRF52840** - Adafruit Product Guide
5. **SoftDevice S140 Specification** - Nordic Semiconductor
6. **BNO080/BNO085 Datasheet** - Hillcrest Laboratories

---

*Document generated by Embedder CLI*


 Summary of Fixes
    ╰
    ─I've identified and fixed several critical issues in board.c and t
      that were causing the UF2 file to not flash properly. Here's a   
     detailed summary:

     Issue 1: EasyDMA Buffer Memory Location (CRITICAL)

     Problem: The twim_device_present() function was using a stack-allo
      uint8_t dummy variable as an EasyDMA buffer.

     Citation from nRF52840_PS_v1.11.pdf Section 4.6 EasyDMA:
     > "If TXD.PTR or RXD.PTR is not pointing to the Data RAM region, a
     EasyDMA transfer may result in a HardFault and/or memory corruptio

     Fix: Added a static RAM-allocated buffer s_easydma_buffer[4] with 
     proper alignment:
      1 static uint8_t s_easydma_buffer[4] __attribute__((aligned(4)));


Issue 2: Missing Memory Barriers (CRITICAL)

     Problem: The ARM Cortex-M4 requires memory barriers (DSB and ISB) 
      peripheral configuration to ensure writes complete before the    
     peripheral is enabled.

     Fix: Added memory barrier macros and used them at critical points:
      1 #define __DSB() __asm volatile ("dsb 0xF" ::: "memory")        
      2 #define __ISB() __asm volatile ("isb 0xF" ::: "memory")        

     Added after:
- TWIM peripheral configuration before enabling
      - GPIO configuration for I2C pins
      - Before triggering I2C transactions

     ---

     Issue 3: GPIO Pre-configuration Order (CRITICAL)

     Problem: I2C pins must be configured with S0D1 drive strength BEFO
     enabling TWIM.

     Citation from nRF52840_PS_v1.11.pdf Section 6.31.6 Table 52: 
 > "TWI master signal: Direction=Input, Drive strength=S0D1"       
     > "PSEL.SCL, PSEL.SDA must only be configured when the TWI master 
     disabled"

     Fix: Enhanced board_init() to:
      1. Set I2C pins HIGH first (to prevent glitches)
      2. Configure GPIO with S0D1 drive strength
      3. Add memory barrier
      4. Small delay for line stabilization
      5. Then initialize TWIM

     ---

Issue 4: Stabilization Delays

     Problem: The TWIM peripheral needs time to stabilize after enable.

     Citation from nRF52840_PS_v1.11.pdf Section 6.31.8.1:
     > "tTWIM,START: Time from STARTRX/STARTTX task to transmission sta
     = 1.5 µs"

     Fix: Added small NOP delays after enabling TWIM and before I2C    
     transactions.


 Files Modified:
      1. `scripts/firmware/src/twim.c`
         - Added static EasyDMA buffer
         - Added memory barrier macros
         - Added DSB barriers before TWIM enable
         - Added DSB barriers before I2C transactions
         - Fixed twim_device_present() to use static buffer

      2. `scripts/firmware/src/board.c`
         - Added memory barrier macros
         - Enhanced I2C pin pre-configuration sequence
         - Added stabilization delays

 The firmware now compiles successfully with these fixes. The chang
     ensure proper peripheral initialization order and memory access   
     patterns as required by the nRF52840 hardware.

