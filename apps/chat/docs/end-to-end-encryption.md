# End-to-end encryption: design note

Status: **paused** (decision, 2026-11-14). Orca Messenger is a small app, so it
keeps server-side storage and says plainly in its privacy policy that messages
are not end-to-end encrypted. Slices 1 and 2 shipped in 1.1 (77): `orca.crypto`
and device keys, which the app publishes after sign-in but nothing uses yet.
Resume from slice 3 if privacy becomes a selling point. Until then, never
describe the app as end-to-end encrypted or as secure messaging.

## Goal and threat model

Make the content of **direct chats and private groups** unreadable to the
Appwrite project, the Chat Function and anyone with database or bucket access.
Only participants' devices hold the keys.

Protected: message text, edits, photo and thumbnail bytes, photo dimensions
and captions.

Not protected (the server still needs or sees these): who talks to whom, room
membership and roles, room names, timestamps, message and photo sizes, push
routing, profiles and avatars. Hiding this metadata is out of scope.

Not covered:

- **Public channels.** Anyone can join, so the server can join too and E2E
  would add nothing. They stay plaintext. Private channels can use the group
  scheme later.
- **A compromised device**, or someone holding an unlocked phone.
- **A malicious server substituting keys** is detected only when users compare
  safety numbers (see [Verification](#key-verification)). Until then the
  model is trust-on-first-use.

## Why Appwrite's own encryption is not enough

Appwrite has no end-to-end encryption. Its encryption features all run on the
server, which holds the keys:

- **Encrypted string attributes** (available on paid Cloud plans) and
  **bucket encryption** protect data at rest, for example against a leaked
  disk or backup. Appwrite decrypts on every read, so the Console, server API
  keys and Functions still see plaintext.
- **TLS** protects data in transit only.

Chat's photo encryption already works this way (AES-GCM in the Function), and
the project administrator can still read everything. The only way to keep
administrators out is to encrypt on the device, with keys that never reach
Appwrite. Appwrite then only stores and routes ciphertext.

## Export compliance

Orca Messenger is not sold in France (removed 2026-11-14), so its Info.plist
keeps `ITSAppUsesNonExemptEncryption=false`. Monocypher is third-party,
standard cryptography. App Store Connect accepts an encryption declaration
only for proprietary cryptography, or for third-party cryptography sold in
France, and rejects uploads that set the key to `true` without an approved
declaration's `ITSEncryptionExportComplianceCode` (error 90592). Returning
to France requires a French encryption declaration document, an approved App
Store Connect declaration, and both Info.plist keys.

## Library: Monocypher

Vendor [Monocypher](https://monocypher.org) 4.x as `libs/monocypher`
(`monocypher.c`/`.h`, plus the optional `monocypher-ed25519` pair). It is
dual-licensed CC0-1.0 / BSD-2-Clause, so add it to `THIRD_PARTY_NOTICES.md`.
It needs no build system and no platform code, which fits the other `libs/`
entries and the iOS/macOS Xcode projects.

| Need | Monocypher primitive |
| --- | --- |
| Device key agreement | X25519 (`crypto_x25519`) |
| Symmetric encryption | XChaCha20-Poly1305 (`crypto_aead_lock` / `unlock`) |
| Signatures | Ed25519 (`crypto_ed25519_*`) |
| Key derivation, fingerprints | BLAKE2b (`crypto_blake2b_keyed`) |

Monocypher has no random number generator. The native wrapper uses
`SecRandomCopyBytes` (or `arc4random_buf`) on Apple platforms and `getrandom`
elsewhere, and fails closed if they are unavailable.

libsodium would work equally well. It is larger, and it has its own build
and per-platform configuration. libsignal (Rust, AGPL) and MLS are heavier
than this release needs; the envelope `v` field below leaves room to adopt
one of them later.

### Native module `orca.crypto`

Register `luaopen_orca_crypto` in `source/orca/orcalib.c`, next to
`orca.credentials`. Keep the Lua surface small and byte-string based, so that
protocol logic stays in Lua and can be tested in standalone Lua:

```lua
local crypto = require "orca.crypto"
crypto.random(n)                              -- n random bytes
crypto.boxKeypair()                           -- x25519 secret, public
crypto.signKeypair()                          -- ed25519 secret, public
crypto.sharedKey(secret, peerPublic, context) -- BLAKE2b(X25519 || both publics || context)
crypto.seal(key, plaintext, ad)               -- nonce(24) || ciphertext || mac(16)
crypto.open(key, sealed, ad)                  -- plaintext, or nil and an error
crypto.sign(secret, msg)                      -- 64-byte Ed25519 signature
crypto.verify(public, msg, sig)               -- boolean
crypto.boxPublicKey(secret)                   -- derive the x25519 public key
crypto.hash(bytes [, size])                   -- BLAKE2b, 32 bytes by default
crypto.equal(a, b)                            -- constant-time comparison
```

Secret keys do not stay in Lua longer than needed. Device secret keys live in
the Keychain through `source/credentials/credentials.c`, under their own
service key (`chat-device:<endpoint>:<project>:<userId>`), with
`kSecAttrAccessibleWhenUnlockedThisDeviceOnly` (the current `orca.credentials`
setting) so they are never synced to iCloud or restored to another device.
That is enough while push notifications carry no text; decrypting in a
Notification Service Extension would need `AfterFirstUnlock`.

## Keys

Each **device** (one app install for one account) generates:

- an X25519 key pair for encryption, and
- an Ed25519 key pair for signing.

There is no shared account-level secret key in v1, because copying one
between devices needs a device-linking flow this release does not have. A
user's identity is the set of their active device keys.

A new private collection, `device-keys`, holds one row per device:

| Field | Meaning |
| --- | --- |
| `user_id` | Owner (the Auth user, set by the Function, never by the client) |
| `device_id` | Random 16-byte id, base64url |
| `box_public`, `sign_public` | Public keys |
| `self_signature` | `sign_public` signs `"orca-chat/device/v1" NUL user_id NUL device_id box_public`, binding the key to its owner |
| `created_at`, `revoked_at` | Lifecycle |

New Function routes:

| Route | Rule |
| --- | --- |
| `publishDeviceKey(device)` | Caller publishes only for themselves, with a valid self-signature; at most 10 active devices and 50 records |
| `listDeviceKeys(userIds)` | Yourself and people you share a direct chat or group with; others are omitted. Up to 50 ids. Revoked devices stay listed |
| `revokeDevice(deviceId)` | Owner only; sign-out and account deletion revoke automatically |

Signing out deletes the device's secret keys from the Keychain and revokes
the device. Signing back in creates a new device, which cannot read earlier
history (see [History](#history-and-new-devices)).

## Message envelope

For an E2E room, `sendMessage` and `editMessage` send an envelope instead of
`body`:

```json
{
  "v": 1,
  "sender_device": "<device_id>",
  "ct": "<base64 seal(K, content, ad)>",
  "keys": { "<recipient device_id>": "<base64 seal(wrapKey, K, ad)>" },
  "sig": "<base64 ed25519 over v|room_id|sender_device|ct|keys>"
}
```

- `K` is a fresh 32-byte content key for each message and each edit.
- `content` is JSON: `{body, photos:[{id, key, width, height, thumb_width, thumb_height}]}`.
- `ad` = `"orca-chat/v1" | room_id | sender_user | sender_device`. It binds the
  ciphertext to its room and sender, so the server cannot move it between
  rooms or reattribute it.
- `wrapKey` = `crypto.sharedKey(senderBoxSecret, recipientBoxPublic, ad)`.
- The recipient list is every active device of every member, **including the
  sender's other devices** so they show sent messages too.
- The signature lets recipients reject envelopes whose sender device does not
  belong to the claimed sender (the server checks `sender_user` itself).

Recipients find their `device_id` in `keys`, unwrap `K`, verify the signature
against the sender's published `sign_public`, and open `ct`. A failure shows a
"This message can't be decrypted" placeholder; it is never shown as text.

### Size limits

The `messages.payload` attribute is 16 384 characters. A 4000-byte body
becomes about 5.4 KB of base64, and each device entry is about 140 bytes. A
direct chat with a few devices per person fits easily. A 50-member group with
two devices each would need about 14 KB of keys and would not fit.

So v1 enables E2E for **direct chats and groups up to 20 members**. Larger
groups need sender keys (see [Groups](#groups-v2-sender-keys)).

The server can no longer measure the 4000-byte text limit. It enforces a
ciphertext size limit instead (5500 base64 characters for `ct`), and clients
enforce 4000 bytes before encrypting.

## Photos

The client:

1. generates a random photo key and seals the JPEG and the thumbnail with it
   (each object is at most 1 MB, so one-shot sealing is fine);
2. uploads the two ciphertexts through the existing reservation and
   `finalize-chat-photo` flow;
3. puts the photo key and dimensions in the message `content`, never in
   attachment metadata.

What changes on the server:

- `finalize-chat-photo` can no longer check JPEG structure, dimensions or
  location metadata. It checks only ownership, room membership, status and
  size (1 000 016 bytes plus the nonce).
- The current server-side AES-GCM layer can stay, since the public
  `post-images` bucket still needs it to keep object keys from being
  guessable. Downloads return the client ciphertext after the existing
  membership checks.
- Recipients validate decrypted bytes with the native media adapter before
  display and reject anything that is not a well-formed JPEG within the
  limits. A malicious sender can now send invalid bytes, so this check must
  happen on the device.

## Server features that change

| Feature | Today | With E2E |
| --- | --- | --- |
| Push notifications | Generic body; payload has only conversation, message and user ids | **No change.** The payload never contains text, so no Notification Service Extension is needed |
| Moderation reports | Server attaches authorized message context and photo references | The reporter's client attaches the decrypted text, photos and `K` for the reported message. The server can verify the reporter's copy against the stored ciphertext with `K`, which proves the ciphertext and the reported plaintext match |
| Photo validation | Function checks JPEG structure | Size and ownership only; JPEG validation on recipients |
| Message text limit | Server counts bytes | Server limits ciphertext size; client enforces bytes |
| Message search (deferred) | Would run on the server | Must be on-device only |
| Edit and delete | Server rules | Unchanged; an edit is a new envelope with a new `K` |
| Channel/room name search | Plaintext names | Unchanged; names remain metadata |

Including `K` in a report reveals only that one message, because each message
has its own key. This is a lightweight form of message franking.

## Key verification

Each direct chat gets a **safety number**: 60 digits derived from
`BLAKE2b(sorted(userA device sign keys) || sorted(userB device sign keys))`,
shown in the conversation's profile view (and later as a QR code). Two people
compare it in person or over another channel.

When a participant's device set changes, the timeline shows a system notice
("Alice's security code changed"). Until people verify, a new device is
accepted (trust on first use). Marking a contact verified and warning before
sending to an unverified new device can come later.

## History and new devices

- A new device, including signing in again, cannot read messages from before
  it existed. The timeline shows one "Earlier messages aren't available on
  this device" notice instead of individual placeholders.
- Someone who joins a group by invitation sees only messages sent after they
  join. This already matches the product rule that leaving revokes history.
- Encrypted backup and device-to-device history transfer are later work.

## Groups (v2): sender keys

For groups over 20 members, and to keep envelopes small:

- each member's device generates a random **sender key** for the group and
  sends it to every other member device with the pairwise scheme above, as a
  hidden key-distribution message;
- that device's messages are then sealed once with its sender key, with no
  `keys` map; the `sig` still identifies the sender device;
- the sender key is rotated whenever anyone leaves or is removed, so former
  members cannot read new messages, and whenever a new device appears.

This is the same structure Signal uses for groups, without the ratchet.
MLS (OpenMLS) would be the replacement if groups grow far beyond the
current 50-member limit.

## Rollout and compatibility

- **Decision (2026-11-14): encryption is on by default, with no per-chat
  switch.** Every new direct chat and private group of up to 20 members is
  end-to-end encrypted. Public channels are never encrypted. Larger groups
  stay plaintext until sender keys land. An opt-in mode was rejected because
  most chats would stay readable, and opting in would mark the sensitive ones.
- Rooms get an `e2e` boolean, fixed at creation and set by these rules, not
  by the user. If a participant has no published device key yet, the chat is
  still created as E2E, and the client asks that person to update before
  they can read or post.
- Existing rooms and their plaintext history are not converted. An existing
  direct chat can be upgraded by both participants; the timeline marks the
  point where encryption started.
- The Function rejects plaintext `body` in an `e2e` room, and envelopes in a
  plaintext room.
- Older clients that receive an envelope show "Update Chat to read this
  message"; they cannot post in E2E rooms.

## Limits of v1

- **No forward secrecy.** Device keys are long-lived, so someone who steals a
  device's X25519 secret can read every message ever sent to that device. The
  mitigation is to rotate device keys (for example every 30 days) and delete
  old secrets. A ratchet (Double Ratchet or MLS) fixes this properly and would
  be envelope `v: 2`.
- **Key compromise impersonation.** With static-static X25519, someone holding
  a recipient's secret can forge messages *to* that recipient. The Ed25519
  signature over the envelope prevents this.
- Metadata stays visible, as described above.

## Implementation slices

1. Vendor Monocypher, add the `orca.crypto` native module, and add C tests
   against Monocypher's test vectors.
2. Device keys: Keychain storage, `device-keys` collection, the three Function
   routes, revocation on sign-out and account deletion, and the offline mock.
3. Envelope encode/decode in pure Lua with standalone tests: round trip,
   wrong room, wrong sender, tampered `ct`/`keys`/`sig`, missing device.
4. E2E direct chats: send, edit, timeline decryption and placeholders.
5. Encrypted photos, plus recipient-side JPEG validation.
6. Safety numbers and security-code-changed notices.
7. Moderation reports with the reported message's key.
8. Groups up to 20 members; then sender keys (v2).

Slices 1–3 need no schema change to existing collections and can land before
any user-visible behavior. Unit tests use the offline mock; live Appwrite
checks only on request, as for the rest of Chat.

## Open questions

- How many devices per account should v1 allow?
- Are room names for private groups sensitive enough to encrypt too? They are
  currently used by owners in renaming and could be moved into an encrypted
  room header.
- Is losing history on sign-out acceptable for v1, or is encrypted backup
  needed before launch?
