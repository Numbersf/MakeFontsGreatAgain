SKIPUNZIP=0

# 检测语言环境
LANGUAGE=$(getprop persist.sys.locale | cut -d'-' -f1)

# 定义多语言提示信息
function ui_print_msg() {
    case "$LANGUAGE" in
        zh)  # 中文
            case "$1" in
                "kernel_low") echo "[!] KernelSU系列管理器 版本低于 11986，正在取消安装..." ;;
                "magisk_low") echo "[!] Magisk系列管理器 版本低于 28.0，正在取消安装..." ;;
                "android_11_or_lower") echo "[!] 检测到 Android 11 或更低版本，正在切换 Emoji 字体兼容方案..." ;;
                "android_12_or_higher") echo "[✓] 检测到 Android 12 或更高版本，移除备用 Emoji 字体..." ;;
                "pif_warning_ksu") echo "[!] 检测到 PlayIntegrityFix 模块存在, 如果挂载方式为overlayfs，请单独打开 Play 服务的卸载模块!" ;;
                "pif_warning_ksu2") echo "[-] 检测到你正在使用 KernelSU系列管理器, 请关闭默认卸载模块功能!" ;;
                "pif_warning_magisk") echo "[!] 检测到 PlayIntegrityFix 模块存在, 请单独打开 Play 服务的配置排除列表!" ;;
                "module_obsolete") echo "[-] 模块 ID 已更改，旧模块已弃用" ;;
                "xiaomi_keep") echo "[!] 设备为米系,所以保留zygisk目录,请删除空字体模块" ;;
                "non_xiaomi_remove") echo "[✓] 非米系设备,删除zygisk目录" ;;
            esac
            ;;
        *)  # 默认英文
            case "$1" in
                "kernel_low") echo "[!] KernelSU version is below 11986, cancelling installation..." ;;
                "magisk_low") echo "[!] Magisk version is below 28.0, cancelling installation..." ;;
                "android_11_or_lower") echo "[!] Android 11 or lower detected, applying Emoji font compatibility..." ;;
                "android_12_or_higher") echo "[✓] Android 12 or higher detected, removing fallback Emoji font..." ;;
                "pif_warning_ksu") echo "[!] PlayIntegrityFix(pif) exists. If the mount method is overlayfs, Please open 'Superuser→Custom→Umount Module' in PlayServices!" ;;
                "pif_warning_ksu2") echo "[-] Detect you are using KernelSU, Turn off the Umount Module by default function!" ;;
                "pif_warning_magisk") echo "[!] PlayIntegrityFix(pif) exists, Please configure 'DenyList' in PlayServices!" ;;
                "module_obsolete") echo "[-] Changing the module ID, the old module is obsolete" ;;
                "xiaomi_keep") echo "[!] Your Device is Xiaomi Series, so keep the zygisk directory and Please DELETE the Emptyfont module!" ;;
                "non_xiaomi_remove") echo "[✓] Non-Xiaomi device detected, removing zygisk directory" ;;
            esac
            ;;
    esac
}

# 版本检查函数
version_check() {
    if [[ $KSU_VER_CODE != "" ]] && [[ $KSU_VER_CODE -lt 11986 || $KSU_KERNEL_VER_CODE -lt 11986 ]]; then
        ui_print_msg "kernel_low"
        abort
    elif [[ $KSU_VER_CODE == "" && $MAGISK_VER_CODE != "" && $MAGISK_VER_CODE -lt 28000 ]]; then
        ui_print_msg "magisk_low"
        abort
    fi
}

# 检查 Android 版本并处理字体
if [ "$API" -le 30 ]; then
    ui_print_msg "android_11_or_lower"
    if [ -f "$MODPATH/system/fonts/NotoColorEmoji.ttf" ]; then
        rm -f "$MODPATH/system/fonts/NotoColorEmoji.ttf"
    fi
    if [ -f "$MODPATH/system/fonts/NotoColorEmoji-fallback.ttf" ]; then
        mv "$MODPATH/system/fonts/NotoColorEmoji-fallback.ttf" \
           "$MODPATH/system/fonts/NotoColorEmoji.ttf"
    fi
else
    ui_print_msg "android_12_or_higher"
    if [ -f "$MODPATH/system/fonts/NotoColorEmoji-fallback.ttf" ]; then
        rm -f "$MODPATH/system/fonts/NotoColorEmoji-fallback.ttf"
    fi
fi

# PlayIntegrityFix 检查
if [ -d "/data/adb/modules/playintegrityfix" ]; then
    if [ -d "/data/adb/ksu/" ]; then
        ui_print_msg "pif_warning_ksu"
        ui_print_msg "pif_warning_ksu2"
    elif [ -f "/sbin/.magisk" ] || [ -d "/data/adb/magisk" ]; then
        ui_print_msg "pif_warning_magisk"
    fi
fi

# 模块 ID 变更
MFGA="/data/adb/modules/Colorfontsproject/"
if [ -d "$MFGA" ]; then
    ui_print_msg "module_obsolete"
    touch "$MFGA"/remove
fi

# 调用版本检查
version_check

# 获取设备信息
BRAND=$(getprop ro.product.brand | tr '[:upper:]' '[:lower:]')
MANUFACTURER=$(getprop ro.product.manufacturer | tr '[:upper:]' '[:lower:]')
MIUI_VERSION=$(getprop ro.miui.ui.version.name)
MIUI_CODE_TIME=$(getprop ro.miui.version.code_time)
ZYGISK_DIR="$MODPATH/zygisk"

# 判断设备是否属于小米（包括 Redmi、POCO）
if [[ "$BRAND" == "xiaomi" || "$BRAND" == "redmi" || "$BRAND" == "poco" || \
      "$MANUFACTURER" == "xiaomi" || -n "$MIUI_VERSION" || -n "$MIUI_CODE_TIME" ]]; then
    ui_print_msg "xiaomi_keep"
else
    ui_print_msg "non_xiaomi_remove"
    rm -rf "$ZYGISK_DIR"
fi

ui_print "- Welcome to MFGA!"