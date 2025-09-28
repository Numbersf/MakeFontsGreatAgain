**`Enlish`** | [简体中文](README.md)
 
[![Channel](https://img.shields.io/badge/Follow-Telegram-blue.svg?logo=telegram)](https://t.me/taichi91) 
 
# MFGA
> [!CAUTION]
> 
> This module is in the testing phase. Since action.sh is used, it requires magisk28.0+,alpha28001+, KernelSU 11986/1.0.2+,(M/R)KernelSU(NEXT)12300+ to be flashed! ! ! 
 
> [!IMPORTANT]
>
> Note! Executing action.sh means clicking the execute (operation) button in the manager
> 
> KernelSU,apatch series kernel managers, please turn off the default uninstall module², Delta Magisk lacks updates and cannot be adapted
> 
>>[If the settings interface cannot be opened by Google or the Play store cannot be opened, please enable the (magisk)hide/uninstall module(ksu)² for the all play service](This is a problem with the "play integrity fix" module「pif」)

 
>>¹ and ² do not conflict, read carefully
 
>MFGA theory adaptation:
```
-  All major categories of native (A9~A16)
-  ColorOS13-16(Please enable Roboto in settings)
-  OxygenOS 12-16
-  RealmeUI
-  Flyme
-  MIUI
-  HyperOS 1-2
-  Samsung
```
 
```[Global Debut (1)]``` Full coverage of English and numbers in the Play Store
 
```[Unique feature]``` Full coverage of WeChat public account articles and their contents
> Huge font library, covering some rare new characters(Mom no longer has to worry about my phone not seeing any symbols~)
 
> If the Play Store does not take effect, please click Execute (action.sh) in the manager and wait for the text to run. No need to restart.
 
```[Most efficient]```MFGA 15.5.1 version,Unicode16 is fully covered;15.8.2 version,Unicode17 is fully covered.... It is equivalent to any symbol officially recognized as existing, of course there are also some private area symbols used by the system official.
 
```[Customization]```Starting from version 15.9.0, the customized content is more visual
 
#  Update log:
 
 16.0.3.09-27-beta3 (1603092703)
 - 1.Fixed the two missing symbols in HarmonyOS 5 private area extension A
```
U+F0AFE(󰫾), U+F0ADD(󰫝)
```
 - 2.Optimized WebUI – added execution results, font previews in the font directory, highlighted section hints, and fixed the zooming issue
 - 3.Rewrote README-en.md
 
 16.0.2.09-18-beta2(1602091802)
 - 1.Completed the missing characters in the Tangut Radicals Supplement block and the Tangut Supplement block.
 - 2.Updated some fonts to the latest version (including updating NotoColorEmoji to Unicode 17).
 - 3.Optimized the formatting of the Chinese README.md.
 - 4.Removed U+1F6D8 from Unicode17-new.
 
 16.0.1.09-15-beta1(1601091501)
 - 1.Added WebUI. New feature: block individual fonts (.ttf, .otf, .ttc). Blocking the main system font is not allowed; the font must come from the module path.
 - 2.Adopted a new deduplication method to completely fix conflicts between NotoColorEmoji.ttf and Symbola-13.otf / Unknown-symbol-supplementRegular.ttf that caused abnormal rendering of some symbol combinations.
 - 3.Adapted for Android 9–11, and resolved the issue where systems below Android 10 could not recognize COLRv1, which made NotoColorEmoji.ttf ineffective.
 - 4.Adapted for Android 16.
 
 15.9.2.08-24(1592082400)
 - 1.Updated the main font and some secondary fonts
 - 2.Fixed the issue of some symbols missing again
 - 3.Adapted to approximately 3,000 new symbols added in HarmonyOS 5, all of which are in the U + F0000 Supplementary Private Use Area - A
 
 15.9.1.06-27(1591062700)
 - 1.Fixed issues with mixing certain symbols and punctuation marks
 - 2.Fixed missing important combining marks in two primary fonts
 - 3.Fixed 5 missing characters in the Emoji and Old Italic blocks (U+1032D..1032F, U+1F547..1F548)
```
𐌭𐌮𐌯🕇🕈
```
 
 15.9.0.06-01-RCTEST(1590060101)
 - 1.Removed conflicting characters from Unknown-symbol-supplementRegular.ttf
 - 2.Removed NazoMin+-Regular.otf
 - 3.Upgraded font configuration to Android 15:Some fonts have been renamed、A few font suffixes changed、Added DroidSansMono.ttf
 - 4.Fixed invisibility issues in certain cases for combining characters: U+0B67, U+2364, U+20DD
```
୧⍤⃝
```
 - 5.Switched main fonts to .ttf format, using numeric values to represent weights
 - 6.Changed some NotoSans font suffixes from -Medium to -VF for consistency with Android 15;this avoids potential font conflict or duplication issues
 
 15.8.5.05-27-beta13(1585052713)
 - 1.Updated UnicodiaFunky.ttf to the latest version to fix the display issue of U+20C1`⃁`in Unicode 17.0.
 - 2.Resolved the issue where punctuation marks following certain emoji would appear as invisible spaces by removing all related punctuation symbols from Symbola-13.otf. This operation does not affect the main font, as emoji display primarily relies on NotoColorEmoji, and Symbola-13.otf only serves as a supplement. Around 200 symbols still rely on Symbola-13.otf for display. If further issues arise, they will be addressed later.
 
 15.8.4.05-15-beta12(1584051512)
 - 1.Fix the issue that Flyme12 cannot be used
 
 15.8.3.05-10-beta11(1583051011)
 - 1.Fixed possible call errors of three ttf (SELFUSE does not have this problem)
 - 2.Update Maplemono to 7.2, Iosevka to 33.2.2
 - 3.Remove NotoSansSymbols2.ttf
 
 15.8.2.05-04-beta10(1582050410)
 - 1.Update Plangotic to 2.9.5777, delete the conflicting parts, and add the following Chinese characters in the extended E area
```
𫜿𬺢𬺣𬺤𬺥𬺦𬺧𬺨𬺩𬺪𬺫𬺬𬺭
```
 - 2.Fix some strange space display issues in Unicode16-new.ttf
 - 3.Benchmark Unicode17 Arabic Presentation Forms-A 25 new symbols(U+FBC3..U+FBD2、U+FD90、U+FD91、U+FDC8..U+FDCE)
```
﷈﷉﷊﷋﷌﷍﷎﶑﶐﯃﯄﯅﯆﯇﯈﯉﯊﯋﯌﯍﯎﯏﯐﯑﯒
```
 
 15.8.1.04-19-beta9(1581041909)
 - 1.Fixed a priority issue caused by an indentation error in font.xml
 - 2.Removed redundant parts of font.xml and renamed some .ttf files
 - 3.Added ZUno-Number.ttf to display all deleted or disabled Unicode symbols before U+FFEF (excluding the Private Use Area), similar to unifont, covering around 1100 symbols
```
For example, the removed 無 --> ⺚
```
 
 15.8.0.04-17-beta8(1580041708)
 - 1.Private-UseTest.ttf: Added some PUA symbol support (U+E880...U+E887, U+E890...U+E895, U+E898...U+E89C, U+E8A0...U+E8B7), a total of 41 symbols
```

```
 
 15.7.9.03-30-beta7(1579033007)
 - 1.Removed U+A7F2...U+A7F4 (Unicode14) from PlangothicP2 to align it with the newly added symbol U+A7F1 in Unicode17.
```
ꟲꟳꟴ
```
 - 2.action.sh: Force close Chrome and Gmail apps immediately at the start to fix the potential issue of "app not opening and restarting".
 - 3.action.sh: Optimized some Chinese and English translations.
 
 15.7.8.03-30-beta6(1578033006)
 - 1.```[Global Debut (17)]``` Leading support for the final newly added block in Unicode 17.0 – the TaiYo block (U+1E6C0..U+1E6FF), consisting of 55 symbols:
```
𞛀 𞛁 𞛂 𞛃 𞛄 𞛅 𞛆 𞛇 𞛈 𞛉 𞛊 𞛋 𞛌 𞛍 𞛎 𞛏
𞛐 𞛑 𞛒 𞛓 𞛔 𞛕 𞛖 𞛗 𞛘 𞛙 𞛚 𞛛 𞛜 𞛝 𞛞 𞛟
𞛠 𞛡 𞛢 𞛣 𞛤 𞛥 𞛦 𞛧 𞛨 𞛩 𞛪 𞛫 𞛬 𞛭 𞛮 𞛯
𞛰 𞛱 𞛲 𞛳 𞛴 𞛵 𞛶 𞛷 𞛸 𞛹 𞛺 𞛻 𞛼 𞛽 𞛾 𞛿
```
 - 2.Optimized the display of certain symbols.
 - 3.Updated all primary fonts and removed conflicting parts.
 - 4.customize.sh: Fixed an issue with retrieving the Android version number.
 - 5.action.sh: Added volume key selection functionality.
 
 15.7.7.03-16-beta5(1577031605)
 - 1.Updated Roboto to beta36
 - 2.Updated Bianhei to 3.6 and removed conflicting characters U+E020..U+E023
 The part about Unicode17:
```
Added 2 non-ideographic symbols: U+2B96 (⮖) and U+18D1D (𘴝)  
Aligned with the Unicode 17.0 draft, fixed 7 modified Chinese characters: 𮈺, 𠁗, 𣋰, 𣫲, 𥟌, 𩆬, 𬶘,Added 4 new Chinese characters: 𫜻, 𫜼, 𫜽, 𫜾
```
 
 15.7.6.03-03-beta3(1576030303)
 - 1.BraillePatterns+SPUA.ttf: supports full-color U+E16A, copied from U+EEEE
 - 2.Call the new font file Unicodia, which will be responsible for the adaptation of Unicode17.0 combined additional mark extended symbols, and has a higher priority than Unicode17-new
 
 15.7.5.03-02-beta2(1575030202)
 - 1.customize.sh: optimize the judgment of Xiaomi, Redmi, POCO devices
 - 2.Private-UseTest.ttf: add some PUA symbol support (U+F83D, U+F849..U+F84F, Nokia..U+F800..U+F81A)
```

```
 - 3.MFGA-SELFUSE version MFGA deletes all PUA symbols in NotoSansSC to ensure the normal display of symbols in Private-UseTest, Plangothic, BraillePatterns+SPUA
 
 15.7.4.03-02-beta1(1574030201)
 - 1.customize.sh: Added device brand detection.Enables `zygisk` for Xiaomi, Redmi, and POCO devices. Removes `zygisk` for other devices.You can now uninstall empty font modules！
 
 15.7.3.02-23-RC15(1573022315)
 - 1.Leading support for Unicode 17.0 Alpha's Chess Symbols block (U+1FA54..U+1FA57), with 4 new symbols:
```
🩔🩕🩖🩗
```
 - 2.Fixed a symbol error in Unicode 17's Symbols for Legacy Computing block (U+1FBFA):
```
🯺
```
 - 3.Leading support for Unicode 17.0 Alpha's Supplemental Arrows-C block (U+1F8D0..U+1F8D8), with 9 new chemical symbols:
```
🣐🣑🣒🣓🣔🣕🣖🣗🣘
```
 - 4.Leading support for new symbols in Unicode 17.0 Alpha from the Unicode 16 Symbols for Legacy Computing Supplement block (U+1CCFA, U+1CCFB, U+1CEBA..U+1CEBF), with 8 new symbols:
```
𜳺𜳻𜺺𜺻𜺼𜺽𜺾𜺿
```
- 5.Leading support for an emoji in Unicode 17: Rolling Stone (U+1F6D8):
```
🛘
```
 
 15.7.2.02-22-RC13(1572022213)
 - 1.Added SatisarSharada-Regular.ttf to support the new partition Sharada Supplement (11B60...11B7F) of Unicode17.0, with a total of 8 symbols, of which 3 self-made symbols may have non-standard problems
```
𑭠 𑭡 𑭢 𑭣 𑭤 𑭥 𑭦 𑭧
```
 
 15.7.2.02-16-RC12(1572021612)
 - 1.```[Global Debut (16)]```leading support for the Chisoi block (U+16D80..U+16DA9) in Unicode 17.0 Alpha, with a total of 40 symbols:
```
𖶀𖶁𖶂𖶃𖶄𖶅𖶆𖶇𖶈𖶉𖶊𖶋𖶌𖶍𖶎𖶏𖶐𖶑𖶒𖶓𖶔𖶕𖶖𖶗 𖶘𖶙𖶚𖶛𖶜 𖶝𖶠𖶡𖶢𖶣𖶤𖶥𖶦𖶧𖶨𖶩
```
 - 2.Support for the U+1CCFC character in the Symbols for Legacy Computing Supplement block in Unicode 17.0 Alpha:
```
𜳼
```
- 3.Optimize the symbol size of U+1CEE0..U+1CEF0
- 4.Synchronize upstream fonts
 
 15.7.1.01-21-RC11(1571012111)
 - 1.Fixed a display error caused by incorrect classification of characters in the range U+1AEC..U+1AEF in the previous version. Further adjustments will be made later, but this update addresses the critical issue
 - 2.Added support for supplementary symbols from multiple sections of Unicode 17.0 (U+A7F1, U+A7D2, U+A7D4, U+088F, U+09FF, U+A7CF)
```
꟱꟒꟔࢏৿꟏
```
 
```
Combining additional symbols:
A᫟A᫯A᫮A᫞A᫭A᫬A᫰
```
 - 3.```[Global Debut (15)]``` Leading support for the Sidetic section in the draft of Unicode 17.0 (U+10940..U+1095F) with a total of 29 symbols.
```
𐥀𐥁𐥂𐥃𐥄𐥅𐥆𐥇𐥈𐥉𐥊𐥋𐥌𐥍𐥎𐥏𐥐𐥑𐥒𐥓𐥔𐥕𐥖𐥗𐥘𐥙𐥚𐥛𐥜
```
 
 15.7.0.01-21-RC10(1570012110)
 - 1.```[Global Debut (14)]``` First to support 26 supplementary symbols across multiple partitions of Unicode 17.0 (U+209D...U+209F, U+1ADF, U+1AEF, U+1AEE, U+1ADE, U+1AED, U+1AEC, U+1AF0, U+1879, U+0984, U+0558, U+058C, U+058B, U+20C1, U+A7E2, U+AB6C, U+AB6D, U+107BB...U+107BE, U+1F1AE, U+1F7DA)
```
₝₞₟᡹঄՘֌֋𐞿⃁꟢꭬꭭𐞻𐞼𐞽𐞾🆮🟚
```
 
 15.6.9.01-19-RC9(1569011909)
 - 1.```[Global Debut (11)]``` Leading support for the Musical Symbols Supplement block (U+1D250..U+1D28F) with 50 symbols in Unicode 17.0 (18.0)
```
𝉐𝉑𝉒𝉓𝉔𝉕𝉖𝉗𝉘𝉙𝉚𝉛𝉜𝉝𝉞𝉟𝉠𝉡𝉢𝉣𝉤𝉥𝉦𝉧𝉩𝉫𝉬𝉭𝉮𝉯𝉰𝉱𝉲𝉳𝉴𝉵𝉶𝉷𝉸𝉹𝉺𝉻𝉼𝉽𝉾𝉿𝊀𝊁
```
 - 2.```[Global Debut (12)]``` Leading support for new symbols in the Alchemical Symbols block (U+1F777, U+1F778, U+1F779) from the Unicode 17.0 draft
```
🝷🝸🝹🝺
```
 - 3.```[Global Debut (13)]``` Leading support for 23 new symbols in the Musical Symbols block (U+1D127, U+1D128, U+1D1EB..U+1D1FF) from the Unicode 17.0 draft
```
𝇫𝇬𝇭𝇮𝇯𝇰𝇱𝇲𝇳𝇴𝇵𝇶𝇷𝇸𝇹𝇺𝇻𝇼𝇽𝇾𝇿𝄧𝄨
```
 
 15.6.8.01-14-RC8(1568011408)
 - 1. Optimize U+2B96 in Unicode17.0 to make it consistent with the U+2BF9 font
```
⮖(U+2B96)⯹(U+2BF9)
```
 
 15.6.8.01-12-RC7(1568011207)
 - 1.Adapt some symbols in Samsung private use area, such as positioning symbol
```

```
 - 2.Add a TTF to manage the customization of Private Use partition (Private-UseTest.ttf)
 - 3.The private area symbol U+F8FE is upgraded to the MFGA module-specific symbol
```

```
 
 15.6.7.01-11-RC6(1567011106)
 - 1.```[Global Debut (9)]``` Leading support for MiscelLaneous Symbols and Arrows partition (U+2B96) in the Unicode 17.0 draft
```
⮖
```
 - 2.```[Global Debut (10)]``` Leading support for Tolong Siki partition (U+11DB0..U+11DEF) in the Unicode 17.0 draft
```
𑶰𑶱𑶲𑶳𑶴𑶵𑶶𑶷𑶸𑶹𑶺𑶻𑶼𑶽𑶾𑶿𑷀𑷁𑷂𑷃𑷄𑷅𑷆𑷇𑷈𑷉𑷊𑷋𑷌𑷍𑷎𑷏𑷐𑷑𑷒𑷓𑷔𑷕𑷖𑷗𑷘𑷙𑷚𑷛𑷠𑷡𑷢𑷣𑷤𑷥𑷦𑷧𑷨𑷩
```
 - 3.Reduce kernelSU limit version to 11986
 
 15.6.6.01-08-RC5(1566010805)
 - 1.```[Global Debut (7)]``` Leading support for the MISCELLANEOUS SYMBOLS SUPPLEMENT partition in the Unicode 17.0 draft(U+1CEC0..U+1CEFF)
PARTHENOPE,HEBE,EGERIA,FLORA,THETIS,AMPHITRITE,METIS,IRENE,MELPOMENE,etc.
```
𜻀𜻁𜻂𜻃𜻄𜻅𜻆𜻇𜻈𜻉𜻊𜻋𜻌𜻍𜻎𜻏𜻐𜻠𜻡𜻢𜻣𜻤𜻥𜻦𜻧𜻨𜻩𜻪𜻫𜻬𜻭
```
 - 2.```[Global Debut (8)]``` Leading support for the Beria Erfe zone (U+16EA0..U+16EDF) in the Unicode 17.0 draft
A total of 50 symbols
```
𖺠𖺡𖺢𖺣𖺤𖺥𖺦𖺧𖺨𖺩𖺪𖺫𖺬𖺭𖺮𖺯𖺰𖺱𖺲𖺳𖺴𖺵𖺶𖺷𖺸𖺻𖺼𖺽𖺾𖺿
```
 
 15.6.4.01-07-RC2(1564010701)
 - 1.Update Roboto to V7.0 Beta34
```
Roboto upstream update log:
a.sharpen crossing corners
b.optimize λ
c.make plain text tag italic in italic style
d.add ignore on >=? for rescript
e.improve currency symbols
f.improve miniute and second symbol
```
 
 15.6.3.01-04-RC1(1563010402)
 - 1.Optimize module installation detection and give some prompts
 - 2.KernelSU strictly limits installation above 11989
 - 3.More comprehensive Chinese-English translation Get the current system language and send instructions in the corresponding language Cover action.sh, customize.sh, service.sh
```
LANGUAGE=$(getprop persist.sys.locale | cut -d'-' -f1)
```
 
 15.6.1.01-02(1561010203)
 - 1.Adding a new update channel, you can update directly in the manager
 
 15.6.0.01-02(1560010201)
 - 1.Update Iosevka Medium to 32.3.1
 - 2.Update Roboto to V7.0 Beta33
```
Roboto upstream update log:
a.Fix visual alignment of w / Δ
b.Optimize α / Ω / ω / Ћ
c.Optimize italics đ / ŋ / þ / ә / τ / ι / γ
d.Optimize sub/sup numbers
e.Ensure all glyphs are vertically aligned
```
 - 3.Change module ID, it will be automatically uninstalled
 
 15.5.6.12-27-AI(1556122701)
 - 1.Update PlangothicP1 and PlangothicP2 to the latest versions and use ChatGPT to complete some patches mentioned in the previous update
 - 2.Update Iosevka Medium to 32.3.0
 
 15.5.5.12-19(1555121902)
 - 1.Delete U+24C2 in Noto Color Emoji
 - 2.Optimize action.sh execution logic and add click to exit
 - 3.Completely solve the problem.The manager stuck after sh execution
 
 15.5.3.12-15(1553121501)
 - 1.Update Roboto to V7.0 Beta32
```
Roboto upstream update log:
a.Correct prebuild nerd font glyphset and post.isFixedPitch: 0 -> 1 
b.Add ignore for list pattern matching in Erlang
c.Fix cv31, cv33, cv35 combos not working in italic style 
d.Add missing ℃, ℉ 
e.More identity info
```
 - 2.Delete U+1F17F, U+1F17E, U+1F171, and U+1F170 in Noto Color Emoji because the colors are inconsistent with U+1F172-U+1F18D (show the corresponding symbols in Symbola-13.otf instead)
- 3.Delete invalid font Noto Color Emoji-flag
 
 15.5.2.12-13(1552121302)
 - 1.Optimize module size
 - 2.Delete and redefine PlangothicP3
 
 15.5.1.12-13-OpenType(1551121301)
 - 1.Optimize the Gurung khema of Unicode16 and comply with the specification
```
U+16100-U+16139(U+1613F)
𖄀𖄁𖄂𖄃𖄄𖄅𖄆𖄇𖄈𖄉𖄊𖄓𖄋𖄌𖄔𖄍𖄕𖄎𖄖𖄏𖄗
```
 - 2.```[Global Debut (6)]```Unicode16 full coverage includes but is not limited to
```
Symbols for Legacy Computing Supplement
𜲒𜲓𜲔𜲕𜲖𜷾𜷿𜷽
Symbols for Legacy Computing
🯋🯍
Egyptian Hieroglyphs Extended-A
𓑠𓑢𓑡𓒃𓒋𓑵𓑵𓒎𓒖𓒤𓒜𓒣𓒫𓔅𓔌𓔃𓔂𓕍𓕔𓕛𓕣𓕦𓖤𓖳𓖽𓖾𓗆𓖞𓟣𓟳𓟊𓟅𓝹𓝩𓛱𓛺𓜋𓚢𓚤𓚡𓖡𓩓𓩒𓩞𓪥𓪯𓬻𓰕𓰓𓰟𓹄𓸄
Kannada
೜
Balinese
᭎᭏᭿
Garay
𐵀𐵁𐵂𐵓𐵛𐵚𐵪𐵫𐵻𐵤𐵍𐵕𐵣𐵴𐵽𐶄𐵻𐵍𐵆𐵇𐵏𐵔𐵣𐵫𐵼
Tulu-Tigalari
𑎀𑎁𑎂𑎃𑎄𑎅𑎆𑎇𑎎𑎖𑎜𑎤𑎚𑎳𑎻𑎲𑎡𑎣𑏌𑏍𑏎
Myanmar Extended-C
𑛑𑛒𑛓𑛔𑛕𑛖𑛗𑛘𑛙𑛚𑛛𑛜𑛝𑛞𑛟𑛠𑛡𑛢𑛣
Sunuwar
𑯀𑯂𑯂𑯋𑯌𑯒𑯑𑯙𑯔𑯍𑯛𑯕𑯴𑯰𑯱𑯹𑯷𑯶𑯸𑯟𑯡
Supplementary Arrows-C
🣀🣁
Arabic Extended-C
𐻂𐻃𐻄𐻼
And so on, a total of 5187 new characters,Here is just an example
```
 
 15.5.0.12-10-OpenType(1550121001)
 - 1.Reconstruct the bottom layer again and replace all the English numerals and basic symbols of the original Roboto
 - 2.Enable the OpenType ligature feature again@帆陌枫(maplemono)
```
[INFO][FIXME][DEBUG][ERROR][FATAL][WARN][TODO]
```
 - 3.Completely delete U+2000-U+200A from PlangothicP2-Regular
 
 15.4.1.12-08(1541120807)
 - 1.```[Global Debut (5)]``` Support for Ol Onal of Unicode16
```
U+1E5D0-U+1E5FF
𞗐𞗑𞗒𞗓𞗔𞗕𞗖𞗟𞗗𞗘𞗙𞗚𞗛𞗜𞗝𞗞𞗦𞗥𞗤𞗣𞗢𞗡𞗠𞗨𞗱𞗺𞗹𞗸𞗴𞗵𞗶𞗷
```
 
 15.4.0.12-08-EE(1540120801)
 - 1.Update Unknown symbol supplementRegular to 32.2.1
 - 2.Adjust the priority of Unknown symbol supplementRegular and Symbolola-13
 - 3.Added font_fallback.xml to solve the adaptation issues of A15 native and some C15 devices (fonts_fallback.xml will not be deleted)
 - 4.Delete the custom function to solve the problem of incorrect glyph characters in the root state, which does not support Unicode16 characters to display errors and swallow characters on some pages.
 
 15.3.7.12-06-RC3(1537120603)
 - 1.Complete the new characters in Unicode16 in Supplemental Arrows-C ~(except U+1F8B2,U+1F8C0,please wait for subsequent updates)~
```
🢴🢵🢶🢷🢸🢹🢺🢻
```
 - 2.Complete the new characters in Unicode16 in Symbols for Legacy Computing ~(except U+1FBCB, U+1FBCD,please wait for subsequent updates)~
```
🯌🯎🯏🯐🯑🯒🯓🯔🯕🯖🯗🯘🯙🯚🯛🯜🯝🯞🯟🯠🯢🯡🯣🯤🯥🯦🯧🯨🯩🯪🯫🯬🯭🯮🯯
```
 - 3.Fix due to fonts.LinearA partition caused by xml writing error (NotoSansLinearA-Regular.otf) The problem that all Unicode can't see
  
 15.3.6.12-06-RC2(1536120602)
- 1.Fix Tibetan display
- 2.Repair of missing symbols in Tibetan (U+0F8C-U+0F8F、U+0FCE, U+0FD0-U+0FDA) A total of 16 symbols
 
 15.3.6.12-06-RC1(1536120601)
 - 1.Urgently fix a large number of Unicode7.0-13.0 emoji missing
 - 2.Added Symbola-13.otf and NotoSansSymbols2-Regular.ttf to cover more emojis (Unicode 7-16)
 - 3.Fixed the problem caused by PlangothicP2-Regular.ttf that spaces U+2000-U+200A will be displayed no matter where (such as YouTube likes)
 
 15.3.5/6.12-03-AI(1535/61203)
 - This is beta version and not published
 
 15.3.4.12-02-AI(15341202)
 - 1.Use ChatGPT to complete most of the Unicode16.0 traditional computer Symbols Supplement  ~(there are still 70 characters that cannot be overwritten, I tried my best)~
```
𜰀𜰁𜰂𜰃𜰄𜰅𜰆𜰇𜰈𜰉𜰊𜰋𜰌𜰍𜰎𜰐𜰑𜰒𜰓𜰔𜰕𜰖𜰤𜰣𜰤𜰥𜴣𜴥𜴽𜵍𜵎𜵆𜰏
```
 - 2.Use ChatGPT to remove Kanchenjunga-Regular Arabic numerals and English uppercase and lowercase letters that come with ttf
 
 15.3.3.11-30-AI(15331130)
 - 1.Use ChatGPT to update and remove the Arabic numerals and English uppercase and lowercase letters that come with supplementary fonts (Noto, a total of 22 ttf/otf)

 15.3.2.11-30(15321130)
 - 1.Fixed the issue that the password input page still uses the default font when restarting on OnePlus devices(Please enable Roboto in settings)
 - 2.Fixed the issue that the font of the ColorOS15 note application cannot be overwritten due to calling sysfont(Please enable Roboto in settings)
 
 15.3.1.11-23-beta(153111232)
 - 1.Fixed the possible missing of 4 Devanagari Unicode characters
```
U+0953( ॓),U+0954( ॔),U+0971(ॱ),U+0978( ॸ)
```
 - 2.Added English translation
 - 3.```[Global Debut (4)]``` Support Unicode16 Gurung khema language
```
U+16100-U+16139(U+1613F)
𖄀𖄁𖄂𖄃𖄄𖄅𖄆𖄇𖄈𖄉𖄊𖄓𖄋𖄌𖄔𖄍𖄕𖄎𖄖𖄏𖄗
```
 
 15.3.0.11-22-alpha(1153011221)
 - 1.NotoSans minority language symbols are updated (2022→2024/2023) to improve coverage
 - 2.BraillePatterns+SPUA is updated to the latest version
 - 3.Modify an error in font.xml
 - 4.```[Global Debut (3)]``` Support Unicode16 Kirat Rai language
```
U+16D40-U+16D79(U+16D7F)
𖵀𖵁𖵂𖵃𖵄𖵅𖵆𖵇𖵈𖵉𖵊𖵋𖵌𖵍𖵎𖵏𖵐𖵑𖵒𖵓𖵕𖵔𖵜𖵞𖵖𖵗𖵝𖵢𖵫𖵪
```
 
 15.2.1.11-10-RC10(15211110)
 - 1.Try to solve the problem that A15 and Pengpai 2.0 cannot be used and the fonts are not fully displayed
 
 15.2.0.11-7-RC9(15201107)
 - 1.Delete invalid fonts
 - 2.Delete invalid commands  Order
 
 15.1.8.11-4-RC7(15181104)
 - 1.Optimize action.sh solves the problem that some people cannot use it, adds more detection (folder existence detection,PM detection to ensure safety)
 - 2.Add more text instructions for the manager to open the action
 - 3.No longer delete data/fonts, but delete data/fonts/*
 - 4.The folder will no longer be automatically deleted when the first flash is entered, and it will only be deleted when the manager manually executes action.sh
 
 15.1.7.11-4-RC6(15171104)
 - 1.Limit the minimum magisk version to 28.0 (28000)
 
 15.1.4.11-3-RC2(15141103)
 - 1.Send notification when the module is activated
 - 2.Completely integrate the killgmsfont module, you can uninstall it
 - 3.Optimize the action.sh shielding code
 - 4.Write a simple UI opening interface for the manager to open the action
 
 14.9.6.11-21(1496112100)
 - 1.```[Global Debut (2)]``` Added action.sh shortcut execution
(Move the content of the command to enable the global font of the Play Store in service.sh to action.sh, which can be executed directly in the manager instead of searching in the folder (15))
 
 14.9.3.09-15(1493091500)
 - 1.Optimize some module codes
 - 2.Update PlangothicP1-Regular.fallback and Plangothi again  cP2 to the latest version
 
 14.9.3.08-29(1493091200)
 - 1.Add full-color characters, double coverage of Braille and private supplementary area A Xiaomi Apple Orange logo
```
U+2800-U+28FF=U+FF000-U+FF0FF U+F8FF U+EEEE(U+E16A) U+EEEF U+F8FF  U+E005
(Xiaomi), (Xiaomi MIUI), (Apple), ⠨(magisk), 󿁝(OnePlus), etc.
```
 
 14.9.2.08-29(1492082900)
 - 1.Update all fonts to the latest version
 - 2.Fix Google  The problem that the Play Store still cannot cover
 - 3.Fix the problem that service.sh does not exist in the path
 - 4.Delete the private area ⿰又昔 in PlangothicP1 and change it to ⿰高考加油E020)
```

```
 - 5.Add private area U+E021-U+E023
```

```
 - 6.Add U+F0000 for the expansion of private area A
```
󰀀
```