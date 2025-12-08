#!/usr/bin/env python3
"""Analyze UF2 file structure"""
import struct
import sys

def analyze_uf2(filename):
    with open(filename, 'rb') as f:
        block_num = 0
        all_blocks = []
        while True:
            data = f.read(512)
            if len(data) < 512:
                break
            
            magic0 = struct.unpack('<I', data[0:4])[0]
            magic1 = struct.unpack('<I', data[4:8])[0]
            flags = struct.unpack('<I', data[8:12])[0]
            target_addr = struct.unpack('<I', data[12:16])[0]
            payload_size = struct.unpack('<I', data[16:20])[0]
            block_no = struct.unpack('<I', data[20:24])[0]
            total_blocks = struct.unpack('<I', data[24:28])[0]
            family_id = struct.unpack('<I', data[28:32])[0]
            magic_end = struct.unpack('<I', data[508:512])[0]
            payload = data[32:32+payload_size]
            
            all_blocks.append({
                'target_addr': target_addr,
                'payload_size': payload_size,
                'payload': payload,
                'block_no': block_no
            })
            
            if block_num == 0:
                print(f"=== UF2 File Analysis: {filename} ===")
                print(f"Magic0:       0x{magic0:08X} (expected 0x0A324655)")
                print(f"Magic1:       0x{magic1:08X} (expected 0x9E5D5157)")
                print(f"MagicEnd:     0x{magic_end:08X} (expected 0x0AB16F30)")
                print(f"Flags:        0x{flags:08X}")
                print(f"  - familyID present: {bool(flags & 0x2000)}")
                print(f"Total Blocks: {total_blocks}")
                print(f"Family ID:    0x{family_id:08X}")
                print(f"Payload Size: {payload_size} bytes")
                print()
            
            block_num += 1
        
        print(f"Total blocks read: {block_num}")
        print()
        
        # Show block layout
        print("Block layout:")
        for blk in all_blocks:
            end_addr = blk['target_addr'] + blk['payload_size']
            print(f"  Block {blk['block_no']}: 0x{blk['target_addr']:08X} - 0x{end_addr:08X} ({blk['payload_size']} bytes)")
        print()
        
        # Show vector table from first block (should be at 0x26000)
        first_block = all_blocks[0]
        if first_block['target_addr'] == 0x26000:
            print("Vector table (first 64 bytes):")
            payload = first_block['payload']
            for i in range(0, min(64, len(payload)), 4):
                word = struct.unpack('<I', payload[i:i+4])[0]
                idx = i // 4
                names = ['SP', 'Reset', 'NMI', 'HardFault', 'MemManage', 'BusFault', 
                         'UsageFault', 'Rsvd', 'Rsvd', 'Rsvd', 'Rsvd', 'SVCall',
                         'DebugMon', 'Rsvd', 'PendSV', 'SysTick']
                name = names[idx] if idx < len(names) else f'IRQ{idx-16}'
                print(f"  [{idx:2d}] 0x{word:08X} - {name}")
        
        # Check for gaps
        print("\nChecking for address coverage:")
        sorted_blocks = sorted(all_blocks, key=lambda x: x['target_addr'])
        for i, blk in enumerate(sorted_blocks):
            end = blk['target_addr'] + blk['payload_size']
            if i < len(sorted_blocks) - 1:
                next_start = sorted_blocks[i+1]['target_addr']
                if next_start != end:
                    gap = next_start - end
                    if gap > 0:
                        print(f"  GAP: 0x{end:08X} - 0x{next_start:08X} ({gap} bytes)")
                    else:
                        print(f"  OVERLAP: at 0x{next_start:08X}")

if __name__ == '__main__':
    filename = sys.argv[1] if len(sys.argv) > 1 else 'build/output/blinky.uf2'
    analyze_uf2(filename)
