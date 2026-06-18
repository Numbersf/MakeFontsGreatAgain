# -*- coding: utf-8 -*-
import copy

from fontTools.pens.boundsPen import BoundsPen
from fontTools.pens.recordingPen import DecomposingRecordingPen


def bbox(glyphset, name):
    bp = BoundsPen(glyphset)
    glyphset[name].draw(bp)
    return bp.bounds


def shape_signature(glyphset, name):
    pen = DecomposingRecordingPen(glyphset)
    glyphset[name].draw(pen)
    out = []
    for op, args in pen.value:
        if args:
            rounded = tuple(
                tuple(round(v, 2) for v in pt) if isinstance(pt, tuple) else pt
                for pt in args
            )
        else:
            rounded = args
        out.append((op, rounded))
    return out


def raw_table_bytes(ttfont, tag):
    entry = ttfont.reader.tables[tag]
    return entry.loadData(ttfont.reader.file)


def tables_identical(font_a, font_b, tag):
    if tag not in font_a.reader.tables or tag not in font_b.reader.tables:
        return tag not in font_a.reader.tables and tag not in font_b.reader.tables
    return raw_table_bytes(font_a, tag) == raw_table_bytes(font_b, tag)


def add_glyph(font, name, glyph_obj, advance, lsb, vmtx=(1000, 0)):
    font["glyf"][name] = glyph_obj
    font["hmtx"][name] = (advance, lsb)
    if "vmtx" in font:
        font["vmtx"][name] = vmtx
