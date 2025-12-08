#!/bin/bash
#
# Convert firmware binary or hex to UF2 format for Adafruit LED Glasses Driver nRF52840.
#
# Wrapper script for uf2conv.py configured for the Adafruit LED Glasses Driver nRF52840
# with S140 SoftDevice 6.1.1. Uses official Microsoft uf2conv.py from:
# https://github.com/microsoft/uf2/blob/master/utils/uf2conv.py
#
# Board Configuration:
#   - Board: Adafruit LED Glasses Driver nRF52840
#   - Board-ID: nRF52840-LedGlasses-revA
#   - Family ID: 0xADA52840 (Nordic nRF52840)
#   - SoftDevice: S140 6.1.1 (reserves 0x00000-0x25FFF = 152KB)
#   - Application Base Address: 0x26000
#
# References:
#   - UF2 Bootloader: https://github.com/adafruit/Adafruit_nRF52_Bootloader
#   - UF2 Spec: https://github.com/microsoft/uf2

set -e

# Configuration for Adafruit LED Glasses Driver nRF52840
FAMILY_ID="0xADA52840"
DEFAULT_BASE_ADDRESS="0x26000"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UF2CONV="${SCRIPT_DIR}/uf2conv.py"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
GRAY='\033[0;90m'
NC='\033[0m' # No Color

# Print colored message
print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Print usage information
usage() {
    print_color "$CYAN" "Adafruit LED Glasses Driver nRF52840 UF2 Creator"
    print_color "$CYAN" "================================================"
    echo ""
    echo "Usage: $0 [options] <input_file>"
    echo ""
    echo "Options:"
    echo "  -o, --output <file>    Output UF2 file (optional)"
    echo "  -b, --base <address>   Base address for BIN files (default: 0x26000)"
    echo "  -d, --deploy           Flash directly to connected device"
    echo "  -w, --wait             Wait for device before flashing"
    echo "  -l, --list             List connected UF2 devices"
    echo "  -i, --info             Display UF2 file information"
    echo "  -h, --help             Show this help message"
    echo ""
    print_color "$YELLOW" "Board Configuration:"
    echo "  Family ID:    ${FAMILY_ID} (Nordic nRF52840)"
    echo "  Base Address: ${DEFAULT_BASE_ADDRESS} (after S140 SoftDevice 6.1.1)"
    echo "  Board-ID:     nRF52840-LedGlasses-revA"
    echo ""
    echo "Examples:"
    echo "  $0 firmware.bin                    # Convert to firmware.uf2"
    echo "  $0 firmware.hex -o output.uf2      # Convert with custom output name"
    echo "  $0 firmware.bin --deploy           # Convert and flash to device"
    echo "  $0 --list                          # List connected devices"
}

# Find Python 3
find_python() {
    for cmd in python3 python py; do
        if command -v "$cmd" &> /dev/null; then
            if "$cmd" --version 2>&1 | grep -q "Python 3"; then
                echo "$cmd"
                return 0
            fi
        fi
    done
    return 1
}

# Check prerequisites
check_prerequisites() {
    PYTHON=$(find_python) || {
        print_color "$RED" "Error: Python 3 is required but not found. Please install Python 3."
        exit 1
    }

    if [[ ! -f "$UF2CONV" ]]; then
        print_color "$RED" "Error: uf2conv.py not found at: $UF2CONV"
        exit 1
    fi
}

# Parse arguments
INPUT_FILE=""
OUTPUT_FILE=""
BASE_ADDRESS="$DEFAULT_BASE_ADDRESS"
DEPLOY=false
WAIT=false
LIST=false
INFO=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -o|--output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        -b|--base)
            BASE_ADDRESS="$2"
            shift 2
            ;;
        -d|--deploy)
            DEPLOY=true
            shift
            ;;
        -w|--wait)
            WAIT=true
            shift
            ;;
        -l|--list)
            LIST=true
            shift
            ;;
        -i|--info)
            INFO=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        -*)
            print_color "$RED" "Unknown option: $1"
            usage
            exit 1
            ;;
        *)
            INPUT_FILE="$1"
            shift
            ;;
    esac
done

# Main execution
check_prerequisites

# Handle --list flag
if [[ "$LIST" == true ]]; then
    "$PYTHON" "$UF2CONV" -l
    exit $?
fi

# Handle --info flag
if [[ "$INFO" == true ]]; then
    if [[ -z "$INPUT_FILE" ]]; then
        print_color "$RED" "Error: Input file required for --info"
        exit 1
    fi
    if [[ ! -f "$INPUT_FILE" ]]; then
        print_color "$RED" "Error: Input file not found: $INPUT_FILE"
        exit 1
    fi
    "$PYTHON" "$UF2CONV" "$INPUT_FILE" -i
    exit $?
fi

# Check for input file
if [[ -z "$INPUT_FILE" ]]; then
    usage
    exit 0
fi

if [[ ! -f "$INPUT_FILE" ]]; then
    print_color "$RED" "Error: Input file not found: $INPUT_FILE"
    exit 1
fi

# Determine output filename
if [[ -z "$OUTPUT_FILE" ]]; then
    OUTPUT_FILE="${INPUT_FILE%.*}.uf2"
fi

# Build arguments
ARGS=("$INPUT_FILE" "-f" "$FAMILY_ID" "-b" "$BASE_ADDRESS" "-o" "$OUTPUT_FILE")

if [[ "$DEPLOY" == true ]]; then
    # Don't add -c flag, let it flash
    if [[ "$WAIT" == true ]]; then
        ARGS+=("-w")
    fi
else
    ARGS+=("-c")  # Convert only, don't flash
fi

print_color "$CYAN" "Converting firmware for Adafruit LED Glasses Driver nRF52840..."
print_color "$GRAY" "  Family ID:    $FAMILY_ID"
print_color "$GRAY" "  Base Address: $BASE_ADDRESS"
print_color "$GRAY" "  Input:        $INPUT_FILE"
print_color "$GRAY" "  Output:       $OUTPUT_FILE"
echo ""

"$PYTHON" "$UF2CONV" "${ARGS[@]}"
EXIT_CODE=$?

if [[ $EXIT_CODE -eq 0 ]]; then
    echo ""
    print_color "$GREEN" "Success!"
    if [[ "$DEPLOY" != true ]]; then
        print_color "$YELLOW" "To flash: Copy '$OUTPUT_FILE' to the LEDGLASSES drive"
    fi
else
    echo ""
    print_color "$RED" "Conversion failed!"
fi

exit $EXIT_CODE
