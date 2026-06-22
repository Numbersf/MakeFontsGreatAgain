#!/system/bin/sh

MOD_DIR="/data/adb/modules/MFGA"
BIN_DIR="$MOD_DIR/bin"
FONT_DIR="$MOD_DIR/system/fonts"
WEIGHTS="100 200 300 400 500 600 700 800 900"

RAW="$1"

MODPATH="$MOD_DIR"
. "$MOD_DIR/lang/lang.sh"

if [ -z "$RAW" ]; then
    echo "$RCG_USAGE"
    exit 1
fi

CP_PART=$(echo "$RAW" | cut -d';' -f1)
COLOR_PART=$(echo "$RAW" | cut -d';' -f2)

CP_HEX=$(echo "$CP_PART" | sed -E 's/^[uU]\+//')
COLOR_HEX=$(echo "$COLOR_PART" | sed -E 's/^#//')

if [ -z "$CP_HEX" ] || [ -z "$COLOR_HEX" ]; then
    echo "$RCG_BAD_FORMAT '$RAW'"
    exit 1
fi

ARCH=$(uname -m)
case "$ARCH" in
    aarch64) BIN="$BIN_DIR/recolor_glyph_arm64" ;;
    arm*)    BIN="$BIN_DIR/recolor_glyph_arm" ;;
    x86_64)  BIN="$BIN_DIR/recolor_glyph_x86_64" ;;
    i*86)    BIN="$BIN_DIR/recolor_glyph_x86" ;;
    *)       echo "[!] $UNI_UNSUPPORTED_ARCH $ARCH"; exit 1 ;;
esac

if [ -f "$BIN" ]; then
    chmod 755 "$BIN"
fi

if [ ! -x "$BIN" ]; then
    echo "$RCG_NO_BIN $BIN"
    exit 1
fi

OK=0
SKIP=0
FAIL=0

for W in $WEIGHTS; do
    FONT_PATH="$FONT_DIR/$W.ttf"
    if [ ! -f "$FONT_PATH" ]; then
        echo "$RCG_SKIP_MISSING $W.ttf"
        SKIP=$((SKIP + 1))
        continue
    fi

    OUTPUT=$("$BIN" "$FONT_PATH" "$CP_HEX" "$COLOR_HEX" 2>&1)
    CODE=$?

    echo "$OUTPUT"

    case "$CODE" in
        0) OK=$((OK + 1)) ;;
        3) SKIP=$((SKIP + 1)) ;;
        *) FAIL=$((FAIL + 1)) ;;
    esac
done

echo ""
DONE_MSG=$(echo "$RCG_DONE" | sed "s/%OK%/$OK/;s/%SKIP%/$SKIP/;s/%FAIL%/$FAIL/")
echo "$DONE_MSG"

if [ "$FAIL" -gt 0 ]; then
    exit 2
fi
exit 0
