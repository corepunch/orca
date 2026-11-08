# Chat with public posts: native screenshots

Captured 8 November 2026 from the production Chat and Octopus XML/Lua views on
Linux/X11 with software OpenGL. Every PNG is an unretouched **390 × 844** native
frame. The data comes from isolated offline mock clients and bundled fictional
portraits; no Appwrite request or live account is used. These are desktop native
captures, not iPhone simulator/device validation.

| Chats | People | Public posts |
| --- | --- | --- |
| ![Chats](social-chats.png) | ![People](social-people.png) | ![Public posts](social-posts.png) |

| Me | Person and public posts | Publish publicly |
| --- | --- | --- |
| ![Me](social-me.png) | ![Person](social-person.png) | ![Composer](social-compose.png) |

[Public posts in light mode](social-posts-light.png).

## Reproduce

Build the native engine following [BUILD.md](../../../../docs/BUILD.md). From the
repository root, launch the isolated capture controller on an otherwise unused
390 × 844 display:

```sh
Xvfb :100 -screen 0 390x844x24 -nolisten tcp
```

In another terminal:

```sh
DISPLAY=:100 CHAT_BACKEND=mock CHAT_APPEARANCE=dark ORCA_CAPTURE_ROUTE=/posts \
  build/bin/orca -test=apps/chat/docs/screenshots/capture.lua
```

Wait for the feed and bundled images to finish loading, then capture that display
with ImageMagick in a third terminal:

```sh
DISPLAY=:100 import -window root /tmp/social-posts.png
```

Use `/`, `/contacts`, `/posts`, `/me`, `/posts/user` or `/posts/write` for
`ORCA_CAPTURE_ROUTE`. Use `CHAT_APPEARANCE=light` for the light capture. Close the
previous fixture before starting another; keep other tests on a different display.

The fixture signs into an isolated mock, opens the requested ordinary route,
selects Discover for feed captures and Alex Morgan for the public-person capture.
It bypasses page-transition animation and supplies explicit paint dimensions
because X11 exposure events can have no size payload. Layout, bindings, media,
styles, posts and actions remain the application's native implementations.
