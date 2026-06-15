#!/usr/bin/env python3
"""精确模拟 C++ CD2Item::ReadData 的位消费，使用 property.txt 中的 savebits"""
import struct, sys

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
        self.data = data
        self.byte_pos = byte_offset
        self.bit_pos = 0

    def read_bits(self, n):
        result = 0
        for i in range(n):
            byte = self.data[self.byte_pos]
            bit = (byte >> self.bit_pos) & 1
            result |= (bit << i)
            self.bit_pos += 1
            if self.bit_pos >= 8:
                self.bit_pos = 0
                self.byte_pos += 1
        return result

    def read_huffman(self):
        node = 0; bits_read = 0
        while True:
            b = self.read_bits(1)
            node = node | (b << bits_read)
            bits_read += 1
            if (node, bits_read) in HUFFMAN_REV:
                return HUFFMAN_REV[(node, bits_read)]
            if bits_read > 9:
                raise ValueError(f"Invalid Huffman at byte {self.byte_pos}")

    def align_byte(self):
        if self.bit_pos > 0:
            self.bit_pos = 0
            self.byte_pos += 1

    def pos(self):
        return self.byte_pos + self.bit_pos / 8.0

def load_property_bits():
    with open("data/property.txt", "r") as f:
        content = f.read()
    props = {}
    for line in content.split("\n"):
        line = line.strip()
        if not line or line[0] == '*':
            continue
        parts = line.split("\t")
        if len(parts) < 2:
            continue
        try:
            pid = int(parts[0])
            ver_min = int(parts[1])
            vals = [int(x) for x in parts[2:] if x.strip()]
            fields = vals[:-1] if vals else []
            field_bits = []
            for i in range(0, len(fields) - 3, 4):
                if fields[i] > 0:
                    field_bits.append(fields[i])
            if pid not in props or ver_min > props[pid][0]:
                props[pid] = (ver_min, field_bits, sum(field_bits))
        except (ValueError, IndexError):
            continue
    return props

PROP_BITS = load_property_bits()
print(f"Loaded {len(PROP_BITS)} property definitions")

def get_prop_bits(prop_id):
    if prop_id in PROP_BITS:
        return PROP_BITS[prop_id][2]
    return 0

def read_item(br, item_idx):
    start_pos = br.pos()
    flags = br.read_bits(32)
    ext = br.read_bits(3)
    location = br.read_bits(3)
    bodyloc = br.read_bits(4)
    col = br.read_bits(4)
    row = br.read_bits(3)
    unknown1 = br.read_bits(1)
    storage = br.read_bits(3)

    code = ""
    for _ in range(4):
        code += br.read_huffman()
    code = code.strip()

    bSimple = (flags >> 21) & 1
    bSocketed = (flags >> 11) & 1
    bRuneWord = (flags >> 26) & 1
    bPersonalized = (flags >> 24) & 1
    bEar = (flags >> 16) & 1
    nGems = flags & 0x7  # simplified

    hdr_end = br.pos()
    print(f"P[{item_idx}]: code={code!r} simple={bSimple} sock={bSocketed} gem={nGems}"
          f" @ {start_pos:.3f} hdr_bits={int((hdr_end-start_pos)*8)}")

    if bEar:
        br.read_bits(3)  # earClass
        br.read_bits(7)  # earLevel
        for _ in range(0x40):
            c = br.read_bits(8)
            if c == 0:
                break
    elif bSimple:
        isGold = (code == 'gld')
        if isGold:
            br.read_bits(1)
            br.read_bits(12)
    else:
        nGems = br.read_bits(3)
        dwGUID = br.read_bits(32)
        iDropLevel = br.read_bits(7)
        iQuality = br.read_bits(4)
        bVarGfx = br.read_bits(1)
        if bVarGfx:
            br.read_bits(3)
        bClass = br.read_bits(1)
        if bClass:
            br.read_bits(11)

        if iQuality == 1:
            br.read_bits(3)
        elif iQuality == 3:
            br.read_bits(3)
        elif iQuality == 4:
            br.read_bits(11); br.read_bits(11)
        elif iQuality == 5:
            br.read_bits(12)
        elif iQuality in (6, 8):
            br.read_bits(8); br.read_bits(8)
            for _ in range(6):
                br.read_bits(1); br.read_bits(1)
        elif iQuality == 7:
            br.read_bits(12)

        if bRuneWord:
            br.read_bits(16)
        if bPersonalized:
            for _ in range(16):
                if br.read_bits(8) == 0:
                    break

        isGold = (code == 'gld')
        if isGold:
            br.read_bits(1); br.read_bits(12)

        bHasRand = br.read_bits(1)
        if bHasRand:
            br.read_bits(32); br.read_bits(32); br.read_bits(32)

        # Property list
        prop_count = 0
        props_start = br.pos()
        while True:
            prop_id = br.read_bits(9)
            if prop_id == 0x1FF:
                break
            prop_bits = get_prop_bits(prop_id)
            if prop_bits > 0:
                val = br.read_bits(prop_bits)
                prop_count += 1
                if item_idx <= 20:
                    print(f"    prop: id={prop_id} bits={prop_bits}")
            else:
                print(f"    UNKNOWN PROP ID={prop_id} @ {br.pos():.3f}")
                break

    br.align_byte()
    body_end = br.pos()

    # Socket gems
    if not bSimple and nGems > 0:
        for gi in range(nGems):
            read_item(br, f"{item_idx}.g{gi}")

    return br.pos()

with open("doc/杜五_v3.1.d2s", "rb") as f:
    data = f.read()

br = BitReader(data, byte_offset=925)
for idx in range(37):
    try:
        read_item(br, idx)
    except Exception as e:
        print(f"P[{idx}]: ERROR: {e} @ {br.pos():.3f}")
        import traceback
        traceback.print_exc()
        break
