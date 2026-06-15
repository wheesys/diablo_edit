#!/usr/bin/env python3
"""增强版 D2R v105 存档分析器 — 追踪 JM header、物品位位置、检测偏移"""
import struct
import sys

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
            node = node | (b << bits)
            bits += 1
            if (node, bits) in HUFFMAN_REV:
                return HUFFMAN_REV[(node, bits)]
            if bits > 9:
                raise ValueError(f"Invalid Huffman seq at byte {self.byte_pos}")

    def pos_str(self):
        return f"byte={self.byte_pos} bit={self.bit_pos}"

    def peek_bytes(self, n):
        if self.bit_pos != 0:
            raise ValueError("Can only peek at byte boundary")
        return self.data[self.byte_pos:self.byte_pos + n]


def read_item_header(br):
    """Read item header bits, return (flags, ext, location, bodyloc, col, row, unknown1, storage, code)"""
    start_byte = br.byte_pos
    start_bit = br.bit_pos

    # JM magic (16 bits)
    wMajic = br.read_bits(16)
    if wMajic != 0x4D4A:
        br.byte_pos = start_byte
        br.bit_pos = start_bit

    flags = br.read_bits(32)
    ext = br.read_bits(3)
    location = br.read_bits(3)
    bodyloc = br.read_bits(4)
    col = br.read_bits(4)
    row = br.read_bits(3)
    unknown1 = br.read_bits(1)
    storage = br.read_bits(3)

    # Huffman decode type code
    code = ""
    for _ in range(4):
        ch = br.read_huffman()
        code += ch

    header_bits = (br.byte_pos - start_byte) * 8 - start_bit + br.bit_pos
    return flags, ext, location, bodyloc, col, row, unknown1, storage, code.strip(), header_bits


def trace_jm_list(data, jm_pos, label, parent_count=None):
    """Trace items in a JM list, return next byte position after all items"""
    if parent_count is None:
        parent_count = struct.unpack_from('<H', data, jm_pos + 2)[0]

    print(f"\n{'='*60}")
    print(f"{label}: JM at byte {jm_pos}, parent_count={parent_count}")
    print(f"{'='*60}")

    br = BitReader(data, byte_offset=jm_pos + 4)  # skip "JM" + u16 count

    for item_idx in range(parent_count):
        start_byte = br.byte_pos
        start_bit = br.bit_pos

        try:
            flags, ext, location, bodyloc, col, row, unknown1, storage, code, hdr_bits = read_item_header(br)

            bSimple = (flags >> 21) & 1
            bSocketed = (flags >> 11) & 1
            bIdentified = (flags >> 4) & 1
            bEar = (flags >> 16) & 1

            print(f"\nP[{item_idx}]: code={code!r} simple={bSimple} sock={bSocketed} "
                  f"loc={location} col={col} row={row} stor={storage} "
                  f"@ byte{start_byte}+{start_bit}bit hdr={hdr_bits}bits")

            # Show raw bytes at header start
            raw = data[start_byte:start_byte+16]
            print(f"  raw: {' '.join(f'{b:02x}' for b in raw)}")

            if bEar:
                # Read Ear data
                earClass = br.read_bits(3)
                earLevel = br.read_bits(7)
                print(f"  ear: class={earClass} level={earLevel}")
                br.align_byte()
            elif bSimple:
                # Simple item: may have trailing field
                # In v105, simple items (socket fillers, location=6) have 8-bit trailing field
                br.align_byte()
                end_pos = br.byte_pos
                if location == 6:
                    if end_pos > 0:
                        trailing = data[end_pos - 1]
                        print(f"  simple (socket filler), trailing_byte at byte{end_pos-1}={trailing:#04x}")
                else:
                    print(f"  simple (non-filler), end @ {br.pos_str()}")
            else:
                # Non-simple item
                nGems = br.read_bits(3)
                itemUID = br.read_bits(32)
                ilvl = br.read_bits(7)
                quality = br.read_bits(4)
                varGfx = br.read_bits(1)
                varGfxIdx = br.read_bits(3) if varGfx else None
                hasClass = br.read_bits(1)
                wClass = br.read_bits(11) if hasClass else None

                print(f"  quality={quality} gems={nGems} ilvl={ilvl} varGfx={varGfx}")
                if varGfx:
                    print(f"  varGfxIdx={varGfxIdx}")
                if hasClass:
                    print(f"  class={wClass}")

                # Quality-specific data
                if quality == 1:
                    loQual = br.read_bits(3)
                    print(f"  loQual={loQual}")
                elif quality == 3:
                    hiQual = br.read_bits(3)
                    print(f"  hiQual={hiQual}")
                elif quality == 4:
                    prefix = br.read_bits(11)
                    suffix = br.read_bits(11)
                    print(f"  magic: prefix={prefix} suffix={suffix}")
                elif quality == 5:
                    setID = br.read_bits(12)
                    print(f"  setID={setID}")
                elif quality in (6, 8):
                    name1 = br.read_bits(8)
                    name2 = br.read_bits(8)
                    print(f"  rare: name1={name1} name2={name2}")
                    # affix bits
                    for ai in range(6):
                        br.read_bits(1)  # has prefix
                        br.read_bits(1)  # has suffix
                elif quality == 7:
                    uniID = br.read_bits(12)
                    print(f"  uniqueID={uniID}")

                # Runeword
                bRuneWord = (flags >> 26) & 1
                if bRuneWord:
                    rw = br.read_bits(16)
                    print(f"  runeword=0x{rw:04X}")

                # Personalized
                bPersonalized = (flags >> 24) & 1
                if bPersonalized:
                    print(f"  personalized (name reading NYI)")

                # Timestamp
                hasRand = br.read_bits(1)
                if hasRand:
                    ts1 = br.read_bits(32)
                    ts2 = br.read_bits(32)
                    ts3 = br.read_bits(32)
                    print(f"  timestamps: {ts1:#010x} {ts2:#010x} {ts3:#010x}")

                # Type-specific info: skip (just align)
                # We'll just track to alignment
                br.align_byte()

                # Socket gems
                if nGems > 0:
                    print(f"  -> {nGems} gem(s):")
                    for gi in range(nGems):
                        gem_start = br.byte_pos
                        try:
                            gflags, gext, gloc, gbodyloc, gcol, grow, gunk1, gstor, gcode, ghdr = read_item_header(br)
                            gSimple = (gflags >> 21) & 1
                            br.align_byte()
                            trailing_info = ""
                            if gSimple and gloc == 6 and br.byte_pos > 0:
                                trailing_val = data[br.byte_pos - 1]
                                trailing_info = f" trail={trailing_val:#04x}"
                            gem_end = br.byte_pos
                            print(f"    G[{gi}]: code={gcode!r} simple={gSimple} "
                                  f"loc={gloc} col={gcol} @ byte{gem_start}{trailing_info} "
                                  f"({gem_end - gem_start}B)")
                        except Exception as e:
                            print(f"    G[{gi}]: ERROR: {e}")
                            break

            print(f"  end @ {br.pos_str()}")

        except Exception as e:
            print(f"\nP[{item_idx}]: ERROR: {e} @ {br.pos_str()}")
            print(f"  raw around: {' '.join(f'{b:02x}' for b in data[br.byte_pos:br.byte_pos+20])}")
            break

    return br.byte_pos


def main():
    filepath = sys.argv[1] if len(sys.argv) > 1 else "doc/杜五_v3.1.d2s"
    with open(filepath, "rb") as f:
        data = f.read()

    print(f"File: {filepath} ({len(data)} bytes)")

    # Find all JM markers
    jm_positions = []
    for i in range(len(data) - 1):
        if data[i:i+2] == b'JM':
            count = struct.unpack_from('<H', data, i + 2)[0]
            # Sanity: parent count should be reasonable
            if count < 200:
                jm_positions.append((i, count))

    print(f"\nJM headers found: {len(jm_positions)}")
    for i, (pos, count) in enumerate(jm_positions):
        label = ["CHAR ITEMS", "DEAD BODY", "MERC ITEMS"][i] if i < 3 else f"JM[{i}]"
        print(f"  {label}: byte {pos}, count={count}")

    # Trace character items
    if jm_positions:
        trace_jm_list(data, jm_positions[0][0], "CHARACTER ITEMS", jm_positions[0][1])

    # Trace merc items if present
    if len(jm_positions) >= 3:
        trace_jm_list(data, jm_positions[2][0], "MERC ITEMS", jm_positions[2][1])


if __name__ == "__main__":
    main()
