#!/usr/bin/env python3
"""分析 D2R v105 存档的位级编码，定位解析偏移问题"""
import struct

HUFFMAN = {
    ' ':(1,2), '0':(223,8), '1':(31,7), '2':(12,6), '3':(91,7), '4':(95,8),
    '5':(104,8), '6':(123,7), '7':(30,5), '8':(8,6), '9':(14,5), 'a':(15,5),
    'b':(10,4), 'c':(2,5), 'd':(35,6), 'e':(3,6), 'f':(50,6), 'g':(11,5),
    'h':(24,5), 'i':(63,7), 'j':(232,9), 'k':(18,6), 'l':(23,5), 'm':(22,5),
    'n':(44,6), 'o':(127,7), 'p':(19,5), 'q':(155,8), 'r':(7,5), 's':(4,4),
    't':(6,5), 'u':(16,5), 'v':(59,7), 'w':(0,5), 'x':(28,5), 'y':(40,7),
    'z':(27,8)
}
# Build reverse lookup: (code, bits) -> char
HUFFMAN_REV = {(code, bits): ch for ch, (code, bits) in HUFFMAN.items()}

class BitReader:
    def __init__(self, data, byte_offset=0):
        self.data = data
        self.byte_pos = byte_offset
        self.bit_pos = 0

    def read_bits(self, n):
        result = 0
        for i in range(n):
            if self.byte_pos >= len(self.data):
                raise EOFError(f"EOF at byte {self.byte_pos}")
            byte = self.data[self.byte_pos]
            bit = (byte >> self.bit_pos) & 1
            result |= (bit << i)
            self.bit_pos += 1
            if self.bit_pos >= 8:
                self.bit_pos = 0
                self.byte_pos += 1
        return result

    def align_byte(self):
        if self.bit_pos > 0:
            self.bit_pos = 0
            self.byte_pos += 1

    def read_huffman(self):
        node = 0
        bits = 0
        while True:
            b = self.read_bits(1)
            node = node | (b << bits)  # LSB-first: first bit is LSB of code
            bits += 1
            if (node, bits) in HUFFMAN_REV:
                return HUFFMAN_REV[(node, bits)]
            if bits > 9:
                raise ValueError(f"Invalid Huffman seq at byte {self.byte_pos}")

def analyze_item(br, idx):
    start_byte = br.byte_pos
    start_bit = br.bit_pos

    flags = br.read_bits(32)
    print(f"  after flags: byte={br.byte_pos} bit={br.bit_pos}")
    ext = br.read_bits(3)
    print(f"  after ext: byte={br.byte_pos} bit={br.bit_pos}")
    location = br.read_bits(3)
    print(f"  after loc: byte={br.byte_pos} bit={br.bit_pos}")
    bodyloc = br.read_bits(4)
    print(f"  after bodyloc: byte={br.byte_pos} bit={br.bit_pos}")
    col = br.read_bits(4)
    print(f"  after col: byte={br.byte_pos} bit={br.bit_pos}")
    row = br.read_bits(3)
    print(f"  after row: byte={br.byte_pos} bit={br.bit_pos}")
    unknown1 = br.read_bits(1)
    print(f"  after unk: byte={br.byte_pos} bit={br.bit_pos}")
    storage = br.read_bits(3)
    print(f"  after stor: byte={br.byte_pos} bit={br.bit_pos}")

    print(f"Item {idx}: byte {start_byte}+{start_bit}bit, flags=0x{flags:08X} ext={ext} loc={location} stor={storage}")
    print(f"  bodyloc={bodyloc} col={col} row={row} unk={unknown1}")

    # Show raw header bytes
    raw_start = start_byte
    raw_bytes = list(br.data[raw_start:raw_start+12])
    print(f"  raw: {' '.join(f'{b:02x}' for b in raw_bytes)}")

    code = ""
    for _ in range(4):
        ch = br.read_huffman()
        code += ch
        print(f"  huffman char: {ch!r}")

    print(f"  code={code.strip()!r}")
    return code.strip()

with open("doc/杜五_v3.1.d2s", "rb") as f:
    data = f.read()

# Check merc items area
# Merc JM at 2014, count=9, item data starts at 2018
# C++ shows: item 0 at byte 907, but JM at 2014...
# Wait, the C++ byte positions might be file-absolute, not relative to JM
# Let me check both

# Actually, debug says "ITEM[0/9] byte=907" which is absolute file position
# The save file has data before the JM marker that's part of the bit stream

# Let me check bytes around 907 (merc item 0, hp1)
for pos in [907, 916, 917]:
    br = BitReader(data, byte_offset=pos)
    try:
        code = analyze_item(br, pos)
        print(f"Byte {pos}: {code!r}")
    except Exception as e:
        print(f"Byte {pos}: ERROR: {e}")
    print()
