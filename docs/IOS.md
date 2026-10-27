# iPhone and iPad

ORCA builds universal iOS apps through either native Xcode projects or Make and
`xcrun clang`. Each application has its own committed `.xcodeproj` beside its
sources. The native project generator follows `orion-ui/tools/xcode/`; the Make
build follows `orion-ui/packaging/ipad/build.mk` and `lua-objc/scripts/ipad/`.

## Native Xcode projects and Xcode Cloud

Open `apps/chat/chat.xcodeproj` and choose the shared `chat-ios` scheme,
`apps/octopus/octopus.xcodeproj` with `octopus-ios`, or
`apps/cluster-demo/cluster-demo.xcodeproj` with `cluster-demo-ios`. Each scheme
supports iPhone, iPad and Simulator, including Run and Archive actions.

```sh
sh tools/xcode/generate.sh chat        # Regenerate one app's project
sh tools/xcode/generate.sh --all       # Regenerate apps/*/package.lua projects
xcodebuild -project apps/chat/chat.xcodeproj -scheme chat-ios \
  -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath build/xcode/chat CODE_SIGNING_ALLOWED=NO build
```

Commit the generated project, its `Generated/ios-Info.plist` and shared scheme.
Generation is offline and uses an existing Python runtime directly; it never
invokes Apple's Python launcher or installs developer tools. You can open the
committed projects without regenerating them.
Set the development team in Xcode, then select this project and scheme when
creating an Xcode Cloud workflow. Each app needs its own App Store Connect
record and workflow; see [Xcode Cloud](../tools/xcode/README.md#xcode-cloud). The native build phases prepare the host code
generator, static dependencies, API files and resources inside DerivedData;
they need no existing local Make build. Regeneration preserves signing settings
and customized shared schemes. No custom Cloud hooks are required for these
build phases; optional hooks can later live in a `ci_scripts` directory beside
the selected project, as described in
[Apple's custom script guide](https://developer.apple.com/documentation/xcode/writing-custom-build-scripts).

Ensure Xcode Cloud can fetch the submodule revisions referenced by ORCA and
download the pinned dependency archives. Public submodules are supported
directly; private repositories need Cloud access, as described in
[Apple's dependency guide](https://developer.apple.com/documentation/xcode/making-dependencies-available-to-xcode-cloud).
Push a changed `libs/platform` commit before publishing a parent revision that
references it. No signing certificates or provisioning profiles belong in Git.

See [the Xcode generator guide](../tools/xcode/README.md) for build commands,
configuration options and regeneration behavior. The Make targets below remain
available independently.

## Requirements

- macOS with full Xcode installed and selected with `xcode-select`.
- Python 3, Make, curl and tar.
- An installed iOS simulator runtime for simulator runs.
- For a physical device: Developer Mode, a trusted/unlocked device, and an
  installed Apple development signing identity with a matching provisioning
  profile. The profile must cover the bundle identifier and device UDID.

Initialize submodules as described in [BUILD.md](BUILD.md). First builds
download pinned, SHA256-verified Lua 5.4.8, LZ4 1.10.0 and
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
`system.getMessage` yields when the display callback's message batch is drained.
Each display callback snapshots the queued messages before resuming Lua. Work
posted while that batch runs waits for the next display callback, so animation
callbacks cannot keep UIKit from delivering a new touch during deceleration.
The queue boundary also applies to peeking and explicit queue flushes.

Set `ORCA_PROFILE_FRAMES=1` in the Xcode scheme's Run environment to log
display callback rates, frame durations and missed display intervals once per
second. This works with optimized builds and is disabled by default.

The platform backend uses UIWindowScene and OpenGL ES 3, Retina backing buffers,
tap/drag events, vertical swipe scrolling and UIKeyInput. The drawable fills the
window, including the status-bar and home-indicator areas. `Screen.ClearColor`
paints the entire canvas and determines status-bar text contrast. By default,
`Screen.RespectSafeArea` keeps child controls inside the system insets while
`Screen.Width` and `Height` retain the full drawable dimensions. The four
`SafeAreaLeft`, `SafeAreaTop`, `SafeAreaRight`, and `SafeAreaBottom` properties
update for native resizable screens. Set `RespectSafeArea=false` when a screen
needs backgrounds or scrolling content under system chrome, then use those
properties to inset its interactive controls. Avoid hardcoded device margins.

The drawable retains its full size when the keyboard appears.
`Screen.KeyboardInsetBottom` reports the overlap of a docked, full-width
keyboard in logical pixels; hidden and floating keyboards report zero.
Safe-area insets retain their physical meaning. `RespectSafeArea` keeps content
above the larger of the bottom safe inset and keyboard overlap. Edge-to-edge
layouts dock their bars with `SafeAreaEdge`: scrolling content runs under them
and is inset by them and by the system insets, while the bar backgrounds reach
the bottom of the window, beneath the keyboard. The shared `Footer` and the
Chat and Octopus application bars work this way; see
[docked bars](UI_SYSTEM.md#safe-area-and-docked-bars).
Viewport metrics and bindings are resolved before layout in the same frame.
`Input.Type` requests
the native keyboard: `Phone` supports international phone entry, `Number` uses
a digit keypad, and `Email`, `Url`, and `Decimal` select their corresponding
layouts. `Password` enables secure native entry. Types are keyboard hints;
applications still validate values, and numeric strings retain leading zeroes.
Input supports UTF-8 insertion and deletion. Marked-text IME composition,
native file dialogs, offscreen platform surfaces and multiple app windows are
not implemented by this backend.

Settings use the app's writable Application Support directory. Bundled project
resources are read-only; applications should use ORCA settings APIs for saved
data. Simulator builds receive a local application identity
(`ORCASIM000.<bundle id>`), so saved sessions use the same secure-storage API as
devices, in a Keychain group private to each app. As with Xcode, the identity is
linked into the binary as simulated entitlements and the ad hoc signature
carries none, because simulators refuse to launch entitled ad hoc signatures.
The simulator signing helper refuses physical-device bundles, which retain
profile-authorized signing and Keychain groups.
The iOS Lua build creates `os.tmpname()` files in the app's `TMPDIR`,
with private permissions, so downloaded photos and avatars can be cached inside
the sandbox. The system-wide `/tmp` directory is not writable by device apps.
Chat attaches still images through the system photo picker. Selection yields
the app coroutine while UIKit remains responsive and grants access only to the
selected image; no photo-library permission is requested. The selected photo
uses the shared native orientation, metadata removal, resizing and JPEG
compression pipeline before preview or upload. Cancelling retains the current
message draft.
Terminal environment variables are not inherited by apps installed on
physical devices. The chat's public Appwrite endpoint, project ID and Function
ID are defaults in `apps/chat/config.lua`, so installed builds connect without
environment variables.

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
