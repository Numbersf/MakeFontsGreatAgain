#!/system/bin/sh

LANGUAGE=$(getprop persist.sys.locale | tr '-' '_')

. "$MODPATH/lang/lang_${LANGUAGE}.sh" \
  || . "$MODPATH/lang/lang_en_US.sh"

msg() {
  eval "echo \"\${$1}\""
}