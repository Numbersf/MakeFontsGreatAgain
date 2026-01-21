#!/system/bin/sh
MODPATH=${0%/*}
. "$MODPATH/lang/lang.sh"

echo "$(msg POWERED)"
echo "$(msg GMS_START)"
echo "$(msg GMS_TEST)"

CONFIRM_ACTION() {
  echo "$(msg ACTION_CONFIRM)"

  SECONDS_WAIT=10
  while true; do
    event=$(timeout $SECONDS_WAIT getevent -ql | grep -m 1 -E "KEY_VOLUMEUP|KEY_VOLUMEDOWN")

    if [ -z "$event" ]; then
      echo "$(msg NO_KEY_DEFAULT)"
      STOP_APPS
      break
    elif echo "$event" | grep -q "KEY_VOLUMEDOWN"; then
      echo "$(msg VOL_DOWN)"
      STOP_APPS
      break
    elif echo "$event" | grep -q "KEY_VOLUMEUP"; then
      echo "$(msg VOL_UP_EXIT)"
      exit 0
    fi
  done
}

STOP_APPS() {
  echo "$(msg STOP_APPS)"
  am force-stop com.android.chrome
  am force-stop com.google.android.gm
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

  find /data -type d -path "*com.google.android.gms/files/fonts*" -exec rm -rf {} \;
  echo "$(msg DEL_ALL_DONE)"
}

CONFIRM_ACTION
STATE_GMSF
DEL_GMSF

echo "$(msg ALL_DONE)"
echo "$(msg WAIT_TOUCH)"
getevent -l | grep -m 1 "BTN_TOUCH"
echo "$(msg TOUCH_EXIT)"