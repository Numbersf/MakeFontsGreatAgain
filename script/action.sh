#!/system/bin/sh
MODPATH=${0%/*}
. "$MODPATH/lang/lang.sh"

MODE="${MODE:-$1}"
echo "$(msg POWERED)"
echo "$(msg GMS_START)"
echo "$(msg GMS_TEST)"

if [ "$MODE" != "web" ]; then
echo "$(msg KEY_WARNING)"
fi

CONFIRM_ACTION() {
  echo "$(msg ACTION_CONFIRM)"

  SECONDS_WAIT=10
  while true; do
    event=$(timeout $SECONDS_WAIT getevent -ql | grep -m 1 -E "KEY_VOLUMEUP|KEY_VOLUMEDOWN")

    if [ -z "$event" ]; then
      echo "$(msg NO_KEY_DEFAULT)"
      exit 0
    elif echo "$event" | grep -q "KEY_VOLUMEUP"; then
      echo "$(msg VOL_UP_EXIT)"
      exit 0
    elif echo "$event" | grep -q "KEY_VOLUMEDOWN"; then
      echo "$(msg VOL_DOWN)"
      break
    fi
  done
}

STOP_APPS() {
  echo "$(msg STOP_APPS)"
  # 强停的应用列表可配置：config/gms_stop_apps（每行一个包名，# 注释）
  STOP_LIST=$(cat "$MODPATH/config/gms_stop_apps" 2>/dev/null \
    | grep -v '^[[:space:]]*#' | grep -v '^[[:space:]]*$')
  [ -z "$STOP_LIST" ] && STOP_LIST="com.android.chrome
com.google.android.gm"
  for APP in $STOP_LIST; do
    am force-stop "$APP" 2>/dev/null
  done
  echo "$(msg STOP_APPS_DONE)"
}

STATE_GMSF() {
  PM="$(command -v pm)"
  GMSF="com.google.android.gms/com.google.android.gms.fonts"

  echo "$(msg PM_CHECK)"
  [ -z "$PM" ] && echo "$(msg PM_MISSING)" && exit 1

  echo "$(msg DISABLE_GMS)"
  UPS=$(ls -d /data/user/* 2>/dev/null)
  [ -z "$UPS" ] && echo "$(msg NO_USER_PROFILE)"

  for UP in $UPS; do
    pm disable --user "${UP##*/}" "$GMSF.update.UpdateSchedulerService"
    pm disable --user "${UP##*/}" "$GMSF.provider.FontsProvider"
  done

  echo "$(msg DISABLE_DONE)"
}

DEL_GMSF() {
  echo "$(msg DEL_GMS_FONT)"

  if [ -d /data/fonts ]; then
    echo "$(msg DEL_DATA_FONTS)"
    rm -rf /data/fonts/*
    echo "$(msg DEL_DATA_DONE)"
  fi

  # 只扫真实用户目录，避免全盘 find /data（慢且可能命中无关路径）
  # shellcheck disable=SC2044
  for d in /data/user /data/user_de /data/data; do
    [ -d "$d" ] || continue
    find "$d" -type d -path "*com.google.android.gms/files/fonts*" -exec rm -rf {} \; 2>/dev/null
  done
  echo "$(msg DEL_ALL_DONE)"
}

if [ "$MODE" != "web" ]; then
  CONFIRM_ACTION
fi
STOP_APPS
STATE_GMSF
DEL_GMSF


if [ "$MODE" != "web" ]; then
  echo "$(msg ALL_DONE)"
  echo "$(msg WAIT_TOUCH)"
  getevent -l | grep -m 1 "BTN_TOUCH"
  echo "$(msg TOUCH_EXIT)"
fi