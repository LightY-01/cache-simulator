import bz2
import struct
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("trace_path", help="Path to the trace file")
parser.add_argument("binary_out", help="Path to the output binary file")
args = parser.parse_args()

trace_path = args.trace_path
binary_out = args.binary_out

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

