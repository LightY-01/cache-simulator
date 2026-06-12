import bz2
import struct

trace_path = "Financial1.spc.bz2"
binary_out = "Financial1.bin"

binary_formatter = struct.Struct('<qi?')

with bz2.open(trace_path, mode='rt', encoding='utf-8') as f, open(binary_out, mode='wb') as out_f:
    count = 0
    for line in f:
        line = line.strip()
        if not line:
            continue
        parts = line.split(',')
        if len(parts) < 5:
            continue
        try:
            address = int(parts[1])
            size = int(parts[2])
            is_read = (parts[3].strip().upper() == 'R')
            
            out_f.write(binary_formatter.pack(address, size, is_read))
            count += 1
        except (ValueError, IndexError):
            continue

print(f"Done! Successfully converted {count} requests from {trace_path} into {binary_out}!")

