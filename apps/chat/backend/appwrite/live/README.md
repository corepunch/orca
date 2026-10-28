# Live Chat backend tests

Routine Appwrite validation uses **only focused unit tests**:
`make test-chat-appwrite` (standalone Lua adapter) or
`make test-chat-appwrite-backend` (Function logic with injected dependencies).
Integration is trusted for routine changes. Do not run this directory's suites,
including `test-live-emulated`, unless integration verification is explicitly
requested. UI-only changes do not require Appwrite unit tests either.

Manual end-to-end tests for the **deployed** Chat backend: native Appwrite
Account sessions plus the `orca-chat-api` Function, using real test accounts.
They are deliberately **not** part of `make test`, `make test-chat` or CI.
Run them **only when the user explicitly requests live verification**. Agents
must not start them automatically after edits, offline checks, builds, or
deployments, including UI-only message and photo changes. Setup, cleanup,
native live checks, and two-account device checks follow the same policy.

For routine Chat presentation changes, use `make test-chat-ui` (offline,
existing binary) or run only the affected test file. Rebuild separately when
native code changes or the binary is missing. Full offline and live suites
are not required after every small UI edit.

The offline suites (`make test-chat`, `make test-chat-appwrite-backend`) only
use mocks and injected repositories. These tests catch what the mocks can't:
a stale deployment, missing indexes or scopes, Appwrite permission changes,
broken discovery, and similar production problems.

```sh
make test-live-setup     # once: create/reconnect the four test accounts
make test-live           # supported features against Octopus (several minutes)
CHAT_LIVE_ONLY=discovery,direct make test-live   # selected suites
CHAT_LIVE=1 make test-chat-live # real Lua/native HTTP and Keychain (macOS)
make test-live-clean     # optional: delete the test accounts' conversations (needs APPWRITE_API_KEY)
make test-live-emulated  # offline: check the suite itself against an in-memory emulator
```

Requires Node.js 22+. There are no npm dependencies. If Node is outside `PATH`,
pass `NODE=/absolute/path/to/node` to Make (including for the bundled Codex
runtime located by `load_workspace_dependencies`).

## Test accounts

`make test-live-setup` uses native Appwrite signup, with no server key, to create
four persistent accounts. Signup creates them; later runs sign in again:

| Account | Email | Username |
| --- | --- | --- |
| alice | `orca_live.alice@example.com` | `@orca_live_alice` |
| bob | `orca_live.bob@example.com` | `@orca_live_bob` |
| carol | `orca_live.carol@example.com` | `@orca_live_carol` |
| dave | `orca_live.dave@example.com` | `@orca_live_dave` |

All four can find each other by username and share
direct chats, groups and channels. The generated password, account IDs and
session cookies are saved in `apps/chat/backend/.live-accounts.json` (mode
0600). It is outside the Function directory, so a deployment can't include it,
and Git ignores it because of the repository's `.*` rule. Never commit it. To share the
accounts with a teammate, pass them that file privately, or set
`CHAT_LIVE_PASSWORD` before the first setup. To use different accounts, edit the
file: `password`, and `accounts.<name>.email`/`username`/`display_name`, or a
per-account `password`.

On 2026-10-27 these accounts were created through the Appwrite plugin with IDs
`orca-live-alice`, `orca-live-bob`, `orca-live-carol`, and `orca-live-dave`.
Setup reuses them. The plugin marked Dave's test email verified so email
discovery can be tested through ordinary user sessions. Keep the other three
emails unverified for privacy assertions. This admin-provisioned fixture does
not test receiving or confirming an email code. For a fresh project, use the
plugin's `users_create` and `users_update_email_verification` on these dedicated
accounts, save generated passwords privately, then run setup.

Contacts are shared-room relationships; these tests do not imply a separate
friend request/acceptance feature.

Saved sessions are reused, so a normal run makes no sign-in requests. Appwrite
allows about ten email sign-ins per account per hour, and setup or an expired
session costs one. A run creates one extra session for Carol, then signs it out
to test sign-out.

Each run restores the profiles: usernames, display names and discovery
switches. This repairs any changes left by an interrupted run.

Run Node and native suites sequentially with these shared fixtures. Parallel
runs can contend on per-user locks (`409 busy`), change usernames during another
run's discovery checks, or exhaust the shared lookup allowance. Use a separate
account prefix/file for independent concurrent runs.

## Configuration

| Variable | Default | Purpose |
| --- | --- | --- |
| `CHAT_LIVE_ENDPOINT` / `APPWRITE_ENDPOINT` | `https://fra.cloud.appwrite.io/v1` | Appwrite endpoint |
| `CHAT_LIVE_PROJECT_ID` / `APPWRITE_PROJECT_ID` | `69f1c06800389dc6a1a0` (Octopus) | Project |
| `CHAT_LIVE_FUNCTION_ID` / `APPWRITE_CHAT_FUNCTION_ID` | `orca-chat-api` | Chat Function |
| `CHAT_LIVE_PREFIX` | `orca_live` | Account email/username prefix (use another for a second set) |
| `CHAT_LIVE_EMAIL_DOMAIN` | `example.com` | Test email domain (nothing is delivered) |
| `CHAT_LIVE_ACCOUNTS` | `apps/chat/backend/.live-accounts.json` | Credentials file |
| `CHAT_LIVE_ONLY` | all supported (excluding `parity`) | Comma-separated suites: `auth,profiles,discovery,direct,groups,channels,invites,messages,photos,security,parity` |
| `APPWRITE_API_KEY` | none | Optional server key, see below |
| `CHAT_LIVE_KEEP=1` | off | Keep this run's conversations for inspection |

### Optional server key

Normal live runs use only the test accounts, with no server key. Email discovery
runs when Dave's fixture is verified and otherwise reports a skip with setup
instructions. Conversations created
by a run stay in the test accounts. A run prints how many it left; most are
private groups whose names start with `live <run-id>`. Public channel tests reuse
one persistent `orca_live public channel` so public search doesn't fill with
test channels.

With `APPWRITE_API_KEY`, which needs `documents.read`, `documents.write`,
`files.write`, `users.read` and `users.write`, the suite:

- deletes the conversations it created, with their messages, invitations,
  memberships and encrypted photo objects;
- lets `make test-live-clean` remove every conversation created by the test
  accounts. It keeps conversations created by real people that only include a
  test account.

Use a dedicated, short-lived key. Keep it in your shell environment, never in
the repository or the app.

## What is covered

| Suite | Checks |
| --- | --- |
| `auth` | Saved sessions, JWT issue and rotation, wrong password, session list, sign-out of one device, anonymous and forged executions, the caller can't be overridden from JSON |
| `profiles` | Profile shape, unverified email hidden, Unicode names and limits, username format, uniqueness, clearing and reuse, identifiers can't be edited, discovery switches need verification, linking reports missing providers clearly, public profile photo lookup |
| `discovery` | **Finding a friend:** exact username with `@`, uppercase or spaces, server-side normalization, every account finds every other, self and unknown searches, unverified email doesn't leak, renaming and clearing usernames, the full *find → direct chat → message → reply → contact* flow, `peer_id` for inbox photos; verified Dave fixture email discovery and opt-out |
| `direct` | Canonical pair in either order, single inbox row, two-way timeline, contacts without identifiers, invalid directs, no administration, outsiders locked out |
| `groups` | Owner and roles, inbox role, everyone posts, empty groups, name and member limits, unknown members leave nothing behind, admin and owner powers, protected staff, removed members lose access; group/channel leave revokes access, invitation rejoin resets privileges, owner rename persists for every member, directs reject both operations |
| `channels` | Subscribers are read-only, posts show the channel as sender, subscribers see only staff, promote/demote, channel-only roles, the public channel can be found case-insensitively and joined, private rooms aren't discoverable |
| `invites` | Code format and expiry, joining sees history, rejoining is harmless, staff-only creation, revocation, cross-room revocation, malformed codes, a removed member can rejoin, channel invites |
| `messages` | Message shape, exact Unicode/RTL/NUL/markup round-trip, 4000-byte boundary, empty text, the sender is always the caller, burst ordering, `limit` window, rooms don't leak into each other, bad room IDs; **editing** (author only, `edited_at`, limits) and **deleting** (author, owner/admin moderation, none in direct chats, removed members, channels) |
| `photos` | Reserve → upload → finalize, key never returned, exact photo and thumbnail bytes for members, ciphertext only in the public bucket, four-photo and photo-only messages, privacy before publishing, no stolen, reused or cross-room photos, discard, immutable and mismatched objects, non-JPEG/GPS rejected, limits, channel and outsider rules, revocation, removing a caption by editing, deleting a message deletes its photos |
| `security` | Chat collections are closed to clients, fixed route allowlist, malformed envelopes, no internals in errors, ID validation, discovery rate limit (runs last) |
| `parity` | Optional deferred-feature probes; excluded from the default acceptance run (see below) |

### Native Lua client

`CHAT_LIVE=1 make test-chat-live` runs `tests/test_chat_live.lua` in ORCA with
the real `api.orca_transport`, native Account HTTP and isolated actual Keychain
entries. It signs in Alice/Bob/Carol with fresh sessions, restores/renews Alice,
discovers Bob, opens the canonical direct, exchanges Unicode text, edits/deletes
and rejects outsider access. It prepares a real photo through
`orca.media.processPhoto`, compares downloaded JPEG/thumbnail bytes, and checks
that deleting its message revokes access.

Without opt-in it reports a skip. With opt-in, absent secure storage, network,
media support, credentials or routes are failures. macOS needs Keychain and
network access; restricted agent sandboxes may require normal process approval.
The timeout defaults to 600 seconds (`CHAT_LIVE_TIMEOUT_MS`, up to 900000).
It removes its messages/photos and revokes its own sessions, keeping Node's
saved sessions and the interactive app's Keychain entry intact. The canonical
direct/contact relation remains. A timeout may leave interrupted-run artifacts.
When device verification is explicitly requested, use a two-account/device UI
pass for touchscreen layout and actual iOS process relaunch; the native suite
does not cover those.

### Telegram parity checklist

`suites/parity.js` lists the Telegram features Chat doesn't have yet: replies, forwards, reactions, pins, mentions, formatting, unread
counts and read receipts, typing and last seen, inbox previews and ordering,
Saved Messages, group descriptions, transferring ownership, bans, slow
mode, polls, people search by name or prefix, blocking, files, voice messages
and more. Each item is a node:test `todo` that tries a **proposed** contract, such
as `react_chat_message`. The report shows these items as `# TODO`. They don't
fail the run. When a feature ships, update its probe to the real contract and
remove `todo`, so it becomes a regular regression test.

Run these exploratory probes only with `CHAT_LIVE_ONLY=parity make test-live`.
They make real requests and may create test data. Passing the required suite
does not claim Telegram parity. Reactions/emoji, stickers/GIFs and larger work
remain deferred in [the scoped roadmap](../../../PRODUCT.md#current-scope-and-deferred-work);
videos and calls are excluded.

Gaps that still need work, such as delivery providers, a staging project,
device interaction and limits that need time or more accounts, are listed in
the repository [TODO](../../../../../TODO.md#chat-complete-live-backend-testing).

## Running notes

### Latest real backend run — 2026-10-27

Octopus (`69f1c06800389dc6a1a0`), Frankfurt, active deployment
`6ae08dfcb0d301fbdd7b`: **123/123 passed**, 11 suites, zero failures,
skips or TODOs, 877.934 seconds (14m38s). This ran the supported suites with
ordinary Alice/Bob/Carol/Dave sessions and no server key, including verified
email discovery, group/channel leave and rename, message edit/delete and
encrypted photos. It retained 28 test conversations (`live mvqmch2j …`).
An initial run against the previous deployment passed 29 checks and caught nine
missing edit/delete routes; those failures disappeared after the update.

The **actual Lua/native client also passed all 33 checks**, exit 0, against this
deployment, including password sessions, real Keychain restore, JWT renewal,
discovery/direct contacts, text/edit/delete, native JPEG preparation, exact
photo/thumbnail bytes and access revocation. It cleaned its own messages,
photo reservations, sessions and Keychain entries. Device UI/relaunch and OTP
delivery are still unverified.

An initial native 180-second timeout was too short, and a parallel retry hit
`409 busy` from shared-account lock contention. The final sequential run passed
without it. The timeout is now ten minutes; run Node/native sequentially. The
interrupted timeout may have left test messages, sessions and isolated Keychain
entries; later runs intentionally clean only artifacts created by that run.

### Execution and retained data

- Tests run one after another, and each Function call takes a fraction of a
  second to a few seconds (occasionally longer). The client paces username
  searches under the server's limit of 20 per minute. A full run takes many
  minutes because of service roundtrips and intentional lookup pacing.
- Appwrite `429` responses are retried after their reset time. `409 busy`
  room-lock responses are retried a few times. A lock that stays busy needs the
  recovery steps in the [backend guide](../README.md#serialization-failure-recovery-and-maintenance).
- A run adds a few messages to the persistent direct chats (alice↔bob,
  carol↔dave, dave↔bob) and to the public test channel.
- `make test-live-emulated` runs the same suites against `emulator.js`, an
  in-memory stand-in for Account, Users, Databases, Storage and executions. It
  runs the real `src/main.js`. It checks the test code and the Function source
  together, but says nothing about the deployed Function.
