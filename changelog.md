CN
 
2026.08.26(20260826)
 - 1.本 fork 改用日期版本号（2026.08.26）
 - 2.安全加固：unicode_filter 整表重建（修复屏蔽后保留字形丢失）、WebUI 注入/XSS/路径穿越防护
 - 3.新增可配置应用列表（config/font_permission_apps、config/gms_stop_apps）与 WebUI 管理区块
 - 4.新增打包发布链路：tools/package 打包脚本 + release workflow + 自动更新字体 PR
 - 5.更新通道改为 releases/latest 直链；Xposed 默认作用域扩至 14 个应用
 - 6.新增 CI 质量门禁（shellcheck / JS / JSON / 版本一致性 / C 工具冒烟测试）
 - 7.覆盖补全：NotoSansPro 新增亚美尼亚文、埃及象形文字、卡纳达文、线性文字A/B 五个脚本（待 CI 重建字体后生效）
 
17.0.0.06-27-alpha(1717180001)
 - 1.同步Roboto到3.0.16(SU)
 - 2.WebUI新增主字体上色，需支持COLRv0，Android10及以上
 - 3.调整主字体中部分组合类符号，修复缺失、在高安卓版本显示异常的情况
 
16.3.5.04-15-beta2(1635041502)
 - 1.action.sh:修复WebUI模式下屏蔽GMS跳过暂停应用的问题
 - 2.WebUI:新增自动翻译、帮助界面；重命名图标标题clear为clean并优化图标大小；修复不统一的换行
 - 3.unicode_filter.sh、service.sh:加入自动翻译
 - 4.新增彩色图标字体MFGA.ttf（）
 - 5.fonts.xml:进一步提高兼容性
 

-------
EN
 
2026.08.26(20260826)
 - 1.This fork switched to a date-based version (2026.08.26).
 - 2.Security hardening: unicode_filter full cmap rebuild (fixed lost glyphs after blocking), WebUI injection/XSS/path-traversal protection.
 - 3.Configurable app lists (config/font_permission_apps, config/gms_stop_apps) with WebUI management section.
 - 4.Packaging & release pipeline: tools/package build script + release workflow + auto font-update PR.
 - 5.Update channels switched to releases/latest; Xposed default scope expanded to 14 apps.
 - 6.New CI quality gates (shellcheck / JS / JSON / version consistency / C smoke tests).
 
17.0.0.06-27-alpha(1717180001)
 - 1.Synchronized Roboto font to version 3.0.16(SU).
 - 2.Added main font colorization in WebUI; requires COLRv0 support, Android 10 and above.
 - 3.Adjusted some composite symbols in the main font, fixing missing glyphs and display issues on higher Android versions.
 
16.3.5.04-15-beta2(1635041502)
 - 1.action.sh:Fixed the issue where GMS blocking skipped the pause application in WebUI mode.
 - 2.WebUI:Added automatic translation and help interface; renamed the icon title "clear" to "clean" and optimized the icon size; fixed inconsistent line breaks.
 - 3.unicode_filter.sh、service.sh:Added automatic translation.
 - 4.Introduce the new color icon font MFGA.ttf.（）
 - 5.fonts.xml:Further improved compatibility.
 

Telegram channel:

https://t.me/AndroidCoreLayer

Power by:

Yiyunlengyu(酷安@Numbersf)