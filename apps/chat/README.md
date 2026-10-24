# ORCA Messenger

A simple person-to-person chat app in Lua, using the `Application` entry point
with separate service modules and a shared native tab shell.
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

`views/shell.lua` owns one persistent `TabBar` for Chats, New chat, and Profile.
Its selection binds to `PageHost.ActivePage`; root pages keep their instances
and form drafts across tab switches. Conversation and sign-in screens are
separate flows, and returning to the root reuses the same shell.

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

With no backend configuration, the app uses an offline mock API and starts at
login. Enter **demo@example.invalid** or **+4915110000001**, then use verification
code **123456** to see contacts, three conversations, and message history.
Email/password login remains available with **demo@example.invalid / password123**.
`CHAT_BACKEND=mock make chat` selects
it explicitly even when Supabase credentials are present. The same client and
models run in both modes; only the transport changes. Each mock client owns
fresh in-memory data. Sign-out keeps its data available for a later login;
restarting resets it. Mock contacts do not generate automatic replies.
Use the window close button to quit. Return or Send submits a message.
On iOS, use the system app switcher to close the app.

## Connect Supabase

Supabase provides Postgres, authentication, a REST API, and row-level security
without requiring a vendor-specific native chat SDK. Its HTTP interface fits
ORCA's existing Lua networking. Stream is another strong option for a complete
chat service, but its SDK/UI features add less value when ORCA owns the UI.

1. Create a Supabase project and run the unapplied migrations in order:
   `backend/schema.sql`, `backend/002_contacts.sql`, `backend/003_messenger.sql`.
2. Configure phone/email code delivery and deploy the photo finalizer/cleanup
   functions as described in [backend setup](backend/README.md).
3. Launch with the public client configuration:

```sh
CHAT_BACKEND=supabase SUPABASE_URL=https://YOUR_PROJECT.supabase.co \
SUPABASE_PUBLISHABLE_KEY=sb_publishable_YOUR_KEY \
build/bin/orca apps/chat
```

Sign in with a phone or email verification code. On New chat, choose a known
contact or find an exact username, discoverable phone, or discoverable email.
Choose Direct, Group, or Channel; group/channel creators become owners and can
manage members and invitations. Public channels support search and subscription.
Open a second client with another account to exchange messages. **Refresh**
checks for incoming messages.

The client uses publishable keys and user JWTs. Database policies restrict
history and sending to room members. Sessions stay in memory, so restarting
requires sign-in. Messages support text and JPEG photos, prepared with native
resizing/compression and stored in a private bucket with thumbnails. History
currently loads the latest 100 messages. Video, live subscriptions, push, and
older-history pagination remain future work. Supabase deployment and live
multi-user validation require your project.

For a physical iOS device, put the public project URL and publishable key in
`config.lua` before building; terminal environment variables are not inherited
by installed iOS apps. Leave the config unchanged to run the offline mock.

See [API.md](API.md) for the Lua interface, transport injection, schema details,
and backend references.
See [PRODUCT.md](PRODUCT.md) for implementation status and remaining product
scope, including video. [Backend setup](backend/README.md) explains the
Supabase/Firebase choice and which live services you need to create.

## Verify

```sh
make test-chat
make test-media
make test-chat-finalizer
make test-network-headers
```

The API suite exercises authentication, refresh, authorization headers, room
creation, message ordering, validation, and HTTP failures. The mock suite
checks the same client with isolated fixtures and guards against network
imports. Model tests cover queries, row methods and relations. The UI test
injects a mock client explicitly, logs in through the form, selects API contacts,
manages groups/channels, sends photos, preserves rejected input, switches shared
tabs, and signs out. Feature tests cover identity verification, privacy, roles,
invitations, subscriptions, and attachment permissions.
It does not depend on environment credentials or an external service.
The HTTP regression uses a localhost echo server to verify that long JWT
headers and binary JPEG bodies reach the server without truncation. Native
ImageIO tests decode processed output and check orientation, size and metadata;
Edge Function tests cover trusted finalization and expired-upload cleanup.

Backend references: [Supabase Auth](https://supabase.com/docs/guides/auth),
[row-level security](https://supabase.com/docs/guides/database/postgres/row-level-security),
[client keys](https://supabase.com/docs/guides/getting-started/api-keys),
and [Stream Chat](https://getstream.io/chat/docs/).
