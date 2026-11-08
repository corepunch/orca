#!/bin/sh
# Installs an ORCA APK on a running emulator or device and checks that it
# starts, stays up and receives a sign-in link through its URL scheme.
#   tools/android/smoke-test.sh APK APPLICATION_ID PROJECT [OUTPUT_DIR]
# The link carries a provider error (as when the user declines on Google's
# page), so the app handles it offline without contacting the backend.
set -eu
apk=$1 id=$2 project=$3 out=${4:-build/android/smoke}
scheme=$(sed -n 's/^URLSchemes *= *{ *"\([^"]*\)".*/\1/p' "$project/package.lua")
[ -n "$scheme" ] || { echo "No URLSchemes entry in $project/package.lua" >&2; exit 1; }
mkdir -p "$out"
trap 'adb logcat -d > "$out/logcat.txt" 2>/dev/null || true' EXIT

fail() {
    echo "FAIL: $*" >&2
    echo "--- app log (orca, crashes) ---" >&2
    log | tail -n 200 >&2 || true
    echo "--- process ---" >&2
    adb shell "ps -A | grep $id; dumpsys activity activities | grep -E 'mResumedActivity|topResumedActivity'" >&2 || true
    exit 1
}
log() { adb logcat -d -s orca:V AndroidRuntime:E DEBUG:V libc:F; }
# wait_for PATTERN SECONDS: polls the app's log until PATTERN appears.
wait_for() {
    i=0
    while [ "$i" -lt "$2" ]; do
        log | grep -q -- "$1" && return 0
        alive || fail "$id exited while waiting for '$1'"
        sleep 1; i=$((i + 1))
    done
    fail "'$1' did not appear within $2 s"
}
alive() { [ -n "$(adb shell pidof "$id" | tr -d '\r')" ]; }
crashed() {
    log | grep -E 'FATAL EXCEPTION|Fatal signal|Chat error|stack traceback|Could not unpack|Cannot open the project' && return 0
    return 1
}
screenshot() { adb exec-out screencap -p > "$out/$1.png"; }

adb wait-for-device
until [ "$(adb shell getprop sys.boot_completed | tr -d '\r')" = 1 ]; do sleep 2; done
adb install -r "$apk"
adb shell input keyevent KEYCODE_WAKEUP
adb logcat -c

echo "Launching $id"
adb shell am start -W -n "$id/org.orca.OrcaActivity"
wait_for "Starting " 120
# First launch unpacks the bundle and builds the sign-in screen.
sleep 15
alive || fail "$id is not running after launch"
crashed && fail "$id reported an error after launch"
screenshot launch

echo "Opening $scheme:// with a Google sign-in callback"
adb shell am start -W -a android.intent.action.VIEW -c android.intent.category.BROWSABLE \
    -d "'$scheme://oauth?provider=google&error=access_denied'" "$id"
# The intent reaches Lua through JNI and orca.signin; Chat rejects the
# declined sign-in without a session.
wait_for "Chat sign-in failed: provider_failed" 60
sleep 3
alive || fail "$id exited after the sign-in link"
crashed && fail "$id reported an error after the sign-in link"
screenshot callback
echo "PASS: $id started and handled a sign-in link"
