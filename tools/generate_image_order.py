#!/usr/bin/env python3
"""
generate_image_order.py — 从 MFC RC 文件和 D2R 数据文件生成 image_order.txt

用法：python3 generate_image_order.py

输入：
    archive/.../DiabloEdit2.rc   — 位图资源顺序
    doc/source/.../weapons.txt    — 武器 code<->name
    doc/source/.../armor.txt      — 防具 code<->name
    doc/source/.../misc.txt       — 杂项 code<->name

输出：
    data/image_order.txt — "code<TAB>path" 格式，323行
"""

import re
import os
import sys

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

RC_FILE = os.path.join(PROJECT_ROOT, "archive", "mfc_original",
                       "Diablo Edit2", "DiabloEdit2.rc")
WEAPONS_TXT = os.path.join(PROJECT_ROOT, "doc", "source", "txt",
                           "3.1", "global", "excel", "weapons.txt")
ARMOR_TXT = os.path.join(PROJECT_ROOT, "doc", "source", "txt",
                         "3.1", "global", "excel", "armor.txt")
MISC_TXT = os.path.join(PROJECT_ROOT, "doc", "source", "txt",
                        "3.1", "global", "excel", "misc.txt")
OUTPUT_FILE = os.path.join(PROJECT_ROOT, "data", "image_order.txt")


def parse_rc_file(path):
    """解析 RC 文件，返回按 index 排序的 [(index, relative_path), ...]"""
    entries = []
    # RC 文件可能是 UTF-16 LE 编码（Windows Visual Studio 生成）
    with open(path, "r", encoding="utf-16", errors="replace") as f:
        for line in f:
            # 统一处理路径分隔符
            line = line.replace("\\\\", "/")
            m = re.match(
                r'\s*IDB_BITMAP(\d+)\s+BITMAP\s+"Pictcures/(.+)"',
                line.strip()
            )
            if m:
                idx = int(m.group(1))
                rel_path = m.group(2)
                entries.append((idx, rel_path))
    entries.sort(key=lambda x: x[0])
    return entries


def parse_d2r_txt(path):
    """解析 D2R txt 文件，返回 {name: code} 映射"""
    name_to_code = {}
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        header = f.readline().strip()
        columns = header.split("\t")
        # 查找 code 和 name 列
        code_idx = name_idx = None
        for i, col in enumerate(columns):
            if col == "code":
                code_idx = i
            elif col == "name":
                name_idx = i
        if code_idx is None or name_idx is None:
            print(f"  WARNING: code/name columns not found in {path}")
            print(f"    Columns: {columns[:10]}...")
            return name_to_code
        for line in f:
            line = line.strip()
            if not line or line.startswith("*"):
                continue
            parts = line.split("\t")
            if len(parts) <= max(code_idx, name_idx):
                continue
            code = parts[code_idx].strip()
            name = parts[name_idx].strip()
            if code and name:
                name_to_code[name] = code
    return name_to_code


def normalize(name):
    """规范化用于匹配：去空格、小写"""
    return name.lower().replace(" ", "").replace("'", "")


# 手动映射表：处理拼写差异、缩写和 D2R 特有物品
MANUAL_MAP = {
    # 拼写差异
    "Hpo": "hpo", "Mpo": "mpo", "Hpf": "hpf", "Mpf": "mpf",
    "Reiuvenation Potion": "rvs",
    "Full Reiuvenation Potion": "rvl",
    "Top of the Horadric Staff": "vip",
    "Scroll of Jnifuss": "jnf",
    "a Jade Figuring": "jnd",
    "Flawed Saphire": "gss",
    "Flawled Ruby": "gpr",
    "Field Mail": "xar",
    "Gaint Axe": "gix",
    "Sabre": "sbr",
    "Tow Handed Sword": "2hs",
    "Kris": "kri",
    "Ganrled Staff": "gst",
    "Khalim's Ear": "rin",
    # 缩写
    "Rps": "rps", "Rpl": "rpl",
    "Bps": "bps", "Bpl": "bpl",
    # 任务物品 / 骨骼部件
    "Skeleton Heart": "hrt", "Skeleton Brain": "brz",
    "Skeleton Jawbone": "jaw", "Skeleton Eye": "eyz",
    "Skeleton Horn": "hrn", "Skeleton Tail": "tal",
    "Skeleton Flag": "flg", "Skeleton Fang": "fng",
    "Skeleton Quill": "qll", "Skeleton Soul": "sol",
    "Skeleton Scalp": "scz", "Skeleton Spleen": "spe",
    # D2R 特有精华
    "Twisted Essence of Destruction": "fed",
    "Twisted Essence of Terror": "bet",
    "Twisted Essence of Hatred": "ceh",
    "Twisted Essence of Suffering": "tes",
}


def match_entries(rc_entries, name_to_code):
    """将 RC 条目匹配到 D2R item code。"""
    results = []
    unmatched = []

    for idx, path in rc_entries:
        filename = os.path.basename(path)
        name_no_ext = os.path.splitext(filename)[0]
        clean_name = re.sub(r'^\d+\s*', '', name_no_ext).strip()

        # 1. 手动映射优先
        code = MANUAL_MAP.get(clean_name)
        # 2. D2R txt 直接匹配
        if code is None:
            code = name_to_code.get(clean_name)
        # 3. 模糊匹配
        if code is None:
            norm = normalize(clean_name)
            for d2r_name, d2r_code in name_to_code.items():
                if normalize(d2r_name) == norm:
                    code = d2r_code
                    break

        if code:
            results.append((code, path))
        else:
            results.append(("????", path))
            unmatched.append((idx, path, clean_name))

    return results, unmatched


def main():
    print("=== generate_image_order.py ===")
    print()

    # 1. 解析 RC 文件
    print(f"1. Parsing RC: {RC_FILE}")
    rc_entries = parse_rc_file(RC_FILE)
    print(f"   Got {len(rc_entries)} entries")
    if len(rc_entries) != 323:
        print(f"   WARNING: expected 323, got {len(rc_entries)}")

    # 2. 解析 D2R txt 文件
    name_to_code = {}
    for path, label in [(WEAPONS_TXT, "weapons"),
                         (ARMOR_TXT, "armor"),
                         (MISC_TXT, "misc")]:
        print(f"2. Parsing {label}: {path}")
        try:
            m = parse_d2r_txt(path)
            print(f"   {len(m)} name->code mappings")
            name_to_code.update(m)
        except FileNotFoundError:
            print(f"   SKIP: file not found")
    print(f"   Total: {len(name_to_code)} unique codes")

    # 3. 匹配
    print("3. Matching RC entries to D2R codes...")
    results, unmatched = match_entries(rc_entries, name_to_code)
    matched = len(results) - len(unmatched)
    print(f"   Matched: {matched}/{len(results)}")
    if unmatched:
        print(f"   Unmatched ({len(unmatched)}):")
        for idx, path, name in unmatched:
            print(f"     [{idx}] {path}  name='{name}'")

    # 4. 验证 toa
    # 4. 验证 toa
    toa_found = [(c, path) for c, path in results if c == "toa"]
    if toa_found:
        toa_idx = results.index(toa_found[0])
        print(f"   'toa' -> index {toa_idx} (expected 318)")
    else:
        print("   WARNING: 'toa' not matched!")

    # 5. 输出
    print(f"\n4. Writing: {OUTPUT_FILE}")
    os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)
    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        for code, path in results:
            f.write(f"{code}\t{path}\n")
    print("   Done!")

    # 6. 摘要
    unique_codes = {c for c, _ in results if c != "????"}
    print(f"\nSummary: {len(unique_codes)} unique codes -> {len(results)} images")
    return 0 if not unmatched else 1


if __name__ == "__main__":
    sys.exit(main())
