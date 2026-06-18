# -*- coding: utf-8 -*-
"""
给所有组合记号建立完整的 GPOS mark-to-base 锚点。

HarfBuzz 之类的排版引擎，只要字体里出现了 GPOS 'mark' 特性，就会认定"这个，所以必须完整
字体自己管理组合记号定位"，从而对整份字体关闭它自带的通用兜底定位算法——
不区分这条 'mark' 规则到底覆盖了哪些字形。所以这里的策略是"宁可全覆盖，不要部分覆盖"：把目标范围内能扫到的组合记号一次性全部建好锚点

分类逻辑:
优先用 Unicode canonical combining class（ccc）判断：
    above  : ccc in (230,234,232,228,216,214) —— 贴在字符上方
    below  : ccc in (220,233,202,218)         —— 贴在字符下方
    center : ccc==1，或 General Category 是 Me（enclosing/overlay 类）

如果当前 Python 自带的 unicodedata 版本还不认识这个码位（很新的 Unicode
版本，category 返回 'Cn'），就完全靠几何形状兜底判断：
    - 字形整体压扁、且没有明显探到基线以下 -> above
    - 字形整体压扁、且主要在基线以下 -> below
    - 字形又高又跨基线很多（比如包围型记号）-> center

比如U+20E7（COMBINING ANNUITY SYMBOL）按 ccc 该归 above，但实际画得很大、明显探到基线以下一截，照搬 ccc 分类会导致它被搬到离字符很远的高处，加了这条几何兜底之后才正常。

如果连几何兜底都猜不准，可以在下面 MANUAL_OVERRIDE 里按码位手动指定
"""
import os
import unicodedata

from fontTools.ttLib import TTFont
from fontTools.feaLib.builder import addOpenTypeFeatures

import config
from utils import bbox

ABOVE_GAP = 30
BELOW_GAP = 30

# 基础字符集合：这些记号可以贴在哪些字符上
LATIN_BASE_CODEPOINTS = (
    list(range(0x30, 0x3A)) + list(range(0x41, 0x5B)) + list(range(0x61, 0x7B))
    + [0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
       0x2D, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x5B, 0x5C,
       0x5D, 0x5E, 0x5F, 0x60, 0x7B, 0x7C, 0x7D, 0x7E]
)
CYRILLIC_BASE_CODEPOINTS = list(range(0x400, 0x460))
KANA_BASE_CODEPOINTS = list(range(0x3041, 0x3097)) + list(range(0x30A1, 0x30FB))

# 记号区块：纯记号区块（不管 gc 是什么，区块里只要有字形就当记号处理），未来 Unicode 在这些区块新增字符也能自动适配
PURE_MARK_BLOCKS = [
    (0x0300, 0x036F),  # Combining Diacritical Marks
    (0x1AB0, 0x1AFF),  # Combining Diacritical Marks Extended
    (0x1DC0, 0x1DFF),  # Combining Diacritical Marks Supplement
    (0x20D0, 0x20FF),  # Combining Diacritical Marks for Symbols
    (0xFE20, 0xFE2F),  # Combining Half Marks
]
# 混合区块：里面混了非记号字符，只挑 gc=Mn/Mc/Me 的当记号
MIXED_BLOCKS_LATIN_BASE = []
MIXED_BLOCKS_CYRILLIC_BASE = [
    (0x0483, 0x0489), (0x2DE0, 0x2DFF), (0xA66F, 0xA67D), (0xA69E, 0xA69F),
]
KANA_MARK_BLOCKS = [(0x3099, 0x309A)]

# 明确排除：不可见控制字符 / 变体选择符，没有可见形状，不需要定位
EXCLUDE_CODEPOINTS = {0x034F}
EXCLUDE_CODEPOINTS.update(range(0xFE00, 0xFE10))

# 几何兜底/ccc 都猜不准的极少数情况，按码位手动指定 "ABOVE"/"BELOW"/"CENTER"
MANUAL_OVERRIDE = {
    # 例: 0x20E7: "CENTER",
}


def classify(cp, b):
    if cp in MANUAL_OVERRIDE:
        return MANUAL_OVERRIDE[cp]

    ccc = unicodedata.combining(chr(cp))
    cat = unicodedata.category(chr(cp))

    if cat == "Cn":
        return _classify_by_geometry(b)

    if cat == "Me":
        return "CENTER"
    if ccc in (230, 234, 232, 228, 216, 214):
        if b is not None and b[1] < -50:
            return "CENTER"
        return "ABOVE"
    if ccc in (220, 233, 202, 218):
        return "BELOW"
    if ccc == 1:
        return "CENTER"
    return _classify_by_geometry(b)


def _classify_by_geometry(b):
    """纯几何兜底：ccc/category 都没用上的情况（包括 Python 自带 unicodedata
    还不认识的全新 Unicode 码位）。"""
    if b is None:
        return "ABOVE"
    y_min, y_max = b[1], b[3]
    height = y_max - y_min
    if y_min >= -20:
        return "ABOVE"
    if y_max <= 20:
        return "BELOW"
    # 跨度很大、明显穿过基线很多的，当作包围/居中类记号处理
    return "CENTER"


def collect_marks(font, pure_blocks, mixed_blocks):
    cmap = font.getBestCmap()
    out = []
    for lo, hi in pure_blocks:
        for cp in range(lo, hi + 1):
            if cp in cmap and cp not in EXCLUDE_CODEPOINTS:
                out.append(cp)
    for lo, hi in mixed_blocks:
        for cp in range(lo, hi + 1):
            if cp not in cmap or cp in EXCLUDE_CODEPOINTS:
                continue
            cat = unicodedata.category(chr(cp))
            if cat in ("Mn", "Mc", "Me"):
                out.append(cp)
    return out


def base_anchor_lines(font, codepoints, cap_height):
    cmap = font.getBestCmap()
    gs = font.getGlyphSet()
    above_y, below_y, center_y = cap_height + ABOVE_GAP, -BELOW_GAP, cap_height / 2.0
    lines = {"ABOVE": [], "BELOW": [], "CENTER": []}
    for cp in codepoints:
        name = cmap.get(cp)
        if name is None:
            continue
        b = bbox(gs, name)
        if b is None:
            continue
        cx = round((b[0] + b[2]) / 2.0)
        lines["ABOVE"].append((name, cx, round(above_y)))
        lines["BELOW"].append((name, cx, round(below_y)))
        lines["CENTER"].append((name, cx, round(center_y)))
    return lines


def mark_anchor_lines(font, codepoints):
    cmap = font.getBestCmap()
    gs = font.getGlyphSet()
    lines = {"ABOVE": [], "BELOW": [], "CENTER": []}
    for cp in codepoints:
        name = cmap.get(cp)
        if name is None:
            continue
        b = bbox(gs, name)
        if b is None:
            continue
        cls = classify(cp, b)
        cx = round((b[0] + b[2]) / 2.0)
        if cls == "ABOVE":
            anchor = (cx, round(b[1]))
        elif cls == "BELOW":
            anchor = (cx, round(b[3]))
        else:
            anchor = (cx, round((b[1] + b[3]) / 2.0))
        lines[cls].append((name, anchor))
    return lines


def kana_lines(font):
    cmap = font.getBestCmap()
    gs = font.getGlyphSet()
    base_lines = []
    for cp in KANA_BASE_CODEPOINTS:
        name = cmap.get(cp)
        if name is None:
            continue
        b = bbox(gs, name)
        if b is None:
            continue
        base_lines.append((name, round(b[2] * 0.85), round(b[3])))
    mark_lines = []
    for lo, hi in KANA_MARK_BLOCKS:
        for cp in range(lo, hi + 1):
            name = cmap.get(cp)
            if name is None:
                continue
            b = bbox(gs, name)
            if b is None:
                continue
            mark_lines.append((name, (round(b[0]), round(b[1]))))
    return base_lines, mark_lines


def build_one_weight(weight, input_dir, output_dir):
    in_path = os.path.join(input_dir, weight)
    font = TTFont(in_path, fontNumber=0)
    cap_height = font["OS/2"].sCapHeight

    fea = []

    latin_marks = collect_marks(font, PURE_MARK_BLOCKS, MIXED_BLOCKS_LATIN_BASE)
    cyr_marks = collect_marks(font, [], MIXED_BLOCKS_CYRILLIC_BASE)

    latin_base_lines = base_anchor_lines(font, LATIN_BASE_CODEPOINTS, cap_height)
    latin_mark_lines = mark_anchor_lines(font, latin_marks)
    cyr_base_lines = base_anchor_lines(font, CYRILLIC_BASE_CODEPOINTS, cap_height)
    cyr_mark_lines = mark_anchor_lines(font, cyr_marks)
    kana_base, kana_mark = kana_lines(font)

    for cls in ("ABOVE", "BELOW", "CENTER"):
        for name, anchor in latin_mark_lines[cls]:
            fea.append(f"markClass {name} <anchor {anchor[0]} {anchor[1]}> @LAT_{cls};")
    for cls in ("ABOVE", "BELOW", "CENTER"):
        for name, anchor in cyr_mark_lines[cls]:
            fea.append(f"markClass {name} <anchor {anchor[0]} {anchor[1]}> @CYR_{cls};")
    for name, anchor in kana_mark:
        fea.append(f"markClass {name} <anchor {anchor[0]} {anchor[1]}> @KANA_M;")

    fea.append("")
    fea.append("feature mark {")
    for cls in ("ABOVE", "BELOW", "CENTER"):
        if not latin_mark_lines[cls]:
            continue
        fea.append(f"    lookup LAT_BASE_{cls} {{")
        for name, x, y in latin_base_lines[cls]:
            fea.append(f"        position base {name} <anchor {x} {y}> mark @LAT_{cls};")
        fea.append(f"    }} LAT_BASE_{cls};")
    for cls in ("ABOVE", "BELOW", "CENTER"):
        if not cyr_mark_lines[cls]:
            continue
        fea.append(f"    lookup CYR_BASE_{cls} {{")
        for name, x, y in cyr_base_lines[cls]:
            fea.append(f"        position base {name} <anchor {x} {y}> mark @CYR_{cls};")
        fea.append(f"    }} CYR_BASE_{cls};")
    if kana_mark:
        fea.append("    lookup KANA_BASE {")
        for name, x, y in kana_base:
            fea.append(f"        position base {name} <anchor {x} {y}> mark @KANA_M;")
        fea.append("    } KANA_BASE;")
    fea.append("} mark;")

    fea_text = "\n".join(fea)
    fea_path = os.path.join(input_dir, f"_{weight}.fea")
    with open(fea_path, "w") as fh:
        fh.write(fea_text)

    addOpenTypeFeatures(font, fea_path, tables=["GPOS", "GDEF"])

    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, weight)
    font.save(out_path)
    n_lookups = len(font["GPOS"].table.LookupList.Lookup)
    print(f"  {weight}: GPOS lookups={n_lookups}  latin marks={len(latin_marks)}"
          f"  cyrillic marks={len(cyr_marks)}  kana marks={len(kana_mark)} -> {out_path}")
    return out_path, (latin_marks, cyr_marks, [cp for lo, hi in KANA_MARK_BLOCKS for cp in range(lo, hi + 1)])


def build_all(input_dir=None, output_dir=None):
    input_dir = input_dir or config.MERGED_DIR
    output_dir = output_dir or config.RESULT_DIR
    results = {}
    for weight in config.WEIGHTS:
        print(f"=== {weight} ===")
        results[weight] = build_one_weight(weight, input_dir, output_dir)
    return results


if __name__ == "__main__":
    build_all()
