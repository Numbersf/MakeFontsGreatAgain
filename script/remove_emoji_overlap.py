#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Remove glyphs from target fonts that overlap with NotoColorEmoji.ttf

Logic:
1. Read all Unicode codepoints from NotoColorEmoji.ttf
2. Remove these codepoints from target fonts
3. Automatically clean unused glyphs via fontTools Subsetter
"""

import sys
from fontTools.ttLib import TTFont
from fontTools.subset import Subsetter, Options


def get_codepoints(font_path):
    """
    Extract all Unicode codepoints from a font
    """
    font = TTFont(font_path)
    codepoints = set()

    for table in font["cmap"].tables:
        codepoints.update(table.cmap.keys())

    font.close()
    return codepoints


def remove_overlap(source_font_path, emoji_codepoints, output_path):
    """
    Remove overlapping Unicode codepoints from source font
    """
    font = TTFont(source_font_path)

    # Collect existing codepoints in source font
    existing_codepoints = set()
    for table in font["cmap"].tables:
        existing_codepoints.update(table.cmap.keys())

    # Determine which codepoints to keep
    keep_codepoints = existing_codepoints - emoji_codepoints

    print(f"\nProcessing: {source_font_path}")
    print(f"Total glyphs before: {len(existing_codepoints)}")
    print(f"Removing: {len(existing_codepoints & emoji_codepoints)}")
    print(f"Keeping: {len(keep_codepoints)}")

    # Configure subsetter
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

    print(f"Saved to: {output_path}")


def main():
    if len(sys.argv) < 4:
        print("Usage:")
        print("python remove_emoji_overlap.py NotoColorEmoji.ttf target1.otf target2.otf ...")
        sys.exit(1)

    emoji_font_path = sys.argv[1]
    target_fonts = sys.argv[2:]

    print("Reading emoji font...")
    emoji_codepoints = get_codepoints(emoji_font_path)
    print(f"Emoji codepoints found: {len(emoji_codepoints)}")

    for font_path in target_fonts:
        output_path = font_path.replace(".otf", "_noEmoji.otf").replace(".ttf", "_noEmoji.ttf")
        remove_overlap(font_path, emoji_codepoints, output_path)

    print("\nDone.")


if __name__ == "__main__":
    main()