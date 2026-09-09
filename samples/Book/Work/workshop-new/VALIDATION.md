# Workshop validation — 2026-09-09

The fresh scene contains 25 named cameras and uses 27 new `.blk` prefabs.
All 28 XML files parse. Every camera has a real JPEG at 1920 × 1440.
Projected overview interaction anchors have at least a 2% frame margin.

The final camera batch was rendered with the deployed Scener bundle
`20260909-193705-317172` on Apple M1,
OpenGL 4.1 Metal, with shadows enabled and 2× supersampling. The contact sheet
was reviewed across all cameras, with individual checks of the establishing
view, pet door, countertop, Marzipan and reverse view. The pet-door fanlight
now meets the rectangular leaf without overlapping faces; the stone threshold
sits above the floorboards instead of sharing their plane.

Scener's focused C tests pass (3/3). Its CLI suite passes on both tested
backends. The shadow regression verifies nine GPU camera views, expected lit
and shadow regions, and rejection of Apple software shadow exports before
creating images. Running Book's real render command under that restricted
backend confirms the error preserves all existing JPEGs and cleans staging.
The successful GPU batch exercises replacement through the same Makefile.

`make sanity ROOM=workshop-new` succeeds, but recognizes only one supported
semantic assembly. It does not certify contacts inside the composite prefabs.
Manual checks corrected wall mounts, bench supports, the broom lean, chair/book
overlap and source-based character placement. XML/reference validation and
visual review supplement the limited existing checker.

Artifacts:

- [Scene](../../Rooms/workshop-new.blks)
- [Overview JPEG](../../Rooms/render/workshop-new/workshop-floor.jpg)
- [Contact sheet](../../Rooms/render/workshop-new/contact.jpg)
- [Layout](../../Rooms/render/workshop-new/layout.jpg)

These initial-state scene assets now run in Book's existing Orca UI.
Story access gates remain owned by ZIL. Changed-state/action images remain
separate work. See [DESIGN.md](DESIGN.md)
for source, coverage and reproduction, and [RENDERING.md](../RENDERING.md) for
the external Scener build, deployment and renderer diagnostics.
