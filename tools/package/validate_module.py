#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
模块 zip 结构校验（打包脚本的“质检”环节，也可单独运行）。

校验项：
  1. module.prop 必需字段（id/name/version/versionCode）
  2. 模块根目录必需文件（脚本、lang、webroot、fonts.xml、fonts_list.yaml）
  3. fonts.xml 引用的字体文件必须全部存在于 system/fonts/（缺=警告，--strict 下报错）
  4. bin/ 下的 8 个架构二进制（缺=警告）
  5. 可执行脚本必须带可执行位（Magisk 会 chmod，但提前发现更稳）
"""
import argparse
import os
import re
import sys

REQUIRED_ROOT = [
    "module.prop",
    "customize.sh", "service.sh", "action.sh", "uninstall.sh",
    "search_dirs.sh", "unicode_filter.sh", "recolor_glyph.sh",
    "lang/lang.sh",
    "fonts.xml", "fonts_list.yaml",
    "config/font_permission_apps", "config/gms_stop_apps",
    "webroot/index.html", "webroot/scripts.js", "webroot/config.json",
    "webroot/assets/kernelsu.js",
]
REQUIRED_PROPS = ["id", "name", "version", "versionCode"]
EXPECTED_BINS = [
    "unicode_filter_arm64", "unicode_filter_arm", "unicode_filter_x86_64", "unicode_filter_x86",
    "recolor_glyph_arm64", "recolor_glyph_arm", "recolor_glyph_x86_64", "recolor_glyph_x86",
]


def load_fonts_xml_refs(xml_path):
    refs = set()
    with open(xml_path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = re.search(r"<font\b[^>]*>([^<\s]+)", line)
            if m:
                refs.add(m.group(1))
    return refs


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("stage", help="模块 staging 目录")
    ap.add_argument("--strict", action="store_true",
                    help="fonts.xml 缺字体 / 缺二进制时报错（默认只警告）")
    args = ap.parse_args()

    stage = args.stage
    errors, warnings = [], []

    # 1. module.prop
    prop_path = os.path.join(stage, "module.prop")
    if not os.path.isfile(prop_path):
        errors.append("缺少 module.prop")
    else:
        props = {}
        with open(prop_path, encoding="utf-8", errors="replace") as f:
            for line in f:
                line = line.strip()
                if "=" in line:
                    k, v = line.split("=", 1)
                    props[k.strip()] = v.strip()
        for k in REQUIRED_PROPS:
            if k not in props or not props[k]:
                errors.append(f"module.prop 缺少字段: {k}")

    # 2. 必需根文件
    for rel in REQUIRED_ROOT:
        if not os.path.exists(os.path.join(stage, rel)):
            errors.append(f"缺少必需文件: {rel}")

    # 3. fonts.xml 引用 vs system/fonts
    xml_path = os.path.join(stage, "fonts.xml")
    fonts_dir = os.path.join(stage, "system", "fonts")
    shipped = set()
    if os.path.isdir(fonts_dir):
        shipped = {f for f in os.listdir(fonts_dir)
                   if f.lower().endswith((".ttf", ".otf", ".ttc"))}
    if os.path.isfile(xml_path):
        refs = load_fonts_xml_refs(xml_path)
        missing = sorted(refs - shipped)
        if missing:
            msg = (f"fonts.xml 引用了 {len(missing)} 个未打包字体"
                   f"（共引用 {len(refs)}，已打包 {len(shipped)}）：\n"
                   + ", ".join(missing[:20]) + ("..." if len(missing) > 20 else ""))
            if args.strict:
                errors.append(msg)
            else:
                warnings.append(msg)

    # 4. bin
    bin_dir = os.path.join(stage, "bin")
    present = set(os.listdir(bin_dir)) if os.path.isdir(bin_dir) else set()
    missing_bins = [b for b in EXPECTED_BINS if b not in present]
    if missing_bins:
        msg = f"bin/ 缺少 {len(missing_bins)} 个二进制: {', '.join(missing_bins)}"
        if args.strict:
            errors.append(msg)
        else:
            warnings.append(msg)

    # 5. 可执行位
    for rel in ["customize.sh", "service.sh", "action.sh", "search_dirs.sh",
                "unicode_filter.sh", "recolor_glyph.sh", "uninstall.sh"]:
        p = os.path.join(stage, rel)
        if os.path.exists(p) and not os.access(p, os.X_OK):
            warnings.append(f"脚本缺少可执行位: {rel}")

    for w in warnings:
        print(f"[WARN] {w}")
    for e in errors:
        print(f"[FAIL] {e}")

    if errors:
        print(f"\n校验失败：{len(errors)} 个错误、{len(warnings)} 个警告")
        return 1
    if warnings:
        print(f"\n校验通过（{len(warnings)} 个警告，--strict 可升级为错误）")
    else:
        print("\n校验通过，无警告")
    return 0


if __name__ == "__main__":
    sys.exit(main())
