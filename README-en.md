**`English`** | [简体中文](README.md)

[![Channel](https://img.shields.io/badge/Follow-Telegram-blue.svg?logo=telegram)](https://t.me/AndroidCoreLayer)

# MFGA

> Combines multiple fonts to achieve complete glyph coverage of all characters defined in ``Unicode Latest`` (including the Private Use Area and Supplementary Private Use Area). ``Reserved`` symbols and undefined PUA symbols are shown with their codepoint info by ``ZUno-Number.ttf``. Font sources & licenses: [LICENSES.md](LICENSES.md).

Main features:
- Block ``GMS`` fonts (via ``action.sh`` or WebUI) to cover most GAPPS; conflicts with PIF modules — enable the uninstall module for Play services/Store only
- Block Unicode ranges in a specific ``ttf/otf`` while the rest keeps working
- Recolor any glyph directly (COLRv0, Android 10+)
- WebUI: block/restore fonts, range blocking, glyph recoloring, GMS blocking, and a configurable list of apps whose built-in fonts are revoked

# This fork

- **Main glyphs: Maruko Gothic SC** (a Zen Maru Gothic extension project, OFL-licensed, full Simplified Chinese coverage): weights 100-500 → Light, 600-900 → Medium
- **Fixed Firefox italic/small-caps**: ``cursive`` / ``casual`` / ``sans-serif-smallcaps`` now point to the main font
- **Security hardening**: unicode_filter full cmap rebuild (fixes lost glyphs after blocking), WebUI injection/XSS/path-traversal protection
- **Configurable**: ``config/font_permission_apps`` (apps with built-in font access revoked), ``config/gms_stop_apps`` (apps force-stopped before GMS blocking)
- **Packaging & release**: one-command module zip build + CI release pipeline (see below)

# Supported

```
- Stock Android 9+
- ColorOS 13+ (enable Roboto in settings for best results)
- OxygenOS 12+ / RealmeUI / Flyme / MIUI / HyperOS / Samsung
```

# Changelog

Latest version and full history: [changelog.md](changelog.md).

```
2026.08.26(20260826)
 - 1.Date-based version for this fork
 - 2.Main glyphs switched to Maruko Gothic SC (full SC coverage, Light by default)
 - 3.Fixed Firefox italic/small-caps; security hardening; packaging & release pipeline
```

# Build & Package (this fork)

- ``tools/package/build_module.sh``: assemble an installable module zip. Uses the upstream release (``--base-zip``) as the font base, overlays this repo's customizations, generates ``module.prop``, outputs ``dist/MFGA-<version>.<versionCode>.zip`` and ``MFGA-SELFUSE-...zip``
- Example: ``tools/package/build_module.sh --base-zip MFGA-17.0.0.06-27-alpha.1717180001.zip --bin-dir tools/prebuilt/bin``
- ``tools/package/validate_module.py``: pre-package validation (module.prop fields, required files, fonts.xml ↔ system/fonts cross-check, bin completeness)
- ``.github/workflows/release.yml``: manual trigger or ``v*`` tag push — compiles C tools, downloads base, packages, validates, uploads artifact, creates GitHub Release
- ``.github/workflows/update-font.yml``: weekly rebuild of NotoSansPro with an auto PR
- Without ``--base-zip``, only the repo's own fonts are packaged (warns about missing fonts; the module still installs but most fonts fall back to system defaults)
