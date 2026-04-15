#!/system/bin/sh

MODULE_DIR="/data/adb/modules/MFGA"
BIN_DIR="$MODULE_DIR/bin"
FONTS_DIR="$MODULE_DIR/system/fonts"

FONT="$1"
RANGES="$2"

MODPATH="$MODULE_DIR"
. "$MODULE_DIR/lang/lang.sh"

ARCH=$(uname -m)
case "$ARCH" in
    aarch64) BIN="$BIN_DIR/unicode_filter_arm64" ;;
    arm*)    BIN="$BIN_DIR/unicode_filter_arm" ;;
    x86_64)  BIN="$BIN_DIR/unicode_filter_x86_64" ;;
    i*86)    BIN="$BIN_DIR/unicode_filter_x86" ;;
    *)       echo "[!] $UNI_UNSUPPORTED_ARCH $ARCH"; exit 1 ;;
esac

if [ -f "$BIN" ]; then
    chmod 755 "$BIN"
fi

if [ ! -x "$BIN" ]; then
    echo "$UNI_BIN_MISSING $BIN"
    exit 1
fi

if [ -z "$FONT" ] || [ -z "$RANGES" ]; then
    echo "$UNI_USAGE $0"
    exit 1
fi

# 强制限定字体目录
FONT_PATH="$FONTS_DIR/$FONT"
if [ ! -f "$FONT_PATH" ]; then
    echo "$UNI_FONT_NOT_FOUND $FONTS_DIR: $FONT"
    exit 1
fi

# 备份原始字体到 uni_backup
BACKUP_DIR="$MODULE_DIR/uni_backup"
mkdir -p "$BACKUP_DIR"
if [ ! -f "$BACKUP_DIR/$FONT" ]; then
    cp "$FONT_PATH" "$BACKUP_DIR/"
    echo "$UNI_BACKUP_CREATED $BACKUP_DIR/$FONT"
fi

echo "$UNI_PROCESSING $FONT $UNI_WITH_RANGES $RANGES"
"$BIN" "$FONT_PATH" "$RANGES"
RET=$?

if [ $RET -eq 0 ]; then
    echo "[✓] $UNI_SUCCESS"
else
    echo "[✗] $UNI_FAIL"
fi
exit $RET