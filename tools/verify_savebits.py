#!/usr/bin/env python3
"""将 property.txt savebits 接入正确的 Python trace，检查是否能复现 C++ 失败"""
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
HUFFMAN_REV = {(code, bits): ch for ch, (code, bits) in HUFFMAN.items()}

class BitReader:
    def __init__(self, data, byte_offset=0):
        self.data = data; self.byte_pos = byte_offset; self.bit_pos = 0
    def read_bits(self, n):
        result = 0
        for i in range(n):
            byte = self.data[self.byte_pos]
            bit = (byte >> self.bit_pos) & 1
            result |= (bit << i)
            self.bit_pos += 1
            if self.bit_pos >= 8: self.bit_pos = 0; self.byte_pos += 1
        return result
    def read_huffman(self):
        node = 0; bits_read = 0
        while True:
            b = self.read_bits(1); node = node | (b << bits_read); bits_read += 1
            if (node, bits_read) in HUFFMAN_REV: return HUFFMAN_REV[(node, bits_read)]
            if bits_read > 9: raise ValueError(f"Invalid Huffman at byte {self.byte_pos}")
    def align_byte(self):
        if self.bit_pos > 0: self.bit_pos = 0; self.byte_pos += 1
    def pos_str(self): return f"byte={self.byte_pos} bit={self.bit_pos}"

def load_props():
    props = {}
    with open("data/property.txt", "r") as f:
        for line in f:
            line = line.strip()
            if not line or line[0] == '*': continue
            parts = line.split()
            if len(parts) < 2: continue
            try:
                pid = int(parts[0]); ver = int(parts[1])
                vals = [int(x) for x in parts[2:]]
                fields = vals[:-1] if vals else []
                bits = []
                for i in range(0, len(fields)-3, 4):
                    if fields[i] > 0: bits.append(fields[i])
                total = sum(bits)
                if pid not in props or ver > props[pid][0]:
                    props[pid] = (ver, bits, total)
            except (ValueError, IndexError): continue
    return props

PROPS = load_props()
print(f"Loaded {len(PROPS)} property definitions\n")

def parse_items(br, nItems, label, indent=""):
    for idx in range(nItems):
        start = br.byte_pos
        flags = br.read_bits(32); br.read_bits(3)
        loc = br.read_bits(3); bodyloc = br.read_bits(4); col = br.read_bits(4)
        row = br.read_bits(3); br.read_bits(1); stor = br.read_bits(3)
        code = ""
        for _ in range(4): code += br.read_huffman()
        code = code.strip()
        bSimple = (flags >> 21) & 1; bEar = (flags >> 16) & 1
        bRuneWord = (flags >> 26) & 1; bPers = (flags >> 24) & 1
        iQuality = -1; prop_count = 0

        if bEar:
            br.read_bits(10)
            for _ in range(0x40):
                if br.read_bits(8) == 0: break
        elif bSimple:
            if code == 'gld': br.read_bits(13)
        else:
            nGems = br.read_bits(3); br.read_bits(32); br.read_bits(7)
            iQuality = br.read_bits(4)
            bVarGfx = br.read_bits(1)
            if bVarGfx: br.read_bits(3)
            bClass = br.read_bits(1)
            if bClass: br.read_bits(11)

            if iQuality == 1: br.read_bits(3)
            elif iQuality == 3: br.read_bits(3)
            elif iQuality == 4: br.read_bits(22)
            elif iQuality == 5: br.read_bits(12)
            elif iQuality in (6,8):
                br.read_bits(16)
                for _ in range(6): br.read_bits(2)
            elif iQuality == 7: br.read_bits(12)

            if bRuneWord: br.read_bits(16)
            if bPers:
                for _ in range(16):
                    if br.read_bits(8) == 0: break

            if code == 'gld': br.read_bits(13)

            bHasRand = br.read_bits(1)
            if bHasRand: br.read_bits(32); br.read_bits(32); br.read_bits(32)

            qtyFlag = br.read_bits(1)
            if qtyFlag: br.read_bits(9)

            prop_count = 0
            while True:
                pid = br.read_bits(9)
                if pid == 0x1FF: break
                if pid in PROPS:
                    total = PROPS[pid][2]
                    if total > 0: br.read_bits(total); prop_count += 1
                    else: print(f"{indent}  WARN: prop {pid} 0bits @ {br.pos_str()}"); return idx
                else:
                    print(f"{indent}  ERROR: unknown prop id={pid} @ {br.pos_str()}")
                    return idx

        br.align_byte()
        nGems_val = 0 if bSimple else (flags & 0x7)
        if nGems_val > 0:
            parse_items(br, nGems_val, "gem", indent+"  ")

        if idx >= 14 and indent == "":
            print(f"{indent}[{idx}] {code} s={bSimple} q={iQuality} p={prop_count} end={br.pos_str()}")

    return nItems

with open("doc/杜五_v3.1.d2s", "rb") as f:
    data = f.read()

br = BitReader(data, byte_offset=921)
br.read_bits(16); nItems = br.read_bits(16)
print(f"Char items: nItems={nItems} start={br.pos_str()}")

try:
    parsed = parse_items(br, nItems, "CHAR")
    print(f"\nParsed {parsed}/{nItems} items")
except Exception as e:
    print(f"\nFAILED: {e} @ {br.pos_str()}")
    import traceback; traceback.print_exc()
