<#
.SYNOPSIS
    Convert firmware binary or hex to UF2 format for Adafruit LED Glasses Driver nRF52840.

.DESCRIPTION
    Wrapper script for uf2conv.py configured for the Adafruit LED Glasses Driver nRF52840
    with S140 SoftDevice 6.1.1. Uses official Microsoft uf2conv.py from:
    https://github.com/microsoft/uf2/blob/master/utils/uf2conv.py

.PARAMETER InputFile
    Path to the input firmware file (.bin or .hex)

.PARAMETER OutputFile
    Path for the output .uf2 file (optional, defaults to input filename with .uf2 extension)

.PARAMETER BaseAddress
    Base address for BIN files (default: 0x26000 for S140 SoftDevice 6.1.1)

.PARAMETER Deploy
    If specified, flash directly to connected device instead of just converting

.PARAMETER List
    List connected UF2 bootloader devices

.PARAMETER Info
    Display UF2 file header information

.EXAMPLE
    .\create-uf2.ps1 -InputFile firmware.bin
    Converts firmware.bin to firmware.uf2

.EXAMPLE
    .\create-uf2.ps1 -InputFile firmware.hex -OutputFile output.uf2
    Converts firmware.hex to output.uf2

.EXAMPLE
    .\create-uf2.ps1 -InputFile firmware.bin -Deploy
    Converts and flashes directly to connected LED Glasses

.EXAMPLE
    .\create-uf2.ps1 -List
    Lists all connected UF2 bootloader devices

.NOTES
    Board Configuration:
    - Board: Adafruit LED Glasses Driver nRF52840
    - Board-ID: nRF52840-LedGlasses-revA
    - Family ID: 0xADA52840 (Nordic nRF52840)
    - SoftDevice: S140 6.1.1 (reserves 0x00000-0x25FFF = 152KB)
    - Application Base Address: 0x26000

    References:
    - UF2 Bootloader: https://github.com/adafruit/Adafruit_nRF52_Bootloader
    - UF2 Spec: https://github.com/microsoft/uf2
#>

param(
    [Parameter(Position = 0)]
    [string]$InputFile,

    [Parameter()]
    [string]$OutputFile,

    [Parameter()]
    [string]$BaseAddress = "0x26000",

    [Parameter()]
    [switch]$Deploy,

    [Parameter()]
    [switch]$List,

    [Parameter()]
    [switch]$Info,

    [Parameter()]
    [switch]$Wait
)

# Configuration for Adafruit LED Glasses Driver nRF52840
$FAMILY_ID = "0xADA52840"  # Nordic nRF52840 family ID
$SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$UF2CONV = Join-Path $SCRIPT_DIR "uf2conv.py"

# Check if Python is available
$python = $null
foreach ($cmd in @("python3", "python", "py")) {
    try {
        $version = & $cmd --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            $python = $cmd
            break
        }
    }
    catch {
        continue
    }
}

if (-not $python) {
    Write-Error "Python 3 is required but not found. Please install Python 3."
    exit 1
}

# Check if uf2conv.py exists
if (-not (Test-Path $UF2CONV)) {
    Write-Error "uf2conv.py not found at: $UF2CONV"
    exit 1
}

# Build arguments
$args = @()

# Handle -List flag
if ($List) {
    & $python $UF2CONV -l
    exit $LASTEXITCODE
}

# Handle -Info flag
if ($Info) {
    if (-not $InputFile) {
        Write-Error "Input file required for -Info"
        exit 1
    }
    if (-not (Test-Path $InputFile)) {
        Write-Error "Input file not found: $InputFile"
        exit 1
    }
    & $python $UF2CONV $InputFile -i
    exit $LASTEXITCODE
}

# Validate input file for conversion
if (-not $InputFile) {
    Write-Host "Adafruit LED Glasses Driver nRF52840 UF2 Creator" -ForegroundColor Cyan
    Write-Host "================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Usage: .\create-uf2.ps1 [-InputFile] <file> [options]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -InputFile <file>    Input firmware file (.bin or .hex)"
    Write-Host "  -OutputFile <file>   Output UF2 file (optional)"
    Write-Host "  -BaseAddress <addr>  Base address for BIN files (default: 0x26000)"
    Write-Host "  -Deploy              Flash directly to connected device"
    Write-Host "  -Wait                Wait for device before flashing"
    Write-Host "  -List                List connected UF2 devices"
    Write-Host "  -Info                Display UF2 file information"
    Write-Host ""
    Write-Host "Board Configuration:" -ForegroundColor Yellow
    Write-Host "  Family ID:    $FAMILY_ID (Nordic nRF52840)"
    Write-Host "  Base Address: $BaseAddress (after S140 SoftDevice 6.1.1)"
    Write-Host "  Board-ID:     nRF52840-LedGlasses-revA"
    exit 0
}

if (-not (Test-Path $InputFile)) {
    Write-Error "Input file not found: $InputFile"
    exit 1
}

# Determine output filename
if (-not $OutputFile) {
    $OutputFile = [System.IO.Path]::ChangeExtension($InputFile, ".uf2")
}

# Build conversion arguments
$args += $InputFile
$args += "-f"
$args += $FAMILY_ID
$args += "-b"
$args += $BaseAddress
$args += "-o"
$args += $OutputFile

if ($Deploy) {
    # Don't add -c flag, let it flash
    if ($Wait) {
        $args += "-w"
    }
}
else {
    $args += "-c"  # Convert only, don't flash
}

Write-Host "Converting firmware for Adafruit LED Glasses Driver nRF52840..." -ForegroundColor Cyan
Write-Host "  Family ID:    $FAMILY_ID" -ForegroundColor Gray
Write-Host "  Base Address: $BaseAddress" -ForegroundColor Gray
Write-Host "  Input:        $InputFile" -ForegroundColor Gray
Write-Host "  Output:       $OutputFile" -ForegroundColor Gray
Write-Host ""

& $python $UF2CONV @args

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "Success!" -ForegroundColor Green
    if (-not $Deploy) {
        Write-Host "To flash: Copy '$OutputFile' to the LEDGLASSES drive" -ForegroundColor Yellow
    }
}
else {
    Write-Host ""
    Write-Host "Conversion failed!" -ForegroundColor Red
}

exit $LASTEXITCODE
