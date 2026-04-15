#!/system/bin/sh
MODDIR=${0%/*}
sleep 32

MODPATH="$MODDIR"
. "$MODPATH/lang/lang.sh"

# 发送开机通知
su -lp 2000 -c "cmd notification post -S bigtext -t '$NOTIFY_TITLE' '$NOTIFY_TAG' '$NOTIFY_BODY'"

# 取消部分APP内置字体权限
find /data/user/0/com.dragon.read/files/font -type f \( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \) -exec chmod 000 {} \;
find /data/user/0/com.qidian.QDReader/files/truetype_fonts -type f \( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \) -exec chmod 000 {} \;
