#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Remove glyphs from target fonts that overlap with emoji defined in all.toml

Logic:
1. Parse all.toml and extract Unicode codepoints from svg filenames
2. Remove these codepoints from target fonts
3. Clean unused glyphs via fontTools Subsetter
"""

import sys
import re
from fontTools.ttLib import TTFont
from fontTools.subset import Subsetter, Options


def parse_all_toml(toml_path):
    """
    Extract emoji Unicode codepoints from all.toml
    """
    emoji_codepoints = set()

    with open(toml_path, "r", encoding="utf-8") as f:
        for line in f:
            # match: emoji_uXXXX.svg or emoji_uXXXX_XXXX.svg
            m = re.search(r'emoji_u([0-9a-fA-F_]+)\.svg', line)
            if not m:
                continue

            sequence = m.group(1)
            parts = sequence.split("_")

            for p in parts:
                try:
                    emoji_codepoints.add(int(p, 16))
                except ValueError:
                    pass

    return emoji_codepoints


def get_font_codepoints(font_path):
    font = TTFont(font_path)
    codepoints = set()

    for table in font["cmap"].tables:
        codepoints.update(table.cmap.keys())

    font.close()
    return codepoints


def remove_overlap(source_font_path, emoji_codepoints, output_path):
    font = TTFont(source_font_path)

    existing_codepoints = set()
    for table in font["cmap"].tables:
        existing_codepoints.update(table.cmap.keys())

    overlap = existing_codepoints & emoji_codepoints
    keep_codepoints = existing_codepoints - emoji_codepoints

    print(f"\nProcessing: {source_font_path}")
    print(f"Total glyphs before: {len(existing_codepoints)}")
    print(f"Emoji removed: {len(overlap)}")
    print(f"Remaining: {len(keep_codepoints)}")

    options = Options()
    options.set(layout_features='*')
    options.recalc_average_width = True
    options.recalc_timestamp = True
    options.hinting = True

    subsetter = Subsetter(options=options)
    subsetter.populate(unicodes=keep_codepoints)
    subsetter.subset(font)

    font.save(output_path)
    font.close()

    print(f"Font saved to: {output_path}")


def main():
    if len(sys.argv) < 3:
        print("Usage:")
        print("python remove_emoji_overlap.py all.toml target1.otf target2.otf ...")
        sys.exit(1)

    toml_path = sys.argv[1]
    target_fonts = sys.argv[2:]

    print("Parsing emoji list from all.toml...")
    emoji_codepoints = parse_all_toml(toml_path)
    print(f"Total emoji codepoints from TOML: {len(emoji_codepoints)}")

    for font_path in target_fonts:
        output_path = font_path.replace(".otf", "_noEmoji.otf").replace(".ttf", "_noEmoji.ttf")
        remove_overlap(font_path, emoji_codepoints, output_path)

    print("\nDone.")


if __name__ == "__main__":
    main()