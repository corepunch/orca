# Offline iPhone promo capture

`Capture.lua` decorates the existing Chat application with mock transport and
fictional data. It selects ordinary application routes and exercises the actual
Contacts panorama. `capture.py` packages it as a separate iPhone simulator app;
`render.py` composes App Store artwork around the resulting captures.

See [`marketing/chat/app-store/README.md`](../../marketing/chat/app-store/README.md)
for reproduction, output folders, research, data provenance and validation.

For a real iPad screenshot, dispatch **Chat iPad screenshots** in GitHub Actions.
It builds and captures in a cloud macOS runner, uses the installed 13-inch iPad
simulator and the same offline fixtures, and publishes only PNGs, small logs and
a manifest with the source commit, simulator, Xcode version and binary hash.
It does not contact Appwrite. Intermediate builds and simulator data stay in the
cloud; the downloadable artifact is retained for 30 days.

With an already-built executable and booted iPad simulator, the capture entry is:

```sh
python3 tools/chat-promo/capture.py --device IPAD_SIMULATOR_UDID \
  --family ipad --bundle /private/tmp/chat-ipad-promo-new.app \
  --output-dir /private/tmp/chat-ipad-screenshots --only inbox
```

Use a new output directory to capture a selected stage initially. Repeating
`--only` in an existing directory preserves the other stages only when the
binary, device and family match. Raw native screenshots need no phone frame
or resizing for the iPad Store slot.
