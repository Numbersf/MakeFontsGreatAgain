CN
 
16.0.7.01-20-beta9(1607012009)
 - 1.第一版本更新maple-font到7.9
 - 2.移除所有原先模块直接内置的font*.xml，现仅保留模块根目录的fonts.xml作为复制源，通过调用search_dirs.sh搜索字体配置文件绝对路径和文件名进行复制
 - 3.action.sh、customize.sh、search_dirs.sh的翻译改调用lang/lang.sh
 
16.0.6.01-19-beta7(1606011907)
 - 1.service.sh:强制将com.qidian.QDReader、com.dragon.read数据目录下的字体文件权限改为000以达到覆盖效果，还原请改为600
 - 2.更新DisableMiFontOverlay到1.5
 - 3.更新部分主字体
 

-------
EN
 
16.0.7.01-20-beta9(1607012009)
 - 1.Initial release updating maple-font to v7.9
 - 2.Removed all previously built-in font*.xml files from the module. Only fonts.xml in the module root is now kept as the copy source. Font configuration files are discovered via search_dirs.sh, which searches for their absolute paths and filenames before copying.
 - 3.Translations in action.sh, customize.sh, and search_dirs.sh are now handled by calling lang/lang.sh
 
16.0.6.01-19-beta7(1606011907)
 - 1.service.sh: Forcibly changes the permissions of font files under the data directories of com.qidian.QDReader and com.dragon.read to 000 to achieve the override effect.
 - 2.Updated DisableMiFontOverlay to v1.5.
 - 3.Updated some primary fonts.
 

Telegram channel:

https://t.me/taichi91

Power by:

Yiyunlengyu(酷安@Numbersf)