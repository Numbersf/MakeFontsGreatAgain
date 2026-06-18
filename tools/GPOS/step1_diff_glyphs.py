# -*- coding: utf-8 -*-
import os

from fontTools.ttLib import TTFont

from utils import shape_signature
import config


def diff_one_weight(source_path, modified_path):
    a = TTFont(source_path, fontNumber=0)
    b = TTFont(modified_path, fontNumber=0)

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
        if sig_a != sig_b or wa != wb:
            changed.append((cp, na, nb))

    return {"added": added, "changed": changed}


def diff_all():
    results = {}
    for weight in config.WEIGHTS:
        source_path = os.path.join(config.SOURCE_DIR, weight)
        modified_path = os.path.join(config.MODIFIED_DIR, weight)
        print(f"=== {weight} ===")
        result = diff_one_weight(source_path, modified_path)
        print(f"  新增码位: {len(result['added'])}")
        print(f"  改动码位: {len(result['changed'])}")
        results[weight] = result
    return results


if __name__ == "__main__":
    diff_all()
