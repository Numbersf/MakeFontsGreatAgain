#!/usr/bin/env bash
# ============================================================
# MFGA 模块打包脚本
#
# 思路：本仓库只维护“定制”部分（自用字体 + 修复后的脚本/WebUI），
# 完整模块 = 上游 release zip 的字体集为基底 + 本仓库覆盖。
#
# 用法：
#   tools/package/build_module.sh [--base-zip UPSTREAM.zip] [--bin-dir DIR]
#                                [--strict] [--out-dir dist]
#
#   --base-zip   上游 MFGA release zip（提供 fonts.xml 需要的全套字体与 zygisk 等）
#                缺省时只打包仓库 fonts/ 下的字体（会缺 143 个字体，仅警告）
#   --bin-dir    已编译好的 8 个 C 二进制目录（unicode_filter_* / recolor_glyph_*）
#                缺省时尝试 tools/prebuilt/bin
#   --strict     字体缺失/二进制缺失视为错误（默认仅警告）
#   --out-dir    输出目录，默认 dist/
#
# 输出：
#   dist/MFGA-<version>.<versionCode>.zip
#   dist/MFGA-SELFUSE-<version>.<versionCode>.zip   （同一内容，SELFUSE 通道名）
# ============================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

BASE_ZIP=""
BIN_DIR=""
STRICT=0
OUT_DIR="dist"

while [ $# -gt 0 ]; do
  case "$1" in
    --base-zip) BASE_ZIP="${2:-}"; shift 2 ;;
    --bin-dir)  BIN_DIR="${2:-}"; shift 2 ;;
    --strict)   STRICT=1; shift ;;
    --out-dir)  OUT_DIR="${2:-}"; shift 2 ;;
    -h|--help)
      grep '^#' "$0" | sed 's/^# \{0,1\}//' | sed -n '1,30p'
      exit 0 ;;
    *) echo "[!] 未知参数: $1"; exit 1 ;;
  esac
done

for cmd in python3 zip unzip; do
  command -v "$cmd" >/dev/null 2>&1 || { echo "[!] 缺少命令: $cmd"; exit 1; }
done

VERSION=$(python3 -c "import json;print(json.load(open('update.json'))['version'])")
VERSION_CODE=$(python3 -c "import json;print(json.load(open('update.json'))['versionCode'])")
echo "==> 版本: $VERSION ($VERSION_CODE)"

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

# ---------- 1. 基底 ----------
if [ -n "$BASE_ZIP" ]; then
  [ -f "$BASE_ZIP" ] || { echo "[!] base zip 不存在: $BASE_ZIP"; exit 1; }
  echo "==> 解包基底: $BASE_ZIP"
  unzip -q -o "$BASE_ZIP" -d "$STAGE"
  if [ ! -d "$STAGE/system/fonts" ]; then
    echo "[!] base zip 缺少 system/fonts，不是有效的模块 zip"; exit 1
  fi
  echo "    基底字体数: $(find "$STAGE/system/fonts" -type f | wc -l)"
else
  mkdir -p "$STAGE/system/fonts"
  echo "[!] 警告: 未提供 --base-zip，只打包仓库字体，fonts.xml 会大量缺字"
fi

# ---------- 2. 覆盖本仓库定制 ----------
echo "==> 覆盖仓库定制内容"
# 字体：仓库 fonts/ 覆盖/补充基底
for f in fonts/*.ttf fonts/*.otf; do
  [ -e "$f" ] || continue
  cp -f "$f" "$STAGE/system/fonts/"
done
# 删除被本 fork 替代的基底字体（主字重已被 Maruko 圆体替换；花哨族已指向主字体）
for f in 100.ttf 200.ttf 300.ttf 400.ttf 500.ttf 600.ttf 700.ttf 800.ttf 900.ttf \
         ComingSoon.ttf DancingScript-Regular.ttf CarroisGothicSC-Regular.ttf; do
  rm -f "$STAGE/system/fonts/$f"
done
cp -f fonts/LICENSE-MIT fonts/LICENSE-OFL "$STAGE/" 2>/dev/null || true

# 脚本（模块根目录）
mkdir -p "$STAGE/lang"
cp -f lang/lang.sh lang/lang_*.sh "$STAGE/lang/"
for s in customize.sh service.sh action.sh uninstall.sh search_dirs.sh \
         unicode_filter.sh recolor_glyph.sh; do
  cp -f "script/$s" "$STAGE/$s"
  chmod 755 "$STAGE/$s"
done

# 字体配置
cp -f fonts.xml fonts_list.yaml "$STAGE/"

# 可配置项
mkdir -p "$STAGE/config"
cp -f config/font_permission_apps config/gms_stop_apps "$STAGE/config/"

# WebUI
mkdir -p "$STAGE/webroot"
cp -f webroot/config.json webroot/index.html webroot/scripts.js \
      webroot/styles.css webroot/icon.png "$STAGE/webroot/"
cp -rf webroot/assets "$STAGE/webroot/"
cp -rf webroot/strings "$STAGE/webroot/"

# ---------- 3. C 二进制 ----------
if [ -z "$BIN_DIR" ] && [ -d "tools/prebuilt/bin" ]; then
  BIN_DIR="tools/prebuilt/bin"
fi
if [ -n "$BIN_DIR" ]; then
  [ -d "$BIN_DIR" ] || { echo "[!] --bin-dir 不存在: $BIN_DIR"; exit 1; }
  mkdir -p "$STAGE/bin"
  cp -f "$BIN_DIR"/unicode_filter_* "$BIN_DIR"/recolor_glyph_* "$STAGE/bin/" 2>/dev/null || true
  chmod 755 "$STAGE/bin/"* 2>/dev/null || true
  echo "==> 二进制: $BIN_DIR ($(ls "$STAGE/bin" | wc -l) 个)"
else
  mkdir -p "$STAGE/bin"
  echo "[!] 警告: 未提供 --bin-dir，区间屏蔽/上色功能将不可用"
fi

# ---------- 4. module.prop ----------
echo "==> 生成 module.prop"
cat > "$STAGE/module.prop" <<EOF
id=MFGA
name=MFGA (Selffont)
version=$VERSION
versionCode=$VERSION_CODE
author=Numbersf | Sumicya/Selffont
description=Make Fonts Great Again - 自用字体整合（本 fork 定制）
minMagisk=28000
updateJson=https://raw.githubusercontent.com/Sumicya/Selffont/main/update.json
EOF

# ---------- 5. 校验 ----------
echo "==> 校验"
STRICT_ARGS=()
[ "$STRICT" -eq 1 ] && STRICT_ARGS+=(--strict)
python3 tools/package/validate_module.py "$STAGE" "${STRICT_ARGS[@]+"${STRICT_ARGS[@]}"}"

# ---------- 6. 打包 ----------
mkdir -p "$OUT_DIR"
ZIP1="$OUT_DIR/MFGA-$VERSION.$VERSION_CODE.zip"
ZIP2="$OUT_DIR/MFGA-SELFUSE-$VERSION.$VERSION_CODE.zip"
rm -f "$ZIP1" "$ZIP2"
( cd "$STAGE" && zip -qr -9 "$ROOT/$ZIP1" . -x '*.tmp' -x '.DS_Store' )
cp -f "$ZIP1" "$ZIP2"
echo "==> 完成"
echo "    $ZIP1"
echo "    $ZIP2"
ls -lh "$ZIP1"
