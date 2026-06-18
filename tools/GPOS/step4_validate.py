# -*- coding: utf-8 -*-
import os

from fontTools.ttLib import TTFont

import config
from utils import tables_identical
from step3_build_gpos import (
    PURE_MARK_BLOCKS, MIXED_BLOCKS_LATIN_BASE, MIXED_BLOCKS_CYRILLIC_BASE,
    KANA_MARK_BLOCKS, EXCLUDE_CODEPOINTS, collect_marks,
)


def covered_mark_names(font):
    covered = set()
    if "GPOS" not in font:
        return covered
    gpos = font["GPOS"].table
    for lookup in gpos.LookupList.Lookup:
        for st in lookup.SubTable:
            if hasattr(st, "MarkCoverage"):
                covered.update(st.MarkCoverage.glyphs)
    return covered


def validate_one_weight(weight):
    source_path = os.path.join(config.SOURCE_DIR, weight)
    result_path = os.path.join(config.RESULT_DIR, weight)

    source = TTFont(source_path, fontNumber=0)
    result = TTFont(result_path, fontNumber=0)

    print(f"--- {weight} ---")
    for tag in ("GSUB", "COLR", "CPAL"):
        if tag in source:
            same = tables_identical(source, result, tag)
            print(f"  {tag} 跟源字体逐字节一致: {same}")
            if not same:
                print(f"  [FAIL] {tag} 被改动了！这是不应该发生的。")

    cmap = result.getBestCmap()
    covered = covered_mark_names(result)

    latin_marks = collect_marks(result, PURE_MARK_BLOCKS, MIXED_BLOCKS_LATIN_BASE)
    cyr_marks = collect_marks(result, [], MIXED_BLOCKS_CYRILLIC_BASE)
    kana_marks = [cp for lo, hi in KANA_MARK_BLOCKS for cp in range(lo, hi + 1) if cp in cmap]

    missing = []
    for cp in latin_marks + cyr_marks + kana_marks:
        name = cmap.get(cp)
        if name is None or name not in covered:
            missing.append(hex(cp))

    if missing:
        print(f"  [FAIL] 以下码位在覆盖范围内但没有出现在任何 GPOS MarkCoverage 里："
              f" {missing}")
    else:
        print(f"  覆盖完整性检查通过：{len(latin_marks) + len(cyr_marks) + len(kana_marks)} "
              f"个目标记号全部有 GPOS 锚点，没有漏网之鱼。")

    print(f"  numGlyphs: source={source['maxp'].numGlyphs} -> result={result['maxp'].numGlyphs}")
    return len(missing) == 0


def validate_all():
    all_ok = True
    for weight in config.WEIGHTS:
        ok = validate_one_weight(weight)
        all_ok = all_ok and ok
    print()
    print("=== 总结 ===")
    print("全部通过" if all_ok else "有失败项，看上面 [FAIL] 标记")
    return all_ok


if __name__ == "__main__":
    validate_all()
