# Chat on Appwrite

Chat uses the existing **Octopus** project (`69f1c06800389dc6a1a0`), Frankfurt
endpoint `https://fra.cloud.appwrite.io/v1`, and database `octopus-db`.
`schema.json` defines seven private collections with shared names. They have
**no client permissions** and `documentSecurity=false`. The Function joins the
existing public Octopus `profiles` with private `account-directory` records;
verified email/phone and discovery preferences never enter public profiles.
The migration below provisions this layout; previous deployments use legacy
`chat_` collections until explicitly migrated and replaced.

Signup, password sign-in, email/phone OTP, session refresh and sign-out call
**native Appwrite Account endpoints directly**, following Banking's direct
integration pattern. They work without a deployed Chat function. The function
handles authenticated Chat domain operations: profiles, discovery, membership,
invitations, messages, identity linking and private photos.

The Node 22 function source and deployment manifest are ready. Its execution
permission is restricted to signed-in Appwrite users. The Function was deployed
and activated in Octopus on 2026-10-25 after approval of its project-wide server
scopes. Its `CHAT_OTP_HMAC_KEY` is configured as a secret Function variable.
Native signup, password rejection/sign-in, cookie authentication, JWT renewal and
logout were verified against Octopus on 2026-10-25 using a disposable account,
which was deleted after verification. A separate live check of the deployed
Function used two disposable accounts to verify profiles, exact username
discovery, canonical direct chats, invitations, group/channel permissions,
Unicode and embedded-NUL messages, native-processed JPEG/thumbnail roundtrips,
and membership revocation. Anonymous bucket reads returned ciphertext; private
photo keys were absent from API responses. All test accounts, documents, state
and objects were removed afterward. No email/SMS code deliveries were performed;
delivery providers are not configured.

Previous production deployment: `6ae08dfcb0d301fbdd7b` (2026-10-27, legacy collection layout). The uploaded source archive
contains `package.json` and `src/`, with SHA-256
`5a3525d8b1d449dc58b88fd73cd2204858f4c544cfe65d1401e80b4afef7fc6e`.
It includes direct-room `peer_id`, message editing/deletion, non-owner group/channel
leave and owner-only renaming. Schema and permissions are unchanged. A live check
against the previous deployment caught nine edit/delete failures (missing routes);
the manual suite must be rerun after deployment, rather than relying on local tests.
The full supported live suite passed all 123 checks on this deployment on
2026-10-27, with no failures, skips or TODOs. It used only four dedicated test
accounts, with no server key; see [run details](live/README.md#latest-real-backend-run--2026-10-27).

## Deployment and configuration

Use `appwrite.config.json` from this directory for deployment.
The entrypoint is `src/main.js`; it uses native `fetch` and Node crypto without
third-party dependencies. An archive needs only `package.json` and `src/`.
The manifest selects `node-22`, grants execution to `users`, disables logging,
and requests these dynamic-key scopes:

| Scope | Purpose |
| --- | --- |
| `users.read` | Validate member IDs and verify discovery/identity uniqueness |
| `users.write` | Apply an email/phone change after ownership proof; synchronize the account display name |
| `documents.read`, `documents.write` | Read and update shared profiles and private conversation documents |
| `files.read`, `files.write` | Store encrypted objects and serve member-authorized plaintext |
| `targets.write`, `messages.write` | Deliver staged identifier-link challenges through Messaging |

Appwrite scopes apply to the project, including its other collections/users;
they cannot be limited to the shared collection allowlist. The handler constructs all backend
paths itself and exposes only its fixed Chat route allowlist. It does not issue
or proxy Auth sessions and has no session-management scope.

Execution identity comes only from Appwrite's runtime `x-appwrite-user-jwt` and
`x-appwrite-user-id` headers. The function validates the JWT against native
`/account` using `X-Appwrite-JWT` without the server key and compares the returned
account ID with the runtime caller ID. JSON `session`, `jwt` or user ID fields
cannot choose the caller. Console/API-key executions without a user JWT return
401. The dynamic key is read from the runtime `x-appwrite-key` header, is used only
for domain storage/verified identity operations, and is never sent to the app.
See [Appwrite function context](https://appwrite.io/docs/products/functions/develop)
for the runtime identity-header contract.

Set the following **server function variables**, without committing secrets:

| Variable | Value |
| --- | --- |
| `CHAT_DATABASE_ID` | `octopus-db` (default) |
| `CHAT_BUCKET_ID` | `post-images` (default) |
| `CHAT_OTP_HMAC_KEY` | A stable random secret of at least 32 characters, required for identity linking |
| `CHAT_EMAIL_PROVIDER_ID` | Configured Appwrite Messaging email provider ID, required to link/change email |
| `CHAT_SMS_PROVIDER_ID` | Configured Appwrite Messaging SMS provider ID, required to link/change phone |

`APPWRITE_FUNCTION_PROJECT_ID` and `APPWRITE_FUNCTION_API_ENDPOINT` come from the
runtime; the source defaults to this Octopus project and Frankfurt endpoint.
The desktop stores only the endpoint, project ID and function ID
`orca-chat-api`, with no API key. Configure it through `Chat.appwrite`.

The project currently has **zero Messaging providers**. Linking returns a clear
configuration error until the matching provider and HMAC secret are configured.
Native email/password and email OTP use Appwrite Auth's delivery configuration;
phone OTP additionally needs a working Appwrite Auth SMS provider. Native Auth
delivery and Messaging providers are separate configurations. Six-digit OTPs
must be enabled for the existing desktop verification form. Delivery was not
tested against real addresses or phone numbers.

## Shared storage migration

| Legacy collection | Shared collection |
| --- | --- |
| `chat_profiles` | Public display metadata in `profiles`; private identifiers/preferences in `account-directory` |
| `chat_rooms` | `conversations` |
| `chat_members` | `conversation-members` |
| `chat_messages` | `messages` |
| `chat_attachments` | `attachments` |
| `chat_invites` | `invitations` |
| `chat_state` | `service-state` |

The runtime uses only shared collections. Existing RPC/REST route names remain
compatible with the desktop client. Conversation, membership, message, attachment
and invitation IDs and JSON payloads are copied unchanged; encrypted file IDs,
keys and authenticated object names are retained. Account IDs and sessions are
never recreated or renamed. Message history orders by the indexed original
`created_at` payload timestamp, so copying a document does not change which
messages appear in the latest history page.

`profiles.username` remains the website's **display label**. The private directory
`handle` is the shared lowercase account name (1–36 letters/digits, dots,
underscores or hyphens, starting with a letter/digit). Chat maps the public label and trusted private handle to
`display_name` and `username` respectively. Owner-writable public `handle`
fields are ignored; only the private unique index establishes handle ownership. Existing website labels, bios,
interests, avatar IDs and permissions take precedence and remain intact when
merging an old Chat profile. The trusted legacy Chat handle, email, phone and
opt-ins go to administrator-only `account-directory`; no opaque Chat profile
payload is copied to public data. New public profiles receive public read and
owner update permissions, matching the social profile contract. Their account
name is synchronized on explicit profile Save; a failed name sync returns a
retryable `profile_sync` error after saving the shared profile.

The operator migration uses native fetch and needs administrative database,
collection, attribute, index and document read/write scopes plus `users.read`.
Pass its key through `APPWRITE_API_KEY` in the environment; never commit it or
place it in command arguments. `APPWRITE_ENDPOINT`, `APPWRITE_PROJECT_ID` and
`CHAT_DATABASE_ID` optionally select the target. Schema operations wait for
available attributes/indexes, preserve matching existing indexes, and refuse
private permission drift or conflicting definitions. Public collection
permissions are never changed.

```sh
# Provision the new collections and private handle/index. No legacy data changes.
node src/migrate.js
# Read-only schema verification.
node src/migrate.js --verify
# Disable the legacy Chat Function and wait until all executions finish FIRST.
node src/migrate.js --migrate-legacy --quiesced
# Check both schema and copied data without writes while the old Function is paused.
node src/migrate.js --verify --migrate-legacy --quiesced
# Optional cleanup only after the copy is verified; never the default.
node src/migrate.js --migrate-legacy --quiesced --drop-legacy
```

Deploy the new Function only after the copy succeeds. Keep the Function paused
through the migration and deployment; a failed copy leaves legacy collections
available for recovery. Do not switch a live old Function to the new schema or
resume it after copying: writes could diverge. The unified deployment script
coordinates this sequence. An interrupted migration is safe to repeat while
quiesced; matching destination documents are reused, differing destination data
requires operator resolution. Reconcile any deliberate post-migration changes
before retrying an old snapshot.

`--drop-legacy` verifies every copied document and rereads all sources before any
collection deletion. It never deletes accounts, public profiles or files. A
legacy lock after draining means an execution may have crashed: inspect it and
confirm no writer remains before explicitly removing that stale lock, then
retry. The migration does not expire or steal locks. It refuses unknown private
profile fields instead of discarding data, conflicting profile owners/handles,
handle collisions with account IDs, and unavailable schema builds.

Existing GUID accounts keep their aliases. New readable Account IDs and profile
handles share a namespace by application policy: the Function checks Users for
collisions and the private directory unique index enforces handle uniqueness.
Only authenticated Function routes may claim a handle; public profile writes cannot
change the authoritative claim. Account creation and handle claiming are separate Appwrite operations, so that
preflight is not a cross-service transaction. Concurrent claims can return a
recoverable conflict after an account was created; clients must report it and
allow authenticated recovery instead of silently choosing another identity.

The JavaScript operator API is `migrateShared(rest, { databaseId, verify,
migrateLegacy, dropLegacy, quiesced, wait })` from `src/migrate.js`. It loads the
local `schema.json`; the deployed handler does not import this operator module.
The report contains collection IDs/counts, without identifiers or payloads.

## Authentication and verified profiles

The desktop's Account adapter owns signup, password login, email/phone OTP,
session refresh and logout. It talks to native `/account`, `/account/sessions/*`,
`/account/tokens/*` and `/account/jwts` endpoints without a server key. Appwrite
therefore enforces its native Auth delivery, session and rate-limit policies.
Native session cookies remain private to the adapter, with synchronous Apple
Keychain persistence in the iOS/macOS app. Launch restores the credential and
obtains a fresh short-lived user JWT for authenticated function execution. Sign-out
revokes the current native session and clears local credentials, including on
network failure. Backend deployment and database availability do not participate
in those Account operations.

Password-only accounts may initially have an unverified email; self profiles
expose email/phone only when Appwrite marks them verified. Public profiles
contain only ID, display name, handle and avatar ID. The function verifies the calling
account for every Chat operation. Discovery/identity-link rate limits use private
claim documents; login throttling belongs to native Account.

Identity linking does not replace the signed-in account or create temporary Auth
users. It stores a private five-minute HMAC challenge, creates a temporary
Messaging target for the requested identifier, and sends a six-digit code.
The account's identifiers remain unchanged before proof. Verification permits
five guesses, consumes the proof, then uses Users API to update the same account
and its verified flag. Appwrite uniqueness prevents taking another user's email
or phone. Targets are removed after completion, failed sends, expired attempts,
or the next request; unattended expired targets need administrative cleanup.
If the update succeeds but marking it verified fails, the identifier remains
unverified and the user must request another code. No unproven identifier enters
discovery. Both existing email and phone are preserved unless that particular
field is being explicitly changed.

Exact discovery requires the corresponding opt-in flag and rechecks the live
native verified identity. Twenty lookups per account/minute are allowed.
Usernames have a unique index. Contacts come from shared authorized rooms;
channel subscribers see themselves and staff, without a peer subscriber list.

## Rooms, messages and encrypted photos

All existing direct, group and channel operations are implemented: canonical
direct pairs, owner/admin/member/subscriber roles, member removal, private
invitations, public channel discovery/join, recent message history, editing and
deleting messages, non-owner leaving, owner renaming, and up to four JPEG
attachments per message. A group or channel may start empty. Room
creation validates up to fifty other native users, writes memberships with
bounded concurrency, then commits visibility with the room document. Queries
follow Appwrite cursors rather than silently returning the default first 25
rows. The 10,000-document safety cap returns an explicit error, never a partial
directory. The foreground client polls the open conversation every 3 seconds
and the visible inbox every 15 seconds, with non-overlapping reads and retry
backoff. History uses cursor pages of 30 messages by default, clamped to 1–100 per page.
`POST /rest/v1/rpc/list_chat_messages` accepts `{target_room, limit?, before?}` and
returns `{messages, next_cursor, has_more}`. Messages in each page are chronological;
`next_cursor` is the oldest returned message ID when older messages remain.
Omit `before` for the newest page, then pass `next_cursor` to load older history.
The compatible `GET /rest/v1/chat_messages?room_id=eq.ID&limit=N&before=ID`
continues returning descending rows without metadata.

The database applies `room_id` equality, indexed original `created_at DESC`,
`$id DESC` as a stable tie breaker, and `cursorAfter(before)`. It selects only
`$id` and `payload`, disables totals, and reads at most N+1 message rows to
establish `has_more`; there are no offsets or full-history reads. A cursor must
belong to the same authorized conversation. Deleted anchors, including a delete
between validation and listing, return `409 stale_cursor`; the client must
explicitly reload the newest page to establish a fresh cursor. New messages do
not move an existing older-page anchor. Attachment hydration is bounded by the
page and four attachments per message, with eight concurrent message views.
History performs no per-message profile or directory reads. Realtime
subscriptions, push, receipts and end-to-end encryption remain outside the
existing Chat feature set.

New database and bucket creation were denied by the current plan quotas. The
existing `post-images` bucket has public read and no per-file security, so it
**must never receive plaintext Chat images**. Its permissions are unchanged.
Each private attachment document holds a fresh random AES-256-GCM key. Photo
and thumbnail objects are encrypted independently with random nonces and their
logical object key as authenticated data. Only ciphertext is uploaded, under
immutable `.jpg` object names accepted by the existing bucket. They cannot be
previewed as images from the public bucket. The key and upload state are stripped
from every client response. This protects public-bucket reads; project
administrators and the trusted function can decrypt and it is not end-to-end
encryption.

Uploads reserve a pending row, inspect JPEG structure/dimensions/location
metadata, encrypt both objects, and finalize only after rechecking the stored
decrypted bytes. Each object is at most 1,000,000 plaintext bytes, with maximum
edges 1600/256 pixels. Unpublished reservations expire after one hour and a user
may have twenty unfinished unexpired photos. Each new reservation also reclaims
up to three expired unpublished photos and their encrypted objects, including
uploads left behind after a failure. This lazy cleanup does not replace periodic
administrator maintenance for abandoned accounts.

A single message document is the publication commit marker. The
function first validates sender, room, expiry and attachment ownership under the
room lock, updates attachment references, then creates that message. A failed
commit leaves photo references private and retryable. Membership is required
to read attached photos; unpublished photos additionally require uploader
ownership and an unexpired reservation. Removing membership revokes future
history and photo requests. Explicit discard also works for an uploader who no
longer belongs to the room and deletes both ciphertext objects.

`edit_chat_message` and `delete_chat_message` re-read the message under the same
room lock. An edit only rewrites the author's text and `edited_at`, while the
author can still post there. Deletion is allowed for the author, or for owners
and admins outside direct chats. It removes the message document first, which
unpublishes it immediately, then deletes its attachment rows and both encrypted
objects. If that cleanup is interrupted, the photo rows reference an absent
message. They become private to their uploader and are reclaimed after their
reservation expiry, like any other unpublished photo.

## Serialization, failure recovery and maintenance

Room writes share a durable lock stored as a uniquely created `service-state`
document. This serializes membership/role changes, invitation joins, posting,
upload finalization and cleanup. Invitation allowance is reserved before the
membership is created, so a failed creation may consume an allowance but cannot
exceed ten joins. The function does not claim full multi-service transactions.

Locks are **fail-closed**: they are released in `finally` but are never stolen
on a timer. A killed or timed-out execution can leave a resource busy until an
administrator recovers it. Automatically deleting an old lock without a fencing
or compare-and-swap primitive would allow an older worker to overwrite a newer
one. Appwrite synchronous executions currently have a 30-second limit even
though the manifest permits a 60-second runtime timeout; network calls have a
20-second request timeout. Member validation/creation and profile/history
reads use concurrency of eight to reduce the risk for large rooms. Production
latency and timeout behavior require deployment verification.

Read requests still verify the caller through Appwrite Account. An unchanged
Chat profile needs one document read without taking a persistent lock. Profile
creation and verified-identifier changes take the profile lock and re-read the
current document before writing, preserving concurrent profile edits and
resetting discovery opt-ins when an identifier changes.

Contact lookup enumerates shared rooms with current membership and channel
visibility checks, ignores unfinished rooms, and fetches each distinct peer's
profile once per request. Room and profile reads use bounded concurrency rather
than walking rooms serially. The client publishes inbox rows before contacts
finish and skips the contacts request for an empty inbox. Message history is
loaded only when a conversation opens.

For a stuck `busy` response, first disable new executions or otherwise quiesce
the function, verify the old execution has ended, and inspect `service-state` rows
with `kind=lock`. Their private `name`, `token` and `acquired_at` identify the
resource and owner. Only after verifying no worker is still running, remove
that specific lock document through the Appwrite console/operator, then restore
execution. Never bulk-delete live locks. Room/message commit markers make partial
work non-visible; orphan membership documents referencing an absent room may
be removed after confirming its creation failed. Orphan unpublished attachments
can be discarded or reclaimed after expiry. Already committed messages remain
authoritative even if a response was lost.

Rate limits use immutable per-window claim documents instead of global locks;
an interrupted discovery or identity-link request cannot hold a global quota lock.
Administrator maintenance should remove expired `rate_claim` and `link_cooldown` state documents, remove expired `link` targets/challenges, and
reclaim expired unpublished photos while preserving any photo referenced by a
committed message. Use `ChatService.cleanupExpiredPhotos(rows, maximum)` from a
trusted administrative script for the photo portion. There is intentionally no
public maintenance or lock-recovery route. Keep maintenance batches below the
query safety cap and run them regularly before large state sets accumulate.

## Wire contract and checks

The application boundary follows Lapis's explicit method/path route pattern:
`main.js` builds request context and authenticates runtime identity, `routes.js`
selects a handler, `service.js` applies Chat domain policy, and `rest.js` owns
Appwrite persistence. Native Account authentication is a separate client service.
This uses no SQL emulation, arbitrary REST proxy, or Lapis server dependency.

Invoke the function synchronously with an authenticated Appwrite user JWT on the
**outer** Appwrite execution request (`X-Appwrite-JWT`) and the JSON envelope:

```json
{"method":"POST","path":"/rest/v1/rpc/list_chat_members","body":{"target_room":"ROOM_ID"}}
```

Binary upload uses `body` as a canonical base64 string and `encoding:"base64"`.
The response body is always a JSON envelope with a numeric Chat `status`, decoded
`body`, and optional `encoding:"base64"` for decrypted JPEG bytes. Appwrite's
outer function response remains JSON/HTTP 200, so the desktop unwraps the inner
status. These paths preserve the existing Chat client contract; no request can
select arbitrary Appwrite collections, users, buckets or URLs.

Run `npm test` in this directory, or `node --test test/*.test.js`. The offline
suite covers runtime JWT authentication and caller-override rejection, absent
Auth-proxy routes,
verified exact discovery, canonical directs, complete fifty-one-member creation,
channel privacy and roles, invitations, image encryption/tamper checks,
publication failure, membership revocation, expiry and cleanup, cursor pagination,
and concurrent immutable rate claims. It uses injected REST/repositories and
does not claim live delivery, runtime deployment or provider verification.
For the deployed Function, run the manual [live suite](live/README.md)
(`make test-live`) with persistent test accounts.
