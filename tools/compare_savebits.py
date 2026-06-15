#!/usr/bin/env python3
"""对比 v105 ItemStatCost.txt 与 property.txt savebits 差异
注意: property.txt 中空制表分隔字段在 C++ 中被解析为 0"""
import sys

def parse_itemstatcost(path):
    result = {}
    with open(path, 'r', encoding='utf-8') as f:
        header = f.readline().rstrip('\n')
        cols = header.split('\t')
        save_bits_idx = cols.index('Save Bits')
        save_add_idx = cols.index('Save Add')

        for line in f:
            line = line.rstrip('\n')
            if not line.strip():
                continue
            fields = line.split('\t')
            if len(fields) < 2:
                continue
            try:
                stat_id = int(fields[1])
            except (ValueError, IndexError):
                continue
            stat_name = fields[0].strip() if len(fields) > 0 else ""

            save_bits = save_add = 0
            if save_bits_idx < len(fields) and fields[save_bits_idx].strip():
                try: save_bits = int(fields[save_bits_idx])
                except ValueError: pass
            if save_add_idx < len(fields) and fields[save_add_idx].strip():
                try: save_add = int(fields[save_add_idx])
                except ValueError: pass

            result[stat_id] = {'save_bits': save_bits, 'save_add': save_add, 'name': stat_name}
    return result

def parse_property(path):
    """解析 property.txt, 模拟 C++ readPropRes() 行为: 空字段 = 0"""
    result = {}
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.rstrip('\n')
            if not line.strip() or line[0] == '*':
                continue
            fields = line.split('\t')
            if len(fields) < 2:
                continue
            try:
                stat_id = int(fields[0])
            except ValueError:
                continue
            # verMin: index 1, 空=0
            verMin = 0
            if len(fields) > 1 and fields[1].strip():
                try: verMin = int(fields[1])
                except ValueError: pass

            # 剩余字段: 模拟 C++ parse() 行为 - 空字符串 → 0
            vals = []
            for f_val in fields[2:]:
                try:
                    vals.append(int(f_val) if f_val.strip() else 0)
                except ValueError:
                    vals.append(0)

            # C++: def = vals.back(); vals.pop_back()
            default = vals.pop() if vals else 0

            # C++: 按4个一组(bits, base, min, max)解析
            field_list = []
            total_bits = 0
            for i in range(0, len(vals), 4):
                if i + 3 < len(vals):
                    bits = vals[i]
                    base = vals[i+1]
                    min_v = vals[i+2]
                    max_v = vals[i+3]
                    if bits > 0:
                        field_list.append((bits, base, min_v, max_v))
                        total_bits += bits
                else:
                    break

            result[stat_id] = {
                'verMin': verMin,
                'fields': field_list,
                'total_bits': total_bits,
                'default': default,
            }
    return result

def main():
    v105_path = sys.argv[1] if len(sys.argv) > 1 else '/home/zl/code/diablo_edit/doc/source/txt/3.1/global/excel/itemstatcost.txt'
    prop_path = sys.argv[2] if len(sys.argv) > 2 else '/home/zl/code/diablo_edit/data/property.txt'

    isc = parse_itemstatcost(v105_path)
    prop = parse_property(prop_path)

    print("=" * 90)
    print("Item Property Savebits 差异 (property.txt 中有定义的 stat)")
    print("=" * 90)

    diffs = []
    for stat_id in sorted(prop.keys()):
        prop_info = prop[stat_id]
        isc_info = isc.get(stat_id, {})

        prop_bits = prop_info['total_bits']
        isc_bits = isc_info.get('save_bits', 0)
        isc_add = isc_info.get('save_add', 0)
        isc_name = isc_info.get('name', '?')

        if prop_bits == 0 and isc_bits == 0:
            continue

        prop_fields = prop_info['fields']
        if not prop_fields and isc_bits == 0:
            continue

        if len(prop_fields) == 1:
            p_bits = prop_fields[0][0]
            p_base = prop_fields[0][1]
            if isc_bits != p_bits or (-isc_add) != p_base:
                diffs.append((stat_id, isc_name, isc_bits, isc_add, f"bits={p_bits} base={p_base}", prop_info['default']))
        elif len(prop_fields) > 1:
            if isc_bits != prop_bits:
                field_str = ",".join(f"({f[0]},{f[1]})" for f in prop_fields)
                diffs.append((stat_id, isc_name, isc_bits, isc_add, f"total={prop_bits} [{field_str}]", prop_info['default']))
        elif isc_bits > 0:
            diffs.append((stat_id, isc_name, isc_bits, isc_add, "(no fields)", 0))

    if diffs:
        print(f"\n{'ID':>4} {'Name':<32} {'ISC_Bits':>8} {'ISC_Add':>8} {'Property_Current':<35} {'Default':>8}")
        print("-" * 105)
        for stat_id, name, isc_bits, isc_add, prop_str, default in diffs:
            print(f"{stat_id:>4} {name:<32} {isc_bits:>8} {isc_add:>8} {prop_str:<35} {default:>8}")
    else:
        print("\n✅ 无差异！property.txt 与 v105 ItemStatCost.txt 完全一致")

    # ISC 中有但 property.txt 中缺失
    missing_in_prop = []
    for stat_id in sorted(isc.keys()):
        if stat_id not in prop and isc[stat_id]['save_bits'] > 0:
            name = isc[stat_id]['name']
            missing_in_prop.append((stat_id, name, isc[stat_id]['save_bits'], isc[stat_id]['save_add']))

    if missing_in_prop:
        print(f"\n--- ISC 中有 savebits 但 property.txt 中缺失 ({len(missing_in_prop)} 个) ---")
        for stat_id, name, bits, add in missing_in_prop:
            print(f"  ID {stat_id}: {name} (SaveBits={bits}, SaveAdd={add})")

    # property.txt 中有但 ISC 中没有
    missing_in_isc = []
    for stat_id in sorted(prop.keys()):
        if stat_id not in isc and prop[stat_id]['total_bits'] > 0:
            missing_in_isc.append(stat_id)

    if missing_in_isc:
        print(f"\n--- property.txt 中有但 ISC 中无 ({len(missing_in_isc)} 个) ---")
        for stat_id in missing_in_isc:
            print(f"  ID {stat_id}: total_bits={prop[stat_id]['total_bits']}")

if __name__ == '__main__':
    main()
