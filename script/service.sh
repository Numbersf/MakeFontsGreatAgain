#!/system/bin/sh
MODDIR=${0%/*}
sleep 32

# 获取系统语言的前缀部分
LANGUAGE=$(getprop persist.sys.locale | cut -d'-' -f1)
if [ "$LANGUAGE" = "zh" ]; then
  # 中文通知
  su -lp 2000 -c "cmd notification post -S bigtext -t 'MFGA' 'Tag' '感谢使用高级全局字体模块MFGA，滑动以删除本通知。Powered by Yiyunlengyu（咿云冷雨）'"
else
  # English notification information
  su -lp 2000 -c "cmd notification post -S bigtext -t 'MFGA' 'Tag' 'Thank for using Advanced Global Font Module MFGA. Swipe to dismiss this notification. Powered by Yiyunlengyu.'"
fi

# 取消部分APP内置字体权限
find /data/user/0/com.dragon.read/files/font -type f \( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \) -exec chmod 000 {} \;
find /data/user/0/com.qidian.QDReader/files/truetype_fonts -type f \( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \) -exec chmod 000 {} \;