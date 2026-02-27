CN
 
16.1.1.02-26-alpha1(1611022601)
 - 1.customize.sh:优化处理管理器版本逻辑，修复可能因为残留导致的误判
 - 2.customize.sh:非小米设备且product中未找到.xml时不再空挂载product分区
 - 3.重新加回NotoUnicode.otf，用于处理低安卓版本Noto系列字体版本过低导致的缺失问题
 - 4.新增NotoSansPro.otf，使大部分Noto系列字体保持最新；同时接入remove_emoji_overlap功能，解决部分符号组合显示冲突
 - 5.删除原先内置的所有Noto*-VF字体
 - 6.优化weiui缩放和按钮
 
16.1.0.02-23-fix6(1610022306)
 - 1.search_dirs.sh:新增撤销功能，你可以在模块根目录添加一个font*.xml并在fonts_list.yaml的reverse下方添加font*.xml，作为同名系统字体配置的新复制源而非全部使用模块根目录的fonts.xml
 - 2.search_dirs.sh:优化关闭功能，在任意- font*.xml前加上#则此行不处理
 - 3.customize.sh:优化小米设备的判断，根据ro.mi.os.version.name或ro.miui.ui.version.name是否存在决定是否处理
 - 4.适配Unicode18.0 Archaic Cuneiform Numerals分区符号(U+12550..U+1268F)
```
𒕐𒕑𒕒𒕓𒕔𒕕𒕖𒕗𒕘𒕙𒕚𒕛𒕜𒕝𒕞𒕟𒕠𒕡𒕢𒕣𒕤𒕥𒕦𒕧𒕨𒕩𒕪𒕫𒕬𒕭𒕮𒕯𒕰𒕱𒕲𒕳𒕴𒕵𒕶𒕷𒕸𒕹𒕺𒕻𒕼𒕽𒕾𒕿𒖀𒖁𒖂𒖃𒖄𒖅𒖆𒖇𒖈𒖉𒖊𒖋𒖌𒖍𒖎𒖏𒖐𒖑𒖒𒖓𒖔𒖕𒖖𒖗𒖘𒖙𒖚𒖛𒖜𒖝𒖞𒖟𒖠𒖡𒖢𒖣𒖤𒖥𒖦𒖧𒖨𒖩𒖪𒖫𒖬𒖭𒖮𒖯𒖰𒖱𒖲𒖳𒖴𒖵𒖶𒖷𒖸𒖹𒖺𒖻𒖼𒖽𒖾𒖿𒗀𒗁𒗂𒗃𒗄𒗅𒗆𒗇𒗈𒗉𒗊𒗋𒗌𒗍𒗎𒗏𒗐𒗑𒗒𒗓𒗔𒗕𒗖𒗗𒗘𒗙𒗚𒗛𒗜𒗝𒗞𒗟𒗠𒗡𒗢𒗣𒗤𒗥𒗦𒗧𒗨𒗩𒗪𒗫𒗬𒗭𒗮𒗯𒗰𒗱𒗲𒗳𒗴𒗵𒗶𒗷𒗸𒗹𒗺𒗻𒗼𒗽𒗾𒗿𒘀𒘁𒘂𒘃𒘄𒘅𒘆𒘇𒘈𒘉𒘊𒘋𒘌𒘍𒘎𒘏𒘐𒘑𒘒𒘓𒘔𒘕𒘖𒘗𒘘𒘙𒘚𒘛𒘜𒘝𒘞𒘟𒘠𒘡
```
 - 5.优化Unicode18.0 Musical Symbols Supplement分区的部分符号
 - 6.更新UnicodiaFunky到3.1.0
 

-------
EN
 
16.1.1.02-26-alpha1(1611022601)
 - 1.customize.sh: Optimized the logic for handling manager versions and fixed potential misjudgments caused by leftover files.
 - 2.customize.sh: On non-Xiaomi devices, the product partition will no longer be mounted empty if no .xml is found in the product directory.
 - 3.Re-added NotoUnicode.otf to fix missing font issues caused by outdated Noto series fonts on lower Android versions.
 - 4.Added NotoSansPro.otf to keep most Noto series fonts up to date; also integrated the remove_emoji_overlap feature to resolve display conflicts for certain symbol combinations.
 - 5.Removed all previously built-in Noto-VF* fonts.
 - 6.Optimized weiui scaling and buttons.
 
16.1.0.02-23-fix6(1610022306)
 - 1.search_dirs.sh: Added an undo feature. You can place a font*.xml in the module root directory and add font*.xml under reverse in fonts_list.yaml as a new copy source for system fonts with the same name, instead of always using the module root fonts.xml.
 - 2.search_dirs.sh: Optimized the disable function. Adding # before any font*.xml line will skip processing that line.
 - 3.customize.sh: Improved Xiaomi device detection. Whether ro.mi.os.version.name or ro.miui.ui.version.name exists determines if processing is done.
 - 4.Adapted to Unicode 18.0 Archaic Cuneiform Numerals block symbols (U+12550..U+1268F):
```
𒕐𒕑𒕒𒕓𒕔𒕕𒕖𒕗𒕘𒕙𒕚𒕛𒕜𒕝𒕞𒕟𒕠𒕡𒕢𒕣𒕤𒕥𒕦𒕧𒕨𒕩𒕪𒕫𒕬𒕭𒕮𒕯𒕰𒕱𒕲𒕳𒕴𒕵𒕶𒕷𒕸𒕹𒕺𒕻𒕼𒕽𒕾𒕿𒖀𒖁𒖂𒖃𒖄𒖅𒖆𒖇𒖈𒖉𒖊𒖋𒖌𒖍𒖎𒖏𒖐𒖑𒖒𒖓𒖔𒖕𒖖𒖗𒖘𒖙𒖚𒖛𒖜𒖝𒖞𒖟𒖠𒖡𒖢𒖣𒖤𒖥𒖦𒖧𒖨𒖩𒖪𒖫𒖬𒖭𒖮𒖯𒖰𒖱𒖲𒖳𒖴𒖵𒖶𒖷𒖸𒖹𒖺𒖻𒖼𒖽𒖾𒖿𒗀𒗁𒗂𒗃𒗄𒗅𒗆𒗇𒗈𒗉𒗊𒗋𒗌𒗍𒗎𒗏𒗐𒗑𒗒𒗓𒗔𒗕𒗖𒗗𒗘𒗙𒗚𒗛𒗜𒗝𒗞𒗟𒗠𒗡𒗢𒗣𒗤𒗥𒗦𒗧𒗨𒗩𒗪𒗫𒗬𒗭𒗮𒗯𒗰𒗱𒗲𒗳𒗴𒗵𒗶𒗷𒗸𒗹𒗺𒗻𒗼𒗽𒗾𒗿𒘀𒘁𒘂𒘃𒘄𒘅𒘆𒘇𒘈𒘉𒘊𒘋𒘌𒘍𒘎𒘏𒘐𒘑𒘒𒘓𒘔𒘕𒘖𒘗𒘘𒘙𒘚𒘛𒘜𒘝𒘞𒘟𒘠𒘡
```
 - 5.Optimized some symbols in the Unicode 18.0 Musical Symbols Supplement block.
 - 6.Updated UnicodiaFunky to version 3.1.0.
 

Telegram channel:

https://t.me/AndroidCoreLayer

Power by:

Yiyunlengyu(酷安@Numbersf)