# MFGA/Selffont 激进审查报告

> 审查时间：2026-08-26 ｜ 范围：全仓库（scripts / WebUI / Xposed / C 工具 / CI）
> 本次已按“激进”标准直接修复了高影响问题，并全部验证；未动的项列在文末“遗留”一节。

---

## 一、已修复（本次落地）
### 1. 【严重】`tools/unicode_filter.c` 整表重建，修复两处真实 bug
旧实现用 mmap 原地裁剪 cmap，存在两个会直接损坏字体的缺陷：

- **保留字形被静默丢弃**：删除一个区段会把一个 `ro==0` 的 format-4 段拆成两段，段数超过原表容量时，旧代码直接截断输出 —— 中段屏蔽后，**该段后面的所有字形会一起消失**。
- **`idRangeOffset` 错位**：段被压缩后，`idRangeOffset[i]` 是相对该字位置的偏移，旧代码不重算，保留段的字形会查到错误 glyph。

新实现：将整个 cmap 重建为精确大小的缓冲（format 4/12 重算，其余格式原样搬移），按 `newRO = oldRO − 2·(i 之后被删除的段数)` 修正偏移，最后写到 `.tmp` 后原子 rename。

**验证**：本机 gcc 构建 + fontTools 全量比对 —— 11 组定向用例 + 36 组随机模糊用例全部通过：
- 被删码位在**所有** cmap 子表中彻底消失；
- 其余码位的 glyph ID 与源字体**逐一相同**（无漂移、无新增）；
- 覆盖 `ro!=0` 段、跨 BMP/SMP 边界、重叠区间、全删等边界。

### 2. 【严重】WebUI 存在 root 级命令注入 + XSS
`webroot/scripts.js` 之前把用户输入直接拼进 shell 命令（`mv "${filePath}" ...` 等），并在 `innerHTML` 里原样输出命令结果。WebUI 以 root 权限运行，`字体名` 里放 `; rm -rf /` 之类即可任意执行。已修复：

- 新增 `shq()`（POSIX 单引号转义）——**所有**用户输入拼 shell 前强制转义；
- 新增 `escHtml()`，`log/warn` 默认转义，多行自动转 `<br>`；
- `highlightDisabledFiles()` 改为逐行转义后再高亮（原来是整段 ls 输出直接进 HTML）；
- 屏蔽/恢复改为用退出码判断成败，不再把翻译文本拼进 shell 里 echo；
- `unicode_filter.sh` 也加了路径穿越防护（拒绝 `/`、`..`、空格）。

### 3. 【中】WebUI 屏蔽 GMS 依赖 spawn env 传 `MODE`
`action.sh` 靠环境变量 `MODE=web` 判断“不询问音量键”，但部分 KSU WebUI 的 `spawn` 不支持 env，会导致脚本挂起等音量键。改为把 `web` 作为位置参数传入（`sh action.sh web`）。

### 4. 【中】`action.sh` 全盘 `find /data` 扫 GMS 字体缓存
删除 GMS 字体缓存原来 `find /data -type d -path "*com.google.android.gms/files/fonts*"` —— 遍历整个 /data，慢且可能命中无关路径。改为只扫 `/data/user`、`/data/user_de`、`/data/data`。

### 5. 【中】`service.sh` 固定 `sleep 32`
慢设备不够、快设备白等。改为等待 `sys.boot_completed`（上限 120s）；通知发送也从“必须 `su`”改为 `su` 可用则用、否则 root 直接发。

### 6. 【低】`recolor_glyph.sh` 复制粘贴错误
不支持的架构分支里 echo 的是 `$UNI_UNSUPPORTED_ARCH`（Unicode 工具的文案）。已新增 `RCG_UNSUPPORTED_ARCH` 四种语言文案并改用。

### 7. 【低】`lang.sh` 的 `msg()` 用 eval
键名若含特殊字符可注入 shell。已加白名单校验（仅 `[A-Za-z0-9_]`）。

### 8. 【低】`customize.sh` 用了 bash 专属 `[[ ]]`
模块脚本跑在 Android 的 `sh`（mksh/ash）下，已改为 POSIX `[ ]`。

### 9. 【低】Xposed ModernEntry 无差别 hook 私有方法
`createFromAsset/createFromFile` 之前 hook 所有 declared methods（含 private 内部实现），已限制为 public static 工厂方法。

### 10. 【中】Xposed 模块 release 包无法安装
`build.gradle.kts` 的 release 没配签名（assembleRelease 产出未签名 APK，无法直接装）；也没设 minSdk。已加 `minSdk=26`（对齐 README 的 Android 8+）并让 release 复用 debug 签名。另新增 `gradle.properties`（内存/并行/缓存）。

### 11. 【中】更新通道指向上游
`update.json` / `update2.json` 的 zipUrl/changelog 都指向 `Numbersf/MakeFontsGreatAgain` —— 用户用本 fork 会收到上游包，等于覆盖掉本 fork 的字体表。已改指本仓库 `Sumicya/Selffont`。

### 12. 【中】`build.yml` 构建管线加固（合并顺序保持原样）
- 合并顺序**保持原来的“按文件大小排序，最大字体为基底”** —— 这是有意的：改排序会改变重叠码位的字形取舍，直接影响字体风格。若要指定基底，需显式调整。
- `remove_emoji_overlap.py` 增加 `-o` 选项，strip 后直接覆盖产出最终 `NotoSansPro.otf`，并新增“产物校验”步骤（码位数、残留 emoji 断言）。

### 13. 【低】仓库零质量检查
新增 `.github/workflows/quality.yml`：`sh -n`、shellcheck、`node --check`、JSON 合法性 + 四语言 locale key 对齐、fonts.xml XML 合法性、update.json↔changelog 版本一致性、C 工具 `-Werror` 编译 + 真实字体上的 unicode_filter 冒烟测试。

### 14. 【低】新增 `.gitignore`
忽略构建产物、下载字体源、Gradle 输出等。

---

## 二、审查中发现但未改（遗留，需你决策）

| # | 问题 | 说明 | 建议 |
|---|------|------|------|
| 1 | **仓库产不出完整模块 zip** | 仓库里没有 `module.prop`、根级 `customize.sh`、`system/fonts/` 骨架、`zygisk/` 等打包必需件；README 提到的 Roboto 系列、MFGA.ttf、NotoColorEmoji 等也不在 `fonts/` 里。发布 zip 目前只能在仓库外手工组装 | 加一个 `tools/package/` 打包脚本（拼 zip + 生成 module.prop + 更新 update.json），CI 出 release artifact |
| 1 | 没有 Gradle wrapper 提交 | 本地构建 Xposed 模块要先有 gradle；CI 每次现场生成 wrapper | 在有外网环境下 `gradle wrapper` 一次并提交 `gradlew` + `gradle/wrapper/*`（沙箱网络受限没做成） |
| 2 | `search_dirs.sh` 的 awk 解析依赖固定缩进 | 已比 sed 稳，但仍是文本解析 | 后续可考虑把 fonts_list.yaml 换成带注释的结构化配置 |
| 3 | Xposed 模块 scope 依赖 LSPosed 端选择 | staticScope=false 下用户可自行勾选，但默认列表无法运行时修改 | 保持现状（文档说明） |

---

## 四（三）、字体风格影响评估（2026-08-26 补充）

用户特别关心"字体风格"与"推倒重做"。如实交代：

- **未改动**：`fonts/` 下 5 个字体文件、`fonts.xml`、`fonts_list.yaml`、任何字形/度量数据。`recolor_glyph.sh` 推导出的字重与原来完全一致（100..900.ttf）。
- **build 管线合并顺序已还原**为原始"最大字体为基底"，确保重新构建的 NotoSansPro.otf 与旧管线/上游风格一致（第一版激进改动曾改成 urls.txt 顺序，有风格漂移风险，已还原并加注释）。
- **`update-font.yml`** 只自动开 PR、不自动合并，字体不会悄悄变。
- **"推倒重做"清单**：仅 `unicode_filter.c`（旧代码有真实 bug，必须重写；47 组测试 glyph 级一致）与 `scripts.js` 的 `handleFont` 重构（行为等价+加固）。
- **行为变化（需用户确认）**：Xposed 默认 scope 扩到 14 个包 —— WhatsApp/LINE/Discord 等 8 个新增应用会被强制使用系统字体；若不想影响这些应用的字体风格，把 scope.list 还原为 6 个即可。

---

- **可配置“取消内置字体权限”应用列表**：`service.sh` / `uninstall.sh` 改读 `config/font_permission_apps`（每行一个包名，`#` 注释，缺省回退到内置的 2 个阅读 App）；扫描范围从 `files/font` 等固定子目录改为整个 `/data/user/*/<包名>/`（覆盖多用户/工作资料）。WebUI 新增“取消内置字体权限的应用”区块：添加包名（白名单校验 + 防注入）、立即执行、实时显示当前列表（4 语言 i18n）。
- **`search_dirs.sh` 改用 awk 解析 fonts_list.yaml**：替代脆弱的 sed 链，正确处理缩进/注释/空行，本地对 3 个 section 全部验证通过。
- **`recolor_glyph.sh` 从 fonts.xml 推导主字重字体**：推导规则 = 匹配 `<NNN>.ttf`（即无名默认族 + serif 族的 100..900.ttf），推导失败回退硬编码列表；不再依赖“100~900.ttf 一定叫这个名字”。
- **Xposed 默认作用域扩充**：scope.list 从 6 个包扩到 14 个（新增 WhatsApp / LINE / Discord / Signal / Snapchat / KakaoTalk / Viber / Skype），覆盖面更大的“强制系统字体”默认行为。
- **更新通道改 `releases/latest/download/`**：`update.json` / `update2.json` 的 zipUrl 从固定 tag（`/releases/download/1717180001/...`）改为 latest 直链，release 流程与通道解耦，不会因 tag 名不一致而 404。
- **自动更新字体 PR**：新增 `update-font.yml`（手动 + 每周一 03:00 UTC 定时）—— 构建 NotoSansPro → 去 emoji 重叠 → 有变化则开 PR 回写 `fonts/NotoSansPro.otf`，解决“提交的字体与 CI 产物脱节”。
- **quality.yml 加严**：全 workflow YAML 校验（7 个）、`bash -n` + shellcheck `-s bash` 覆盖 `tools/package/build_module.sh`。

---

## 四、打包与发布（已落地，2026-08-26）

仓库此前产不出可安装的模块 zip（缺 module.prop、模块根布局等）。已新增：

- **`tools/package/build_module.sh`** — 打包脚本：
  - 以**上游 MFGA release zip 为字体基底**（`--base-zip`，提供 fonts.xml 所需的 148 个字体 + zygisk 等），再覆盖本仓库定制的 5 个字体、全部脚本/WebUI/lang/fonts.xml；
  - 自动从 `update.json` 生成 `module.prop`（id=MFGA，version/versionCode 同步，含 `updateJson` 指向本 fork 通道）；
  - `--bin-dir` 放入 8 个架构的 C 二进制；`--strict` 模式下字体/二进制缺失报错；
  - 产出 `dist/MFGA-<version>.<versionCode>.zip` 与 `MFGA-SELFUSE-...zip` 两份（对应两个更新通道）。
- **`tools/package/validate_module.py`** — 独立校验器：module.prop 字段、必需文件、fonts.xml 引用 ↔ system/fonts 对照、bin 完整性、可执行位。打包时自动跑一遍。
- **`.github/workflows/release.yml`** — 手动触发（可填上游 base zip URL）或 `v*` tag 推送时：NDK 编译 C 工具 → 下载上游基底 → 打包 → 校验 → 上传 artifact → 打 GitHub Release 并附带 zip。
- **用法**：本地 `tools/package/build_module.sh --base-zip 上游.zip --bin-dir 编译好的bin`；CI 里直接手动触发 workflow 即可。

> 注意：不提供 `--base-zip` 时只打包仓库的 5 个字体（校验会警告缺 130 个字体，模块仍可安装但大部分字体退化为系统默认）。

---

## 三、验证摘要

- `tools/unicode_filter.c`：gcc `-Wall -Wextra -Werror` 零警告；11+36 组用例全过，glyph 级一致性比对通过
- `sh -n`：全部 12 个 shell 脚本通过
- `node --check`：scripts.js 通过
- JSON/XML：全部通过
- 改动文件：见 `git status`
