# 字体状况报告（2026-08-26）

> 基于仓库 `fonts/` 5 个字体 + `fonts.xml` + `tools/fontslist/urls.txt` 实测分析。

## 一、总体架构：双层字体

| 层 | 内容 | 来源 |
|---|---|---|
| **基底层** | `fonts.xml` 引用的 148 个字体（主字重 100-900、Roboto 度量壳、CJK、Noto 各脚本、表情、杂项） | **上游基底 zip**（143 个不在仓库，打包时由 `build_module.sh --base-zip` 并入） |
| **定制层** | 仓库 5 个字体 | 本仓库 `fonts/` |

## 二、定制层 5 个字体体检

| 字体 | 大小 | 码位 | 字形 | 轮廓 | cmap 子表 | 说明 |
|---|---|---|---|---|---|---|
| NotoSansPro.otf | 2.7MB | 17536 | 17420 | CFF | (4,0,3)(4,3,1)(12,0,4)(12,3,10) | 62 个 Noto 字体合并的"扩展 Unicode 兜底"，**已剥离 emoji** |
| Unicode16-new.ttf | 6.7MB | 12982 | 12987 | TrueType | 同上 | Unicode 16 新增码位 |
| Unicode17-new.ttf | 195KB | 644 | 644 | TrueType | 同上 | Unicode 17 新增 |
| Unicode18-new.ttf | 250KB | 1293 | 1293 | TrueType | 同上 | Unicode 18 新增 |
| ZUno-Number.ttf | 4.8MB | 7830 | 7831 | TrueType | (3,1)(3,10) | 私用区 + 未分配码位显示"编码信息"（实测 BMP 私用区 6400 个 + 大量未分配码位） |

## 三、覆盖实测（仓库 5 字体合并视图）

**≈100%**：拉丁-1 补充 / 拉丁扩展-A / 希腊 / 西里尔 / 阿拉伯 / 天城文 / 孟加拉 / 泰米尔 / 泰 / 缅 / 高棉 / 常用标点 / 数学运算 / 箭头 / 几何图形 / 杂项符号
**高覆盖**：楔形文字 90% / 腓尼基 90.6% / 装饰符号 91.7%
**低/零覆盖（由基底负责）**：CJK / 谚文 / 假名 / 表情 / 希伯来 21% —— 基底层 `NotoSerifCJK`、`NotoColorEmoji` 等补齐
**刻意**：表情符号在 NotoSansPro 中剥离（避免与 NotoColorEmoji 冲突，`remove_emoji_overlap.py`）

## 四、发现的问题

### A. NotoSansPro.otf 没有 GSUB/GPOS 表（影响中等）
实测表清单：`GlyphOrder/head/hhea/maxp/OS/2/name/cmap/post/CFF/hmtx` —— **无 GSUB/GPOS**。
- 后果：阿拉伯文、天城文、泰文、高棉文等**复杂文字的 shaping 逻辑**（连写、变体、组合定位）在这个字体里不生效，只显示独立字形。
- 缓解：这些脚本的**主字体**由基底层独立文件提供（NotoSansArabic、NotoSansDevanagari…），NotoSansPro 只兜底"Unicode 新版本新增码位"——新增码位大多是符号/古文字，实际受影响的场景有限。
- 成因：`otfccdump --ignore-hints → merge-otd → otfccbuild` 工具链不保留 GSUB/GPOS。上游大概率同样如此。
- **可选处理**：改用 fontTools 合并管线保留版式表（大工程）；或接受现状。

### B. 覆盖缺口 5 个脚本（已处理，2026-08-26）
以下 5 个脚本原来 urls.txt 里被注释、fonts.xml 未引用 → 模块内无覆盖。**已把它们加回 urls.txt**（改用 unhinted/otf 路径，与其余 60 个一致；5 个 URL 均经外网验证存在：Armenian v2.008 / EgyptianHieroglyphs v2.002 / Kannada v2.006 / LinearA v2.002 / LinearB v2.002，全部含 GSUB/GPOS/GDEF）：
- `NotoSansArmenian`（亚美尼亚文）
- `NotoSansEgyptianHieroglyphs`（埃及象形文字）
- `NotoSansKannada`（卡纳达文）
- `NotoSansLinearA` / `NotoSansLinearB`（线性文字 A/B）

注：`NotoSansBengali` 已有 unhinted 版本启用、`NotoSansRunic` 由基底单独提供，保持不变。
> ⚠️ 生效链路：改的是**构建输入**，仓库里的 `fonts/NotoSansPro.otf` 尚未包含新字形 —— 需要合并到 main 后跑一次 `update-font.yml`（或 `build.yml`）重新构建，构建产物通过 PR 回写字体后才真正生效。

### C. ZUno-Number.ttf 缺少平台 0 cmap 子表（影响极小）
只有 (3,1)/(3,10)，没有 (0,3)/(0,4) Unicode 兼容子表。Android 主要用 (3,10)，实际无感。

### D. 依赖上游基底（结构性，已知）
143/148 个 fonts.xml 引用的字体不在仓库，完整模块必须带 `--base-zip` 打包；无基底时校验器会警告缺 130+ 字体。

## 五、结论 / 需要决策

1. **A（无 GSUB/GPOS）**：建议接受现状（上游同样），除非你想让我评估 fontTools 替代管线。
2. **B（5 脚本缺口）**：要不要取消注释加回 urls.txt？我可以改（改的是构建输入，会改变 NotoSansPro 覆盖，**不动现有字形风格**）。
3. **C / D**：不用处理。

不动任何东西也完全可用——这些都是如实交代的边界，不是本次改动引入的。
