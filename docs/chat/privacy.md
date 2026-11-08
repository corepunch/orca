# Orca Messenger Privacy Policy

Last updated: November 8, 2026

Orca Messenger is a messaging app provided by Igor Chernakov. This policy
explains the information used by the app and its online service, including
the shared Octopus account and profile service.

## Information used by the service

- **Account and profile information:** your account identifier, email address,
  display name, optional public username, profile picture, and any phone number
  you choose to add. Authentication is handled by Appwrite. If you choose Apple
  or Google sign-in, that provider handles authentication and shares the account
  information you authorize with Appwrite.
- **Conversations and content:** messages, photos and their thumbnails,
  conversation names, group and channel membership, roles, invitations,
  timestamps, and the identifiers needed to associate this content with your
  account and conversations.
- **Preferences:** settings such as whether people can find you by a verified
  identifier and whether notification alerts include message previews.
- **Notification and connection information:** if you enable notifications,
  a device push token is associated with your account for delivery. Requests to
  the service also involve your IP address and authentication/session information.
  Appwrite retains session records containing the sign-in IP address, a country
  inferred from that address, and device, operating-system and client information.
  These records support authentication and account security. This does not
  involve access to your device's precise location or GPS.

This information is used to authenticate you, deliver and display messages and
photos, manage conversations, find people according to their discovery settings,
deliver notifications, and operate and protect the service. The app does not
use advertising or cross-app tracking.

## What other people can see

Your display name and optional public username identify you to other users.
Chat contacts come from conversations and authorized relationships; the app does
not upload your device's address book.

Exact username lookup is supported. Email discovery requires a verified address
and your opt-in setting. A phone number entered in Profile is private and does
not make you searchable. Older accounts may have a verified login phone; only
a verified identifier with discovery enabled can be used for discovery. Your
email address and phone number are not returned in other users' public profiles.

Your profile photo is **public** and is shared with Octopus and its website
through the same account. It is separate from photos sent in conversations.

Messages and conversation photos are made available to authorized conversation
members. Public channels can be discovered and joined by other users. People
who receive content may save or share their own copies outside the app.

## Hosting, service providers and security

The service uses Appwrite Cloud's Frankfurt endpoint for accounts, application
data, files, server functions and notification delivery. Appwrite processes data
to provide these services. See [Appwrite's privacy policy](https://appwrite.io/privacy).

On Apple devices, push notifications pass through Apple's push notification
service. Alerts can include a sender name and a short message preview or photo
label. Turn off **Message previews** in Profile and save the setting to request
generic new-message alerts instead. You can also control notification permission
and lock-screen previews in iOS Settings.

Connections to the online service use HTTPS. Conversation photos and thumbnails
are encrypted on the server before storage and returned after a membership
check. This is **server-side encryption, not end-to-end encryption**: the service
can process conversation content. Public profile photos do not receive the
private-conversation-photo protection.

On iOS and macOS, the app stores its private sign-in session credential in Apple
Keychain to restore your session. The app also keeps working data and temporary
photo previews locally. Signing out clears its local credentials and temporary
photo previews; it does not delete your server account or conversations.

## Managing and deleting information

Profile lets you change your display name, public username, optional phone
number, discovery preferences and notification-preview preference. You can
change your profile picture and use the supported email-verification flow to
change a linked email address.

You can delete your messages while you remain a conversation member. Deleting
a message removes that message and its attached conversation photos from the
service for everyone in the conversation. Group and channel owners and admins
can also delete messages according to their roles. Removing content from the
service cannot remove copies another person has already saved independently.

Account, profile and conversation records remain stored so the service can
provide account access and message history. Signing out or leaving a
conversation does not erase those records. For requests concerning access,
correction or deletion of account information, use the private contact below.

## Contact and policy updates

For privacy questions or requests to access, correct or delete your account
information, email [igor.chernakov@gmail.com](mailto:igor.chernakov@gmail.com).
Please do not send passwords or sign-in codes. We may need to verify account
ownership before acting on a request.

For general support, use the
[Orca issue tracker](https://github.com/corepunch/orca/issues). Issues are public:
do not post passwords, sign-in codes, private messages or other personal data
there.

This policy page is hosted on GitHub Pages. When you visit it or use the issue
tracker, GitHub processes connection and usage information under its
[privacy statement](https://docs.github.com/en/site-policy/privacy-policies/github-general-privacy-statement).

Updates to this policy will be published on this page with a revised date.
