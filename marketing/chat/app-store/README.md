# Chat — iPhone App Store artwork

Eight English screenshots, in display order, with populated fictional accounts,
conversations, unread counts, groups, a channel and a photo exchange. The first
two focus on the new wide Contacts portrait wall. A separate landscape panorama
shows that wall across two viewports with more app screens continuing right.

## Deliverables

- `chat-iphone-app-store-en-US.zip`: two complete, alternative size sets.
- `en-US/iphone-1320x2868/`: eight RGB PNG screenshots for the large iPhone slot.
- `en-US/iphone-1206x2622/`: eight RGB PNG screenshots for the smaller slot.
- `contact-sheet.jpg`: overview for review; not an upload screenshot.
- `panorama-wide.png`: full 4096 × 1740 unfolded campaign artwork.
- `panorama-preview.jpg`: smaller landscape review copy.
- `creatives/header-3840x1646.png`: dedicated product-page header.
- `creatives/search-results-3840x2560.png`: dedicated 3:2 search-results artwork.
- `captures/`: original iPhone simulator screenshots, including one auxiliary
  Contacts All view used in the landscape composition.
- `manifest.json`: ordering, captions, capture/export hashes and binary provenance.

Upload one appropriate size set to App Store Connect in filename order. Do not
upload the ZIP or overview images. Nothing has been submitted or published.
Apple's [screenshot specifications](https://developer.apple.com/help/app-store-connect/reference/app-information/screenshot-specifications)
were checked for accepted portrait sizes and RGB PNG/JPEG requirements.

The two creative PNGs are separate from the screenshot carousel. Use Header and
Search Results in Product Page Information for their corresponding placements.
Apple's [creative asset specifications](https://developer.apple.com/help/app-store-connect/reference/app-information/creative-assets-specifications)
were checked on 2026-10-29: the header is 3840 × 1646 and the search image is
3840 × 2560, both RGB without alpha. Their upload remains pending until Computer
Use is available for the requested Safari/integrated-browser workflow.

## Art direction and research

The campaign takes the short benefit headline, populated inbox and large phone
display seen in [WhatsApp](https://apps.apple.com/us/app/whatsapp-messenger/id310633997),
[Telegram](https://apps.apple.com/us/app/telegram-messenger/id686449807) and
[Signal](https://apps.apple.com/us/app/signal-private-messenger/id874139669), and
combines them with the user's Windows Phone panorama references: thin lowercase
type, a black surface unfolding beyond an outlined device, and a wide portrait
grid. The outlined device and captured application are iPhone.

Microsoft's [Panorama overview](https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff941104(v=vs.105)),
[horizontal section contract](https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff941126(v=vs.105))
and [People/Pictures examples](https://learn.microsoft.com/en-us/previous-versions/windows/apps/hh202912(v=vs.105))
identify the public control model as Panorama/PanoramaItem sections containing
ordinary grids and lists. They do not identify the proprietary system hubs'
implementation classes. The [historical unfolded screenshot technique](https://blog.scottlogic.com/2011/07/21/capturing-windows-phone-7-panorama-images.html)
also informed the landscape composition.

Inside Chat, Contacts uses the shared native `Panorama` and `PanoramaItem`, a
data-bound `ItemsControl` portrait grid and the existing avatar loader. Wide
section measurement and gesture/snap behavior live in the engine. The app has a
Contacts screen, without adding a separate social hub. See
[`apps/chat/docs/contacts-panorama.md`](../../../apps/chat/docs/contacts-panorama.md).

Phone screens are real captures. The unfolded contact wall stitches their
overlapping grid regions and re-typesets one shared heading; it is an exploded
marketing composition rather than another app route. The landscape combines
separate routes to the right. No calls, stories or end-to-end encryption claims
are made. Fictional account data follows Apple's
[accurate metadata guidance](https://developer.apple.com/app-store/review/guidelines/#accurate-metadata).

## Fictional photos and data

64 synthetic adult personal photos vary in setting, lighting, angle, expression
and camera quality. They were generated using the built-in image generation
tool, then split with the existing deterministic crop tool. The original image,
exact prompt and individual portraits are in
[`demo-assets/profiles/candid`](../../../demo-assets/profiles/candid/README.md).
The lake photograph was generated with the same tool; its prompt is in
`tools/chat-promo/assets/lake-prompt.txt`.

The capture bundle injects `api.chat`'s existing mock transport and deterministic
fixtures: 64 people, 63 saved contacts, nine populated conversations, and a fixed
09:41 clock. The same people keep the same portraits across screens. Assets and
fixtures are local to the separate `org.orca.chat.promoshots` simulator bundle.
No live users, accounts, Appwrite data or production startup are changed.

## Reproduce

From the repository root, build the simulator executable if native code changed:

```sh
make ios-sim
python3 tools/chat-promo/capture.py --device IPHONE_SIMULATOR_UDID \
  --bundle /private/tmp/chat-promo-new.app
python3 tools/chat-promo/render.py
```

Rendering requires Pillow and the macOS Helvetica Neue font collection. Use the
bundled Codex Python runtime if the system Python lacks Pillow. The capture tool
requires a new temporary bundle directory and an explicitly selected, booted
iPhone simulator. It uses public route/selection APIs and a genuine swipe for
the second portion of the contact wall. Capture logs are ignored.

## Validation

Native Metro wide-section regressions, focused Contacts tests and Chat app tests
passed. Desktop and iPhone simulator builds passed. Export dimensions, RGB mode,
capture hashes, text bounds and ZIP contents are checked by the renderer. The
broader headless run stopped at the existing secure-credential/Keychain check;
the focused UI checks passed. No live Appwrite integration was requested or run.
