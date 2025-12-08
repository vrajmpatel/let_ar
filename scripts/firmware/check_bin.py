#!/usr/bin/env python3
import struct
import sys

filename = sys.argv[1] if len(sys.argv) > 1 else 'build/output/blinky.bin'
f = open(filename, 'rb')
d = f.read()
f.close()

print(f'Binary size: {len(d)} bytes')
print()

print('First 16 vectors (at offset 0x0, loaded at 0x26000):')
for i in range(0, min(64, len(d)), 4):
    word = struct.unpack('<I', d[i:i+4])[0]
    idx = i // 4
    names = ['SP', 'Reset', 'NMI', 'HardFault', 'MemManage', 'BusFault', 
             'UsageFault', 'Rsvd', 'Rsvd', 'Rsvd', 'Rsvd', 'SVCall',
             'DebugMon', 'Rsvd', 'PendSV', 'SysTick']
    name = names[idx] if idx < len(names) else f'IRQ{idx-16}'
    print(f'  [{idx:2d}] 0x{word:08X} - {name}')

print()

# Reset handler offset relative to start of binary
reset_vector = struct.unpack('<I', d[4:8])[0]
reset_offset = reset_vector - 0x26000  # Subtract load address
print(f'Reset vector points to: 0x{reset_vector:08X}')
print(f'Binary offset for Reset_Handler: 0x{reset_offset:X} (offset {reset_offset} in binary)')

if reset_offset < len(d):
    print(f'First 16 bytes at Reset_Handler (offset 0x{reset_offset:X}):')
    print(' '.join(f'{b:02X}' for b in d[reset_offset:reset_offset+16]))
else:
    print(f'ERROR: Reset offset 0x{reset_offset:X} is beyond binary size {len(d)}!')
