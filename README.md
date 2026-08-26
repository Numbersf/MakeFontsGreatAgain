**`简体中文`** | [English](README-en.md)
 
[![Channel](https://img.shields.io/badge/Follow-Telegram-blue.svg?logo=telegram)](https://t.me/AndroidCoreLayer)
[![Coolapk](https://img.shields.io/badge/Follow-Coolapk-3DDC84?style=flat-square&logo=android&logoColor=white)](http://www.coolapk.com/u/28259173)
 
# MFGA

> 通过联合多个字体实现对 ``Unicode Latest`` 已定义字符的完整字形覆盖（含私用区和补充私用分区）。``Reserved`` 符号以及私用区未定义符号由 ``ZUno-Number.ttf`` 显示其编码信息。字体来源与许可见 [LICENSES.md](LICENSES.md)。

主要功能：
- 屏蔽 ``GMS`` 字体（通过 ``action.sh`` 或 WebUI），覆盖大部分 GAPPS；与 PIF 模块冲突，请对 play 服务和商店单独开启卸载模块
- 指定 ``ttf/otf`` 屏蔽字体区间，区间外部分继续生效
- 直接修改任意符号的颜色（COLRv0，Android 10+）
- WebUI 管理：屏蔽/恢复字体、区间屏蔽、字符上色、GMS 屏蔽、取消内置字体权限的应用列表

# 本 fork 定制

- **主字形为马路口圆体 Maruko Gothic SC**（Zen Maru Gothic 补字计划，OFL 开源，简体中文全覆盖）：100-500 字重 → Light 细体，600-900 → Medium
- **修复火狐花体/小型大写**：``cursive`` / ``casual`` / ``sans-serif-smallcaps`` 三个族指向主字体
- **安全加固**：unicode_filter 整表重建（修复屏蔽后保留字形丢失）、WebUI 注入/XSS/路径穿越防护
- **可配置**：``config/font_permission_apps``（取消内置字体权限的应用）、``config/gms_stop_apps``（GMS 屏蔽前强停的应用）
- **打包发布**：一键打包模块 zip + CI 自动构建 release，见下方"构建与打包"

# 理论适配

```
- 各大类原生 A9+
- ColorOS13+ (最佳效果请在设置中启用Roboto)
- OxygenOS 12+
- RealmeUI / Flyme / MIUI / 澎湃OS / 三星
```

# 更新日志

最新版本与完整历史见 [changelog.md](changelog.md)。

```
2026.08.26(20260826)
 - 1.本 fork 改用日期版本号
 - 2.主字形切换为马路口圆体 Maruko Gothic SC（简体全覆盖，默认 Light 细体）
 - 3.修复火狐花体/小型大写；安全加固；打包发布链路
```

# 构建与打包（本 fork）

- ``tools/package/build_module.sh``：组装可安装模块 zip。以 ``--base-zip`` 指定的上游 release 为字体基底，覆盖本仓库定制内容，自动生成 ``module.prop``，产出 ``dist/MFGA-<版本>.<versionCode>.zip`` 与 ``MFGA-SELFUSE-...zip``
- 示例：``tools/package/build_module.sh --base-zip MFGA-17.0.0.06-27-alpha.1717180001.zip --bin-dir tools/prebuilt/bin``
- ``tools/package/validate_module.py``：打包前质检（module.prop 字段、必需文件、fonts.xml 引用 ↔ system/fonts 对照、bin 完整性）
- ``.github/workflows/release.yml``：手动触发或推送 ``v*`` tag 自动：编译 C 工具 → 下载基底 → 打包 → 校验 → 上传 artifact 并创建 GitHub Release
- ``.github/workflows/update-font.yml``：每周自动重建 NotoSansPro 并开 PR 回写
- 不带 ``--base-zip`` 时只打包仓库内字体（会警告缺字，模块仍可安装但大部分字体退化为系统默认）
