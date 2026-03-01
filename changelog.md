CN
 
16.2.1.03-01-alpha2(1621030102)
 - 1.WebUI新增允许屏蔽字体区间，除区间外的部分将继续生效
```

1.
输入单个字体文件
点击屏蔽/恢复，重启生效
例:BraillePatterns.ttf
2.
输入单个字体文件+逗号[英]+括号[英](括号内填写Unicode编码)
点击屏蔽/恢复，立即生效，fallback字体需重启生效
例:BraillePatterns.ttf,(U+2800-U+28FF)
3*.
输入单个字体文件+逗号[英]+括号[英](括号内填写Unicode编码、区间之间用分号[英]隔开)
例:Private-UseTest.ttf,(U+F002B-U+F003B;U+F0038-U+F003F)

```
 - 2.支持WebUI屏蔽GMS字体，用于解决音量键无效的情况
 - 3.新定义U+E111为一加符号(非官方)
 
16.1.1.02-26-alpha1(1611022601)
 - 1.customize.sh:优化处理管理器版本逻辑，修复可能因为残留导致的误判
 - 2.customize.sh:非小米设备且product中未找到.xml时不再空挂载product分区
 - 3.重新加回NotoUnicode.otf，用于处理低安卓版本Noto系列字体版本过低导致的缺失问题
 - 4.新增NotoSansPro.otf，使大部分Noto系列字体保持最新；同时接入remove_emoji_overlap功能，解决部分符号组合显示冲突
 - 5.删除原先内置的所有Noto*-VF字体
 - 6.优化weiui缩放和按钮
 

-------
EN
 
16.2.1.03-01-alpha2(1621030102)
 - 1.WebUI now allows blocking specific font Unicode ranges. Glyphs outside the specified ranges will remain effective.
```

1.
Enter a single font file.
Click Disable/Restore. Takes effect after reboot.
ep:BraillePatterns.ttf
2.
Enter a single font file + comma + parentheses (Unicode range inside the parentheses).
Click Disable/Restore. Takes effect immediately. Fallback fonts require a reboot.
ep: BraillePatterns.ttf,(U+2800-U+28FF)
3*.
Enter a single font file + comma + parentheses (Unicode ranges inside the parentheses, separated by semicolons).
Example: Private-UseTest.ttf,(U+F002B-U+F003B;U+F0038-U+F003F)

```
 - 2.Added support for blocking GMS fonts in WebUI, which can resolve cases where volume buttons become ineffective.
 - 3.Newly define U+E111 as a OnePlus symbol (unofficial).
 
16.1.1.02-26-alpha1(1611022601)
 - 1.customize.sh: Optimized the logic for handling manager versions and fixed potential misjudgments caused by leftover files.
 - 2.customize.sh: On non-Xiaomi devices, the product partition will no longer be mounted empty if no .xml is found in the product directory.
 - 3.Re-added NotoUnicode.otf to fix missing font issues caused by outdated Noto series fonts on lower Android versions.
 - 4.Added NotoSansPro.otf to keep most Noto series fonts up to date; also integrated the remove_emoji_overlap feature to resolve display conflicts for certain symbol combinations.
 - 5.Removed all previously built-in Noto-VF* fonts.
 - 6.Optimized weiui scaling and buttons.
 

Telegram channel:

https://t.me/AndroidCoreLayer

Power by:

Yiyunlengyu(酷安@Numbersf)