SKIPUNZIP=0
. "$MODPATH/lang/lang.sh"

version_check() {
  if [ -d /data/adb/ksu ]; then
    if [ -z "$KSU_VER_CODE" ] || [ -z "$KSU_KERNEL_VER_CODE" ]; then
      ui_print "$(msg KSU_LOW)"
      abort
    fi
    if [ "$KSU_VER_CODE" -lt 11986 ] || [ "$KSU_KERNEL_VER_CODE" -lt 11986 ]; then
      ui_print "$(msg KSU_LOW)"
      abort
    fi
  elif [ -d /data/adb/magisk ]; then
    if [ -z "$MAGISK_VER_CODE" ] || [ "$MAGISK_VER_CODE" -lt 28000 ]; then
      ui_print "$(msg MAGISK_LOW)"
      abort
    fi
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

MFGA_OLD_ID="/data/adb/modules/Colorfontsproject"
[ -d "$MFGA_OLD_ID" ] && ui_print "$(msg MODULE_OBSOLETE)" && touch "$MFGA_OLD_ID/remove"

version_check

MIUI_NAM=$(getprop ro.miui.ui.version.name)
MIOS_NAM=$(getprop ro.mi.os.version.name)
if [[ -n "$MIUI_NAM" || -n "$MIOS_NAM" ]]; then
  ui_print "$(msg XIAOMI_KEEP)"
else
  ui_print "$(msg NON_XIAOMI_REMOVE)"
  rm -rf "$MODPATH/zygisk" "$MODPATH/system/product/fonts"
fi

. "$MODPATH/search_dirs.sh"

ui_print "- Welcome to MFGA!"