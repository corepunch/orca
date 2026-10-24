# Chat Lua API

`require "api.chat"` provides one client for Supabase and the offline mock.
Only the HTTP adapter imports `orca.network` and `orca.parsers.json`. Remote
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
Sessions and passwords stay in memory; restarting requires sign-in. Never log
session objects. Protected requests use the user's JWT and a publishable key.

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
With Supabase's [secure email change](https://supabase.com/docs/reference/javascript/auth-updateuser),
both inboxes may need confirmation. Until the requested identifier appears in
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
| `createInvite(roomId)` | `{code,room_id,expires_at,max_uses,uses,...}` |
| `joinInvite(code)` | Joined room with role and posting capability |
| `revokeInvite(roomId,code)` | Boolean; false when the invite is unknown |
| `findChannels(query)` | Public channels matching a name query of at least two characters |
| `joinPublicChannel(roomId)` | Public channel joined as a subscriber |
| `listMessages(roomId,{limit=100})` | Latest 1–100 messages in chronological display order |
| `sendMessage(roomId,body,attachmentIds)` | Persisted message with embedded `attachments` |

Names contain 1–80 characters; `memberIds` contains up to 50 other Auth UUIDs.
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
sender in that room. Direct REST inserts are disabled. History refresh is
explicit; realtime subscriptions, push notifications, older-history pagination,
read receipts and end-to-end encryption are not implemented.

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
generated EXIF is accepted. The trusted Edge finalizer independently validates
stored bytes and metadata before marking a reservation ready; its JPEG check
is structural and does **not** fully decode compressed pixel entropy.

Storage uses the private `chat-photos` bucket. Upload reserves a pending row,
uploads immutable photo/thumbnail objects, then calls `finalize-chat-photo`.
Photos transition `pending -> ready -> attached`, or `discarded` before posting.
Unpublished photos are visible only to their uploader while still a member;
attached photos are readable by room members. Publishing and finalization
recheck posting rights. Reservations expire after one hour, with at most twenty
unfinished recent uploads per user. Failed uploads attempt cleanup. Local
preview files are private temporary files and the app clears them on logout.

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
`find`, and `table_name`. Rooms and Messages support `create`; message creation
forwards `attachment_ids`. Rows expose fields plus custom methods and relations.
`find` searches the fetched page; a message lookup needs `room_id` and only
covers that history window. A missing row returns nil; API errors return
`nil,error`. There are no SQL strings, update/delete operations, automatic
observability or automatic relation preloading.

Create the remote client with `Chat.supabase {url="https://PROJECT.supabase.co",
publishableKey="sb_publishable_..."}`. Apply [schema.sql](backend/schema.sql),
[002_contacts.sql](backend/002_contacts.sql), then
[003_messenger.sql](backend/003_messenger.sql), and deploy the finalizer before
photo uploads. Follow [backend setup](backend/README.md) for Auth providers,
templates, private storage, Edge secrets and integration checks. The client
rejects service/secret keys; those belong only in the trusted server function.
No live project is provisioned or verified by the offline suites.

## Mock and tests

`Chat.mock {clock=function() return unixSeconds end}` creates a fresh isolated
store. `Chat.demo()` additionally signs in as `demo@example.invalid` with
password `password123`; Alex, Maya and Jules use `name@example.invalid` with
the same password. The six-digit mock code is defined only in
[mock_fixtures.lua](api/mock_fixtures.lua). Fixtures can be supplied as a table
or `fixtures=function(now) return table end`. No fake replies are generated.

The mock uses the same client, protocol paths, token refresh and models. Its
IDs/tokens are readable fixture strings and password signup immediately returns
a session. It emulates authorization and OTP limits in memory; it cannot test
email/SMS delivery, JWT cryptography or actual database/storage RLS.

Inject `transport=function(request) ... end` into `Chat.supabase` for protocol
tests. Requests contain `method`, `url`, `headers`, optional table or binary
string `body`, optional `responseType="binary"`, and explicit `arrayFields`
for empty JSON array arguments. Responses contain numeric `status` and decoded
JSON or raw binary `body`. The ORCA adapter preserves embedded NUL bytes and
encodes marked empty lists as `[]`.

Run `make test-chat` for API, mock, model, feature and headless UI checks.
The four non-UI Lua suites also run with standalone Lua 5.3+, e.g.
`lua tests/test_chat_features.lua`; ORCA additionally checks actual native
photo processing when available. `make test-media` verifies macOS image
preparation, `make test-chat-finalizer` checks the Edge handler and JPEG
inspector, and `make test-network-headers` checks native binary HTTP and long
headers against a local server. These checks need no Supabase credentials.
