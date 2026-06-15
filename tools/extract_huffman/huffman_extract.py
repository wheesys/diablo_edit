#!/usr/bin/env python3
"""
从 D2R 3.1 游戏二进制文件中提取 Huffman 编码表。

Huffman 树在 D2Item.cpp 中以预序遍历存储:
  - 0x01 = 内部节点（有左右子节点）
  - character > 1 = 叶节点，存储该字符
  - 0x00 = 空节点（叶节点的子节点终止符）

当前 D2R 2.x 的表包含 37 个字符: 0-9, a-z, 空格
"""

import sys
from collections import Counter


def parse_tree(data, offset):
    """
    从 data[offset] 开始尝试解析 Huffman 树。
    返回 (leaves_set, tree_size) 或 None。
    格式: 1=内部节点, char>1=叶节点(后跟两个0), 0=终止符
    """
    pos = offset
    leaves = set()

    def parse_node():
        nonlocal pos
        if pos >= len(data):
            return False
        c = data[pos]
        pos += 1

        if c == 0:
            return False
        elif c == 1:
            # 内部节点: 必须有两个子节点
            if not parse_node():
                return False
            parse_node()  # 右子树可以为空
            return True
        elif c > 1:
            leaves.add(c)
            # 叶节点后应有两个0终止符
            if pos + 1 < len(data) and data[pos] == 0:
                pos += 1
            if pos < len(data) and data[pos] == 0:
                pos += 1
            return True
        return False

    try:
        if parse_node():
            return leaves, pos - offset
    except (IndexError, RecursionError):
        pass
    return None


def is_valid_charset(leaves):
    """检查是否是有效的物品code字符集: 0-9, a-z, 空格"""
    target = set(b'0123456789abcdefghijklmnopqrstuvwxyz ')
    return leaves == target


def dump_tree_raw(data, offset, max_len=300):
    """导出原始树的字节序列"""
    result = []
    pos = offset

    def parse_node():
        nonlocal pos
        if pos >= len(data) or pos - offset > max_len:
            return
        c = data[pos]
        pos += 1
        if c == 0:
            result.append(0)
        elif c == 1:
            result.append(1)
            parse_node()
            parse_node()
        elif c > 1:
            result.append(c)
            parse_node()  # left null
            parse_node()  # right null

    parse_node()
    return result


def format_cpp_array(tree_bytes):
    """将树的字节序列格式化为C++数组（与D2Item.cpp中相同的格式）"""
    lines = []
    line = []
    for b in tree_bytes:
        if b == 1:
            line.append('1')
        elif b == 0:
            line.append('0')
        elif 0x20 <= b < 0x7f:
            line.append(f"'{chr(b)}'")
        else:
            line.append(f'{b}')

        if len(line) >= 10:
            lines.append('\t' + ',\t'.join(line) + ',')
            line = []

    if line:
        lines.append('\t' + ',\t'.join(line))

    return ',\n'.join(lines)


def search_binary(filepath):
    """在二进制文件中搜索可能的Huffman树结构"""
    with open(filepath, 'rb') as f:
        data = f.read()

    print(f"文件大小: {len(data):,} bytes")

    candidates = []
    # 搜索所有位置 (跳过最后100字节)
    for offset in range(len(data) - 100):
        b = data[offset]
        if b != 1:
            continue

        result = parse_tree(data, offset)
        if result is None:
            continue

        leaves, size = result

        if len(leaves) == 37:
            candidates.append((offset, size, leaves))
        elif 30 <= len(leaves) <= 45:
            candidates.append((offset, size, leaves))

        if len(candidates) > 500:
            break

    return data, candidates


def main():
    target = sys.argv[1] if len(sys.argv) > 1 else 'doc/source/other/3.1/D2R.exe'

    print(f"=== D2R 3.1 Huffman 表提取工具 ===\n")
    print(f"目标: {target}")

    # 搜索
    data, candidates = search_binary(target)
    print(f"扫描完成, 找到 {len(candidates)} 个候选树 (30-45叶节点)")

    # 精确匹配
    perfect = [(off, sz, lv) for off, sz, lv in candidates if is_valid_charset(lv)]

    if perfect:
        print(f"\n🎯 找到 {len(perfect)} 个精确匹配的 Huffman 树!\n")

        for i, (offset, size, leaves) in enumerate(perfect):
            print(f"{'='*65}")
            print(f"候选 {i+1}: offset=0x{offset:X} ({offset}), size={size}")
            print(f"{'='*65}")

            tree_raw = dump_tree_raw(data, offset)
            leaf_count = sum(1 for b in tree_raw if b > 1)
            internal = sum(1 for b in tree_raw if b == 1)
            nulls = sum(1 for b in tree_raw if b == 0)
            leaf_chars = ''.join(chr(b) for b in tree_raw if b > 1)

            print(f"内部节点: {internal}, 叶节点: {leaf_count}, 空节点: {nulls}")
            print(f"字符集: {leaf_chars}")

            # 验证: 构建编码表
            build_codes(tree_raw)

            print(f"\n// C++ 代码 (可直接替换 src/core/D2Item.cpp 中的 HUFFMAN[]):")
            print("const BYTE HUFFMAN[] = {")
            print(format_cpp_array(tree_raw))
            print("};")

            # 同时输出原始字节序列
            print(f"\n// 原始字节 (验证用):")
            hex_str = ', '.join(f'0x{b:02X}' for b in tree_raw)
            for j in range(0, len(hex_str), 120):
                print(f"  {hex_str[j:j+120]}")
    else:
        print(f"\n⚠️  未找到精确匹配的树 (37字符: 0-9, a-z, 空格)")
        print(f"\n最接近的候选 (按叶节点数排序):")

        # 排序并去重
        seen = set()
        uniq = []
        for off, sz, lv in candidates:
            key = (off, sz)
            if key not in seen:
                seen.add(key)
                uniq.append((off, sz, lv))

        uniq.sort(key=lambda c: abs(len(c[2]) - 37))

        for off, sz, lv in uniq[:30]:
            leaf_str = ''.join(chr(b) for b in sorted(lv))
            missing = set(b'0123456789abcdefghijklmnopqrstuvwxyz ') - lv
            extra = lv - set(b'0123456789abcdefghijklmnopqrstuvwxyz ')
            info = ""
            if missing:
                info += f" 缺:{''.join(chr(b) for b in sorted(missing))}"
            if extra:
                info += f" 多:{''.join(chr(b) for b in sorted(extra))}"
            print(f"  offset=0x{off:08X} size={sz:3d} leaves={len(lv):2d}: {leaf_str}{info}")

        # 尝试扩展搜索: 检查游戏的 .rdata 段
        print(f"\n=== 扩展搜索: 仅检查 PE .rdata 段 ===")
        search_rdata_only(target)


def search_rdata_only(filepath):
    """仅搜索PE文件的只读数据段 (.rdata)，这里通常存放常量表"""
    try:
        with open(filepath, 'rb') as f:
            # 读取PE头
            dos_hdr = f.read(64)
            if dos_hdr[0:2] != b'MZ':
                print("不是有效的PE文件")
                return

            pe_offset = int.from_bytes(dos_hdr[0x3C:0x40], 'little')
            f.seek(pe_offset)
            pe_sig = f.read(4)
            if pe_sig != b'PE\0\0':
                print("不是有效的PE文件")
                return

            # COFF头 (20 bytes) + Optional头
            coff = f.read(20)
            num_sections = int.from_bytes(coff[2:4], 'little')
            opt_size = int.from_bytes(coff[16:18], 'little')

            # 跳过optional头
            f.seek(opt_size, 1)

            # 读取section headers
            sections = []
            for _ in range(num_sections):
                sh = f.read(40)
                name = sh[0:8].rstrip(b'\0').decode('ascii', errors='ignore')
                vaddr = int.from_bytes(sh[12:16], 'little')
                vsize = int.from_bytes(sh[8:12], 'little')
                raddr = int.from_bytes(sh[20:24], 'little')
                rsize = int.from_bytes(sh[16:20], 'little')
                sections.append((name, vaddr, vsize, raddr, rsize))

            # 读取整个文件
            f.seek(0)
            all_data = f.read()

            for name, vaddr, vsize, raddr, rsize in sections:
                if name not in ('.rdata', '.data'):
                    continue

                print(f"\n搜索段: {name} (vsize={vsize}, rsize={rsize})")
                seg_data = all_data[raddr:raddr+rsize]

                candidates = []
                for offset in range(len(seg_data) - 100):
                    if seg_data[offset] != 1:
                        continue
                    result = parse_tree(seg_data, offset)
                    if result is None:
                        continue
                    leaves, size = result
                    if 30 <= len(leaves) <= 45:
                        candidates.append((raddr + offset, size, leaves))

                print(f"  找到 {len(candidates)} 个候选")

                for off, sz, lv in candidates:
                    if is_valid_charset(lv):
                        print(f"\n  🎯 精确匹配! offset=0x{off:X} ({off}), size={sz}")
                        tree_raw = dump_tree_raw(all_data, off)
                        leaf_chars = ''.join(chr(b) for b in tree_raw if b > 1)
                        print(f"  字符集: {leaf_chars}")
                        print(f"\n  const BYTE HUFFMAN[] = {{")
                        print(format_cpp_array(tree_raw))
                        print("  };")
                        return

                # 显示最接近的
                candidates.sort(key=lambda c: abs(len(c[2]) - 37))
                for off, sz, lv in candidates[:10]:
                    leaf_str = ''.join(chr(b) for b in sorted(lv))
                    missing = set(b'0123456789abcdefghijklmnopqrstuvwxyz ') - lv
                    info = ""
                    if missing:
                        info += f" 缺:{''.join(chr(b) for b in sorted(missing))}"
                    print(f"    offset=0x{off:08X} sz={sz} n={len(lv)}: {leaf_str}{info}")

    except Exception as e:
        print(f"PE解析错误: {e}")


def build_codes(tree_bytes):
    """从树字节序列构建并打印编码表（验证用）"""
    pos = 0
    codes = {}

    def traverse(depth=0, code=0):
        nonlocal pos
        if pos >= len(tree_bytes):
            return
        c = tree_bytes[pos]
        pos += 1
        if c == 0:
            return
        elif c == 1:
            traverse(depth + 1, code)
            traverse(depth + 1, code + (1 << depth))
        elif c > 1:
            codes[chr(c)] = (code, depth)

    traverse()

    # 打印编码表
    print("\n字符编码表:")
    for ch in sorted(codes.keys()):
        code, bits = codes[ch]
        binary = format(code, f'0{bits}b') if bits > 0 else '0'
        print(f"  '{ch}' => {binary} ({bits} bits)")
    return codes


if __name__ == '__main__':
    main()
