#!/system/bin/sh
MODDIR=${0%/*}

MODPATH="$MODDIR"
. "$MODPATH/lang/lang.sh"

# 等待开机完成（最多 120 秒），替代固定 sleep 32：
# 慢设备上 32 秒不够，快设备上又白等
i=0
until [ "$(getprop sys.boot_completed 2>/dev/null)" = "1" ]; do
  i=$((i + 1))
  [ "$i" -ge 120 ] && break
  sleep 1
done

# 发送开机通知（root shell 直接可用时无需 su）
if command -v su >/dev/null 2>&1; then
  su -lp 2000 -c "cmd notification post -S bigtext -t '$NOTIFY_TITLE' '$NOTIFY_TAG' '$NOTIFY_BODY'" 2>/dev/null \
    || cmd notification post -S bigtext -t "$NOTIFY_TITLE" "$NOTIFY_TAG" "$NOTIFY_BODY" 2>/dev/null
else
  cmd notification post -S bigtext -t "$NOTIFY_TITLE" "$NOTIFY_TAG" "$NOTIFY_BODY" 2>/dev/null
fi

# ── 取消部分APP内置字体权限 ──────────────────────────────
# 应用列表可配置：config/font_permission_apps（每行一个包名，# 开头为注释），
# 缺省使用内置列表。开机时把这些应用的内部字体设为不可读，强制走系统字体。
FONT_PERM_APPS=$(cat "$MODPATH/config/font_permission_apps" 2>/dev/null \
    | grep -v '^[[:space:]]*#' | grep -v '^[[:space:]]*$')
[ -z "$FONT_PERM_APPS" ] && FONT_PERM_APPS="com.dragon.read
com.qidian.QDReader"

for APP in $FONT_PERM_APPS; do
    # 覆盖所有用户（含工作资料），匹配应用目录下的任何字体文件
    find /data/user/*/$APP -type f \( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \) -exec chmod 000 {} \; 2>/dev/null
done
