# Contacts panorama

Chat's Contacts root uses the shared `Panorama` and `PanoramaItem` controls,
with a horizontal **recent** section and an ordinary **all** section. It keeps
the existing chats, new conversation and profile commands in the shared
ApplicationBar. New group, channel, invitation and discovery flows remain in
New message.

The source reference is the user-supplied Windows Phone People/Pictures panorama
images. They establish a visual direction, rather than an API or an additional
social product. Microsoft's [PanoramaItem orientation documentation](https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff941126(v=vs.105))
describes horizontal items containing wide content and stopping within the same
section before advancing. Chat consumes that engine control, rather than adding
a second horizontal scroller or duplicating the page.

Contacts is declared data, with no view code of its own. `Pages/Contacts.xml`
binds the session's Inbox scope (`Data/Inbox.xml`): `People` (everyone but the
reader, by name), `ContactList` (a local search view), `Recent` (the portrait
wall) and the `OpenContact` command. `providers/chat.lua` holds their
operations, filters and sort. Native `ItemsControl` retains and orders the
portrait and contact rows; `Templates/ContactPortrait` and `ContactRow` own
their visual structure. Recent uses uninterrupted portrait tiles; full names remain
in their presentation records and the All list. The existing avatar loader
resolves profile photos, with initials while a photo is absent or loading.

Recent direct conversations order the portrait wall, followed by the remaining
saved contacts. The current account is excluded. Up to 32 contacts occupy an
eight-column, four-row grid, 720 points wide; collections of 16 or fewer use a
320-point grid. No dummy people, activity, account permissions or directory
entries are added by this feature. Promotional fixtures are separate tooling.
All contacts uses a retained vertical list, filtered by display name or username.
An unmatched search differs from an empty directory; new discovery is available
through “find someone new”.

Tapping either a portrait or a list row creates or reuses a private direct
conversation through the existing Rooms model. Errors keep the Contacts view
usable. A page-owned feedback strip stays visible while panning either section
and collapses completely when healthy. Refreshes retain useful rows and
provide retry; session changes discard results. A direct-open response cannot
navigate over a newer user navigation. Switching root pages retains the Contacts
page, search text, section selection and native item identities.

For native capture, open `/contacts`. Public nodes are `ContactsPanorama`,
`ContactsRecent`, `ContactsAll`, `ContactsMosaic`, `ContactsList`, `ContactsSearch`,
`ContactsTile<user-id>` and `ContactsRow<user-id>`. The app exposes
`contactsPanorama`; `app.scope:member("Contacts").loaded` reports loaded data. `SelectedIndex=0` selects Recent;
`SelectedIndex=1` selects All. Swipe left within Recent to pan the wide portrait
wall; the outer Panorama owns this motion. Its final internal snap remains
SelectedIndex 0, and the next committed swipe reaches All.

Focused offline validation uses the current desktop binary:

```sh
build/bin/orca -test=tests/test_chat_contacts.lua
```

This covers native item/photo layout, local search, retained identities and root
navigation, direct conversation activation, failures/retries, and late request
handling across navigation and session changes. It does not contact Appwrite.
