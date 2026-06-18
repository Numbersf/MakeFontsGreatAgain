# -*- coding: utf-8 -*-
import os

from fontTools.ttLib import TTFont

from utils import shape_signature
import config

SUSPICIOUS_WIDTH_RATIO = 1.25
SUSPICIOUS_WIDTH_EM_FRACTION = 0.92

def is_suspicious_width_jump(units_per_em, old_width, new_width):
    if old_width <= 0:
        return False
    if new_width < units_per_em * SUSPICIOUS_WIDTH_EM_FRACTION:
        return False
    return new_width >= old_width * SUSPICIOUS_WIDTH_RATIO


def diff_one_weight(source_path, modified_path):
    a = TTFont(source_path, fontNumber=0)
    b = TTFont(modified_path, fontNumber=0)
    units_per_em = a["head"].unitsPerEm

    cmap_a = a.getBestCmap()
    cmap_b = b.getBestCmap()

    added_cps = sorted(set(cmap_b) - set(cmap_a))
    shared_cps = sorted(set(cmap_a) & set(cmap_b))

    gs_a = a.getGlyphSet()
    gs_b = b.getGlyphSet()
    hmtx_a = a["hmtx"]
    hmtx_b = b["hmtx"]

    added = [(cp, cmap_b[cp]) for cp in added_cps]

    changed = []
    suspicious = []
    for cp in shared_cps:
        na, nb = cmap_a[cp], cmap_b[cp]
        try:
            sig_a = shape_signature(gs_a, na)
            sig_b = shape_signature(gs_b, nb)
        except Exception as e:
            changed.append((cp, na, nb, f"ERROR: {e}"))
            continue
        wa = hmtx_a[na][0]
        wb = hmtx_b[nb][0]
        if sig_a == sig_b and wa == wb:
            continue
        if is_suspicious_width_jump(units_per_em, wa, wb):
            suspicious.append((cp, na, nb, wa, wb))
            continue
        changed.append((cp, na, nb))

    return {"added": added, "changed": changed, "suspicious": suspicious}


def diff_all():
    results = {}
    for weight in config.WEIGHTS:
        source_path = os.path.join(config.SOURCE_DIR, weight)
        modified_path = os.path.join(config.MODIFIED_DIR, weight)
        print(f"=== {weight} ===")
        result = diff_one_weight(source_path, modified_path)
        print(f"  新增码位: {len(result['added'])}")
        print(f"  改动码位: {len(result['changed'])}")
        if result["suspicious"]:
            cps = ", ".join(f"{hex(cp)}({chr(cp)}) {wa}->{wb}" for cp, na, nb, wa, wb in result["suspicious"])
            print(f"  !! 跳过 {len(result['suspicious'])} 个疑似全角化假改动（保留源字体原样）: {cps}")
        results[weight] = result
    return results


if __name__ == "__main__":
    diff_all()
