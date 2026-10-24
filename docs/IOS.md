# iPhone and iPad

ORCA builds a standalone universal iOS `.app` directly with Make, `xcrun clang`
and Apple SDKs. There is no `.xcodeproj` or Xcode build step. The build pattern
follows the sibling `orion-ui/packaging/ipad/build.mk` and `lua-objc/scripts/ipad/`
projects.

## Requirements

- macOS with full Xcode installed and selected with `xcode-select`.
- Python 3, Make, curl and tar.
- An installed iOS simulator runtime for simulator runs.
- For a physical device: Developer Mode, a trusted/unlocked device, and an
  installed Apple development signing identity with a matching provisioning
  profile. The profile must cover the bundle identifier and device UDID.

Initialize submodules as described in [BUILD.md](BUILD.md). First builds
download pinned, SHA256-verified Lua 5.4.8, FreeType 2.13.3, LZ4 1.10.0 and
curl 8.10.1 sources, then build static libraries for the selected SDK and CPU.
Downloads are cached under `build/ios/`. curl uses Apple's Secure Transport
for HTTPS. Host Homebrew libraries are never linked into iOS executables.
Host code generation uses the macOS SDK's libxml2 if pkg-config has no entry.

## Build and launch

Run these commands from the repository root:

```sh
make ios-sim                         # Build the simulator .app
make ios-run                         # Install and launch an iPhone simulator
make ios-run DEVICE_TYPE=iPad        # Install and launch an iPad simulator
make ios                             # Build the arm64 device .app
make ios-devices                     # List devices and simulators
make ios-install DEVICE=<identifier> # Sign, install and launch a physical iPhone
make ios-install DEVICE_TYPE=iPad DEVICE=<identifier>
```

Simulator selection prefers a booted device; pass `DEVICE=<UDID-or-name>` to
choose one explicitly. Physical deployment requires an unambiguous available
device of the requested type. Use `PROFILE=/path/to/profile.mobileprovision`
and `TEAM=<team-id>` to choose signing credentials when needed.

The default project is `apps/chat`, with bundle identifier `org.orca.chat`.
Override `PROJECT=apps/your-app`, `BUNDLE_ID=your.identifier` and
`DISPLAY_NAME="Your App"` for another package.lua project. `IOS_MIN` defaults
to 16.0. Device builds use arm64; simulator builds use the host architecture
and also support `ARCH=x86_64` where that simulator runtime is available.
`BUILD_DIR` defaults to `build/ios`.

Outputs are isolated by SDK and architecture:

```text
build/ios/iphoneos-arm64/Orca.app
build/ios/iphonesimulator-arm64/Orca.app
```

`make ios` produces an unsigned device bundle. `make ios-install` selects a
matching development profile/certificate, embeds the profile, signs the bundle
and uses `devicectl` to install and launch it.

## Runtime

The bundle contains `share/` and a copy of the chosen project in `project/`.
Lua modules and engine plugins are linked statically. UIKit owns the main run
loop; a display link resumes the Lua application coroutine each frame, and
`system.getMessage` yields when the platform queue is empty.

The platform backend uses UIWindowScene and OpenGL ES 3, Retina backing buffers,
safe-area layout, tap/drag events, vertical swipe scrolling and UIKeyInput.
The chat screen tracks the available size, including keyboard appearance and
rotation. Input supports UTF-8 insertion and deletion. Marked-text IME composition,
native file dialogs, offscreen platform surfaces and multiple app windows are
not implemented by this backend.

Settings use the app's writable Application Support directory. Bundled project
resources are read-only; applications should use ORCA settings APIs for saved
data. Terminal environment variables are not inherited by apps installed on
physical devices. For the chat's Supabase mode, configure its public URL and
publishable key in `apps/chat/config.lua` before packaging; the default is the
local demo.

The backend lives in the `libs/platform` Git submodule. When committing this
feature, include its `ios/` sources in that repository and update ORCA's submodule
revision along with the main repository changes.

## Verify

```sh
make test-ios-tools
make test-headless
make test-chat
```

`test-ios-tools` runs offline packaging, signing-selection and device-selection
tests. Runtime smoke checks should include both an iPhone and an iPad simulator:
open a conversation, focus the composer, type/send text and scroll the history.
