#!/system/bin/sh

LANG_DIR="$MODPATH/lang"
SYSTEM_LOCALE=$(getprop persist.sys.locale | tr '-' '_')
[ -z "$SYSTEM_LOCALE" ] && SYSTEM_LOCALE="zh_CN"

try_source() {
  [ -f "$1" ] && . "$1" && return 0
  return 1
}

LANGUAGE_LOADED=0

LANG_CODE=$(echo "$SYSTEM_LOCALE" | cut -d_ -f1 | tr 'A-Z' 'a-z')
LOCALE_SUFFIX=$(echo "$SYSTEM_LOCALE" | cut -s -d_ -f2-)

if [ -n "$LOCALE_SUFFIX" ]; then
  try_source "$LANG_DIR/lang_${LANG_CODE}_$(echo $LOCALE_SUFFIX | tr 'a-z' 'A-Z').sh" && LANGUAGE_LOADED=1
fi

if [ "$LANGUAGE_LOADED" -eq 0 ] && [ "$LANG_CODE" = "zh" ] && [ -n "$LOCALE_SUFFIX" ]; then
  SCRIPT_TYPE=$(echo "$LOCALE_SUFFIX" | cut -d_ -f1)
  REGION_CODE=$(echo "$LOCALE_SUFFIX" | cut -s -d_ -f2)
  if [ "$SCRIPT_TYPE" = "Hans" ] || [ "$SCRIPT_TYPE" = "Hant" ]; then
    [ -n "$REGION_CODE" ] && try_source "$LANG_DIR/lang_${LANG_CODE}_${REGION_CODE}.sh" && LANGUAGE_LOADED=1
  fi
fi

if [ "$LANGUAGE_LOADED" -eq 0 ]; then
  EXISTING_FILE=$(ls "$LANG_DIR"/lang_"$LANG_CODE"_*.sh 2>/dev/null | head -n1)
  [ -n "$EXISTING_FILE" ] && try_source "$EXISTING_FILE" && LANGUAGE_LOADED=1
fi

[ "$LANGUAGE_LOADED" -eq 0 ] && . "$LANG_DIR/lang_zh_CN.sh"

msg() {
  eval "echo \"\${$1}\""
}