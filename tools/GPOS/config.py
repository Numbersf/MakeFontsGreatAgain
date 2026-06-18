# -*- coding: utf-8 -*-
"""
    源（没被 FontCreator 等工具碰过的原始字体，GSUB/GPOS/COLR/CPAL 全部完整)
    /storage/emulated/0/123

    修改（在 FontCreator 等工具里改完/加完符号之后存出来的版本，
          版式表可能已经被工具裁剪/弄坏，但新加的字形是对的）
    /storage/emulated/0/124

    结果（脚本跑完后的最终产物：以"源"的版式表为底，把"修改"里新增/改动的
          字形原样搬过来，再补全所有组合记号的 GPOS 定位）
    /storage/emulated/0/125
"""

SOURCE_DIR = "/storage/emulated/0/123"
MODIFIED_DIR = "/storage/emulated/0/124"
RESULT_DIR = "/storage/emulated/0/125"

# 字重文件名（不含路径），按需增删
WEIGHTS = ["200.ttf", "300.ttf", "400.ttf", "700.ttf"]

# 中间产物路径（合并完字形、但还没建 GPOS 的版本），方便分步调试
MERGED_DIR = "/storage/emulated/0/125_merged_tmp"
