# End-to-end encryption: dropped

Decision, 2026-11-14: Orca Messenger does not do end-to-end encryption. It is a
small, low-effort app, so it keeps server-side storage and says so plainly in
its [privacy policy](../assets/privacy.md): messages are not end-to-end
encrypted, and the operator accesses content only for abuse reports, legal
requests or running the service.

Rules that follow from this:

- Never describe the app as end-to-end encrypted or as secure messaging, in
  the app, the App Store listing or marketing.
- Info.plist keeps `ITSAppUsesNonExemptEncryption=false`. The app uses only
  HTTPS from iOS and server-side photo encryption.

History: a first version (Monocypher, an `orca.crypto` Lua module and
published per-device keys) shipped in 1.1 build 77 and was then removed. That
build publishes keys after sign-in. Once the backend routes were removed, it
gets a "not found" reply and retries quietly in the background; messaging is
unaffected. The full design (envelopes, photos, safety numbers, sender keys)
is in git history at commit `fa5f61d4`, if privacy ever becomes a selling
point.

Orca Messenger was removed from sale in France during that release, to avoid
an encryption declaration. That is no longer needed, so France can be
restored.
