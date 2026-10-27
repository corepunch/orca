# Live Chat backend tests

Manual end-to-end tests for the **deployed** Chat backend: native Appwrite
Account sessions plus the `orca-chat-api` Function, using real test accounts.
They are deliberately **not** part of `make test`, `make test-chat` or CI. Run
them yourself after a deployment or from time to time.

The offline suites (`make test-chat`, `make test-chat-appwrite-backend`) only
use mocks and injected repositories. These tests catch what the mocks can't:
a stale deployment, missing indexes or scopes, Appwrite permission changes,
broken discovery, and similar production problems.

```sh
make test-live-setup     # once: create/reconnect the four test accounts
make test-live           # run every suite against Octopus (several minutes)
CHAT_LIVE_ONLY=discovery,direct make test-live   # selected suites
make test-live-clean     # optional: delete the test accounts' conversations (needs APPWRITE_API_KEY)
make test-live-emulated  # offline: check the suite itself against an in-memory emulator
```

Requires Node.js 22+. There are no npm dependencies.

## Test accounts

`make test-live-setup` uses native Appwrite signup, with no server key, to create
four persistent accounts. Signup creates them; later runs sign in again:

| Account | Email | Username |
| --- | --- | --- |
| alice | `orca_live.alice@example.com` | `@orca_live_alice` |
| bob | `orca_live.bob@example.com` | `@orca_live_bob` |
| carol | `orca_live.carol@example.com` | `@orca_live_carol` |
| dave | `orca_live.dave@example.com` | `@orca_live_dave` |

All four are friends with each other. They find each other by username and share
direct chats, groups and channels. The generated password, account IDs and
session cookies are saved in `apps/chat/backend/.live-accounts.json` (mode
0600). It is outside the Function directory, so a deployment can't include it,
and Git ignores it because of the repository's `.*` rule. Never commit it. To share the
accounts with a teammate, pass them that file privately, or set
`CHAT_LIVE_PASSWORD` before the first setup. To use different accounts, edit the
file: `password`, and `accounts.<name>.email`/`username`/`display_name`, or a
per-account `password`.

Saved sessions are reused, so a normal run makes no sign-in requests. Appwrite
allows about ten email sign-ins per account per hour, and setup or an expired
session costs one. A run creates one extra session for Carol, then signs it out
to test sign-out.

Each run restores the profiles: usernames, display names and discovery
switches. This repairs any changes left by an interrupted run.

## Configuration

| Variable | Default | Purpose |
| --- | --- | --- |
| `CHAT_LIVE_ENDPOINT` / `APPWRITE_ENDPOINT` | `https://fra.cloud.appwrite.io/v1` | Appwrite endpoint |
| `CHAT_LIVE_PROJECT_ID` / `APPWRITE_PROJECT_ID` | `69f1c06800389dc6a1a0` (Octopus) | Project |
| `CHAT_LIVE_FUNCTION_ID` / `APPWRITE_CHAT_FUNCTION_ID` | `orca-chat-api` | Chat Function |
| `CHAT_LIVE_PREFIX` | `orca_live` | Account email/username prefix (use another for a second set) |
| `CHAT_LIVE_EMAIL_DOMAIN` | `example.com` | Test email domain (nothing is delivered) |
| `CHAT_LIVE_ACCOUNTS` | `apps/chat/backend/.live-accounts.json` | Credentials file |
| `CHAT_LIVE_ONLY` | all | Comma-separated suites: `auth,profiles,discovery,direct,groups,channels,invites,messages,photos,security,parity` |
| `APPWRITE_API_KEY` | none | Optional server key, see below |
| `CHAT_LIVE_KEEP=1` | off | Keep this run's conversations for inspection |

### Optional server key

Without a key, everything except one discovery case runs. Conversations created
by a run stay in the test accounts. A run prints how many it left; most are
private groups whose names start with `live <run-id>`. Public channel tests reuse
one persistent `orca_live public channel` so public search doesn't fill with
test channels.

With `APPWRITE_API_KEY`, which needs `documents.read`, `documents.write`,
`files.write`, `users.read` and `users.write`, the suite:

- deletes the conversations it created, with their messages, invitations,
  memberships and encrypted photo objects;
- temporarily marks Dave's email as verified to test email discovery,
  opt-in and opt-out, then restores it;
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
| `discovery` | **Finding a friend:** exact username with `@`, uppercase or spaces, server-side normalization, every account finds every other, self and unknown searches, unverified email doesn't leak, renaming and clearing usernames, the full *find → direct chat → message → reply → contact* flow, `peer_id` for inbox photos; with a key, email discovery and opt-out |
| `direct` | Canonical pair in either order, single inbox row, two-way timeline, contacts without identifiers, invalid directs, no administration, outsiders locked out |
| `groups` | Owner and roles, inbox role, everyone posts, empty groups, name and member limits, unknown members leave nothing behind, admin and owner powers, protected staff, removed members lose access |
| `channels` | Subscribers are read-only, posts show the channel as sender, subscribers see only staff, promote/demote, channel-only roles, the public channel can be found case-insensitively and joined, private rooms aren't discoverable |
| `invites` | Code format and expiry, joining sees history, rejoining is harmless, staff-only creation, revocation, cross-room revocation, malformed codes, a removed member can rejoin, channel invites |
| `messages` | Message shape, exact Unicode/RTL/NUL/markup round-trip, 4000-byte boundary, empty text, the sender is always the caller, burst ordering, `limit` window, rooms don't leak into each other, bad room IDs |
| `photos` | Reserve → upload → finalize, key never returned, exact photo and thumbnail bytes for members, ciphertext only in the public bucket, four-photo and photo-only messages, privacy before publishing, no stolen, reused or cross-room photos, discard, immutable and mismatched objects, non-JPEG/GPS rejected, limits, channel and outsider rules, revocation |
| `security` | Chat collections are closed to clients, fixed route allowlist, malformed envelopes, no internals in errors, ID validation, discovery rate limit (runs last) |
| `parity` | Telegram parity checklist, without video or calls (see below) |

### Telegram parity checklist

`suites/parity.js` lists the Telegram features Chat doesn't have yet: editing,
deleting, replies, forwards, reactions, pins, mentions, formatting, unread
counts and read receipts, typing and last seen, inbox previews and ordering,
Saved Messages, leaving a group, renaming, transferring ownership, bans, slow
mode, polls, people search by name or prefix, blocking, files, voice messages
and more. Each item is a node:test `todo` that tries a **proposed** contract, such
as `edit_chat_message`. The report shows these items as `# TODO`. They don't
fail the run. When a feature ships, update its probe to the real contract and
remove `todo`, so it becomes a regular regression test.

## Running notes

- Tests run one after another, and each Function call takes a fraction of a
  second to a few seconds. The client paces username searches under the
  server's limit of 20 per minute. A full run takes several minutes, mostly
  waiting for that limit.
- Appwrite `429` responses are retried after their reset time. `409 busy`
  room-lock responses are retried a few times. A lock that stays busy needs the
  recovery steps in the [backend guide](../README.md#serialization-failure-recovery-and-maintenance).
- A run adds a few messages to the persistent direct chats (alice↔bob,
  carol↔dave, dave↔bob) and to the public test channel.
- `make test-live-emulated` runs the same suites against `emulator.js`, an
  in-memory stand-in for Account, Users, Databases, Storage and executions. It
  runs the real `src/main.js`. It checks the test code and the Function source
  together, but says nothing about the deployed Function.
