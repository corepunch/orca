# Metro on iPad: design for Chat and Octopus

Status: proposal, 14 November 2026. Nothing here is implemented yet. It
extends the [Windows Phone UI cheatsheet](windows-phone-ui-cheatsheet.md),
whose labels apply: **Historical** is sourced period practice, **ORCA** is
current engine behaviour, **Recommendation** is our design judgement.

## What Microsoft did on tablets

**Historical.** Microsoft never stretched Windows Phone onto tablets. The
tablet expression of Metro was Windows 8's "Modern UI" (2012), a sibling
language with its own controls and metrics:

- **Panorama became the Hub.** A Windows 8 app opens on a *hub page*. This
  is a horizontally panning canvas of differently shaped sections under one
  large page title. Section headers are tappable and drill into a section
  page; items drill into a detail page (*hub → section → detail*). The hub
  often starts with a borderless *hero* section, typically an image that
  bleeds to the left edge, runs top to bottom and has no section header.
  ([Hub control introduction][hub-blog],
  [hierarchical navigation][hier-nav])
- **Content pans sideways, the chrome stays put.** Pages share a 20-pixel
  grid. Content starts 6 units (120 px) from the left and the page title
  sits in the top 5 units (100 px). Commands sit in an 88 px app bar that
  appears on demand. Windows 8.1 hubs switch to vertical sections below
  500 px wide, keeping the same content. ([Windows 8 design sheet][sheet],
  [Hub control introduction][hub-blog])
- **Many sections are visible at once.** On a 1366-pixel tablet a hub shows
  two or three sections side by side and pans for more. Pivot-style switching
  between related views gave way to sections you can see together. Semantic
  Zoom offered a pinch-out overview for long collections.
  ([Semantic Zoom guidelines][semzoom])
- **Social content panned as well.** The Windows 8 People app's *What's new*
  was a sideways-scrolling feed of Facebook and Twitter updates. It is the
  tablet relative of the phone People hub. ([PCWorld][people])
- **The phone and tablet languages later converged.** Windows Phone 8.1
  adopted the tablet Hub as the replacement for Panorama. Windows 10 made
  Pivot universal (it became a tab strip on large screens) and dropped the
  Hub's wrap-around and parallax. ([Porting to UWP][uwp-porting])
- **Metro on iOS was Windows 8 style, not phone style.** Microsoft's own
  2012 Xbox SmartGlass for iPhone and iPad took "the look and feel of the
  Windows 8 app" rather than Windows Phone's. ([TNW][smartglass])

So yes: on tablets Microsoft went with Windows 8. Its rules were the same
as the phone's (content before chrome, big lowercase type, flat colour, the
edge cut off to hint at more), applied to a wide, horizontally panning canvas.

## Principles for ORCA on iPad

**Recommendation.**

1. **Adapt by width, not by device.** Below 600 points (iPhone, iPad Slide
   Over and narrow Split View) keep today's phone layouts unchanged. At 600
   points and above, use the *wide* layouts below. Rotation and Split View
   resizing move between the two live, without losing data or position.
2. **Never stretch a phone column.** Today an iPad shows one phone column
   stretched to 1032 points. Chat's current iPad screenshots show that
   inbox, with names and times nearly a screen apart. Wide layouts instead
   use fixed-width columns (*sections*) and let the canvas pan.
3. **Pivots unfold into a hub.** On a wide screen, a Pivot's items appear
   side by side as Hub sections. The ones that fit are visible together, and
   the canvas pans horizontally for the rest. The selected header becomes a
   section header above its column, and tapping a section header scrolls
   that section to the left edge. This turns the phone metaphor (*the screen
   is a window onto something wider*) into a literal layout. It also keeps
   the app's identity: what the Pivot hints at on iPhone, the iPad shows.
4. **Panoramas get room, not a new shape.** A Panorama stays one canvas with
   its oversized title. On a wide screen, sections take their natural widths
   instead of `0.875 × viewport`, so two or more are visible and the title
   runs across them.
5. **Details open as pages with columns.** Drilling into an item still
   pushes a page with the turnstile and the round back button. In landscape
   a detail page uses two columns, the item on the left and its secondary
   content on the right, rather than one long scroll. Chat's conversation is
   the exception: messaging apps on tablets (Windows 8 Messaging, iPad
   Messages) keep the list and the conversation side by side.
6. **Keep the chrome Metro.** Keep the bottom ApplicationBar, with glyphs
   labelled, and spread commands from both edges (Windows 8 put global
   commands at the right and contextual ones at the left). Keep the small
   uppercase app title and lowercase headers. No sidebars, tab bars or
   iPadOS navigation bars.

## Metrics

**Recommendation**, in ORCA logical points. Windows 8 designed for 1366 × 768
at 100%, which is close to a landscape 13-inch iPad's 1376-point width. We
therefore reuse its grid directly, then step it down.

| Width | Left margin | Section width | Section gap | Page title |
| --- | --- | --- | --- | --- |
| < 600 (compact) | 12 (today) | viewport | — | today's Pivot 15/44, Panorama 110 |
| 600–1099 (iPad portrait, wide Split View) | 40 | 400 (feeds 440) | 40 | Pivot title 15, headers 44 |
| ≥ 1100 (iPad landscape) | 120 | 400 (feeds 440) | 80 | as above |

- The grid is 20 points. The top of the hub title aligns at 60 (portrait) or
  80 (landscape) below the safe-area top.
- Text-heavy columns stay at 440 points or less, about 60–70 characters at
  15–16 point body type.
- The last visible section is cut off at the right edge whenever more content
  follows. Like the Panorama next-section hint, the cut-off is the
  affordance.
- Section headers are 36-point light lowercase, white when focused and dim
  otherwise.

## Octopus

| Screen | Wide layout |
| --- | --- |
| Home (Pivot: home, connect, discover) | A hub titled **octopus**. In landscape, *home* (440), *connect* (400) and the start of *discover* are visible; pan or tap *discover* to reach it. In portrait, two sections plus a cut edge. Each section scrolls vertically on its own, and pull-to-refresh is per section. While you follow fewer than five people, *people to follow* stays as a narrow section between home and connect, rather than at the top of home. |
| Post | A page. In landscape the post fills a 560-point left column, with its full-size photo or album, counts and actions, and the threaded replies with the comment field form the right column. In portrait, today's single column, capped at 640 and left-aligned at the margin. |
| Profile | A page: identity, counts and follow on the left (360), posts and reposts on the right. |
| Search | A page: search field and people results on the left, posts with the tag on the right. |
| New post | A page: the four type tiles become a left rail (a 2 × 2 grid, 360 wide), and the selected type's form is on the right. |
| Me / edit profile | A single form column of 560 at the margin. Forms don't widen. |

This is also the iPad store story. A landscape iPad screenshot can show home,
connect and discover side by side, and the one-title-across-three-slots
panorama from the iPhone set carries over directly.

## Chat

| Screen | Wide layout |
| --- | --- |
| Inbox + conversation | Split, Windows 8 Messaging style: the inbox Pivot (all, unread, pinned) is a 360-point left column, and the open conversation fills the rest. Bubbles keep their phone maximum width; they don't stretch. In portrait the same split uses 320 + rest. |
| Contacts (Panorama) | One canvas: the *recent* portrait wall at its full 720-point width with the *all* list (360) beside it, both visible in landscape, and the oversized **contacts** title above them. |
| New message, group and channel flows | Pages, single column of 560. |
| Settings | Groups in two columns of 400 in landscape, one column in portrait. |
| Photo viewer | Unchanged: fitted image on black. |

## Engine work (ORCA)

None of these layouts exist yet. The phone controls today assume one viewport
per item:

- **Pivot wide mode**: lay out `PivotItem`s side by side at a set section
  width when the Pivot is at least 600 points wide. Turn the header strip
  into per-section headers that are tapped to scroll. Selection then means
  "scrolled into view". Each section keeps its own vertical scroll.
- **Panorama natural widths**: when the viewport is wide, size sections by a
  declared width rather than the 0.875 factor, and pan freely across them.
- **Width classes**: expose a compact/wide value (or the viewport width) to
  XML/CSS bindings, so pages switch layouts declaratively. Rotation and Split
  View resizing must keep scopes, drafts and scroll positions.
- **Two-column pages**: a shared detail layout (fixed left column plus
  flexible right column) that falls back to a single column below 1024.
- **ApplicationBar labels** shown at wide widths.

The phone layouts stay as they are. Changes to wide mode must keep the
existing 390-point tests passing, and new tests should cover 834 × 1194
(11-inch portrait) and 1376 × 1032 (13-inch landscape).

## Open decisions

- Should Octopus home in landscape show all three sections, or keep *home*
  dominant, with connect and discover narrower?
- Should the iPad store set be landscape (more visibly unique) or portrait
  (matches most iPad listings)?
- Should Chat's split view also be used for Octopus (feed left, post right)?
  This proposal says no: in Windows 8, hub apps pushed detail pages.

[hub-blog]: https://blogs.windows.com/windowsdeveloper/2014/02/05/introducing-the-new-hub-control-for-heterogeneous-data/
[hier-nav]: https://learn.microsoft.com/en-us/previous-versions/windows/apps/dn263197(v=win.10)
[sheet]: https://learn.microsoft.com/en-us/archive/blogs/codefoster/windows-8-app-design-sheet
[semzoom]: https://learn.microsoft.com/en-us/previous-versions/windows/jj883708(v=win.10)
[people]: https://www.pcworld.com/article/455568/windows-8-deep-dive-get-to-know-the-people-app.html
[uwp-porting]: https://learn.microsoft.com/nl-be/windows/uwp/porting/w8x-to-uwp-porting-xaml-and-ui
[smartglass]: https://thenextweb.com/news/microsoft-renames-xbox-live-ios-app-to-smartglass-and-adds-more-console-controlling-features
