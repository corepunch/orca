# Native Xcode projects

Generate a project beside an application's sources with the offline launcher:

```sh
sh tools/xcode/generate.sh chat
open apps/chat/chat.xcodeproj

sh tools/xcode/generate.sh cluster-demo
sh tools/xcode/generate.sh --all
```

Each project lives at `apps/<app>/<app>.xcodeproj` and provides a shared
`<app>-ios` scheme. The native application target supports iPhone, iPad,
Simulator, Build, Run and Archive. Choose a simulator to run immediately;
select a development team in Xcode's Signing & Capabilities for a physical
device. The default deployment target is iOS 16.

Generation only reads and writes local files. It does not invoke `xcrun`,
`xcodebuild`, `plutil`, installers or download tools. The launcher selects an
existing Python 3.9+ runtime directly, including the one bundled with an installed
Xcode. If none exists, it stops without installing anything. Override the runtime
with `ORCA_PYTHON=/absolute/path/to/python3` if needed.

Avoid the bare `python3` command when it resolves to Apple's `/usr/bin/python3`:
that launcher can offer to download developer tools before our script starts.
The committed `.xcodeproj` files can also be opened directly without regeneration.

Generation accepts `--team`, `--bundle-id`, `--display-name` and `--minimum`:

```sh
sh tools/xcode/generate.sh chat --team YOUR_TEAM \
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

## App icons

Place an iOS icon in `Assets.xcassets/AppIcon.appiconset` inside the app folder.
Chat includes the selected purple orca icon as a single 1024 × 1024 image.
Regenerate the app's project after adding its first catalog. Xcode compiles the
catalog and supplies the iPhone and iPad icon metadata automatically. Make
packaging compiles the same catalog with `actool` and merges its icon metadata
into the bundle's Info plist. Catalog sources are omitted from runtime resources.
Generation itself does not run the asset compiler.

## Privacy manifest and export compliance

Place `PrivacyInfo.xcprivacy` in the app folder; generation adds it to the
target's Resources phase and Xcode copies it to the bundle root. Make packaging
copies the same file. Every ORCA app needs one for App Store and TestFlight
uploads: the engine reads file metadata with `stat()`, so each
manifest declares `NSPrivacyAccessedAPICategoryFileTimestamp` (`C617.1`, plus
`3B52.1` when the app opens user-picked files). Also list the data the app
collects. Chat's manifest is the reference.

Generated Info plists set `ITSAppUsesNonExemptEncryption` to `false`: ORCA
apps use only the system's HTTPS, so TestFlight builds need no manual export
compliance answer. Change it if an app adds its own encryption.

## Build phases and dependencies

The project uses Xcode's native compiler and linker. Its Prepare phase writes
unity sources, generated C API files and static Lua plugin registrations into
`DERIVED_FILE_DIR`. The resource phase copies the shared runtime and selected
application into `share/` and `project/` inside Xcode's application bundle. It
does not replace Xcode's Info plist or the entire bundle. Nested Xcode projects,
workspaces, `ci_scripts`, hidden development files and build folders are omitted.

The navigator's Engine group contains only generated `u_*.c` unity files, one
per engine module plus the Unix platform unit. The Plugins group contains the
generated plugin unity files, including UIKit, SceneKit and SpriteKit. Each file
includes its implementation sources. Generated Objective-C units appear under Native Sources;
the raw engine, plugin and platform directories are not added to the navigator.

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
dependency download hosts (lua.org, download.savannah.gnu.org, codeload.github.com
and curl.se): every Cloud archive starts from a clean DerivedData and downloads
them again. The project does not contain private signing keys or provisioning
profiles.

Xcode Cloud workflows belong to one app, so each app that ships needs its own:
a workflow for Chat does not build Octopus. For each app:

1. In App Store Connect, create the app record for the project's bundle
   identifier under the project's team (`BM2R8F5YHC`): `org.orca.chat` for
   Chat, `org.orca.octopus` for Octopus.
2. Open `apps/<app>/<app>.xcodeproj` in Xcode and choose
   **Integrate › Create Workflow…** (or **Product › Xcode Cloud**). Select the
   `<app>-ios` scheme, add an **Archive** action for iOS with
   **TestFlight (Internal Testing Only)** or App Store distribution, and add a
   **TestFlight Internal Testing** post-action with a tester group.
3. Choose a start condition, such as branch changes on `main`. To skip
   builds for unrelated commits, add a files-and-folders condition for
   `apps/<app>`, `source`, `plugins`, `share`, `libs` and `tools/xcode`.
4. Grant Cloud access to the repository and its submodules when asked.

Xcode Cloud assigns each build number. Chat's and Octopus's public Appwrite
settings are defaults in `apps/<app>/config.lua`, so Cloud needs no
environment variables or secrets for them.
