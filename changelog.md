CN
 
16.2.2.03-15-alpha3(1622031503)
 - 1.适配Unicode18.0 Miscellaneous Symbols and Arrows Extended分区符号(U+1DB00..U+1DB1C)
```
𝬀𝬁𝬂𝬃𝬄𝬅𝬆𝬇𝬈𝬉𝬊𝬋𝬌𝬍𝬎𝬏𝬐𝬑𝬒𝬓𝬔𝬕𝬖𝬗𝬘𝬙𝬚𝬛𝬜
```
 - 2.适配Unicode18.0部分散装符号比如几何形状扩展、杂类符号补充分区
```
几何形状扩展:
🟱🟲🟳🟴🟵🟶🟷🟸🟹🟺🟻🟼🟽🟾🟿
杂类符号补充:
𜻒𜻓𜻔𜻝𜻞𜻟𜻱𜻲𜻳𜻴𜻵𜻶𜻷𜻸𜻹𜻺𜻻𜻼𜻽
```
 - 3.主字体同步、修复主分支同步问题、优化部分逻辑
 
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
 

-------
EN
 
16.2.2.03-15-alpha3(1622031503)
 - 1.Added support for symbols in the Unicode 18.0 Miscellaneous Symbols and Arrows Extended block (U+1DB00..U+1DB1C).
```
𝬀𝬁𝬂𝬃𝬄𝬅𝬆𝬇𝬈𝬉𝬊𝬋𝬌𝬍𝬎𝬏𝬐𝬑𝬒𝬓𝬔𝬕𝬖𝬗𝬘𝬙𝬚𝬛𝬜
```
 - 2.Added support for some Unicode 18.0 scattered symbols, such as those in the Geometric Shapes Extended、Miscellaneous Symbols Supplement block.
```
Geometric Shapes Extended:
🟱🟲🟳🟴🟵🟶🟷🟸🟹🟺🟻🟼🟽🟾🟿
Miscellaneous Symbols Supplement:
𜻒𜻓𜻔𜻝𜻞𜻟𜻱𜻲𜻳𜻴𜻵𜻶𜻷𜻸𜻹𜻺𜻻𜻼𜻽
```
 - 3.Synced the main font, fixed issues with main branch synchronization, and optimized some logic.
 
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
 

Telegram channel:

https://t.me/AndroidCoreLayer

Power by:

Yiyunlengyu(酷安@Numbersf)