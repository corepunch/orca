# Chat on Appwrite

Chat uses the existing **Octopus** project (`69f1c06800389dc6a1a0`), Frankfurt
endpoint `https://fra.cloud.appwrite.io/v1`, and database `octopus-db`.
`schema.json` defines ten private collections with shared names. They have
**no client permissions** and `documentSecurity=false`. The Function joins the
existing public Octopus `profiles` with private `account-directory` records;
verified email, the private phone number and discovery preferences never enter
public profiles.
The migration below provisions this layout; previous deployments use legacy
`chat_` collections until explicitly migrated and replaced.

Signup, password sign-in, email OTP, session refresh and sign-out call
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

Current production deployment: `6af832a329a5af00aaeb` (2026-11-14), source
archive SHA-256 `20d0cec4e6a6fc1ecbdff6b99f34fc51e5c39a51999387f6b3386476d91280d0`
from commit `58357ac0`, which removed the end-to-end encryption device-key
routes again. Function settings, scopes and variables were unchanged; no live
suite was run. The empty `device-keys` collection created for those routes was
deleted the same day. Node `fetch` may need
`--dns-result-order=ipv4first` on networks where IPv6 connections to Appwrite
time out.

Earlier production deployment: `6ae08dfcb0d301fbdd7b` (2026-10-27, legacy collection layout). The uploaded source archive
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
| `users.write` | Apply a verified identity change; synchronize display names; block and delete the caller's shared account after confirmed deletion |
| `documents.read`, `documents.write` | Read and update shared profiles and private conversation documents |
| `files.read`, `files.write` | Store encrypted objects and serve member-authorized plaintext |
| `targets.write` | Create temporary identifier-link challenge targets |
| `messages.write` | Deliver identifier-link challenges and new-message push through Messaging |


When using the Appwrite operator `functions_update` tool, read the current
Function first and explicitly preserve its existing configuration alongside
the intended change. Omitted fields can be replaced by tool defaults: an
events/interval-only update was observed to reset `execute` to `[]`, `scopes`
to `[]`, and `logging` to `true`. Include the existing runtime, execution
roles, scopes, logging, timeout, enabled state, entrypoint, build commands and
build/runtime specifications, then read them back before activating a deployment.
This update does not require reading or resubmitting secret variable values.

Settings and variable changes may leave `live=false` even after selecting a
ready deployment built earlier. Rebuild the approved source with the current
configuration (for example, with `functions_create_duplicate_deployment`).
That operator tool was observed to return `activate=true`; finish schema and
configuration review before creating the rebuild. Then wait for `status=ready`, and verify both the active deployment ID and
`live=true`. Keep the previous deployment and configuration for rollback.
These administrative checks do not require a Function execution or a live
integration test. See [Appwrite's Function model](https://appwrite.io/docs/references/cloud/models/function)
for the meaning of `live`.

Appwrite scopes apply to the project, including its other collections/users;
they cannot be limited to the shared collection allowlist. The handler constructs all backend
paths itself and exposes only its fixed Chat route allowlist. It does not issue
or proxy Auth sessions and has no session-management scope.

Execution identity comes only from Appwrite's runtime `x-appwrite-user-jwt` and
`x-appwrite-user-id` headers. The function validates the JWT against native
`/account` using `X-Appwrite-JWT` without the server key and compares the returned
account ID with the runtime caller ID. JSON `session`, `jwt` or user ID fields
cannot choose the caller. Ordinary console/API-key executions without a user JWT
return 401. Platform event/schedule executions process only durable deletion jobs,
without taking user IDs or arbitrary paths from an ordinary client request.
The dynamic key is read from the runtime `x-appwrite-key` header, is used only
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
| `CHAT_PUSH_ENABLED` | `true` enables new-message push; absent/other values keep it disabled |
| `CHAT_APPLE_CLIENT_ID` | Apple Services ID used by Appwrite's browser OAuth flow; required to delete an Apple-linked account |
| `CHAT_APPLE_CLIENT_SECRET` | Secret ES256 client JWT for that Services ID; maintain/rotate it before its Apple expiration |
| `CHAT_BLOCKED_TERMS` | Optional JSON array of additional prohibited whole words/phrases (up to 200 entries); built-in text filtering remains active when absent |

`APPWRITE_FUNCTION_PROJECT_ID` and `APPWRITE_FUNCTION_API_ENDPOINT` come from the
runtime; the source defaults to this Octopus project and Frankfurt endpoint.
The desktop stores only the endpoint, project ID and function ID
`orca-chat-api`, with no API key. Configure it through `Chat.appwrite`.

Messaging providers require separate operator configuration. Linking returns a clear
configuration error until the matching provider and HMAC secret are configured.
Native email/password and email OTP use Appwrite Auth's delivery configuration.
Chat sends no SMS: phone numbers are saved in settings without a code. Native Auth
delivery and Messaging providers are separate configurations. Six-digit OTPs
must be enabled for the existing desktop verification form. Delivery was not
tested against real addresses or phone numbers.

## New-message push (APNs)

The Apple client registers its native APNs token as an Appwrite Account push
target using its ordinary authenticated session. The public client configuration
`pushProviderId` in [`apps/chat/config.lua`](../../config.lua)
(or `APPWRITE_PUSH_PROVIDER_ID` in a local run)
selects the provider. Neither device registration nor the app contains a server
key. Registering, rotating and removing targets is owned by the client, including
removal on sign-out.

Chat defaults to Octopus's enabled production APNs provider
`6ae23f3b0014b18bf3dd` (`Production sender`, Bundle ID `org.orca.chat`,
Apple Team ID `BM2R8F5YHC`, sandbox disabled; configuration checked 2026-10-28).
This default targets App Store and TestFlight builds. The provider ID is public;
its authentication key remains in Appwrite. Function activation still requires
deploying the push-capable source and setting `CHAT_PUSH_ENABLED=true` as below.

To activate push for a signed Apple build:

1. Enable Apple's Push Notifications capability for that build's explicit App ID
   and provision the app with the corresponding push entitlement. See the
   [native push API](../../../../docs/LUA_API.md#native-push-notifications) and
   `apps/chat/Push.entitlements` (iOS),
   `Push-macOS-development.entitlements` or `Push-macOS-production.entitlements`
   for the matching Apple build. Xcode defaults `ORCA_APNS_ENVIRONMENT` to
   `development` in Debug and `production` in Release; override it when the
   selected signing profile uses a different environment. `make ios-install`
   requires a development profile with APNs enabled. For macOS use a signed
   application bundle; see [local macOS packaging and signing](../../../../tools/macos/README.md).
   The unbundled `make chat` executable does not register for remote notifications.
2. Create an [Appwrite APNs provider](https://appwrite.io/docs/products/messaging/apns)
   in the target project using the Apple Team ID, Bundle ID, key ID and `.p8`
   authentication key. Keep the key in Appwrite; never commit or bundle it.
   Match the provider's sandbox/production setting to the build's signing
   environment. Use separate provider IDs when shipping both environments.
3. Set the matching provider ID in the public Chat client configuration and
   allow notifications in the OS permission prompt.
4. Deploy this Function source with the manifest's existing `messages.write`
   dynamic-key scope, then set its server variable `CHAT_PUSH_ENABLED=true`.
   Restart/redeploy the Function as required for the variable to take effect.

After a text or photo message commits, the Function queues one
[Messaging push](https://appwrite.io/docs/products/messaging/send-push-notifications)
per current conversation member except the author. It resolves recipients from
the private membership collection under the room lock, with cursor pagination;
request JSON cannot choose recipients. Message previews default on. Direct
alerts use the saved sender display name (then account name or `Member`) and a
short message preview. Group alerts use the room name and `Sender: preview`;
channel alerts use the channel name and preview without the real author's name.
Photo-only messages say `Sent a photo` or `Sent N photos`; a caption takes
priority. Titles and bodies remove control characters and newlines and are
limited to 120 and 240 Unicode code points, including any truncation ellipsis.
There is no sender photo. The default sound remains enabled.

The recipient's private `notification_previews` profile boolean controls alert
content. `false` sends only `ORCA Chat` / `New message`; missing legacy values
default to `true`. This preference lives in the administrator-only account
directory payload and is excluded from public profiles, contacts and discovery.
Failed room, sender or recipient metadata lookups fall back to a generic alert.
Preview text is sent through Appwrite and APNs; iOS Show Previews controls when
the phone reveals it, including after Face ID with When Unlocked selected.
Data contains only `conversationId`, `messageId` and the recipient
`userId`. The client checks the signed-in account before opening the conversation;
the ordinary history route still enforces current membership. Edits and deletions
do not notify, and a failed message commit never queues a push.

Queueing is best-effort: membership, room/profile lookups and eight concurrent
recipient tasks share a three-second deadline. A Messaging/provider failure or timeout never
turns a persisted message into a failed send. Appwrite then owns asynchronous
delivery to every push target of the selected user. A deterministic Messaging
message ID per committed message/recipient makes dispatch replay idempotent;
an already-created notification is not queued again. There is no durable outbox
or automatic retry worker, and the existing post route does not deduplicate a
new client send after a lost response. Provider failures, revoked OS permission,
absent targets or deadline exhaustion can therefore leave a message without an
alert. Inspect Appwrite Messaging delivery status/errors when diagnosing
delivery; Function diagnostics use only a fixed message and numeric HTTP status
when Function logging is enabled.

Unit checks use injected REST/repositories and cover publication, membership,
privacy, disabled configuration, deduplication, deadline and failure behavior.
No APNs credentials, provider activation or real-device delivery were verified
as part of this implementation. Live provider/device verification is on demand.

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

The desktop's Account adapter owns signup, password login, email OTP,
session refresh and logout. It talks to native `/account`, `/account/sessions/*`,
`/account/tokens/*` and `/account/jwts` endpoints without a server key. Appwrite
therefore enforces its native Auth delivery, session and rate-limit policies.
Native session cookies remain private to the adapter, with synchronous Apple
Keychain persistence in the iOS/macOS app. Launch restores the credential and
obtains a fresh short-lived user JWT for authenticated function execution. When
a push target is registered, sign-out removes it before revoking the native
session. A failed target cleanup retains the credentials and offers a retry;
an expired session instead requires the original target owner to sign in again
to complete cleanup. Once target removal succeeds, local login credentials are
cleared even if native session revocation encounters a network failure. Backend
deployment and database availability do not participate in those Account
operations.

### Magic links and Sign in with Apple

Both use Appwrite's token flow and need no Function changes. The app opens
`/account/tokens/oauth2/apple` (Google follows the same route) in a private
browser session, `ASWebAuthenticationSession` on iOS and macOS; an emailed link
comes from `/account/tokens/magic-url`. Each returns `userId` and `secret` to
`appwrite-callback-<PROJECT_ID>://`, which the app exchanges at
`/account/sessions/token`. One-time console setup:

1. **Platforms:** add an Apple platform for each bundle identifier that runs the
   app (for example `org.orca.chat`). Appwrite accepts the
   `appwrite-callback-<PROJECT_ID>` scheme only for registered platforms. The
   scheme is listed in `apps/chat/package.lua` (`URLSchemes`) and the packagers
   add it to the bundle's `CFBundleURLTypes`.
2. **Magic URL:** enable *Auth → Settings → Magic URL*. Email clients may not
   open custom-scheme links, so publish [`backend/link/index.html`](../link/index.html)
   on https (for example as an Appwrite Site), add its host as a Web platform and
   set `CHAT_MAGIC_LINK_URL` to its address. The page forwards `userId` and
   `secret` to the app and removes them from browser history. Without the
   variable the email links straight to the app scheme.
3. **Apple:** in Apple Developer, create a Services ID with *Sign in with Apple*
   and the return URL Appwrite shows, plus a Sign in with Apple key. Enter the
   Services ID, key ID, team ID and `.p8` key under *Auth → Settings → Apple*.

4. **Google:** in Google Cloud, create an OAuth client of type *Web
   application* with the redirect URI Appwrite shows, then enter its client ID
   and secret under *Auth → Settings → Google*. Add an Android platform for each
   package name (for example `org.orca.chat`) as well.

The browser session shares no cookies with Safari, so each sign-in starts clean.
On Android the page opens in a Custom Tab and returns through the app's
`appwrite-callback-<PROJECT_ID>` intent filter.

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

## Blocking, reports and moderation

The backend persists user blocks in administrator-only `user-blocks`, with a
unique blocker/blocked pair. `set_chat_user_block` accepts
`{target_user,blocked}`; `list_chat_blocks` returns only the verified caller's
own blocked-person list. Neither RPC accepts a caller override. Blocks apply
in both directions to exact discovery, direct-room creation, direct text/photo
publication and direct-message edits. A shared pair lock serializes direct
publication with blocking. They also prevent adding a blocked person to a new
group/channel, using an invitation from a blocked inviter/room owner, joining
a blocked owner's public channel, and discovering their public channels.
Existing direct history and shared-group messages remain readable; blocking
does not remove the person from shared groups or hide all shared-group content.

`report_chat_content` accepts `{target_user,reason,details?,target_message?}`.
Reasons are `spam`, `harassment`, `sexual_content`, `violence`, `hate`, `other`;
details have a 2000-byte UTF-8 limit. A message report checks current room access
and verifies its author against the target user. A person report requires a
shared conversation or an existing public username. The private
`moderation-reports` record contains authenticated reporter, target, reason,
details, status, timestamp, and optional room/message IDs plus original message
text and ordered photo attachment IDs. Reports are limited to 10 per hour per
account. There is no public queue/list/resolve/takedown/suspension RPC. Neither
report content nor another user's block list enters client responses.

Server text filtering runs before creating/editing messages, changing public
profile display names/handles, and creating/renaming groups or channels. It
normalizes Unicode width/case and invisible formatting and matches a built-in
whole-word/phrase list for severe profanity, hateful slurs and threats. The
optional server `CHAT_BLOCKED_TERMS` array extends it. Legacy display names from
the shared public profile collection are filtered to `Member` when read. This
is a limited deterministic text filter: it is not a semantic classifier, cannot
catch every obfuscation/language, and does not classify photo pixels. Photo-only
messages remain supported. Photo/message reports and human review with actual
takedown/suspension are the image-abuse handling path; do not claim automated
image moderation or blanket quarantine.

`get_chat_rules_state` returns current rules version (`2026-11-08`) and the
caller's accepted version; `accept_chat_rules` records that exact version and
timestamp privately. Acceptance is an explicit client action, not inferred
from sign-in. Existing deployed clients are not silently gated on a new field.

### Manual moderator workflow

Use a private operator key with `documents.read/write`, `files.read/write` and
`users.read/write`. No new client or Function scopes are granted. Keep the key
in a private env file, never a command argument, source archive or repository.
From this directory, after setting the endpoint/project and operator key:

```sh
node --env-file=/private/tmp/chat-moderation.env src/moderation.js list
node --env-file=/private/tmp/chat-moderation.env src/moderation.js show REPORT_ID
node --env-file=/private/tmp/chat-moderation.env src/moderation.js photo REPORT_ID ATTACHMENT_ID /private/tmp/reported-photo.jpg
node --env-file=/private/tmp/chat-moderation.env src/moderation.js takedown REPORT_ID --confirm 'Reviewed prohibited photo'
node --env-file=/private/tmp/chat-moderation.env src/moderation.js suspend REPORT_ID --confirm 'Reviewed repeated abuse'
node --env-file=/private/tmp/chat-moderation.env src/moderation.js dismiss REPORT_ID --confirm 'Reviewed; no violation'
```

`list` emits open report summaries, and `show` emits private evidence for the
authorized reviewer; do not paste its output into public issues. `photo` exports
the decrypted full JPEG only when that attachment is listed in the selected
report and its uploader/message still match. Its explicit absolute output path
is created mode0600 without overwriting files. It exports no keys and exposes
no client endpoint. Inspect the image in a private local viewer, then remove
the temporary evidence file after reviewing it. Never put exported photos in
Git, shared folders or public issues. A takedown
unpublishes only the reported message, then deletes its associated encrypted
photos/thumbnails and metadata. Partial file cleanup is retryable from the
report's original attachment IDs. Suspension sets the reported shared Appwrite
account's status to false, stopping its authenticated Chat, Octopus and website
access. It is an operator decision, never an automatic consequence of receiving
a report. Resolution records action, reviewer notes and timestamp. Repeating the
same resolution is idempotent; changing an already-resolved decision fails.
Respond promptly to reports and check this queue during operations; there is no
background human moderator or external report-delivery service implied by this
tool. Reports involving either side of a deleted account are removed by the
account-deletion worker along with its blocking and acceptance records.

Run focused offline checks with
`node --test --test-name-pattern='safety' test/backend.test.js`. These exercise
server enforcement and the executable operator workflow using injected data.

## Account deletion

`POST /rest/v1/rpc/delete_chat_account` accepts only
`{"confirmation":"DELETE"}`. Runtime identity selects the caller; the request
cannot select another user. It creates a durable private `account-deletions` job
before blocking the shared Appwrite account and returns
`{"accepted":true,"deleted":false}`. The client clears its private session and
push-target stores after acceptance. This is a permanent deletion request, not
an immediate deletion-complete response. It affects Chat, Octopus and the shared
website account.

Before deploying this feature, run the shared schema migration to create the
administrator-only `account-deletions` collection and attachment room index.
Deploy the Function with both configured deletion-collection create/update
events and the **60-minute recovery interval** from the manifest. The interval
works on Appwrite Cloud Free; minute cron schedules require a paid plan. This
does not request a plan upgrade. A request is not a substitute for deploying the
worker settings. The first worker step blocks the account and waits at least
65 seconds so previously authenticated 60-second Chat invocations can finish.
Cleanup normally starts on the next hourly recovery sweep after this grace
period, and failures can delay completion further. The recovery interval then
resumes cleanup. Each successful checkpoint emits
one update event for the next bounded step. Missing/failed events and ordinary
upstream errors retain the job for recovery; no client connection is required.
A persisted sweep cursor rotates past failed jobs rather than starving later
accounts. Per-job locks serialize event/interval/duplicate deliveries. Worker
REST work has a 35-second total deadline, with independent short lock-release
signals reserving time to unwind before the 60-second Function timeout.

Cleanup deletes the caller's Chat messages/photos/uploads, invitations, private
identity state, discovery entry and public profile. Group/channel ownership
passes to an existing admin or another remaining member; their messages remain.
Empty rooms and their inaccessible history are reclaimed. Direct chats retain
the other participant's history under a deleted-account label. It also removes
authored Octopus posts and feed copies, comments, likes, reposts, follows, and
uploaded files whose Storage ACL establishes ownership, including abandoned
uploads. A post's child comments/reactions are removed with that post. A user
cannot authorize deletion of somebody else's file merely by putting its ID in
their editable profile or post. Auth and its sessions/identities/targets are
removed last, followed by the private job, so failed cleanup remains retryable.
Appwrite removes its associated identities and targets asynchronously in its own
background cleanup; blocking at acceptance does not claim immediate session or
push-target deletion. See [Appwrite identity cleanup](https://appwrite.io/docs/products/auth/identities#clean-up-identities).
Content already copied or saved outside the service cannot be recalled.

For Sign in with Apple, the request checks linked Appwrite identities before
accepting deletion. The job privately retains the provider refresh token (or
access-token fallback), and the worker calls Apple's `/auth/revoke` endpoint
before deleting Auth. It does not assume that Appwrite user deletion revokes the
Apple authorization. Missing client credentials or provider tokens reject the
request without blocking the account; failed revocation leaves the durable job
pending. Configure and maintain the two Apple variables above before enabling
this feature for Apple-linked accounts. Never log job payloads or include them
in client responses. See [Appwrite identity fields](https://appwrite.io/docs/references/cloud/models/identity),
[Apple token revocation](https://developer.apple.com/documentation/signinwithapplerestapi/revoke-tokens),
and [Appwrite interval scheduling](https://appwrite.io/changelog/entry/2026-10-15-1).

The focused offline cases are
`node --test test/account-deletion.test.js`. They use injected REST data and do
not delete real accounts. As with other room mutations, a worker killed while
holding an existing non-expiring room lock needs operator inspection; never
steal a live room lock or log the job's identity/token contents. Apple-linked
deletion and live event/interval delivery require separate explicitly requested
live verification after deployment.

## Rooms, messages and encrypted photos

All existing direct, group and channel operations are implemented: canonical
direct pairs, owner/admin/member/subscriber roles, member removal, private
invitations, public channel discovery/join, recent message history, editing and
deleting messages, non-owner leaving, owner renaming, and up to nine JPEG
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
History performs no per-message profile or directory reads. New-message push is
available with the APNs setup above. Realtime subscriptions, receipts and
end-to-end encryption remain outside the existing Chat feature set.

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
