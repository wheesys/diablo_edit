#!/usr/bin/env python3
"""位级精确 trace：模拟 C++ D2S_Struct::ReadData 的位流消费路径"""
import struct

with open("doc/杜五_v3.1.d2s", "rb") as f:
    data = f.read()

class BitTrace:
    def __init__(self, data):
        self.data = data
        self.byte_pos = 0
        self.bit_pos = 0
        self.log = []

    def _read(self, n):
        result = 0
        for i in range(n):
            if self.byte_pos >= len(self.data):
                raise EOFError(f"EOF")
            byte = self.data[self.byte_pos]
            bit = (byte >> self.bit_pos) & 1
            result |= (bit << i)
            self.bit_pos += 1
            if self.bit_pos >= 8:
                self.bit_pos = 0
                self.byte_pos += 1
        return result

    def read_bits(self, n):
        v = self._read(n)
        self.log.append(f"  bits({n}) = {v} @ byte{self.byte_pos}:{self.bit_pos}")
        return v

    def read_byte(self):
        assert self.bit_pos == 0, f"Need byte alignment, bit_pos={self.bit_pos}"
        v = self.data[self.byte_pos]
        self.byte_pos += 1
        self.log.append(f"  BYTE = 0x{v:02X} @ byte{self.byte_pos}")
        return v

    def read_word(self):
        assert self.bit_pos == 0, f"Need byte alignment"
        v = struct.unpack_from('<H', self.data, self.byte_pos)[0]
        self.byte_pos += 2
        self.log.append(f"  WORD = 0x{v:04X} @ byte{self.byte_pos}")
        return v

    def read_dword(self):
        assert self.bit_pos == 0, f"Need byte alignment"
        v = struct.unpack_from('<I', self.data, self.byte_pos)[0]
        self.byte_pos += 4
        self.log.append(f"  DWORD = 0x{v:08X} @ byte{self.byte_pos}")
        return v

    def read_bytes(self, n):
        assert self.bit_pos == 0, f"Need byte alignment"
        raw = self.data[self.byte_pos:self.byte_pos+n]
        self.byte_pos += n
        self.log.append(f"  BYTES[{n}] @ byte{self.byte_pos} = {raw[:16].hex()}")
        return raw

    def pos(self):
        return f"byte{self.byte_pos}:{self.bit_pos}"

bt = BitTrace(data)

print("=== C++ D2S_Struct::ReadData 位级 trace ===")

# 1. Header
print("\n--- Header ---")
dwMajic = bt.read_dword()
assert dwMajic == 0xAA55AA55, f"Bad magic: 0x{dwMajic:08X}"
dwVersion = bt.read_dword()
print(f"  Version = 0x{dwVersion:08X} ({dwVersion})")
dwSize = bt.read_dword()
offCrc = bt.byte_pos
dwCRC = bt.read_dword()

isV31 = dwVersion >= 0x69
dwWeaponSet = bt.read_dword()
if not isV31:
    bt.read_bytes(16)  # Name[16]

bt.read_byte()  # charType
bt.read_byte()  # charTitle
bt.read_byte()  # unkown1
bt.read_byte()  # charClass
bt.read_word()  # unkown2
bt.read_byte()  # charLevel
bt.read_bytes(5)  # unkown3
bt.read_dword()  # dwTime
bt.read_bytes(8)  # unkown4
bt.read_dword()  # dwSkillKey
bt.read_dword()  # dwLeftSkill1
bt.read_dword()  # dwRightSkill1
bt.read_dword()  # dwLeftSkill2
bt.read_dword()  # dwRightSkill2
bt.read_bytes(44)  # outfit
bt.read_bytes(64)  # colors
bt.read_byte()  # Town
bt.read_dword()  # dwMapSeed
bt.read_word()  # unkown5
bt.read_word()  # bMercDead
bt.read_word()  # unkown6
bt.read_dword()  # dwMercControl
bt.read_word()  # wMercName
bt.read_word()  # wMercType
bt.read_dword()  # dwMercExp
bt.read_dword()  # unkown7
print(f"  After merc fields @ {bt.pos()}")

# NamePTR
if isV31:
    bt.read_bytes(0x94)
elif dwVersion >= 0x62:
    bt.read_bytes(0x3C)
else:
    bt.read_bytes(0x10)

unkown8 = bt.read_bytes(0x90)  # large unknown block
print(f"  After NamePTR + unkown8 @ {bt.pos()}")

if dwVersion >= 0x61 and not isV31:
    bt.read_bytes(4)

# 2. QuestInfo
print(f"\n--- QuestInfo @ {bt.pos()} ---")
qmagic = bt.read_dword()
assert qmagic == 0x216F6F57
bt.read_dword()  # dwActs
bt.read_word()  # wSize
for di in range(3):
    bt.read_bytes(96)
print(f"  After Quests @ {bt.pos()}")

# 3. Waypoints
print(f"\n--- Waypoints @ {bt.pos()} ---")
wmagic = bt.read_word()
assert wmagic == 0x5357
bt.read_word()  # unknown
bt.read_word()  # wSize
if dwVersion >= 0x61 and not isV31:
    bt.read_bytes(4)
for di in range(3):
    bt.read_bytes(24)
print(f"  After Waypoints @ {bt.pos()}")

# 4. NPC bytes
print(f"\n--- NPC @ {bt.pos()} ---")
if isV31:
    curPos = bt.byte_pos
    begin = data[curPos:]
    gf_pos = begin.find(b'gf')
    npcLen = gf_pos if gf_pos != -1 else 0x30
    print(f"  gf_search: curPos={curPos} gf_at_raw_offset={gf_pos} npcLen={npcLen}")
    if npcLen > 0:
        bt.read_bytes(npcLen)
else:
    bt.read_bytes(0x30)
print(f"  After NPC @ {bt.pos()}")

# 5. PlayerStats
print(f"\n--- PlayerStats @ {bt.pos()} ---")
marker = data[bt.byte_pos:bt.byte_pos+2]
print(f"  Marker: {marker.hex()}")
if marker == b'gf':
    bt.read_word()
    while True:
        sid = bt.read_bits(9)
        if sid == 0x1FF:
            break
        bits_map = {0:10,1:10,2:10,3:10,4:10,5:8,6:21,7:21,8:21,9:21,10:21,11:21,12:7,13:32,14:25,15:25}
        n = bits_map.get(sid, 0)
        if n == 0:
            print(f"  UNKNOWN stat id={sid}")
            break
        bt.read_bits(n)
    if bt.bit_pos > 0:
        bt.bit_pos = 0
        bt.byte_pos += 1
print(f"  After Stats @ {bt.pos()}")

# 6. Skills
print(f"\n--- Skills @ {bt.pos()} ---")
marker = data[bt.byte_pos:bt.byte_pos+2]
print(f"  Marker: {marker.hex()}")
if marker == b'if':
    bt.read_word()
    bt.read_bytes(30)
print(f"  After Skills @ {bt.pos()}")

# 7. Char items
print(f"\n--- Char Items @ {bt.pos()} ---")
marker = data[bt.byte_pos:bt.byte_pos+2]
print(f"  Marker: {marker.hex()} (expected JM=4d4a)")
if marker == b'JM':
    bt.read_word()
    nItems = bt.read_word()
    print(f"  nItems = {nItems}")
else:
    print(f"  *** WRONG MARKER ***")
    # Show surrounding context
    print(f"  Context: {data[bt.byte_pos-10:bt.byte_pos+20].hex()}")

print(f"\n=== Expected ===")
print(f"Char items JM at byte 921 (from Python trace)")
print(f"Actual position: byte {bt.byte_pos}")
print(f"Delta: {bt.byte_pos - 921:+d} bytes")
