# UF2 Firmware Tools for Adafruit LED Glasses Driver nRF52840

Tools for creating and flashing UF2 firmware files to the Adafruit LED Glasses Driver nRF52840.

## Board Configuration

| Parameter | Value | Description |
|-----------|-------|-------------|
| **Board** | Adafruit LED Glasses Driver nRF52840 | Target hardware |
| **Board-ID** | `nRF52840-LedGlasses-revA` | Bootloader identifier |
| **Family ID** | `0xADA52840` | Nordic nRF52840 UF2 family |
| **SoftDevice** | S140 6.1.1 | BLE stack (reserves 0x00000-0x25FFF) |
| **Base Address** | `0x26000` | Application start address |
| **Bootloader** | UF2 Bootloader 0.8.0 | Adafruit nRF52 UF2 bootloader |

## Files

| File | Description |
|------|-------------|
| `uf2conv.py` | Official Microsoft UF2 conversion tool |
| `uf2families.json` | UF2 family ID definitions |
| `create-uf2.ps1` | PowerShell wrapper for Windows |
| `create-uf2.sh` | Bash wrapper for Linux/macOS |

## Prerequisites

- **Python 3.x** - Required for uf2conv.py

## Quick Start

### Windows (PowerShell)

```powershell
# Convert firmware binary to UF2
.\create-uf2.ps1 firmware.bin

# Convert with custom output name
.\create-uf2.ps1 firmware.bin -OutputFile custom.uf2

# Convert Intel HEX file
.\create-uf2.ps1 firmware.hex

# List connected UF2 devices
.\create-uf2.ps1 -List

# Convert and flash directly
.\create-uf2.ps1 firmware.bin -Deploy

# Display UF2 file information
.\create-uf2.ps1 output.uf2 -Info
```

### Linux/macOS (Bash)

```bash
# Make script executable (first time only)
chmod +x create-uf2.sh

# Convert firmware binary to UF2
./create-uf2.sh firmware.bin

# Convert with custom output name
./create-uf2.sh firmware.bin -o custom.uf2

# Convert Intel HEX file
./create-uf2.sh firmware.hex

# List connected UF2 devices
./create-uf2.sh --list

# Convert and flash directly
./create-uf2.sh firmware.bin --deploy

# Display UF2 file information
./create-uf2.sh output.uf2 --info
```

### Direct Python Usage

```bash
# Convert BIN file with nRF52840 family and base address
python uf2conv.py firmware.bin -c -f 0xADA52840 -b 0x26000 -o firmware.uf2

# Convert HEX file (address extracted automatically)
python uf2conv.py firmware.hex -c -f 0xADA52840 -o firmware.uf2

# List connected devices
python uf2conv.py -l

# Flash directly to device
python uf2conv.py firmware.bin -f 0xADA52840 -b 0x26000

# Display UF2 file information
python uf2conv.py firmware.uf2 -i

# Using family name instead of hex ID
python uf2conv.py firmware.bin -c -f NRF52840 -b 0x26000 -o firmware.uf2
```

## Flashing the UF2 File

### Method 1: Drag and Drop

1. **Enter bootloader mode**: Double-tap the reset button on the LED Glasses Driver
2. A USB drive named `LEDGLASSES` should appear
3. Copy the `.uf2` file to the drive
4. The board will automatically reboot and run the new firmware

### Method 2: Direct Flash (via script)

```powershell
# Windows - waits for device then flashes
.\create-uf2.ps1 firmware.bin -Deploy -Wait
```

```bash
# Linux/macOS - waits for device then flashes
./create-uf2.sh firmware.bin --deploy --wait
```

## Memory Map

The S140 SoftDevice 6.1.1 and bootloader define the memory layout:

```
+------------------+ 0x100000 (1MB Flash End)
|    Bootloader    |
+------------------+ 0xF4000
|   Boot Settings  |
+------------------+ 0xF0000
|                  |
|   Application    |
|                  |
+------------------+ 0x26000 <- Application Start (Base Address)
|                  |
|  S140 SoftDevice |
|     (152 KB)     |
|                  |
+------------------+ 0x00000
```

## Troubleshooting

### "No drive to deploy" Error

- Ensure the LED Glasses Driver is in bootloader mode (double-tap reset)
- Check that the `LEDGLASSES` drive is mounted
- On Linux, check mount points in `/media/` or `/run/media/`

### Python Not Found

- Ensure Python 3 is installed and in your PATH
- Try `python3 --version` or `py --version` to verify

### Conversion Fails

- Verify the input file is a valid `.bin` or `.hex` file
- For HEX files, ensure they contain valid Intel HEX format data
- Check that the base address is correct for your firmware

### UF2 Not Recognized by Bootloader

- Verify the family ID is `0xADA52840`
- Ensure the base address is `0x26000` (not conflicting with SoftDevice)
- Check the UF2 file info with `-i` / `--info` flag

## References

- [Adafruit nRF52 Bootloader](https://github.com/adafruit/Adafruit_nRF52_Bootloader)
- [Microsoft UF2 Specification](https://github.com/microsoft/uf2)
- [UF2 Family IDs](https://github.com/microsoft/uf2/blob/master/utils/uf2families.json)
- [Nordic S140 SoftDevice](https://www.nordicsemi.com/Products/Development-software/s140)

## License

- `uf2conv.py` and `uf2families.json` are from [Microsoft UF2](https://github.com/microsoft/uf2) under MIT License
- Wrapper scripts are provided for convenience
