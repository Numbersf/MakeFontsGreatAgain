SKIPUNZIP=0
. "$MODPATH/lang/lang.sh"

version_check() {
  if [[ $KSU_VER_CODE != "" ]] && [[ $KSU_VER_CODE -lt 11986 || $KSU_KERNEL_VER_CODE -lt 11986 ]]; then
    ui_print "$(msg KSU_LOW)"
    abort
  elif [[ $KSU_VER_CODE == "" && $MAGISK_VER_CODE != "" && $MAGISK_VER_CODE -lt 28000 ]]; then
    ui_print "$(msg MAGISK_LOW)"
    abort
  fi
}

if [ "$API" -le 30 ]; then
  ui_print "$(msg ANDROID_11_OR_LOWER)"
  mv -f "$MODPATH/system/fonts/NotoColorEmoji-fallback.ttf" "$MODPATH/system/fonts/NotoColorEmoji.ttf"
else
  ui_print "$(msg ANDROID_12_OR_HIGHER)"
  rm -f "$MODPATH/system/fonts/NotoColorEmoji-fallback.ttf"
fi

if [ -d "/data/adb/modules/playintegrityfix" ]; then
  [ -d "/data/adb/ksu" ] && ui_print "$(msg PIF_WARNING_KSU)" && ui_print "$(msg PIF_WARNING_KSU2)"
  [ -d "/data/adb/magisk" ] && ui_print "$(msg PIF_WARNING_MAGISK)"
fi

MFGA="/data/adb/modules/Colorfontsproject"
[ -d "$MFGA" ] && ui_print "$(msg MODULE_OBSOLETE)" && touch "$MFGA/remove"

version_check

BRAND=$(getprop ro.product.brand | tr '[:upper:]' '[:lower:]')
MANUFACTURER=$(getprop ro.product.manufacturer | tr '[:upper:]' '[:lower:]')

if [[ "$BRAND" == "xiaomi" || "$BRAND" == "redmi" || "$BRAND" == "poco" || "$MANUFACTURER" == "xiaomi" ]]; then
  ui_print "$(msg XIAOMI_KEEP)"
else
  ui_print "$(msg NON_XIAOMI_REMOVE)"
  rm -rf "$MODPATH/zygisk" "$MODPATH/system/product/fonts"
fi

. "$MODPATH/search_dirs.sh"

ui_print "- Welcome to MFGA!"