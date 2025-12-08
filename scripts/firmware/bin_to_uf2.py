#!/usr/bin/env python3
"""Convert binary firmware to UF2 format for Adafruit nRF52840 bootloader."""

import struct
import sys

def bin_to_uf2(bin_path, uf2_path, base_addr):
    with open(bin_path, 'rb') as f:
        bin_data = f.read()
    
    FAMILY_ID = 0xADA52840  # nRF52840
    BLOCK_SIZE = 256
    
    num_blocks = (len(bin_data) + BLOCK_SIZE - 1) // BLOCK_SIZE
    
    with open(uf2_path, 'wb') as f:
        for block_no in range(num_blocks):
            offset = block_no * BLOCK_SIZE
            chunk = bin_data[offset:offset + BLOCK_SIZE]
            chunk = chunk.ljust(BLOCK_SIZE, b'\x00')  # Pad if needed
            
            # UF2 header (32 bytes)
            header = struct.pack('<IIIIIIII',
                0x0A324655,  # Magic 1 "UF2\n"
                0x9E5D5157,  # Magic 2
                0x00002000,  # Flags (familyID present)
                base_addr + offset,  # Target address
                len(chunk),  # Payload size
                block_no,    # Block number
                num_blocks,  # Total blocks
                FAMILY_ID    # Family ID
            )
            
            # UF2 footer (4 bytes)
            footer = struct.pack('<I', 0x0AB16F30)
            
            # Pad data to 476 bytes (512 - 32 header - 4 footer = 476)
            padded_data = chunk + b'\x00' * (476 - len(chunk))
            
            f.write(header + padded_data + footer)
    
    print(f'Created {uf2_path}: {num_blocks} blocks, {len(bin_data)} bytes from 0x{base_addr:08X}')
    
    # Verify the first block
    with open(uf2_path, 'rb') as f:
        block = f.read(512)
        sp = struct.unpack('<I', block[32:36])[0]
        reset = struct.unpack('<I', block[36:40])[0]
        print(f'Vector table check:')
        print(f'  Stack Pointer: 0x{sp:08X}')
        print(f'  Reset Handler: 0x{reset:08X}')

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print(f'Usage: {sys.argv[0]} <input.bin> <output.uf2> <base_address_hex>')
        sys.exit(1)
    
    bin_path = sys.argv[1]
    uf2_path = sys.argv[2]
    base_addr = int(sys.argv[3], 16) if sys.argv[3].startswith('0x') else int(sys.argv[3])
    
    bin_to_uf2(bin_path, uf2_path, base_addr)
