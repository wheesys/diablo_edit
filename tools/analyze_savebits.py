#!/usr/bin/env python3
"""分析 v105 ItemStatCost.txt 中 Save Param Bits > 0 的 stat (复合属性/np)"""
import sys

def parse_isc_full(path):
    result = {}
    with open(path, 'r', encoding='utf-8') as f:
        header = f.readline().rstrip('\n')
        cols = header.split('\t')
        indices = {}
        for key in ['Stat', '*ID', 'Save Bits', 'Save Add', 'Save Param Bits']:
            if key in cols:
                indices[key] = cols.index(key)

        for line in f:
            line = line.rstrip('\n')
            if not line.strip():
                continue
            fields = line.split('\t')
            if len(fields) < 2:
                continue
            try:
                stat_id = int(fields[indices['*ID']])
            except (ValueError, IndexError, KeyError):
                continue

            def get_int(col_name):
                idx = indices.get(col_name, -1)
                if idx >= 0 and idx < len(fields) and fields[idx].strip():
                    try: return int(fields[idx])
                    except ValueError: pass
                return 0

            result[stat_id] = {
                'name': fields[indices['Stat']].strip() if 'Stat' in indices else '?',
                'save_bits': get_int('Save Bits'),
                'save_add': get_int('Save Add'),
                'save_param_bits': get_int('Save Param Bits'),
            }
    return result

def parse_property(path):
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
            vals = []
            for f_val in fields[2:]:
                try: vals.append(int(f_val) if f_val.strip() else 0)
                except ValueError: vals.append(0)
            default = vals.pop() if vals else 0
            field_list = []
            total_bits = 0
            for i in range(0, len(vals), 4):
                if i + 3 < len(vals) and vals[i] > 0:
                    field_list.append((vals[i], vals[i+1]))
                    total_bits += vals[i]
                elif i + 3 < len(vals):
                    break
            result[stat_id] = {
                'fields': field_list,
                'total_bits': total_bits,
                'default': default,
            }
    return result

def main():
    isc = parse_isc_full('/home/zl/code/diablo_edit/doc/source/txt/3.1/global/excel/itemstatcost.txt')
    prop = parse_property('/home/zl/code/diablo_edit/data/property.txt')

    print("=" * 90)
    print("v105 复合属性 (Save Param Bits > 0)")
    print("=" * 90)
    for stat_id in sorted(isc.keys()):
        info = isc[stat_id]
        spb = info['save_param_bits']
        if spb > 0:
            prop_info = prop.get(stat_id, {})
            prop_fields = prop_info.get('fields', [])
            field_str = ",".join(f"bits={f[0]}" for f in prop_fields) if prop_fields else "(none)"
            print(f"  ID {stat_id:>4}: {info['name']:<35} SaveBits={info['save_bits']:>3} SaveParamBits={spb:>3}  Prop: {field_str}")

    print(f"\n{'='*90}")
    print("需要添加 savebits 的属性 (ISC 有 >0, property.txt 无 fields)")
    print("=" * 90)
    for stat_id in sorted(isc.keys()):
        info = isc[stat_id]
        if info['save_bits'] > 0:
            prop_info = prop.get(stat_id, {})
            prop_fields = prop_info.get('fields', [])
            if not prop_fields:
                print(f"  ID {stat_id:>4}: {info['name']:<40} SaveBits={info['save_bits']:>3} SaveAdd={info['save_add']:>4} SaveParamBits={info['save_param_bits']:>3}")

if __name__ == '__main__':
    main()
