#!/bin/sh
# 将部分屏蔽权限字体恢复
find /data/user/0/com.dragon.read/files/font -type f \( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \) -exec chmod 600 {} \;
find /data/user/0/com.qidian.QDReader/files/truetype_fonts -type f \( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \) -exec chmod 600 {} \;