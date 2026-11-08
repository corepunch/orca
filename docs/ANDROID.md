# Android

ORCA builds an Android app as one native library, `libmain.so`, linked
statically like the iOS app, plus a small Java activity
(`org.orca.OrcaActivity`, a `NativeActivity`). Gradle packages the library, the
`share/` runtime and the chosen project into an APK.

## Requirements

- Android NDK (r26 or later), with `ANDROID_NDK_HOME` set.
- Android SDK with platform 35, `ANDROID_HOME` set, Java 17 and Gradle 8.
- CMake, Git and curl, for the pinned dependencies.
- A host `make modules` (code generation runs on the build machine).

## Build

```sh
make modules
make chat-android                          # arm64-v8a debug APK
make chat-android ABIS="arm64-v8a x86_64"   # device and emulator
make cluster-demo-android APPLICATION_ID=org.example.cluster
adb install -r tools/android/project/app/build/outputs/apk/debug/app-debug.apk
```

`tools/android/dependencies.py` builds mbedTLS 3.6.2 and curl 8.10.1 from
SHA256-verified archives, and libxml2 2.13.5 comes from the `libs/libxml2` submodule.
curl trusts Android's system CA directory. Lua and LZ4 come from the
submodules. Outputs are cached under `build/android/<abi>/`.

`tools/android/package.py` stages the assets and writes `AndroidManifest.xml`:
the launcher activity and one `VIEW` intent filter per `URLSchemes` entry in
`package.lua`, so sign-in links open the app.

## Smoke test

With an emulator or device attached, `tools/android/smoke-test.sh` installs an
APK, checks that the app starts and stays up, then opens the project's
`URLSchemes` link with a declined Google sign-in
(`...://oauth?provider=google&error=access_denied`). The link must reach Lua
through the intent filter, JNI and `orca.signin`. Chat logs
`Chat sign-in failed: provider_failed` without contacting Appwrite.

```sh
tools/android/smoke-test.sh tools/android/project/app/build/outputs/apk/debug/app-debug.apk \
    org.orca.chat apps/chat build/android/smoke
```

Screenshots and `logcat.txt` are written to the output directory. CI's
`test-android` job runs it on an API 34 x86_64 emulator against the
`build-android` APK and uploads the results as `orca-chat-android-smoke`.

## Runtime

- `android_main` (`source/android/android_main.c`) unpacks the bundled files to
  private storage once per build, using the generated `orca-files.txt`, then runs
  the same blocking `Application` loop as the desktop.
- The platform backend (`libs/platform/android`) drives `ALooper`, creates the
  EGL/GLES 3 surface with the window and keeps the GL context across
  backgrounding. Sizes are density-independent points.
- The soft keyboard follows the focused `Input` and its `Type` (number, phone,
  email, URL, password). A one-pixel Java view owns the IME connection and
  sends composed text as it changes.
- Window insets set `Screen.SafeArea*` and `KeyboardInsetBottom`, as on iOS.
- `orca.credentials` stores values AES-GCM encrypted with a non-exportable
  Android Keystore key.
- `orca.signin.open` shows the page in a Custom Tab; the provider's redirect to
  the app's scheme, or an emailed link, returns through the intent filter. It
  fails, rather than waiting, when no browser can open the page.
- stdout and stderr (Lua `print`, Chat error reports) are forwarded to logcat
  under the `orca` tag: `adb logcat -s orca`.

## Not yet on Android

Photo picking and processing (`orca.media`) and push notifications report
unsupported. File monitoring is off, as on iOS.
