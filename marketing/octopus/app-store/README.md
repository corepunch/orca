# Octopus — iPhone App Store artwork

Eight en-GB screenshots in display order. They are built from real, offline
iPhone simulator captures, using the same fictional cast and portraits as
[Chat's artwork](../../chat/app-store/README.md).

## Art direction: the panorama opener

Screens 1–3 form one continuous 3960 × 2868 canvas, cut into three store
slots. One oversized **octopus social** title runs unbroken across all three
and past the right edge, as on a Windows Phone Panorama. Each slot's phone
shows one real Pivot section (home, connect and discover) and bleeds off the
bottom. Scrolling the store carousel therefore reads as a sideways pan through
the app, which distinguishes this listing from the usual one-phone-per-slot
layout. Screens 4–8 use Chat's layout: a framed phone on a black band, with
neighbouring screens visible at the edges. Each slot's footer reads
**SWIPE SIDEWAYS.**

| # | Screen | Headline |
|---|---|---|
| 1 | home | home, connect, discover. |
| 2 | connect | see who's listening. |
| 3 | discover | find someone new. |
| 4 | photo post + thread | every post, a conversation. |
| 5 | profile | get to know the author. |
| 6 | new post chooser | four ways to post. |
| 7 | photo album post | share up to nine photos. |
| 8 | home, light | light or dark, it's yours. |

## Deliverables

- `en-GB/iphone-1320x2868/` and `en-GB/iphone-1206x2622/`: two alternative
  upload sets (RGB PNG, filename order = display order).
- `octopus-iphone-app-store-en-GB.zip`: both sets.
- `contact-sheet.jpg`: the carousel side by side, for review only.
- `panorama-wide.png` / `panorama-preview.jpg`: the unfolded Pivot plus a post
  and an album, for campaign or review use.
- `captures/`: the raw simulator screenshots; `manifest.json` records their
  hashes, the binary hash and the exports.

Nothing has been uploaded to App Store Connect yet. Octopus targets iPhone and
iPad, so a 13-inch iPad set is still needed before submission.

## Data

`tools/octopus-promo/fixtures.lua` seeds Octopus's in-memory Appwrite mock
with 12 fictional people (candid portraits from
[`demo-assets/profiles/candid`](../../../demo-assets/profiles/candid/README.md)),
16 posts of every type, follows, likes, reposts and threaded comments, under a
fixed 09:41 clock. The lake photo is
`tools/chat-promo/assets/lake.jpg`. The capture bundle
`org.orca.octopus.promoshots` is separate from the app. No Appwrite request,
account or production data is used. The mock's developer status line is
suppressed in the capture controller only.

## Reproduce

```sh
make ios-sim
python3 tools/octopus-promo/capture.py --device IPHONE_SIMULATOR_UDID \
  --bundle /private/tmp/octopus-promo-new.app
python3 tools/octopus-promo/render.py
```

`--only STAGE …` recaptures selected stages. Rendering requires Pillow and the
macOS Helvetica Neue font collection.
