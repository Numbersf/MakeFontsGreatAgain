# -*- coding: utf-8 -*-
import copy
import os

from fontTools.ttLib import TTFont
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.pens.cu2quPen import Cu2QuPen
from fontTools.pens.recordingPen import DecomposingRecordingPen

import config
from step1_diff_glyphs import diff_one_weight


def is_cff(font):
    return "CFF " in font or "CFF2" in font


def flattened_glyph(font_b, name_b, source_is_cff):
    gs_b = font_b.getGlyphSet()
    tt_pen = TTGlyphPen(None)
    cu2qu_pen = Cu2QuPen(tt_pen, max_err=1.0, reverse_direction=source_is_cff)
    dpen = DecomposingRecordingPen(gs_b)
    gs_b[name_b].draw(dpen)
    dpen.replay(cu2qu_pen)
    return tt_pen.glyph()


def merge_one_weight(weight):
    source_path = os.path.join(config.SOURCE_DIR, weight)
    modified_path = os.path.join(config.MODIFIED_DIR, weight)

    diff = diff_one_weight(source_path, modified_path)

    font_a = TTFont(source_path, fontNumber=0)
    font_b = TTFont(modified_path, fontNumber=0)
    if "glyf" not in font_a:
        raise NotImplementedError(
            f"{source_path} 不是 TrueType(glyf)轮廓字体。处理无效"
        )
    source_is_cff = is_cff(font_b)

    cmap_b = font_b.getBestCmap()
    hmtx_b = font_b["hmtx"]
    vmtx_b = font_b["vmtx"] if "vmtx" in font_b else None

    glyph_order_a = set(font_a.getGlyphOrder())
    added_pairs = []  # (codepoint, final_name_in_result)

    # 换轮廓+宽度 
    for item in diff["changed"]:
        if len(item) == 4:
            cp, na, nb, err = item
            print(f"  [WARN] {hex(cp)} 对比时出错，跳过：{err}")
            continue
        cp, na, nb = item
        glyph_obj = flattened_glyph(font_b, nb, source_is_cff)
        font_a["glyf"][na] = glyph_obj
        font_a["hmtx"][na] = hmtx_b[nb]
        if vmtx_b is not None and "vmtx" in font_a:
            font_a["vmtx"][na] = vmtx_b[nb]

    # 分配新名字，追加到字形表末尾
    for cp, nb in diff["added"]:
        name = nb
        suffix = 0
        while name in glyph_order_a:
            suffix += 1
            name = f"{nb}_{suffix}"
        glyph_obj = flattened_glyph(font_b, nb, source_is_cff)
        font_a["glyf"][name] = glyph_obj  # 会自动同步进 glyphOrder，不要再手动 append
        font_a["hmtx"][name] = hmtx_b[nb]
        if vmtx_b is not None and "vmtx" in font_a:
            font_a["vmtx"][name] = vmtx_b[nb]
        glyph_order_a.add(name)
        added_pairs.append((cp, name))

    # 同步 cmap
    for table in font_a["cmap"].tables:
        for cp, name in added_pairs:
            if table.format == 4 and cp > 0xFFFF:
                continue
            table.cmap[cp] = name

    final_order = font_a.getGlyphOrder()
    assert len(final_order) == len(set(final_order)), "异常!字形顺序出现重名!"
    font_a["maxp"].numGlyphs = len(final_order)

    os.makedirs(config.MERGED_DIR, exist_ok=True)
    out_path = os.path.join(config.MERGED_DIR, weight)
    font_a.save(out_path)
    print(f"  {weight}: 改动 {len(diff['changed'])} 个、新增 {len(added_pairs)} 个 -> {out_path}")
    return out_path


def merge_all():
    out_paths = {}
    for weight in config.WEIGHTS:
        print(f"=== {weight} ===")
        out_paths[weight] = merge_one_weight(weight)
    return out_paths


if __name__ == "__main__":
    merge_all()
