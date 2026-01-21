CN
 
16.0.9.01-21-fix(1609012102)
 - 1.customize.sh:调整version_check检查逻辑及缩进，避免可能存在的误判及unknown operand
 - 2.customize.sh:修复检查安卓版本替换表情字体的逻辑在上一版本中被误删导致无法正确处理的问题
 - 3.action.sh:音量上下开启屏蔽gms字体逻辑对换，现在音量下是开始
 - 4.lang.sh:优化识别翻译语言方式
 - 5.新增日文、俄文翻译
 
16.0.7.01-20-beta9(1607012009)
 - 1.第一版本更新maple-font到7.9
 - 2.移除所有原先模块直接内置的font*.xml，现仅保留模块根目录的fonts.xml作为复制源，通过调用search_dirs.sh搜索字体配置文件绝对路径和文件名进行复制
 - 3.action.sh、customize.sh、search_dirs.sh的翻译改调用lang/lang.sh
 

-------
EN
 
16.0.9.01-21-fix(1609012102)
 - 1.customize.sh: Adjusted the version_check logic and indentation to avoid potential misjudgments and unknown operand errors.
 - 2.customize.sh: Fixed an issue where the logic for checking the Android version to replace emoji fonts was accidentally removed in the previous release, causing incorrect handling.
 - 3.action.sh: Swapped the volume key behavior for enabling GMS font blocking — Volume Down now starts the process.
 - 4.lang.sh: Optimized the method for detecting the translation language.
 - 5.Added Japanese and Russian translations.
 
16.0.7.01-20-beta9(1607012009)
 - 1.Initial release updating maple-font to v7.9
 - 2.Removed all previously built-in font*.xml files from the module. Only fonts.xml in the module root is now kept as the copy source. Font configuration files are discovered via search_dirs.sh, which searches for their absolute paths and filenames before copying.
 - 3.Translations in action.sh, customize.sh, and search_dirs.sh are now handled by calling lang/lang.sh
 

Telegram channel:

https://t.me/taichi91

Power by:

Yiyunlengyu(酷安@Numbersf)