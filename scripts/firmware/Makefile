#------------------------------------------------------------------------------
# Makefile for LED Glasses IMU Firmware
#
# Target: nRF52840 with SoftDevice S140 6.1.1
# Toolchain: ARM GCC
#
# Citations:
# - nRF52840_PS_v1.11.pdf: Target MCU specifications
# - FIRMWARE_DESIGN.md: Project structure and build configuration
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Project Configuration
#------------------------------------------------------------------------------
PROJECT_NAME := led_glasses_imu
VERSION      := 1.0.0

# Output directories
BUILD_DIR    := build
OUTPUT_DIR   := $(BUILD_DIR)/output

#------------------------------------------------------------------------------
# Toolchain Configuration
# Requires ARM GCC toolchain (arm-none-eabi-gcc)
#------------------------------------------------------------------------------
PREFIX       := arm-none-eabi-
CC           := $(PREFIX)gcc
CXX          := $(PREFIX)g++
AS           := $(PREFIX)gcc -x assembler-with-cpp
LD           := $(PREFIX)gcc
OBJCOPY      := $(PREFIX)objcopy
OBJDUMP      := $(PREFIX)objdump
SIZE         := $(PREFIX)size
NM           := $(PREFIX)nm

#------------------------------------------------------------------------------
# Target MCU Configuration
# Citation: nRF52840_PS_v1.11.pdf - ARM Cortex-M4F with FPU
#------------------------------------------------------------------------------
CPU          := cortex-m4
FPU          := fpv4-sp-d16
FLOAT_ABI    := hard

# MCU flags
MCU_FLAGS    := -mcpu=$(CPU) -mthumb -mfloat-abi=$(FLOAT_ABI) -mfpu=$(FPU)

#------------------------------------------------------------------------------
# Source Files
#------------------------------------------------------------------------------

# C source files
C_SOURCES := \
    src/main.c \
    src/board.c \
    src/twim.c \
    src/bno085.c \
    src/softdevice.c \
    src/ble_stack.c \
    src/ble_advertising.c \
    src/ble_imu_service.c

# Assembly startup file (to be created)
ASM_SOURCES := \
    src/startup_nrf52840.s

# Include directories
INCLUDES := \
    -Iinclude

#------------------------------------------------------------------------------
# Linker Configuration
# Citation: FIRMWARE_DESIGN.md - Memory map with SoftDevice S140
#------------------------------------------------------------------------------
LINKER_SCRIPT := linker/nrf52840_s140.ld

#------------------------------------------------------------------------------
# Compiler Flags
#------------------------------------------------------------------------------

# Common flags
COMMON_FLAGS := $(MCU_FLAGS) -Wall -Wextra -ffunction-sections -fdata-sections

# C flags
CFLAGS := $(COMMON_FLAGS) -std=c11
CFLAGS += -DNRF52840_XXAA
CFLAGS += -DBOARD_LEDGLASSES
CFLAGS += -DS140
CFLAGS += -DSOFTDEVICE_PRESENT

# Debug vs Release
ifdef DEBUG
CFLAGS += -O0 -g3 -DDEBUG
else
CFLAGS += -O2 -DNDEBUG
endif

# Assembler flags
ASFLAGS := $(MCU_FLAGS) -g

# Linker flags
LDFLAGS := $(MCU_FLAGS)
LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(OUTPUT_DIR)/$(PROJECT_NAME).map
LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs
LDFLAGS += -lm -lc -lnosys

#------------------------------------------------------------------------------
# Object Files
#------------------------------------------------------------------------------
OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

#------------------------------------------------------------------------------
# Output Files
#------------------------------------------------------------------------------
ELF_FILE := $(OUTPUT_DIR)/$(PROJECT_NAME).elf
BIN_FILE := $(OUTPUT_DIR)/$(PROJECT_NAME).bin
HEX_FILE := $(OUTPUT_DIR)/$(PROJECT_NAME).hex
UF2_FILE := $(OUTPUT_DIR)/$(PROJECT_NAME).uf2

#------------------------------------------------------------------------------
# Build Rules
#------------------------------------------------------------------------------

# Default target
all: $(UF2_FILE)
	@echo "Build complete: $(UF2_FILE)"

# Create build directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(OUTPUT_DIR): | $(BUILD_DIR)
	@mkdir -p $(OUTPUT_DIR)

# Compile C sources
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Assemble startup code
$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) $(ASFLAGS) -c $< -o $@

# Link
$(ELF_FILE): $(OBJECTS) | $(OUTPUT_DIR)
	@echo "LD $(notdir $@)"
	@$(LD) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SIZE) $@

# Create binary
$(BIN_FILE): $(ELF_FILE)
	@echo "OBJCOPY $(notdir $@)"
	@$(OBJCOPY) -O binary $< $@

# Create hex
$(HEX_FILE): $(ELF_FILE)
	@echo "OBJCOPY $(notdir $@)"
	@$(OBJCOPY) -O ihex $< $@

# Create UF2
# Citation: FIRMWARE_DESIGN.md:
#   "Family ID: 0xADA52840"
#   "Base Address: 0x26000"
$(UF2_FILE): $(BIN_FILE)
	@echo "UF2 $(notdir $@)"
	@python ../uf2conv.py $< -c -f 0xADA52840 -b 0x26000 -o $@

#------------------------------------------------------------------------------
# Utility Targets
#------------------------------------------------------------------------------

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)

# Print size information
size: $(ELF_FILE)
	@$(SIZE) --format=berkeley $<

# Disassemble
disasm: $(ELF_FILE)
	@$(OBJDUMP) -d -S $< > $(OUTPUT_DIR)/$(PROJECT_NAME).lst

# Symbol table
symbols: $(ELF_FILE)
	@$(NM) -n $< > $(OUTPUT_DIR)/$(PROJECT_NAME).sym

# Flash via UF2
# Citation: FIRMWARE_DESIGN.md "Flashing Procedure":
#   1. Double-tap reset to enter bootloader
#   2. Copy UF2 to LEDGLASSES drive
flash: $(UF2_FILE)
	@echo "Copy $(UF2_FILE) to LEDGLASSES drive to flash"
ifdef UF2_DRIVE
	@cp $(UF2_FILE) $(UF2_DRIVE)/
	@echo "Flashed to $(UF2_DRIVE)"
else
	@echo "Set UF2_DRIVE environment variable to auto-flash"
	@echo "Example: make flash UF2_DRIVE=/media/LEDGLASSES"
endif

# Help
help:
	@echo "LED Glasses IMU Firmware Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build firmware and UF2 (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  size     - Print memory usage"
	@echo "  disasm   - Generate disassembly listing"
	@echo "  symbols  - Generate symbol table"
	@echo "  flash    - Copy UF2 to device (set UF2_DRIVE)"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1  - Build with debug symbols and no optimization"
	@echo ""
	@echo "Example:"
	@echo "  make clean all DEBUG=1"
	@echo "  make flash UF2_DRIVE=/media/LEDGLASSES"

.PHONY: all clean size disasm symbols flash help
