# Chat Lua API

`require "api.chat"` provides one client for Appwrite and the offline mock.
Only the HTTP adapter imports `orca.network`; `api.json` handles strict JSON. Remote
methods yield through `network.fetch`; call them from an ORCA event coroutine.

```lua
local Chat = require "api.chat"
local client = Chat.mock() -- isolated, signed out, no network
assert(client:requestCode("demo@example.invalid"))
assert(client:verifyCode("demo@example.invalid", "123456")) -- mock fixture code only
local profile = assert(client:getProfile())
local rooms = assert(client:listRooms())
local sent = assert(client:sendMessage(rooms[1].id, "Hello!"))
```

Methods return `value, nil` on success and `nil, error` on failure. A successful
RPC may return `false`; test for `value == nil` when that distinction matters.
Errors contain `code`, `message`, and an optional numeric HTTP `status`.
Passwords and short-lived JWTs stay in memory. On iOS and macOS, Chat saves its
private Account credential synchronously in the app's Apple Keychain, scoped by
endpoint and project. On launch it revalidates the account and requests a fresh
JWT; temporary failures retain the credential and offer Retry. Expired/revoked
sessions require sign-in. Platforms without secure storage remain memory-only.
Never log session objects. Appwrite
Account requests use a private per-client session cookie; Function executions
send a short-lived `X-Appwrite-JWT`. The Function verifies the Appwrite-provided
runtime caller through Account on every protected operation.

## Appwrite configuration

```lua
local client = assert(Chat.appwrite {
  endpoint = "https://fra.cloud.appwrite.io/v1",
  projectId = "69f1c06800389dc6a1a0", -- Octopus, public identifier
  functionId = "orca-chat-api",
})
```

This is the default in `config.lua`. `APPWRITE_ENDPOINT`, `APPWRITE_PROJECT_ID`
and `APPWRITE_CHAT_FUNCTION_ID` override the public configuration; no API key is
accepted or bundled. `CHAT_BACKEND=mock` selects offline data explicitly.
The shared methods below keep the same return shapes and model bindings.

`api.appwrite_auth` handles signup, password sign-in, email/phone OTP,
JWT renewal and logout through native Appwrite Account endpoints. Its private
session cookie is extracted from response headers, kept private to that client,
and optionally persisted through `sessionStore` (`load`, `save`, `clear`). It is
never written to ORCA's shared cookie file. These operations work independently
of Chat Function deployment. Native Auth owns OTP rate limits and delivery.

`api.appwrite_transport` serializes authenticated domain requests as
`{method,path,body}` and submits a synchronous Function execution with the user JWT.
The function returns `{status,body}`; JPEG bodies use `encoding="base64"` in both
directions. The adapter validates the execution result and decodes raw photo
bytes before returning them to the shared client. Service failures never select
mock data. JSON escaping and empty array arguments survive both envelope layers.

The [Appwrite backend](backend/appwrite/README.md) owns authorization, role and
invite checks, verification challenges, rate limits, and encrypted photo storage.
Its seven private collections and indexes are provisioned in Octopus's existing
`octopus-db`. The Function is deployed with execution restricted to signed-in users. Phone sign-in and
identity linking also require delivery providers; missing providers produce an
explicit error. Session refresh revalidates the native Account session and issues
a new user JWT. It does not extend an expired native session or expose its cookie.

## Identity and profiles

| Method | Result |
| --- | --- |
| `requestCode(identifier)` | `{identifier,delivery,expires_in,retry_after}`; sends login/signup OTP |
| `verifyCode(identifier,code)` | Session after six-digit OTP verification |
| `requestIdentityLink(identifier)` | Starts a verified email/phone change for the current account |
| `verifyIdentityLink(identifier,code)` | Verified self profile once the requested identifier is confirmed |
| `getProfile()` | Self `{id,display_name,username,phone,email,discoverable_phone,discoverable_email}`; only verified identifiers |
| `updateProfile(values)` | Self profile after changing public name, username or discovery flags |
| `findPeople(query)` | Public `{id,display_name,username}` matches for an exact username or opted-in identifier |
| `listContacts()` | Other authorized shared-room profiles; no login identifiers or credentials |
| `getSession()` | Defensive session copy or nil; no request |
| `restoreSession()` | Validates the saved Appwrite credential and obtains a fresh JWT; called explicitly on app launch |
| `refreshSession()` | Rotated session; also runs near expiry or once after an authenticated HTTP 401 |
| `signOut()` | Clears local credentials and requests local-session revocation, including on network failure |
| `signIn(email,password)` | Existing email/password login; returns a session |
| `signUp(email,password,displayName)` | Existing password signup; session or `{confirmationRequired=true}` |

An identifier is an email or an international phone number beginning with `+`.
The client lowercases email and removes spaces, parentheses and hyphens from
phones, requiring 8–15 digits with a nonzero country prefix. Usernames are
unique lowercase strings of 3–32 letters, numbers or underscores, beginning
with a letter. An empty username clears it. Profiles cannot edit Auth identifiers.

Discovery flags default to false and require the corresponding verified
identifier. Search uses exact matches, with optional `@` for usernames, and
allows 20 lookups per minute per account. A known username does not grant room
membership. Contacts come from authorized relationships; channel subscribers
can see themselves and staff in the member list, without a peer subscriber
directory. Owners and admins can see the full membership.

Linking verifies ownership and preserves the account ID. An identifier already
owned by another account is rejected. The client checks the verified self
profile after Auth verification; the same user ID alone is insufficient proof.
Email changes may need confirmation from both inboxes. Until the requested identifier appears in
the verified profile, `verifyIdentityLink` returns `verification_pending` and
the UI should keep the verification form open for the remaining code. A
different-user verification response cannot replace the existing session.

The mock uses a five-minute code lifetime, 60-second resend cooldown, and five
attempts per challenge. Configure real Auth delivery, OTP lengths, expiry and
rate limits as described in [backend setup](backend/README.md); the request
result's timing fields are UI hints, not a replacement for server enforcement.

## Conversations, roles and invitations

| Method | Result |
| --- | --- |
| `listRooms()` | Member-visible rooms, including `kind`, `visibility`, `role`, `can_post` |
| `createRoom(name,memberIds,options)` | Room; options are `{kind="group",visibility="private"}` by default |
| `listMembers(roomId)` | Authorized `{user_id,display_name,username,role}` rows |
| `setMemberRole(roomId,userId,role)` | Owner-managed role change |
| `removeMember(roomId,userId)` | Owner/admin removal subject to role restrictions |
| `leaveRoom(roomId)` | Non-owner leaves a group/channel; membership and access are revoked |
| `renameRoom(roomId,name)` | Owner renames a group/channel; returns the updated room |
| `createInvite(roomId)` | `{code,room_id,expires_at,max_uses,uses,...}` |
| `joinInvite(code)` | Joined room with role and posting capability |
| `revokeInvite(roomId,code)` | Boolean; false when the invite is unknown |
| `findChannels(query)` | Public channels matching a name query of at least two characters |
| `joinPublicChannel(roomId)` | Public channel joined as a subscriber |
| `listMessages(roomId,{limit=100})` | Latest 1–100 messages in chronological display order |
| `sendMessage(roomId,body,attachmentIds)` | Persisted message with embedded `attachments` |
| `editMessage(messageId,body)` | The author's message with new text and `edited_at` |
| `deleteMessage(messageId)` | `true`; the message and its photos are removed for everyone |

Names contain 1–80 characters; `memberIds` contains up to 50 other Auth user IDs.
Groups and channels may start with an empty member list. The creator becomes
owner automatically. A private direct chat requires exactly one other person;
the canonical pair reuses its existing conversation in either participant order.

Groups allow owners, admins and members to post. Channels allow owners and
admins to post; ordinary members have role `subscriber` and `can_post=false`.
Only owners change roles. Admins can remove ordinary members, while owners
manage admins; the owner cannot be removed or demoted. Direct chats do not
expose membership administration. Removing a member revokes room, message and
private-photo access. Invitations require staff rights, expire after 24 hours,
allow ten joins, and can be revoked. Rejoining with a valid invite does not use
another allowance. Public channel discovery does not grant posting rights.

Messages allow up to 4000 UTF-8 bytes and four photos; photo-only messages use
`body=""`. Publishing uses the atomic `post_chat_message` RPC, which derives
the sender from Auth and attaches only ready, unexpired photos owned by that
sender in that room. Direct REST inserts are disabled.

Only the author edits a message, and only while they may still post in that
room. An edit changes the text, sets `edited_at` and keeps `created_at`, the
timeline position and photos. A photo message's caption can be removed. Edits
follow the same 4000-byte rule. A message deletes for everyone. Its author can
delete it while still a member, and group or channel owners and admins can delete
any message. Direct chats have no moderators. Deleting a message also deletes its
encrypted photos. In the app, tapping a message you may change opens a bottom
sheet with **edit** and **delete**. Edit loads the text into the composer under an
"Editing message" banner. Sending saves the edit and cancelling discards it, and
either way the unsent draft comes back. Delete asks for confirmation. Readers see
"edited" next to the time.

While foregrounded, the
open conversation polls the latest 100 messages every 3 seconds and the visible
inbox polls conversations and contacts every 15 seconds. Intervals begin after
the previous request completes; failures back off up to 60 seconds. Navigation,
sign-out and iOS scene deactivation cancel scheduled work and invalidate stale
responses. Foreground return requests an immediate refresh. Manual refresh is
still available and shares the read locks.

Polling uses a retained `core.Timer` component and `Timer.Tick`, canceling each
native repeating timer before fetching or scheduling another. Unchanged results
retain their existing rows; changed timelines preserve reading position and
drafts, following new messages only if the reader was already at the bottom.
Realtime subscriptions, push notifications, older-history pagination, read
receipts and end-to-end encryption are not implemented.

## Private photos

```lua
local media = require "orca.media"
local photo, err = media.pickPhoto() -- or media.processPhoto(sourceBytes)
if photo then
  local attachment = assert(client:uploadPhoto(room.id, photo))
  local message = assert(client:sendMessage(room.id, "", { attachment.id }))
  local thumbnailJpeg = assert(client:getPhoto(message.attachments[1].id, true))
end
```

`uploadPhoto(roomId,{jpeg,thumbnail,width,height})` returns attachment metadata:
`id`, `room_id`, `uploader_id`, `status`, dimensions, byte counts, object keys
and optional `message_id`. Native preparation also returns thumbnail dimensions
and `mime_type="image/jpeg"`. `getPhoto(id,thumbnail)` returns raw JPEG bytes;
`discardPhoto(id)` removes an unpublished photo and both private objects.

The native macOS encoder decodes and orients source pixels, flattens alpha,
converts to sRGB, strips source private metadata, and produces a JPEG plus a
thumbnail. Output limits are 1,000,000 bytes per object, 1600 pixels on the
photo's longest edge and 256 pixels for the thumbnail. Client preflight and
the mock inspect JPEG structure, dimensions and location metadata. Harmless
generated EXIF is accepted. The trusted Chat Function independently validates
stored bytes and metadata before marking a reservation ready; its JPEG check
is structural and does **not** fully decode compressed pixel entropy.

Photos are encrypted in the private Appwrite bucket. Upload reserves a pending row,
uploads immutable photo/thumbnail objects, then calls `finalize-chat-photo`.
Photos transition `pending -> ready -> attached`, or `discarded` before posting.
Unpublished photos are visible only to their uploader while still a member;
attached photos are readable by room members. Publishing and finalization
recheck posting rights. Reservations expire after one hour, with at most twenty
unfinished recent uploads per user. Failed uploads attempt cleanup. Local
preview files are private temporary files and the app clears them on logout.

Appwrite uses the same reservation/finalization/publishing API. Octopus's plan
does not allow another bucket, so photo objects are encrypted with AES-256-GCM
before upload to its public `post-images` bucket. Per-attachment keys remain in
private Chat records and are removed from all client metadata. The Function
rechecks membership and publication state before decrypting and returning bytes.
This is server-side encryption; the server can read photos. See the Appwrite
backend guide for reservation cleanup and concurrency/recovery limitations.

## Profile photos

| Method | Result |
| --- | --- |
| `getAvatars(userIds)` | `{[userId]=photoId or false}`; no session needed, cached per client |
| `getAvatar(photoId)` | JPEG or PNG bytes |

Photos are public Octopus data: `profiles.avatarId` names a `post-images` file,
set from the Octopus website or app and keyed by the shared Appwrite user ID.
The contract routes are read-only and unauthenticated:

| Route | Appwrite |
| --- | --- |
| `GET /rest/v1/profile_photos?select=user_id,avatar_id&user_id=in.(a,b)` → `[{user_id,avatar_id}]` | `GET /databases/octopus-db/collections/profiles/documents` with `equal(userId)`, `select` and `limit(100)` queries |
| `GET /storage/v1/object/public/profile-photos/<photoId>` → bytes | `GET /storage/buckets/post-images/files/<photoId>/view` |

The Appwrite adapter answers both natively with only `X-Appwrite-Project`: no
JWT, Account cookie or Function execution. Lookups take up to 100 IDs per
request; people without an Octopus profile or photo map to `false`. Chat shows
photos only where a tile stands for one person (direct chats with `peer_id`,
members, contacts and your profile); groups and channels keep their initials.
Previews are JPEG only, so PNG photos also keep the initials.

## Models and backend setup

```lua
local models = require("Chat/models").new(client)
local room = assert(models.Rooms:create {
  name = "News", member_ids = {}, kind = "channel", visibility = "private",
})
local messages = room:get_messages { limit = 20 }
local sent = models.Messages:create { room_id = room.id, body = "Hello!" }
local sender = sent:get_sender()
```

`Contacts`, `Rooms` and `Messages` are client-bound collections with `select`,
`find`, `find_all` and `table_name`. `find_all(ids, options)` fetches one authorized
provider result and filters exact IDs (or `options.key`) locally, preserving
provider order; it does not add SQL or search beyond that provider's window.
Rooms and Messages support `create`; message creation
forwards `attachment_ids`. Rows expose fields plus custom methods and relations.
`find` searches the fetched page; a message lookup needs `room_id` and only
covers that history window. A missing row returns nil; API errors return
`nil,error`. Message rows support `row:edit(body)`, which updates the row in place,
and `row:delete()`. There are no SQL strings, other update/delete operations, automatic
observability or automatic relation preloading.

Like [Lapis models](https://leafo.net/lapis/reference/models.html), model classes
own collection operations while row instances own domain methods. A room also
supports `get_members()`, `create_invite()` and `send_message(body, attachmentIds)`.
Use `models.Messages:preload_senders(messages)` to load sender relations with
one contact request per batch, including missing senders. Caches belong to that
batch and current session, and invalidate after logout, account switch or token
replacement. A session change during a yielding fetch rejects stale results.
Server `routes.js` maps methods and paths to domain services, following
[Lapis flows](https://leafo.net/lapis/reference/flows.html)'s separation of operations.

Create the remote client with `Chat.appwrite {endpoint=..., projectId=...,
functionId=...}`; [config.lua](config.lua) holds Octopus's public values. Follow
[backend setup](backend/README.md) and [Appwrite setup](backend/appwrite/README.md)
for the schema, Function deployment and integration checks. API keys and Function
secrets belong only in the trusted server function, never in the client.
The offline suites do not verify live deployment or delivery. Octopus's Chat
schema was provisioned separately through the Appwrite plugin. A separate native
cloud check on 2026-10-25 verified signup, password sign-in/rejection, session
cookies, JWT renewal and logout without a Chat Function. A second check against
the deployed Function verified two-account text/photo exchange, profiles and
discovery, direct/group/channel operations, invitations, role enforcement and
revoked-member access. Public bucket reads returned encrypted photo bytes.
All disposable accounts, documents and objects were deleted afterward.
Live email/SMS delivery remains unverified and requires configured providers.

## Mock and tests

`Chat.mock {clock=function() return unixSeconds end}` creates a fresh isolated
store. `Chat.demo()` additionally signs in as `demo@example.invalid` with
password `password123`; Alex, Maya and Jules use `name@example.invalid` with
the same password. The six-digit mock code is defined only in
[mock_fixtures.lua](api/mock_fixtures.lua). Fixtures can be supplied as a table
or `fixtures=function(now) return table end`. No fake replies are generated.
Fixture `avatars` maps user IDs to demo portraits in `assets/avatars/<name>.jpg`,
loaded when present; tests can set `profilePhotos` (user ID → photo ID) and
`photoFiles` (photo ID → bytes) directly.

The mock uses the same client, protocol paths, token refresh and models. Its
IDs/tokens are readable fixture strings and password signup immediately returns
a session. It emulates authorization and OTP limits in memory; it cannot test
email/SMS delivery, JWT cryptography or actual Appwrite permissions.

Use `Chat.client {url=..., transport=function(request) ... end}` for protocol
tests. Requests contain `method`, `url`, `headers`, optional table or binary
string `body`, optional `responseType="binary"`, and explicit `arrayFields`
for empty JSON array arguments. Responses contain numeric `status` and decoded
JSON or raw binary `body`. The ORCA adapter preserves embedded NUL bytes and
encodes marked empty lists as `[]`.

Run `make test-chat` for API, mock, model, feature and headless UI checks.
The four non-UI Lua suites also run with standalone Lua 5.3+, e.g.
`lua tests/test_chat_features.lua`; ORCA additionally checks actual native
photo processing when available. `make test-media` verifies macOS image
preparation, `make test-chat-appwrite-backend` checks the Function and JPEG
inspector, and `make test-network-headers` checks native binary HTTP and long
headers against a local server. These checks need no Appwrite credentials.
