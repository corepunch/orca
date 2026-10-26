# ORCA Messenger

A simple person-to-person chat app in Lua, using the `Application` entry point
with separate service modules and a shared Windows Phone application shell.
Runs on macOS with a **390 × 844** portrait viewport and on iPhone/iPad with
an adaptive viewport, touch scrolling and the native keyboard. iOS builds use
Make and the Xcode command-line tools without an Xcode project.

![ORCA Messenger in dark mode](preview.png)

[Light mode](preview-light.png) · [Message thread preview](thread-preview.png)

[New conversation](new-preview.png) · [Profile](profile-preview.png) · [Sign in](auth-preview.png)

## Design

The app follows Facebook Messenger on Windows Phone and is built from the
engine's Windows Phone controls (see `docs/UI_SYSTEM.md`):

- The inbox is a `Pivot` titled MESSENGER whose items are the all, unread
  and pinned lists. Swipe the list or tap a header to switch.
- Every other page is a locked one-item `Pivot`, so its title and page name
  share the inbox metrics.
- Commands live in an `ApplicationBar`. On the root pages its round buttons
  switch between chats, new chat and profile, and its menu holds refresh and
  sign out. In a conversation the bar holds back, attach, send and pin, with
  conversation info and refresh in the menu.
- Text boxes are pale with dark text in both themes and turn white with an
  accent border when focused. Secondary buttons are 2-pixel outlines,
  primary commands are filled with the accent, and links are plain accent
  text.
- Avatars are square solid-colour tiles with initials, replaced by the
  person's Octopus profile photo in direct chats, member and contact lists and
  your profile. Message bubbles are square, with
  Windows Phone tails, and sized to their text. Consecutive messages from one
  sender stack tightly and share one tail, sender names appear only in group
  chats, and the time has its own trailing row so text and metadata cannot overlap.
  Photos fill their bubble edge to edge above the caption and time.

`theme.lua` holds a black and a white palette with Messenger blue. The OS
appearance chooses between them; set `CHAT_APPEARANCE=light` or `dark` to
override it. On Linux, ORCA reports dark mode when `GTK_THEME` contains
`dark`. `Styles/app.css` adds hover and focus states with `:dark` variants.
Icons are stroke-based Lucide SVGs with inherited tint.

`views/shell.lua` owns one persistent `ApplicationBar` for Chats, New chat, and
Profile. It sets `PageHost.ActivePage` and disables the button for the page in
view. Root pages keep their instances and form drafts when you switch pages.
Conversation and sign-in screens are separate flows, and returning to the root
reuses the same shell.

The inbox prepares all three filtered lists when data or search changes. Swiping
only updates selection, keeping row instances and scroll positions intact instead
of allocating rows during the transition. Pivot also retains spare repeating
headers while dragging and keeps the page opaque when a short swipe settles back.
This trades a few retained lists for less work during touch interaction.

The performance approach follows Windows Phone 7's use of GPU/composition-thread
animations to reduce work on the UI thread ([Microsoft's WP7 performance article](https://download.microsoft.com/download/3/1/b/31b6c512-d66f-4138-b639-94711e95faca/msdn_0411dg.pdf)).
Off-thread input for Panorama and Pivot was a later [Windows Phone 8 improvement](https://blogs.windows.com/windowsdeveloper/2012/11/05/windows-phone-8-developer-platform-highlights/).
These changes remove allocation and a visible fade from ORCA's swipe path; ORCA
still performs layout during animation, so device frame-time profiling remains
necessary before claiming WP7-like smoothness.

Search matches conversation names and message previews. The all, unread and
pinned pivots filter locally; opening a conversation clears its local unread
badge. Use pin in a conversation's application bar to add or remove it from
the pinned pivot. Drafts survive navigation
and pinning. Pins and unread state last for this app session.

Open conversations update automatically every 3 seconds; the visible inbox
updates every 15 seconds. Polling pauses with the iOS scene and refreshes on
return, backs off on service errors, and preserves scroll position and drafts.
The manual refresh commands remain available.

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

The default backend is Appwrite's **Octopus** project in Frankfurt. Signup,
password sign-in, verification-code sign-in, renewal and logout use Appwrite's
Account API directly, just as Banking does. They do not require a Chat Function.
The Function serves conversations and other Chat data; see [Appwrite setup](backend/appwrite/README.md).
The app starts signed out and never falls back to demo data after a service error.

For the offline mock, run `CHAT_BACKEND=mock make chat`.
Enter **demo@example.invalid** or **+4915110000001**, then use verification
code **123456** to see contacts, three conversations, and message history.
Email/password login remains available with **demo@example.invalid / password123**.
The same client and models run in both modes; only the transport changes. Each mock client owns
fresh in-memory data. Sign-out keeps its data available for a later login;
restarting resets it. Mock contacts do not generate automatic replies.
Use the window close button to quit. Return or Send submits a message.
On iOS, use the system app switcher to close the app.

## Appwrite / Octopus

The Lua client follows Banking's HTTP integration pattern while keeping Chat's
shared models, validation, and views. Public configuration is in `config.lua`:

```sh
CHAT_BACKEND=appwrite APPWRITE_ENDPOINT=https://fra.cloud.appwrite.io/v1 \
APPWRITE_PROJECT_ID=69f1c06800389dc6a1a0 \
APPWRITE_CHAT_FUNCTION_ID=orca-chat-api make chat
```

Seven private `chat_` collections and their indexes are provisioned in the
existing `octopus-db` database. They cover profiles and discovery, conversations,
membership and roles, messages, photos, invitations, and verification/rate-limit
state. Access goes through the trusted Function; clients have no direct database
permissions and contain no API key. Each client retains its own session cookie
in memory and uses a short-lived user JWT for authenticated Function calls.

The Function implements verified identity linking, private direct/group chats,
public/private channels, member
management, invitations, and JPEG uploads/downloads. Linking identifiers needs
configured Appwrite Messaging providers; phone sign-in needs an Auth SMS provider.
These services are not simulated when their configuration is missing.

Profile photos are Octopus's public `profiles.avatarId` files, read directly
from Appwrite with the project ID alone; set yours on the Octopus website or
app. See [profile photos](API.md#profile-photos).

Octopus's plan allows one database and one bucket. Chat uses its own prefixed
collections and encrypts photo/thumbnail bytes with AES-256-GCM before writing to
the existing `post-images` bucket. Keys stay in private Chat records; the Function
checks membership before decrypting a photo. This protects photos in that public
bucket and is server-side encryption, not end-to-end encryption.

The Function is deployed in Octopus. Only signed-in users may execute it;
it has no session-creation permission and exposes no login proxy. Its server-only
verification secret is configured; delivery providers still need setup.
See the [deployment manifest](backend/appwrite/appwrite.config.json) and
[backend implementation/setup](backend/appwrite/README.md) for the exact requirements.
Live delivery verification still requires configured email/SMS providers.
Two disposable accounts verified live text/photo exchange, discovery,
invitations, channel roles and membership revocation on 2026-10-25. The check
confirmed ciphertext in the public bucket and removed all test data afterward.

The architecture follows [Lapis](https://github.com/leafo/lapis)'s separation of
models, rows, relations and reusable flows. Client-bound models expose domain
methods such as `room:send_message()` and `room:get_members()`. Bulk selection
and sender preloading avoid repeated contact requests and invalidate cached
relations when the account or session changes. Native authentication, transport,
server route handlers and domain services each have a separate owner.

## Using the live backend

`make chat` connects to the Octopus Appwrite project from [config.lua](config.lua).

Sign in with a phone or email verification code. On New chat, choose a known
contact or find an exact username, discoverable phone, or discoverable email.
Choose Direct, Group, or Channel; group/channel creators become owners and can
manage members and invitations. Public channels support search and subscription.
Open a second client with another account to exchange messages. **Refresh**
checks for incoming messages.

The client uses Appwrite sessions and short-lived JWTs. The Chat Function restricts
history and sending to room members. Sessions stay in memory, so restarting
requires sign-in. Messages support text and JPEG photos, prepared with native
resizing/compression and stored in a private bucket with thumbnails. History
currently loads the latest 100 messages. Video, live subscriptions, push, and
older-history pagination remain future work.

Installed iOS apps do not inherit terminal environment variables, so the public
Appwrite values are defaults in `config.lua`. Set `CHAT_BACKEND=mock` for the
offline mock on the desktop.

Unexpected route, event, and background callback failures display a **Chat error**
screen with source locations and a scrollable report. The report also catches
an oversized or displaced home footer and includes its measured bounds. Common
credentials and email/phone identifiers are redacted. **Return to chat** rebuilds
the shell while retaining the session and message drafts; it does not retry sends.
This reports errors while the process is running, not native crashes that terminate it.

See [API.md](API.md) for the Lua interface, transport injection, schema details,
and backend references.
See [PRODUCT.md](PRODUCT.md) for implementation status and remaining product
scope, including video. [Backend setup](backend/README.md) describes the
live Appwrite services.

## Verify

```sh
make test-chat
make test-chat-appwrite-backend
make test-media
make test-network-headers
```

The Appwrite backend suite covers server authorization, role and invitation rules,
private encrypted photos, verification challenges, and database pagination without
contacting the cloud or sending verification messages.
The API suite exercises authentication, refresh, authorization headers, room
creation, message ordering, validation, and HTTP failures. The mock suite
checks the same client with isolated fixtures and guards against network
imports. Model tests cover queries, row methods and relations. The UI test
injects a mock client explicitly, logs in through the form, selects API contacts,
manages groups/channels, sends photos, preserves rejected input, switches pages
through the shared application bar, and signs out. `make test-metro` covers the
engine's Pivot, Panorama and ApplicationBar controls. Feature tests cover identity verification, privacy, roles,
invitations, subscriptions, and attachment permissions.
It does not depend on environment credentials or an external service.
The HTTP regression uses a localhost echo server to verify that long JWT
headers and binary JPEG bodies reach the server without truncation. Native
ImageIO tests decode processed output and check orientation, size and metadata.

Backend references: [Appwrite Auth](https://appwrite.io/docs/products/auth),
[Functions](https://appwrite.io/docs/products/functions) and
[Databases](https://appwrite.io/docs/products/databases).

## Demo portraits

The shared [portrait library](../../demo-assets/profiles/README.md) contains 64
generated JPGs, including teens and adults approximately ages 14–60, plus the
original grid, splitter instructions and Appwrite file IDs. This app bundles
portraits for its offline fixture identities. Live seed profiles use the same
public `profiles.avatarId` data in both Chat and Octopus.
