#!/bin/sh
# 将部分屏蔽权限字体恢复（与 service.sh 的应用列表保持一致）
# 应用列表可配置：config/font_permission_apps（每行一个包名，# 开头为注释）
MODPATH=${0%/*}

FONT_PERM_APPS=$(cat "$MODPATH/config/font_permission_apps" 2>/dev/null \
    | grep -v '^[[:space:]]*#' | grep -v '^[[:space:]]*$')
[ -z "$FONT_PERM_APPS" ] && FONT_PERM_APPS="com.dragon.read
com.qidian.QDReader"

for APP in $FONT_PERM_APPS; do
    find /data/user/*/$APP -type f \( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \) -exec chmod 600 {} \; 2>/dev/null
done
