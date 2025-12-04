#!/bin/bash

# 获取系统语言环境
LANGUAGE=$(getprop persist.sys.locale | cut -d'-' -f1)

# 定义多语言输出函数
print_message() {
    case $LANGUAGE in
        zh)
            echo "$1" # CN
            ;;
        *)
            echo "$2" # EN
            ;;
    esac
}

# 确认操作函数
confirm_action() {
    print_message "即将执行GMS屏蔽脚本，包括禁用服务和删除字体文件，关闭部分应用(Gmail、Chrome)！请按音量上键继续, 按音量下键退出。" "The GMS blocking script will be executed, including disabling services, deleting fonts, and closing Gmail and Chrome apps. Press Volume Up to continue, or Volume Down to exit."
    
    SECONDS_WAIT=10
    while true; do
        event=$(timeout $SECONDS_WAIT getevent -ql | grep -m 1 -E "KEY_VOLUMEUP|KEY_VOLUMEDOWN")
        
        if [ -z "$event" ]; then
            print_message "未检测到音量键输入，默认继续执行..." "No volume key detected; proceeding by default..."
            STOP_APPS
            break
        elif echo "$event" | grep -q "KEY_VOLUMEUP"; then
            print_message "检测到音量上键, 继续操作..." "Volume Up detected, proceeding..."
            STOP_APPS
            break
        elif echo "$event" | grep -q "KEY_VOLUMEDOWN"; then
            print_message "检测到音量下键, 退出脚本..." "Volume Down detected, exiting..."
            exit 0
        fi
    done
}
# 输出信息
print_message "=====Powered by yiyunlengyu=====" "=====Powered by yiyunlengyu====="
print_message "正在为你启用 GMS 字体全局屏蔽!" "Enabling global GMS font blocking for you!"
print_message "请测试是否成功，如果失败请及时反馈！" "Please test whether it is successful, and provide feedback if it fails!"

# 停止指定应用
STOP_APPS() {
    print_message "正在强行停止部分应用 (com.android.chrome, com.google.android.gm)..." "Force stopping specific applications (com.android.chrome, com.google.android.gm)..."
    am force-stop com.android.chrome
    am force-stop com.google.android.gm
    print_message "[✓]应用已强行停止。" "Applications stopped."
}

# 禁用 GMS 字体服务
STATE_GMSF() {
    PM="$(command -v pm)"
    GMSF="com.google.android.gms/com.google.android.gms.fonts"

    print_message "检测 pm 命令是否可用..." "Checking if 'pm' command is available..."
    if [ -z "$PM" ]; then
        print_message "[✕]错误: 找不到 pm 命令！请确认运行环境。" "Error: 'pm' command not found! Please confirm the runtime environment."
        exit 1
    fi

    print_message "正在禁用 GMS 字体服务..." "Disabling GMS font services..."
    UPS=$(ls -d /data/user/* 2>/dev/null)
    if [ -z "$UPS" ]; then
        print_message "[!]警告: 未找到用户配置文件，可能会影响功能。" "Warning: No user configuration file found, functionality may be affected."
    fi

    for UP in $UPS; do
        "$PM" disable --user "${UP##*/}" "$GMSF.update.UpdateSchedulerService"
        "$PM" disable --user "${UP##*/}" "$GMSF.provider.FontsProvider"
    done
    print_message "[✓]GMS 字体服务已禁用。" "GMS font services disabled."
}

# 删除 GMS 生成的字体
DEL_GMSF() {
    local GMSFD="com.google.android.gms/files/fonts"

    print_message "正在清理 GMS 生成的字体文件..." "Cleaning up GMS-generated font files..."
    if [ -d /data/fonts ]; then
        print_message "删除 /data/fonts/ 目录中的文件..." "Removing files from /data/fonts/ directory..."
        rm -r /data/fonts/*
        print_message "[✓]/data/fonts/ 清理完成,开始处理其它服务 请等待10s~1min,这取决于你手机的性能" "/data/fonts/ Cleaning completed, starting to process other services, please wait 10s~1min, this depends on the performance of your phone."
    fi

    find /data -type d -path "*$GMSFD*" -exec rm -r {} \;
    print_message "[✓]所有 GMS 生成的字体文件已被清理。" "All GMS-generated font files have been removed."
}

# 获取用户事件函数
get_event() {
    getevent -l | grep -m 1 -E "KEY_VOLUMEDOWN|KEY_VOLUMEUP|BTN_TOUCH" &>/dev/null
}

# 执行用户交互
print_message "===================" "==================="
confirm_action
print_message "开始禁用 GMS 字体服务..." "Step 1: Disabling GMS font services..."
STATE_GMSF
print_message "清理 GMS 生成的字体文件..." "Step 2: Cleaning up GMS-generated font files..."
DEL_GMSF
print_message "=============================" "============================="
print_message "一切准备就绪, 点击屏幕退出" "All operations have been completed! Please test the effect. To exit, tap the screen."
print_message "等待屏幕触摸退出事件..." "Waiting for screen touch to exit script..."
getevent -l | grep -m 1 "BTN_TOUCH"
print_message "已检测到屏幕点击, 退出脚本..." "Screen tap detected, exiting script..."
print_message "Powered by yiyunlengyu" "Powered by yiyunlengyu"