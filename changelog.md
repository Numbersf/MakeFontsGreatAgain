CN
 
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
 
16.1.0.01-23-fix3(1610012303)
 - 1.lang.sh:适配更多场景，改为符合BCP 47的新识别写法
 - 2.search_dirs.sh:新增黑白名单功能，由模块根目录fonts_list.yaml控制
 

-------
EN
 
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
 
16.1.0.01-23-fix3(1610012303)
 - 1.lang.sh: Adapted for more scenarios; switched to a new recognition format compliant with BCP 47.
 - 2.search_dirs.sh: Added whitelist/blacklist support, controlled by fonts_list.yaml in the module root directory.
 

Telegram channel:

https://t.me/AndroidCoreLayer

Power by:

Yiyunlengyu(酷安@Numbersf)