#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
generate_itemdata.py - 为 D2R 3.1 生成新版 itemdata.txt

功能：
  1. 读取现有 data/itemdata.txt，保留全部原有条目和分段注释
  2. 读取 doc/source/txt/3.1/global/excel/armor.txt 和 misc.txt 获取新物品类型信息
  3. 将 33 个新增物品插入到合适的分组位置
  4. 输出新版 itemdata.txt

用法：
  python3 tools/generate_itemdata/generate_itemdata.py [--output <输出路径>]

默认输出到 data/itemdata_31.txt
"""

import os
import sys
import argparse
from typing import List, Tuple, Dict, Optional

# 项目根目录
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# 3.1 源文件路径
ARMOR_TXT_31 = os.path.join(PROJECT_ROOT, "doc", "source", "txt", "3.1", "global", "excel", "armor.txt")
MISC_TXT_31 = os.path.join(PROJECT_ROOT, "doc", "source", "txt", "3.1", "global", "excel", "misc.txt")

# 现有 itemdata.txt
ITEMDATA_TXT = os.path.join(PROJECT_ROOT, "data", "itemdata.txt")

# 默认输出
DEFAULT_OUTPUT = os.path.join(PROJECT_ROOT, "data", "itemdata_31.txt")

# 3.1 新增的物品 code 列表
NEW_CODES_31 = [
    # wa1-waf (15个，armor类型 "grim"=魔法书)
    "wa1", "wa2", "wa3", "wa4", "wa5",
    "wa6", "wa7", "wa8", "wa9", "waa",
    "wab", "wac", "wad", "wae", "waf",
    # ua1-ua5 (5个，quest items)
    "ua1", "ua2", "ua3", "ua4", "ua5",
    # um1-um6 (6个，quest items)
    "um1", "um2", "um3", "um4", "um5", "um6",
    # xa1-xa5 (5个，quest items)
    "xa1", "xa2", "xa3", "xa4", "xa5",
    # cjw (1个，jewel)
    "cjw",
    # cs2 (1个，charm)
    "cs2",
]


def read_tab_file(filepath: str, skip_empty: bool = True) -> List[Dict[str, str]]:
    """
    读取 Tab 分隔的文本文件（带标题行），返回字典列表。
    第一行为标题行，后续行为数据行。
    """
    rows = []
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    if not lines:
        return rows

    # 解析标题行（去除末尾换行符，按 Tab 分割）
    headers = [h.strip() for h in lines[0].rstrip("\n").split("\t")]

    for line in lines[1:]:
        line = line.rstrip("\n")
        if skip_empty and not line.strip():
            continue
        vals = line.split("\t")
        row = {}
        for i, h in enumerate(headers):
            row[h] = vals[i] if i < len(vals) else ""
        rows.append(row)
    return rows


def find_new_items_armor() -> List[Dict]:
    """
    从 3.1 armor.txt 中找出 wa* 系列新物品，返回物品信息列表。
    每条包含: code, name, type, invwidth, invheight
    """
    if not os.path.exists(ARMOR_TXT_31):
        print(f"[警告] 找不到 armor.txt: {ARMOR_TXT_31}")
        return []

    rows = read_tab_file(ARMOR_TXT_31)
    new_items = []
    for row in rows:
        code = row.get("code", "").strip()
        if code in NEW_CODES_31:
            new_items.append({
                "code": code,
                "name": row.get("name", "").strip(),
                "type": row.get("type", "").strip(),
                "invwidth": row.get("invwidth", "0").strip(),
                "invheight": row.get("invheight", "0").strip(),
                "source": "armor.txt",
            })
    return new_items


def find_new_items_misc() -> List[Dict]:
    """
    从 3.1 misc.txt 中找出非 wa* 的新物品，返回物品信息列表。
    每条包含: code, name, type, invwidth, invheight, useable
    """
    if not os.path.exists(MISC_TXT_31):
        print(f"[警告] 找不到 misc.txt: {MISC_TXT_31}")
        return []

    rows = read_tab_file(MISC_TXT_31)
    new_items = []
    for row in rows:
        code = row.get("code", "").strip()
        if code in NEW_CODES_31:
            new_items.append({
                "code": code,
                "name": row.get("name", "").strip(),
                "type": row.get("type", "").strip(),
                "invwidth": row.get("invwidth", "0").strip(),
                "invheight": row.get("invheight", "0").strip(),
                "useable": row.get("useable", "0").strip(),
                "source": "misc.txt",
            })
    return new_items


def build_itemdata_row(code: str, pic: str = "-1", name: str = "",
                       range_val: str = "", equip: str = "",
                       b_simple: str = "", b_normal: str = "", b_white: str = "",
                       b_new: str = "", b_def: str = "", b_dur: str = "",
                       b_stack: str = "", b_monst: str = "", b_charm: str = "",
                       i_spell: str = "", b_unique: str = "", b_craft: str = "",
                       b_gem: str = "", dmg1min: str = "", dmg1max: str = "",
                       dmg2min: str = "", dmg2max: str = "", ipad: str = "",
                       b_pad: str = "") -> str:
    """
    构建一行 itemdata 数据（Tab 分隔，共 24 列）。
    列顺序: code, Pic, Name, Range, Equip, bSimple, bNormal, bWhite, bNew,
            bDef, bDur, bStack, bMonst, bCharm, iSpell, bUnique, bCraft, bGem,
            Dmg1Min, Dmg1Max, Dmg2Min, Dmg2Max, iPadBits, bPad
    """
    fields = [
        code, pic, name, range_val, equip,
        b_simple, b_normal, b_white, b_new,
        b_def, b_dur, b_stack, b_monst, b_charm,
        i_spell, b_unique, b_craft, b_gem,
        dmg1min, dmg1max, dmg2min, dmg2max,
        ipad, b_pad,
    ]
    return "\t".join(fields)


def build_new_items_lines(new_armor_items: List[Dict],
                          new_misc_items: List[Dict]) -> Dict[str, List[str]]:
    """
    根据从源文件读取的物品类型信息，构建各组新物品的 itemdata 行。
    返回 {"<section_key>": ["line1", "line2", ...]} 的字典。
    section_key 用于确定插入位置:
      - "jewel": 珠宝类（插入 jew 之后）
      - "charm": 护身符类（插入 cm3 之后）
      - "grim": 魔法书类（插入 nef 之后）
      - "quest": 任务物品类（插入 fed 之后）
    """
    sections: Dict[str, List[str]] = {
        "jewel": [],
        "charm": [],
        "grim": [],
        "quest": [],
    }

    # --- 处理 armor.txt 中的 wa* 物品 (grim 类型) ---
    for item in new_armor_items:
        code = item["code"]
        item_type = item["type"]
        if item_type == "grim":
            # 魔法书：副手装备，Equip=3, Range=22, bDef=1, bDur=1, bWhite=1
            sections["grim"].append(
                build_itemdata_row(
                    code=code, pic="-1",
                    range_val="22", equip="3",
                    b_normal="1", b_white="1",
                    b_def="1", b_dur="1",
                )
            )
        else:
            print(f"[警告] 未知 armor 类型: {code} -> type={item_type}")

    # --- 处理 misc.txt 中的物品 ---
    for item in new_misc_items:
        code = item["code"]
        item_type = item["type"]

        if item_type == "cjwl":
            # Colossal Jewel: 珠宝类，bGem=1
            sections["jewel"].append(
                build_itemdata_row(code=code, b_gem="1")
            )
        elif item_type == "csch":
            # Crafted Sunder Charm: 护身符类，bCharm=1, Range=13
            sections["charm"].append(
                build_itemdata_row(
                    code=code, range_val="13",
                    b_charm="1",
                )
            )
        elif item_type == "ques":
            # 任务物品：根据 useable 属性设置 iSpell
            useable = item.get("useable", "0")
            if useable == "1":
                # 可使用任务物品（如 xa* Worldstone Shards）
                sections["quest"].append(
                    build_itemdata_row(
                        code=code, pic="-1",
                        b_unique="1",
                    )
                )
            else:
                # 普通任务物品
                sections["quest"].append(
                    build_itemdata_row(
                        code=code, pic="-1",
                    )
                )
        else:
            print(f"[警告] 未知 misc 类型: {code} -> type={item_type}")

    return sections


def generate_itemdata(input_path: str, output_path: str) -> int:
    """
    主生成逻辑：读取原文件，插入新条目，写入输出文件。
    返回新增条目数。
    """
    # 读取原文件
    with open(input_path, "r", encoding="utf-8", errors="replace") as f:
        original_lines = f.readlines()

    # 读取 3.1 源文件
    new_armor = find_new_items_armor()
    new_misc = find_new_items_misc()

    if not new_armor and not new_misc:
        print("[错误] 未从 3.1 源文件中找到任何新物品，请检查文件路径。")
        return 0

    print(f"[信息] 从 armor.txt 读取到 {len(new_armor)} 个新物品")
    for item in new_armor:
        print(f"  {item['code']}: {item['name']} (type={item['type']})")
    print(f"[信息] 从 misc.txt 读取到 {len(new_misc)} 个新物品")
    for item in new_misc:
        print(f"  {item['code']}: {item['name']} (type={item['type']})")

    # 构建新物品行
    new_sections = build_new_items_lines(new_armor, new_misc)

    # 插入位置规则（基于原文件中的 code 匹配）
    # 格式: {"<anchor_code>": ("<section_key>", need_section_header)}
    insertion_rules = [
        ("jew", "jewel", False),    # cjw 跟在 jew 之后
        ("cm3", "charm", False),    # cs2 跟在 cm3 之后
        ("nef", "grim", True),      # wa* 跟在 nef 之后，需要新分段注释
        ("fed", "quest", True),     # quest items 跟在 fed 之后，需要新分段注释
    ]

    # 构建新文件行
    new_lines: List[str] = []
    inserted_sections = set()
    total_inserted = 0

    for line in original_lines:
        new_lines.append(line)

        # 检查当前行是否匹配插入锚点
        stripped = line.rstrip("\n").rstrip("\r")
        # 提取行首的 code（Tab 分隔的第一个字段）
        if stripped and not stripped.startswith("*"):
            first_field = stripped.split("\t")[0].strip()
        else:
            first_field = ""

        for anchor_code, section_key, need_header in insertion_rules:
            if first_field == anchor_code and section_key not in inserted_sections:
                items = new_sections.get(section_key, [])
                if not items:
                    continue
                if need_header:
                    # 添加分段注释行
                    if section_key == "grim":
                        new_lines.append("************魔法书************\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n")
                    elif section_key == "quest":
                        new_lines.append("************3.1任务物品************\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n")
                for item_line in items:
                    new_lines.append(item_line + "\n")
                inserted_sections.add(section_key)
                total_inserted += len(items)
                print(f"[插入] 在 {anchor_code} 之后插入 {len(items)} 个 {section_key} 类物品")

    # 检查是否有未插入的分组
    for section_key, items in new_sections.items():
        if section_key not in inserted_sections and items:
            print(f"[警告] {section_key} 类物品未找到合适的插入位置，追加到文件末尾")
            for item_line in items:
                new_lines.append(item_line + "\n")
            total_inserted += len(items)
            inserted_sections.add(section_key)

    # 写入输出文件
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8", newline="") as f:
        f.writelines(new_lines)

    print(f"\n[完成] 共插入 {total_inserted} 个新物品条目")
    print(f"[输出] {output_path}")
    return total_inserted


def main():
    parser = argparse.ArgumentParser(
        description="为 D2R 3.1 生成新版 itemdata.txt"
    )
    parser.add_argument(
        "--input", "-i",
        default=ITEMDATA_TXT,
        help=f"输入文件路径（默认: {ITEMDATA_TXT}）",
    )
    parser.add_argument(
        "--output", "-o",
        default=DEFAULT_OUTPUT,
        help=f"输出文件路径（默认: {DEFAULT_OUTPUT}）",
    )
    args = parser.parse_args()

    if not os.path.exists(args.input):
        print(f"[错误] 找不到输入文件: {args.input}")
        sys.exit(1)

    count = generate_itemdata(args.input, args.output)
    if count == 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
