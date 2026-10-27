# Octopus

A phone client for [Octopus](https://github.com/corepunch/octopus), the small
early-Twitter–style blogging site, written in Lua on ORCA's `Application`
entry point. It talks to the same Appwrite project, collections and storage
bucket as the website, so posts, follows, likes, reposts and comments made in
either place show up in both. It runs on macOS and Linux in a **390 × 844**
portrait window and on iPhone/iPad with an adaptive viewport.

Octopus works without an account. Guests start on the discover timeline and read posts,
comments, profiles and search; writing, liking, reposting, replying and
following ask them to sign in, then return them to where they were.

## Design

The app follows Windows Phone and its Twitter client, sharing its building
blocks with [Chat](../chat/README.md):

- Home is a `Pivot` titled OCTOPUS with Twitter's **home**, **connect** and
  **discover** items. Home is your timeline: your posts, the posts of people
  you follow, and what they reposted, each marked "Maya Chen reposted".
  Connect lists who liked, reposted or replied to your posts and who followed
  you. Discover is everyone's latest posts. Signed-in readers start on home and
  guests on discover; swipe or tap a header to switch.
- Every other page is a locked one-item `Pivot`, so its title and page name
  share the timeline metrics: **post**, **quote**, **link** or **photo** for a
  single post, the person's name for a profile, then **search**, **new post**
  and **me**.
- One persistent `ApplicationBar` switches between home, search, write and me;
  its menu holds refresh and sign in, create account or sign out. A post page's
  bar holds like, repost and comment, with the author, refresh and
  delete (for your own posts) in the menu. Delete asks for a second tap.
- Opening a post or a profile plays the Windows Phone turnstile: the page in
  view turns away on a hinge at the left edge of the screen, top to bottom,
  then the post turns in the same way. Going back plays it in reverse.
  Those pages get a round back button before the small OCTOPUS title; the
  title line and a swipe in from the left edge go back too.
- Cards show a square avatar tile (the author's profile photo, or their
  initials on a tile colour), the author and a relative time, the post by
  type (title and excerpt, a pull quote beside an accent rule, a link badge, or
  a photo shaped like the picture, between 4:5 and 1.91:1), its tags, and comment/repost/like counts. Liked hearts turn red and
  reposts green. Profiles list posts and reposts together, as on Twitter.
- A post's page shows its full time and large like, repost and comment
  counts between hairlines, then the threaded replies.
- Text boxes are pale with dark text in both themes; primary commands are
  filled with the reef teal from the website, secondary ones are 2-pixel
  outlines and links are plain accent text. Icons are Lucide SVGs and the
  octopus mark comes from the website.
- Write uses a two-column icon chooser, taking inspiration from the
  [official Tumblr Windows Phone app's post chooser](https://www.windowscentral.com/official-tumblr-app-comes-windows-phone-8).
  Each type keeps its own draft, including tags and selected photos, when you
  switch types or leave the page. Publishing clears the submitted draft;
  newer edits and drafts for other types stay available.

`theme.lua` holds black and white palettes. The OS appearance chooses between
them; set `OCTOPUS_APPEARANCE=light` or `dark` to override it.

## Run

From the repository root, build ORCA as described in `docs/BUILD.md`, then:

```sh
make octopus                         # live Octopus Appwrite project
OCTOPUS_BACKEND=mock make octopus    # offline demo data
```

The offline mock signs in with **demo@example.invalid / password123** and
keeps changes for the session. For iOS, open `octopus.xcodeproj` and run the
`octopus-ios` scheme, or use the Make workflow from
[iOS setup and deployment](../../docs/IOS.md) with
`PROJECT=apps/octopus BUNDLE_ID=org.orca.octopus DISPLAY_NAME=Octopus`.

## Backend

`config.lua` holds the website's public values: the Frankfurt endpoint, the
Octopus project, the `octopus-db` database and the `post-images` bucket. The
client calls Appwrite's REST API directly with the same document-level
permissions as the browser; there is no server function and no API key.

| Feature | Appwrite |
| --- | --- |
| Accounts | `POST /account`, email/password sessions, sign-out |
| Timelines, search, tags | `posts` with `orderDesc($createdAt)`, `search(title)`, `equal(tags)` and cursor paging |
| Home, profiles | `posts` and `reposts` merged by time, paged with `lessThan($createdAt)` |
| Connect | `likes`, `reposts` and `comments` on your posts and `follows` of you, named from `profiles` |
| Following | `follows` (readable by signed-in users only, so guests see no follower counts) |
| Likes, reposts, comments | `likes` (posts and comments), `reposts`, `comments` with `parentId` replies |
| Profiles | `profiles`, one document per user ID, created at sign-up or first sign-in |
| Profile photos | `profiles.avatarId`, a `post-images` file shared with the website and Chat |
| Photos | `post-images`; JPEG and PNG are shown directly, other formats through the preview endpoint |

Sign-up asks for a name, email and a confirmed password of at least eight
characters, creates the account, signs in and creates the profile, as the
website does. The session cookie stays in memory for the client that created
it, so restarting signs you out.

The four compose forms match the
[website's composer](https://github.com/corepunch/octopus/blob/main/js/pages/create.js):

| Post type | Required | Optional |
| --- | --- | --- |
| Text | Title, post body | Tags |
| Photo | Photo | Caption, tags |
| Quote | Quote text | Source, background photo, your thoughts, tags |
| Link | Title, HTTP or HTTPS address | Description, tags |

**Change photo** on the me page uploads a profile photo, the same
`avatarId` the website's **Change Photo** sets, so it appears on both and in
Chat. Avatar tiles built in one pass share a single `profiles` lookup, and
file IDs are cached for the session.

Photo selection needs the native picker (macOS and iOS). Photo and quote
forms show a preview and let you replace or remove the image. Quote images
appear with the quote on the phone and as its background on the website.
Cancelling the picker preserves the selection; upload failures keep the draft
for retry. Markdown reads as plain text on the phone; the website renders it.
Links can be copied to the clipboard because ORCA has no open-in-browser API yet.

## Code

| File | Owns |
| --- | --- |
| `App.lua` | Routes, session lifetime, the home timelines and background tasks |
| `api/octopus.lua` | The Octopus client: validation, Appwrite queries and response mapping |
| `api/appwrite.lua` | HTTP over `orca.network`, query and JSON encoding |
| `api/mock.lua`, `api/fixtures.lua` | An in-memory Appwrite with the website's permissions and unique indexes |
| `views/` | The shell, home/connect/discover, post cards, post page, people and search, compose and sign-in screens |
| `media.lua` | Photo picking and the private image cache |

Network calls run in `orca.async` coroutines, so pages appear immediately with
a centered shared activity indicator over the content being loaded. Navigation
and inputs stay available while requests are suspended. Feed refresh keeps the
existing posts until a successful response replaces them; failures keep those
posts and expose an error with refresh still available. Each request checks its
page, request and session before applying results.
Sections populate as results arrive: profile details appear before the timeline,
post counts before comments, and search posts before the people lookup finishes.

`photo_loader.lua` checks unloaded photo frames after painting, including scroll
offsets, clipping, Pivot positions and hidden pages. Only photos intersecting
the viewport are requested, with at most two requests at once. Repeated photos
share a request and cached file; each card owns its image view. Downloads that
finish after navigation or sign-out cannot update an inactive view. Failed
downloads or image decode failures show a centered broken-image icon in the reserved photo area; tapping
that area retries the download. Photos use a white image tint in both themes.
Visible pending photos show a centered activity indicator. Once loaded, the
placeholder background disappears and the full image is centered without
cropping or changing the reserved height. Offscreen indicators stop animating.

## Verify

```sh
make test-octopus
```

The API suite covers guest reads, home and profile timelines with reposts and
paging, connect activity, sessions, every post type, validation,
likes, reposts, comments, follows, profiles, photo upload and deletion, and
the exact Appwrite query, cookie and body encoding of the HTTP transport. The
UI suite drives the app through real clicks: the guest discover timeline, post,
profile and tag search, sign-in that returns to the page you came from,
home with reposts, connect, all four compose forms, independent drafts, photo
selection and removal, upload retries, publishing during navigation, comments
and replies, likes, reposts, following, profile editing, delete confirmation,
sign-up and service errors.
The photo suite covers viewport loading, concurrent requests, cache reuse,
retries and stale sessions, including native image resources. None of these
suites contacts the network.

## Demo portraits

The shared [portrait library](../../demo-assets/profiles/README.md) contains 64
generated JPGs, including teens and adults approximately ages 14–60, plus the
original grid, splitter instructions and Appwrite file IDs. This app bundles
portraits for its offline fixture identities. Live seed profiles use the same
public `profiles.avatarId` data in both Chat and Octopus.
