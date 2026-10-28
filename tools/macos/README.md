# Local macOS application bundles

Build ORCA, then package Chat as a local `.app`:

```sh
make unite
python3 tools/macos/bundle.py --bundle build/macos/ORCAChat.app \
  --identifier org.orca.chat --display-name 'ORCA Chat'
open build/macos/ORCAChat.app
```

Use `make no-lz4` when the local machine lacks LZ4 development files. Choose a
new output path for every package operation; the helper never overwrites an
existing application. It packages the built runtime, runtime plugins, Chat and
shared resources, excludes backend/session/signing files, and adjusts internal
library references. Homebrew dependencies such as Lua remain local dependencies;
this is a local developer bundle rather than a portable distribution installer.

`ORCAProject=project` in the bundle's Info plist selects
`Contents/Resources/project` when LaunchServices starts the executable without
arguments. Runtime libraries, plugins and shared resources are resolved inside
the bundle even when an explicit project or `-test` argument is supplied.

The helper applies an ad hoc signature so the local application can launch.
APNs requires an Apple signing identity and a provisioning profile that enables
Push Notifications for this exact bundle identifier. The templates in
`apps/chat/Push-macOS-development.entitlements` and
`apps/chat/Push-macOS-production.entitlements` supply the corresponding
`com.apple.developer.aps-environment` value. Merge the appropriate template into
the application's complete signing entitlements, including the team/application
identifiers authorized by its provisioning profile. Embed that profile as
`Contents/embedded.provisionprofile`, then apply the Apple signature:

```sh
cp /path/to/chat.provisionprofile build/macos/ORCAChat.app/Contents/embedded.provisionprofile
codesign --force --sign 'Apple Development: YOUR IDENTITY' \
  --entitlements /path/to/complete-chat.entitlements build/macos/ORCAChat.app
codesign --verify --strict build/macos/ORCAChat.app
```

The Appwrite APNs provider must use the matching bundle identifier and
development/production environment. Configure Chat's provider ID before
expecting notification registration. Launching the unbundled `make chat`
executable reports native notifications as unsupported.

These packaging and compilation checks do not send notifications, request
permission or verify APNs delivery. Live delivery remains an explicitly
requested check after provider credentials and Apple signing are configured.

```sh
python3 -m unittest discover -s tools/macos -p test_bundle.py
```
