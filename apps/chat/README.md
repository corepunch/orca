# ORCA Messenger

A simple person-to-person chat app in Lua, following the existing
`samples/Banking` application, route, package, and service-module patterns.
Runs on macOS with a **390 × 844** portrait viewport and on iPhone/iPad with
an adaptive viewport, touch scrolling and the native keyboard. iOS builds use
Make and the Xcode command-line tools without an Xcode project.

![ORCA Messenger on macOS](preview.png)

[Message thread preview](thread-preview.png)

[New conversation](new-preview.png) · [Profile](profile-preview.png) · [Sign in](auth-preview.png)

## Design

The midnight surfaces and violet accents follow `samples/Example`. A shared
palette in `theme.lua`, reusable Lua components, and `Styles/app.css` carry
the design across the inbox, conversations, account, and forms. Icons are
stroke-based Lucide SVGs with inherited tint.

Search matches conversation names and message previews. All, Unread, and Pinned
filters work locally; opening a conversation clears its local unread badge.
Use the pin in a thread to add or remove a shortcut. Drafts survive navigation
and pinning. Pins and unread state last for this app session.

The visual research included [Orbit Messenger on Dribbble](https://dribbble.com/shots/27279050-Orbit-Messenger-Dark-Mode-UI-Concept),
[Chat App Design on Behance](https://www.behance.net/gallery/186704313/Chat-App-Design-Dark-UI),
and [Appetite UI's chat patterns](https://www.appetiteui.com/screens/patterns/chat).
These informed the avatar shortcuts, inbox hierarchy, quiet metadata, and fixed composer.

## Run

From the repository root, build ORCA as described in `docs/BUILD.md`, then run:

```sh
build/bin/orca apps/chat
# Or, with an existing build:
make chat
```

For iOS, use `make ios-run` (iPhone simulator),
`make ios-run DEVICE_TYPE=iPad` (iPad simulator), or `make ios-install`
(connected iPhone with matching development signing credentials).
See [iOS setup and deployment](../../docs/IOS.md).

With no backend configuration, three local sample conversations let you open
threads, send messages, and create conversations immediately. Messages last
until the app restarts; demo contacts do not generate automatic replies.
Use the window close button to quit. Return or Send submits a message.
On iOS, use the system app switcher to close the app.

## Connect Supabase

Supabase provides Postgres, authentication, a REST API, and row-level security
without requiring a vendor-specific native chat SDK. Its HTTP interface fits
ORCA's existing Lua networking. Stream is another strong option for a complete
chat service, but its SDK/UI features add less value when ORCA owns the UI.

1. Create a Supabase project and run `backend/schema.sql` in its SQL editor.
2. Enable email/password authentication and confirm the test users' emails.
3. Launch with the public client configuration:

```sh
SUPABASE_URL=https://YOUR_PROJECT.supabase.co \
SUPABASE_PUBLISHABLE_KEY=sb_publishable_YOUR_KEY \
build/bin/orca apps/chat
```

Sign in or create an account inside the app. Create a conversation using the
other participant's Auth user UUID, available in the project's Authentication
dashboard. The creator is added automatically. Leave the member field blank
for a personal room. Open a second client with another account to exchange
messages. **Refresh** checks for incoming messages.

The client uses publishable keys and user JWTs. Database policies restrict
history and sending to room members. Sessions stay in memory, so restarting
requires sign-in. The initial app supports text and the latest 100 messages;
live subscriptions, push, attachments, and older-history pagination are future
work. Supabase deployment and live multi-user validation require your project.

For a physical iOS device, put the public project URL and publishable key in
`config.lua` before building; terminal environment variables are not inherited
by installed iOS apps. Leave the config unchanged to run the local demo.

See [API.md](API.md) for the Lua interface, transport injection, schema details,
and backend references.

## Verify

```sh
make test-chat
make test-network-headers
```

The API suite exercises authentication, refresh, authorization headers, room
creation, message ordering, validation, and HTTP failures with an injected
transport. The ORCA UI test covers conversation rendering, sending, retaining
input after failure, new-chat controls, and sign-in controls. Run UI tests with
the Supabase environment variables unset.
The HTTP regression uses a localhost echo server to verify that long JWT
headers reach the server without truncation.

Backend references: [Supabase Auth](https://supabase.com/docs/guides/auth),
[row-level security](https://supabase.com/docs/guides/database/postgres/row-level-security),
[client keys](https://supabase.com/docs/guides/getting-started/api-keys),
and [Stream Chat](https://getstream.io/chat/docs/).
