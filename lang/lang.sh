#!/system/bin/sh

LANG_DIR="$MODPATH/lang"

# 读取系统 locale，如果为空默认 zh_CN
SYSTEM_LOCALE=$(getprop persist.sys.locale | tr '-' '_')
[ -z "$SYSTEM_LOCALE" ] && SYSTEM_LOCALE="zh_CN"

try_source() {
  [ -f "$1" ] && . "$1" && return 0
  return 1
}

LANGUAGE_LOADED=0

# 拆分 locale: language[-script][-region][-variant]
set -- $(echo "$SYSTEM_LOCALE" | tr '_' ' ')
LANG_CODE=$1
SCRIPT_PART=$2
REGION_PART=$3
VARIANT_PART=$4

LANG_CODE=$(echo "$LANG_CODE" | tr 'A-Z' 'a-z')
[ -n "$SCRIPT_PART" ] && SCRIPT_PART="$(echo ${SCRIPT_PART:0:1} | tr 'a-z' 'A-Z')$(echo ${SCRIPT_PART:1} | tr 'A-Z' 'a-z')"
[ -n "$REGION_PART" ] && REGION_PART=$(echo "$REGION_PART" | tr 'a-z' 'A-Z')

if [ -n "$SCRIPT_PART" ] && [ ${#SCRIPT_PART} -eq 4 ]; then
  REGION_CODE="$REGION_PART"
else
  REGION_CODE="$SCRIPT_PART"
fi

try_source "$LANG_DIR/lang_${LANG_CODE}.sh" && LANGUAGE_LOADED=1

if [ "$LANGUAGE_LOADED" -eq 0 ] && [ -n "$REGION_CODE" ]; then
  try_source "$LANG_DIR/lang_${LANG_CODE}_$REGION_CODE.sh" && LANGUAGE_LOADED=1
fi

if [ "$LANGUAGE_LOADED" -eq 0 ]; then
  EXISTING_FILE=$(ls "$LANG_DIR"/lang_"$LANG_CODE"_*.sh 2>/dev/null | head -n1)
  [ -n "$EXISTING_FILE" ] && try_source "$EXISTING_FILE" && LANGUAGE_LOADED=1
fi

[ "$LANGUAGE_LOADED" -eq 0 ] && . "$LANG_DIR/lang_zh_CN.sh"


msg() {
  eval "echo \"\${$1}\""
}