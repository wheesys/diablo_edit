#!/usr/bin/env python3
"""
扩展搜索: 尝试多种格式的 Huffman 表存储方式。
1. 扁平编码表 (bit_length, code 对)
2. Canonical Huffman 表 (bit_length counts + symbols)
3. 在 D2R_loader.dll 中搜索
"""

import sys


def load_file(path):
    with open(path, 'rb') as f:
        return f.read()


def search_flat_codes(data):
    """搜索连续37个目标字符"""
    target_chars = set(b'0123456789abcdefghijklmnopqrstuvwxyz ')
    results = []

    for offset in range(len(data) - 37):
        chunk = set(data[offset:offset+37])
        if chunk == target_chars:
            results.append(offset)
            if len(results) <= 5:
                ctx = ' '.join(f'{b:02X}' for b in data[offset-4:offset+41])
                print(f"  37字符集 @ 0x{offset:08X}: ...{ctx}...")

    return results


def search_canonical(data):
    """
    搜索 canonical Huffman 表格式:
    [count_len1, count_len2, ..., count_lenN][symbol1, symbol2, ..., symbol37]
    其中 sum(counts) == 37
    """
    target = set(b'0123456789abcdefghijklmnopqrstuvwxyz ')
    results = []

    for offset in range(len(data) - 80):
        total = 0
        counts = []

        for i in range(16):
            if offset + i >= len(data):
                break
            c = data[offset + i]
            counts.append(c)
            total += c
            if c > 37:
                break
            if total == 37:
                sym_off = offset + i + 1
                if sym_off + 37 <= len(data):
                    syms = set(data[sym_off:sym_off+37])
                    if syms == target:
                        results.append((offset, counts, list(data[sym_off:sym_off+37])))
                        break
            if total > 37:
                break

    return results


def build_codes(counts, symbols):
    """从 canonical 参数构建编码表"""
    codes = {}
    code = 0
    idx = 0
    for bit_len in range(1, len(counts) + 1):
        cnt = counts[bit_len - 1]
        for _ in range(cnt):
            if idx < len(symbols):
                codes[symbols[idx]] = (code, bit_len)
                idx += 1
                code += 1
        code <<= 1
    return codes


def search_256_table(data):
    """搜索 256 条目编码表 (每个字节是 bit length)"""
    target = set(b'0123456789abcdefghijklmnopqrstuvwxyz ')

    for offset in range(len(data) - 256):
        chunk = data[offset:offset+256]
        # 统计非零条目
        nonzero = [(i, b) for i, b in enumerate(chunk) if 0 < b <= 32]
        if len(nonzero) == 37:
            symbols = {i for i, b in nonzero}
            if symbols == {c for c in target}:
                print(f"  🎯 256表 @ 0x{offset:08X}")
                print(f"  Bit lengths: {dict((chr(k), v) for k, v in nonzero[:10])}...")
                return offset
    return None


def main():
    exe = load_file('doc/source/other/3.1/D2R.exe')
    dll = load_file('doc/source/other/3.1/D2R_loader.dll')

    for name, data in [("D2R.exe", exe), ("D2R_loader.dll", dll)]:
        print(f"\n{'='*60}")
        print(f"搜索: {name} ({len(data):,} bytes)")
        print(f"{'='*60}")

        print("\n[1] 37字符集连续出现:")
        flat = search_flat_codes(data)
        if flat:
            print(f"  共找到 {len(flat)} 处")

        print("\n[2] Canonical Huffman 表 (counts+symbols):")
        canon = search_canonical(data)
        for off, counts, syms in canon:
            print(f"  🎯 offset=0x{off:08X}")
            print(f"  counts={counts}")
            chars = ''.join(chr(b) for b in syms)
            print(f"  symbols={chars}")

            codes = build_codes(counts, syms)
            print(f"  编码表:")
            for ch in sorted(codes):
                code, bits = codes[ch]
                print(f"    '{chr(ch)}' => {code:0{bits}b} ({bits} bits)")

        print("\n[3] 256字节编码表:")
        search_256_table(data)

    # 同时搜索 PE 所有段
    print(f"\n{'='*60}")
    print("搜索 PE 所有段")
    print(f"{'='*60}")
    search_pe_sections('doc/source/other/3.1/D2R.exe')


def search_pe_sections(path):
    with open(path, 'rb') as f:
        data = f.read()

    pe_off = int.from_bytes(data[0x3C:0x40], 'little')
    num_sec = int.from_bytes(data[pe_off+6:pe_off+8], 'little')
    opt_size = int.from_bytes(data[pe_off+20:pe_off+22], 'little')
    sec_off = pe_off + 24 + opt_size

    target = set(b'0123456789abcdefghijklmnopqrstuvwxyz ')

    for i in range(num_sec):
        sh = data[sec_off + i*40 : sec_off + (i+1)*40]
        name = sh[0:8].rstrip(b'\0').decode('ascii', errors='ignore')
        raddr = int.from_bytes(sh[20:24], 'little')
        rsize = int.from_bytes(sh[16:20], 'little')
        seg = data[raddr:raddr+rsize]

        # 搜索 canonical 格式
        for off in range(len(seg) - 80):
            total = 0
            counts = []
            for j in range(16):
                c = seg[off + j]
                counts.append(c)
                total += c
                if c > 37:
                    break
                if total == 37:
                    sym_off = off + j + 1
                    if sym_off + 37 <= len(seg):
                        syms = set(seg[sym_off:sym_off+37])
                        if syms == target:
                            real_off = raddr + off
                            sym_list = list(seg[sym_off:sym_off+37])
                            print(f"🎯 {name} @ 0x{real_off:08X}")
                            print(f"   counts={counts}")
                            print(f"   symbols: {bytes(sym_list).decode()}")
                            codes = build_codes(counts, sym_list)
                            for ch in sorted(codes):
                                code, bits = codes[ch]
                                bstr = format(code, f'0{bits}b')
                                print(f"     '{chr(ch)}' => {bstr} ({bits}b)")
                            return
                if total > 37:
                    break

    print("  未找到")


if __name__ == '__main__':
    main()
