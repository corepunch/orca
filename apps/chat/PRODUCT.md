# Messenger: identity, discovery, conversations, and media

Implementation status, 2026-10-25. The app now implements phone/email OTP,
verified linking, usernames and opt-in exact discovery, direct/group/channel
conversations, roles/invitations/public channel subscriptions, and JPEG photo
messages through the shared client and offline mock. Native processing and
selection are available on macOS. Video, QR/profile sharing links, address-book
matching, block/report UI, push, and live subscriptions remain later work.

The [API contract](API.md) and [backend setup](backend/README.md) describe the
implemented interfaces. Appwrite's Octopus project is the default backend; its
seven private Chat collections and indexes are provisioned. Native Account signup,
password sign-in, JWT renewal and logout were verified against Octopus with a
disposable account, which was then deleted. These operations do not require a
Chat Function. The authenticated data Function is deployed in Octopus with its
server verification secret configured. A two-account live check verified
profiles/discovery, direct/group/channel operations, invitations, text/photo
exchange and permission revocation; all test resources were removed afterward.
Messaging providers are not configured, so verified identity linking still needs
delivery setup.
The sections below retain the design rationale and distinguish the remaining roadmap from current behavior.

## Identity and login

Support phone and email as verified ways to access one account. Use phone OTP
as the main mobile entry and offer email OTP as an equal alternative; require
at least one verified identifier, not both. Keep the current password flow
during migration rather than breaking existing accounts.

Use a stable internal Auth user ID for all messages, memberships, and attachments.
Phone numbers, email addresses, and usernames can change without rewriting
history. Store phone numbers in international E.164 form and verify ownership.
An account may have a verified phone, verified email, or both.

Attaching a second identifier is an authenticated account-settings operation
that verifies the new identifier. Do not create another account for this flow,
merge accounts from unverified profile fields, or move an identifier already
owned by another account. Independent accounts need a deliberate, verified
merge/recovery procedure rather than guessing that they belong together.

Telegram's phone-based accounts and public usernames are useful references.
WhatsApp's official username FAQ still requires a phone number to register and
describes optional usernames rolling out separately. Allowing email-only
accounts is our deliberate extension of these patterns.
Sources: [Telegram FAQ](https://telegram.org/faq#usernames-and-t-me),
[WhatsApp usernames](https://www.whatsapp.com/usernames-faq/).

Appwrite supports [phone OTP](https://appwrite.io/docs/products/auth/phone-sms)
and [email OTP](https://appwrite.io/docs/products/auth/email-otp).
Phone OTP requires configuring an SMS delivery provider; email needs configured
delivery/templates. The mock must simulate requests, verification, expiry,
resend cooldown, wrong codes, and linking without sending SMS or email.
Fixed test codes belong only to the mock transport and its fixtures.

## Finding people

Treat discovery and login as separate capabilities:

- Offer a unique, optional username, shareable profile link, and QR code.
- Allow exact phone or email lookup only when that account has enabled the
  corresponding discoverability setting. Hide these values in public profiles.
- Ask separately before matching the device address book. Manual lookup works
  without Contacts permission. No silent address-book upload.
- Return the permitted public profile, not its login credentials or private
  identifiers. An unsuccessful lookup should not distinguish an unknown account
  from an account that has disabled discovery.
- Apply server-side lookup limits and block/report rules. Validate usernames
  and their uniqueness on the server, including case normalization.

Replace the prototype's browse-all `chat_profiles` directory with these scoped
operations before opening the service beyond test accounts. Existing contacts
and shared-room participants can be listed through authorized relationships;
private login identifiers must not become downloadable directory columns.
Phone-number hashes alone do not make an uploaded address book anonymous.

## Conversation types

Use one conversation model with an explicit kind and server-enforced roles:

| Kind | Posting | Membership and discovery |
| --- | --- | --- |
| Direct | The two participants | One canonical conversation per pair; contact/profile action opens it |
| Group | Members, subject to admin restrictions | Owner/admin/member roles; private invitations first |
| Channel | Owner and permitted admins | Subscribers read; private invite links or an explicitly public channel |

This follows the group-versus-broadcast distinction in the
[Telegram FAQ](https://telegram.org/faq#q-how-are-channels-and-groups-different).
Channel posts display the channel's identity. Keep the real author internally
for permissions/auditing. Subscriber status must never permit publishing.
Channel comments, if added later, belong in a linked discussion group.

Use a common message timeline and composer components. Hide the composer for
channel subscribers and enforce the same restriction in backend policies and
the mock; changing the UI alone does not establish publishing permissions.
Invite links should be revocable and have explicit expiry/use limits.

Evolve the existing rooms/members/messages schema with `kind`, ownership,
membership roles, and invite records. Add uniqueness for direct-chat pairs and
atomic create/join/post operations. Do not encode a group's members in a title
or model a channel as a group whose composer happens to be hidden.

## Photos: first media release

Suggested initial policy, to tune with real photo and screenshot fixtures:

| Setting | Initial value |
| --- | --- |
| Format | JPEG, single still image |
| Longest edge | At most 1600 pixels; preserve aspect ratio; never enlarge |
| Initial JPEG quality | 0.80 |
| Preferred size | Around 500 KB or less, depending on content |
| Hard upload limit | 1,000,000 bytes per photo |
| Preview | Separate thumbnail, at most 256 pixels on the longest edge |

Correct orientation, convert to sRGB, and omit GPS and unnecessary source
metadata. JPEG cannot retain transparency: flatten transparent images against
a defined neutral background. Animated images, Live Photos, and original-file
document sharing are separate features, not silently preserved by JPEG export.

Downsample during decode to bound memory rather than fully decoding an enormous
image and resizing afterward. Try lower quality and then smaller dimensions if
necessary, while keeping a quality floor. Verify the encoded file's actual
byte count; a quality number is not a size guarantee. If a photo cannot fit
without unacceptable degradation, return a clear failure and retain the draft.
Do the conversion before upload so the original consumes neither upload
bandwidth nor permanent media storage.

On macOS and iOS, implement this behind an ORCA media adapter using Apple's
[Image I/O](https://developer.apple.com/documentation/imageio/cgimagedestination)
and [thumbnail/downsampling API](https://developer.apple.com/documentation/imageio/cgimagesourcecreatethumbnailatindex(_:_:_:)).
Its [JPEG quality setting](https://developer.apple.com/documentation/imageio/kcgimagedestinationlossycompressionquality)
accepts values from 0 to 1. Image picking and encoding are explicit app actions;
the OS does not automatically enforce our policy just because a photo is selected.

## Storage and message lifecycle

Keep bytes in private object storage and metadata/relations in private database collections. Store
room ID, uploader, object key, media kind, MIME type, byte count, dimensions,
thumbnail key, and eventual message ID. Do not store permanent public URLs for
private conversations.

An upload lifecycle should reserve an attachment, upload its processed bytes,
validate/finalize it, then atomically attach it to a message. Only finalized
attachments appear in the shared timeline. Preserve drafts and retry state on
failure; expire and clean abandoned uploads. Authorization must check current
room membership for upload and retrieval, and channel publish permission for
new channel media. Use authenticated downloads where immediate membership
revocation is required; temporary signed URLs are transferable bearer links.

Enforce size/type limits in storage as well as the client. Validate actual
media bytes and dimensions on the trusted backend before finalization; client
metadata and filename extensions are not proof of image format. Appwrite
buckets support [permissions and upload limits](https://appwrite.io/docs/products/storage/buckets).

## Video: next phase

Keep video upload out of the first media release. Design attachment kinds and
the composer so it can be added without changing the room/message model.

For a later short-clip release, start with a 5,000,000-byte hard maximum,
roughly 4.5 MB target, and a 60-second duration ceiling. Use MP4/H.264/AAC for
broad playback compatibility; adapt resolution/frame rate along with bitrate.
Offer trimming instead of reducing arbitrarily long clips to unreadable video.

The size budget depends on duration, including audio and container overhead:

```text
video bitrate ≈ (8 × target bytes × overhead allowance / duration) − audio bitrate
```

For example, a 5 MB budget with 8% reserved overhead and 64 kbit/s audio leaves
approximately 1.16 Mbit/s video at 30 seconds, 0.55 Mbit/s at 60 seconds, and
0.24 Mbit/s at two minutes. These are starting budgets, not guaranteed file
sizes or quality levels. Measure the completed output, retry within a bounded
budget, or ask the user to trim; never truncate the timeline silently.

Apple's AVFoundation provides export and reader/writer APIs on both macOS and
iOS, with configurable [video bitrate/compression settings](https://developer.apple.com/documentation/avfoundation/avvideocompressionpropertieskey).
Use those native encoders first. FFmpeg can be a future adapter for non-Apple
platforms or server processing if required; it is not necessary for the initial
Apple implementation. Keep encoding asynchronous with progress and cancellation.

## Implementation slices and offline coverage

1. Phone/email OTP, verified linking, username, privacy-aware exact discovery,
   and login/profile UI. Extend the same client and mock protocol together.
2. Explicit direct/group/channel kinds, roles, invitations, posting rules, and
   creation/join UI. Test unauthorized reads/writes and duplicate direct pairs.
3. Photo selection and native JPEG processing, private uploads, timeline
   previews, retry/cancel, and orphan cleanup. Test orientation, metadata removal,
   large images, actual output limits, membership loss, and failed uploads.
4. Short videos after photo upload and retrieval are stable. Test duration/size
   boundaries, rotations, audio-only/no-audio inputs, cancellation, and export
   failures before enabling the feature in the composer.

API tests use the same model/client operations against isolated in-process
fixtures. Mock binary storage can be in memory or a test-owned temporary folder;
it must not fetch sample media from the internet. Native encoding tests use
local fixtures and exercise the real encoder separately from transport tests.
Real SMS/email delivery, database policies, and object storage remain explicit
integration checks against a configured backend.
