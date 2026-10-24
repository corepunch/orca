# Native Xcode projects

Generate a project beside an application's sources with Python 3.9 or newer:

```sh
python3 tools/xcode/generate.py chat
open apps/chat/chat.xcodeproj

python3 tools/xcode/generate.py cluster-demo
python3 tools/xcode/generate.py --all
```

Each project lives at `apps/<app>/<app>.xcodeproj` and provides a shared
`<app>-ios` scheme. The native application target supports iPhone, iPad,
Simulator, Build, Run and Archive. Choose a simulator to run immediately;
select a development team in Xcode's Signing & Capabilities for a physical
device. The default deployment target is iOS 16.

Generation accepts `--team`, `--bundle-id`, `--display-name` and `--minimum`:

```sh
python3 tools/xcode/generate.py chat --team YOUR_TEAM \
  --bundle-id com.example.chat --display-name "My Chat" --minimum 16.0
```

Regeneration preserves existing signing settings and shared scheme edits.
Project IDs are deterministic, and unchanged generated files keep their
timestamps. Regenerate after adding or removing app folders or changing the
engine module configuration so the project navigator and native compile units
stay current. Regular C, Objective-C, Lua, XML, CSS and asset edits rebuild
without regenerating the project.

Commit each `.xcodeproj`, its generated Info plist and shared schemes. User
settings, DerivedData, certificates, provisioning profiles and build products
remain local.

## Build phases and dependencies

The project uses Xcode's native compiler and linker. Its Prepare phase writes
unity sources, generated C API files and static Lua plugin registrations into
`DERIVED_FILE_DIR`. The resource phase copies the shared runtime and selected
application into `share/` and `project/` inside Xcode's application bundle. It
does not replace Xcode's Info plist or the entire bundle. Nested Xcode projects,
workspaces, `ci_scripts`, hidden development files and build folders are omitted.

Host code generation uses the macOS SDK, independently of the iOS target SDK.
Lua, FreeType, LZ4 and curl are built as static libraries for every architecture
in `ARCHS`; curl uses Apple's Secure Transport for HTTPS. Source archives are
pinned and checked with SHA256 by `tools/ios/dependencies.py`. The first build
downloads their sources, so it needs network access. Later builds reuse their
DerivedData cache. Existing verified archives in `build/ios/downloads` can seed
that cache but are optional.

All generated code, native dependency libraries and host tools stay in
DerivedData. Upstream FreeType and curl builds use temporary scratch directories
under `/tmp`, then copy their static libraries and headers into DerivedData;
the scratch directories are removed afterward. Xcode builds do not write to the repository's
`generated/` or reuse Makefile-built libraries. The existing command-line
Makefile workflow remains available separately. No Homebrew packages,
XcodeGen or CocoaPods are required. DerivedData paths containing spaces are
supported.

## Command-line builds

```sh
xcodebuild -project apps/chat/chat.xcodeproj -scheme chat-ios \
  -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath build/xcode/DerivedData build

# Compile for a physical device before configuring signing.
xcodebuild -project apps/chat/chat.xcodeproj -scheme chat-ios \
  -destination 'generic/platform=iOS' \
  -derivedDataPath build/xcode/DerivedData-device CODE_SIGNING_ALLOWED=NO build
```

## Xcode Cloud

Select the committed app project and its `<app>-ios` shared scheme when creating
the workflow. Configure the development team and distribution signing in Xcode
and Xcode Cloud for archives. Cloud uses the same native preparation, compile,
link and resource phases as a local build; no custom `ci_scripts` hooks are
needed. Ensure Cloud can access the repository's Git submodules and the pinned
dependency download hosts. The project does not contain private signing keys
or provisioning profiles.
