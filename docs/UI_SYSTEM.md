# UI System

Use this file when changing UIKit controls, layout, XML UI, popups, or styling behavior.

## Main paths

- `plugins/UIKit/UIKit.cgen`
- `plugins/UIKit/*.c`
- `docs/app/styling.md`
- `docs/app/xml/triggers-actions-messages.md`
- `tests/test_layout.lua`
- `tests/test_styles_lua.lua`

## Core rules

- UIKit is built on the same object/component system as the rest of ORCA.
- If you add a control, property, or message, follow `.cgen -> make modules -> C handlers -> tests`.
- Use messages and properties for cross-system behavior; do not make core depend on UIKit headers.

## Layout and test rule

For tests with explicit dimensions, always set `ResizeMode="NoResize"` in XML or `ResizeMode = "NoResize"` in Lua.
Otherwise the window size can override the intended test dimensions.

## Password inputs

Use `Input { Type = "Password" }` for passwords. UIKit masks the prepared
display text with one `*` per Unicode codepoint while retaining the original
`Text` value for editing and submission. Empty inputs show their placeholder.
Applications should pass the real value to authentication without masking it.

## Scrolling

Set `ContentOffset` through the property system. Scroll-only frames reuse
measurement and arrangement and update descendant transforms. Content changes
and viewport resizing still invalidate layout. Native scroll-wheel and
`Node2D.SetScrollTop` handling use this same path.

On iOS phone builds, vertical direct-finger scrolling has inertia and
rubber-band edges by default on overflowing `Node2D` containers (including
stacks and grids with `OverflowY = "Scroll"`). No application gesture handlers
are needed. The touched container stays captured as its content moves; a new
touch stops its momentum, release decelerates, and cancellation returns any
overscroll to the edge without a fling. Taps and horizontal Pivot swipes keep
their existing routing. Containers whose content fits pass through to an
overflowing ancestor; they do not bounce on their own. Release momentum uses
the normal UIScrollView deceleration rate of 0.998 velocity retention per
millisecond, integrated over elapsed time independently of frame cadence.

Set `InertiaScrolling = false` to disable release momentum, or
`RubberBandScrolling = false` to hard-clamp touch scrolling at the boundaries.
Both default to true and affect direct-touch gestures only. Desktop wheel
events stay immediate and clamped. Programmatic `ContentOffset` changes stop
touch motion so a scheduled animation cannot overwrite the requested offset.
Content-size changes use the current bounds when settling. Animation updates
use the property system and reuse measurement/arrangement like ordinary scroll
frames.

The platform marks touch-wheel events with Begin/Update/End/Cancel and the
sample's unsigned millisecond timestamp. UIKit exposes these as
`Node.MouseMessage.scrollPhase` (`ScrollPhase`) and `timestamp`; phase `None`
identifies an ordinary wheel event. Direct producers must send Begin before
Update, and End or Cancel to finish the sequence. The iOS platform submodule
and Orca must be rebuilt together for this event contract.

Touch never hovers. A finger leaves no pointer behind, so an event carrying
`AX_POINTER_TOUCH` clears the hovered object instead of setting it, and
`:hover` rules apply only to a mouse, trackpad or pencil. Give touch feedback
through the pressed or selected state, not `:hover`.

Focus and a visible keyboard-navigation ring are separate states. `:focus`
matches the focused object, including a text field tapped for editing.
`:focus-visible` matches it only in keyboard mode. Key-down events enable
keyboard mode; mouse button-down and direct-touch events clear it, updating
the current object's styles even when focus itself does not move. Text-input
events alone do not enable keyboard mode. Programmatic focus follows the
current mode. Use `:focus-visible` for button rings and `:focus` for editing
feedback such as text-field borders. This also lets hardware keyboards reveal
button focus on a touch device.

## Safe area and docked bars

Edge-to-edge screens (`Screen.RespectSafeArea = false`) let scrolling content
run under the status bar, the home indicator and translucent bars, like a
full-screen `UIScrollView`. Its frame covers the whole screen; only its content
is inset, as `adjustedContentInset` insets it on iOS.

- A child with `SafeAreaEdge = "Top"` or `"Bottom"` is a docked bar. A plain
  `Node2D` parent measures it like an `auto` row, stacks it at that edge below
  the bars of its ancestors, and lays its other children out under it.
- The safe area reaching a node is what it overlaps of the system insets of
  its `Screen` and of the bars docked by its ancestors.
- `ContentInsetAdjustment` decides what a node does with it. `Automatic`, the
  default, insets scrolling content: it rests below the top bars and scrolls
  clear of the bottom ones. `Always` insets any content, and a docked bar with
  `Always` also extends through the system inset at its edge, keeping its own
  content clear of it. `Never` leaves the content under the bars. Layout
  publishes the applied inset as `AdjustedContentInsetTop` and
  `AdjustedContentInsetBottom`: the visible part of a scrolling node lies
  between them, for example to find the first message below the header.
- `ScrollAnchor = "Bottom"`, after SwiftUI's `defaultScrollAnchor(.bottom)`,
  keeps a scrolling node that is at its end at its end when layout changes its
  content or insets: a chat stays at its newest message as the keyboard opens
  or the composer grows. Scrolled elsewhere, the node keeps the content at its
  visible bottom edge in place when that edge moves, so the keyboard does not
  cover what was being read. Touch scrolling in progress is left alone.
- A node that insets its content consumes the safe area: its children see none.
  A `Pivot` consumes the top inset with its header; with
  `ExtendBodyBehindHeader = true`, its items extend behind the header, which
  becomes a top bar styled by the generated `pivot-bar` part. A `BackIcon`
  puts a back glyph in a ring (the `pivot-back` part) before the title, which
  moves right to make room: tapping the title line, or swiping in from the
  left edge, fires `BackRequested`, so a page needs no row for its back
  button. Tapping the selected header fires `HeaderClick`, for example to
  open details of a conversation.
- A `Pivot` is also the unit of page navigation. `NavigationTransition`
  (`TransitionType`: `Turnstile` by default, `Slide`, `Fade` or `None`) is its
  motion, and setting `TransitionMode` plays a phase of it from the next frame.
  `ForwardOut` or `BackwardOut` takes the page you leave away and holds it
  there; `ForwardIn` or `BackwardIn` brings the next page in and returns to
  `None`. `TransitionCompleted` fires at the end, and the page takes no input
  meanwhile. The turnstile turns the page about a hinge left of the screen,
  feathered: lower parts start later. `share/orca/UIKit/PageTransition.lua`
  plays the page a screen shows out and then the next page in. See
  [Windows Phone page transitions](windows-phone-transitions.md) for the
  sources, values and parts.

`share/orca/UIKit/Footer.lua` builds this shell for applications; see
[App styling](app/styling.md#layout-and-sizing). Frost the bars with
`backdrop-filter` and a translucent background.

## Loading content

Attach the screen shell and usable controls before starting network work. Use
`orca.async` or `orca.core.async.run` to fetch data and populate each pending
content region as its result arrives. A coroutine that waits for every request
before attaching the screen still leaves the user waiting for that screen.
Keep existing content during refresh, and check the current request, route and
session before applying a completion. Failed requests should leave a retry path.

`ActivityIndicator` is UIKit's shared indeterminate loading control, available
in Lua and XML. It draws twelve fading dots in a circle, inherits `Foreground`,
and defaults to 64 by 64 points with auto margins for centering in its parent.
It shrinks uniformly to fit a smaller content region, including its padding.
It ignores hit testing. `IsActive=false` hides the dots and stops animation
while retaining layout; hidden or detached indicators stop animation too.
Place it in a Grid covering only the pending content region, so navigation and
other controls remain interactive. Set `Width` and `Height` to change its size.

```lua
local pending = ui.Grid { IgnoreHitTest = true }
pending:addChild(ui.ActivityIndicator { Foreground = colors.muted })
content:addChild(pending)
orca.async(function()
    local result, err = service:load()
    -- Check that this is still the current view/request before updating it.
    pending:removeFromParent()
    showResultOrRetry(result, err)
end)
```

## Images and viewport notifications

For thumbnail/full stages, queue priority, authenticated delivery and decoder
choices, read [Loading image previews](app/image-loading.md). A decoder's input
callbacks and an app's ready-preview callbacks are different interfaces.

Load network images asynchronously as their views need them. Apple recommends
[cached asynchronous image loading](https://developer.apple.com/documentation/uikit/asynchronously-loading-images-into-table-and-collection-views)
and [prefetching upcoming cells](https://developer.apple.com/documentation/uikit/prefetching-collection-view-data),
with cancellation of unnecessary work. A small displayed avatar can still point
to a large original file; its visual size is not a reason to download an entire
list's images.

Lua applications can override `Application.afterPaint()` to update a shared
`orca.UIKit.ViewportObserver`. Custom run loops must invoke the hook after
successful `Window.Paint` dispatch, once layout and scroll transforms settle.

```lua
local Observer = require "orca.UIKit.ViewportObserver"
local observer = Observer.new { root = function() return app.screen end, margin = 0 }
local unsubscribe = observer:observe(frame, function(event)
    -- event.isVisible changes at viewport entry/exit; event.detached retires a view.
    -- Enqueue asynchronous work here; never fetch or decode inside layout.
end)
-- In afterPaint:
observer:update()
```

The observer intersects axis-aligned view bounds with clipping ancestors and
scroll/Pivot translations, suppresses hidden pages, and retains registrations
through Lua GC. `retainedRoot` can preserve a shell while another screen is
active. Unsubscribe completed work; detached registrations are removed. This
Lua helper does not calculate occlusion or arbitrary rotated 3D geometry.

Octopus post photos and Chat attachment previews load only when visible, with
fixed-size placeholders and two concurrent downloads per loader. Shared
`orca.UIKit.AvatarLoader` batches profile metadata for visible/nearby people,
prioritizes visible avatars, and prefetches within 128 points. It deduplicates
file requests and reuses cached files. Cached avatars can be applied immediately
without networking, even when rebuilding a retained page.

Queued requests with no visible consumers are skipped. An already running,
shared request may finish and populate the cache; session and view guards
prevent stale completions from changing another reader's UI. Failed requests
do not retry on every paint. Large photos offer explicit retry; avatar failures
leave initials and can retry when a new view is created. Cache lifetime is owned
by the app: Chat's private previews are temporary; Octopus can persist completed
guest image variants in its bounded cache while keeping preview snapshots
temporary. Neither cache is a list virtualization system.
The demo portraits are already 256-pixel JPEGs. Production uploads should provide
small avatar variants rather than assuming full-size originals are inexpensive.

## XML message shorthand

Common patterns:

- `{Screen.ShowModal Path=...}`
- `{Popup.ClosePopup}`
- `{Node.RightButtonUp}`

Use the shorthand when it is clearer than a full trigger/action block.

## Popup rules

- Prefer `Popup` roots for modal content.
- `Screen.ShowModal` should point to the popup asset path.
- Use `Popup.ClosePopup` to dismiss and optionally return data.

## Navigation rules

- Use `PageHost.ActivePage` bindings for peer selection such as footer tabs.
- Use `NavigationHost` for hierarchical push/pop flows above a tab shell.
- Push detail pages with `{NavigationHost.Push 'Pages/Detail.xml'}`; the triggering node's `DataContext` is inherited when the message context is unset.
- Pop with `{NavigationHost.Pop}` and bind navigation chrome to `NavigationHost.CanGoBack` when conditional back UI is needed.
- Pushed templates must have a `Page` root. Popups remain modal overlays and should not be used as navigation pages.

## Windows Phone controls

UIKit ships the Windows Phone 8 navigation controls under their original
names. Metrics follow the WP8 templates scaled to a 390-point portrait screen.
`apps/Metro` shows them in XML.

- `Pivot` with `PivotItem` children: a small-caps `Title`, then large
  lowercase headers. The selected header leads, the others follow dimmed and
  wrap around; a short strip keeps repeating them until it reaches the right
  edge. Tap a header or swipe the content; `SelectedIndex` and
  `SelectionChanged` report the item. `IsLocked` hides the other headers and
  disables swiping. Generated parts carry the `pivot-title` and
  `pivot-header` (`PivotHeaderItem`) CSS classes.
- `Panorama` with `PanoramaItem` children: an oversized lowercase `Title`
  scrolls with parallax over sections that are slightly narrower than the
  screen, so the next one peeks in. Swipes wrap from the last section to the
  first. `PanoramaItem.Orientation="Horizontal"` allows content wider than
  the screen, measured from the content's natural or explicit width. A swipe
  reveals the next viewport within that section before proceeding to the next
  section; backwards navigation enters its trailing edge. `SelectedIndex`
  and `SelectionChanged` refer to sections, so panning within a wide section
  keeps the same selection. This follows the original
  [Windows Phone wide-section contract](https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff941126(v=vs.105)).
  Use a normal `Grid` inside the item for portrait or photo matrices.
  Set `ContentInsetAdjustment="Always"` on an edge-to-edge panorama to keep
  its title below the iPhone safe area. Generated parts carry
  `panorama-title` and `panorama-header`.
- `ApplicationBar` with `ApplicationBarIconButton` (`IconUri`, `Text`,
  `IsEnabled`, `Click`) and `ApplicationBarMenuItem` (`Text`, `IsEnabled`,
  `Click`). Collapsed, it shows outlined round buttons and an ellipsis. Tap
  the bar to expand it upwards with lowercase labels and the menu. Tapping a
  command or anywhere outside collapses it. `Mode="Minimized"` keeps only the
  ellipsis strip. Its `Background` and `Foreground` default to the WP chrome
  and foreground colours for the current OS appearance.

```xml
<Grid Rows="1fr 58px">
  <Pivot Title="Messenger">
    <PivotItem Header="recent">...</PivotItem>
    <PivotItem Header="online">...</PivotItem>
  </Pivot>
  <ApplicationBar>
    <ApplicationBarIconButton IconUri="App/Icons/plus.svg" Text="new"/>
    <ApplicationBarMenuItem Text="settings"/>
  </ApplicationBar>
</Grid>
```

## Data-driven layout pattern

Separate visual data from layout structure by driving layout properties through data bindings rather than CSS classes or hardcoded attributes.

### Anti-pattern: layout encodes data

Before — each indent level required a distinct CSS class:

```xml
<!-- Screen layout — six individual placeholders, each with a CSS class for indent -->
<LayerPrefabPlaceholder Name="Line1" class="model-indent-root" .../>
<LayerPrefabPlaceholder Name="Line2" class="model-indent-child" .../>
<LayerPrefabPlaceholder Name="Line3" class="model-indent-grandchild" .../>
```
```css
.model-indent-root   { padding-left: 0; }
.model-indent-child  { padding-left: 16; }
.model-indent-grandchild { padding-left: 32; }
```

### Pattern: data drives layout

After — `GridBox` + `ItemsSource` + `ItemTemplate`, indent lives in data:

```xml
<!-- Screen layout — single GridBox, no CSS indent classes -->
<GridBox Columns="1fr"
  ItemsSource="Example/DataSources/ApplicationData:XmlModel"
  ItemTemplate="Example/Prefabs/XmlModelNode"/>
```

```xml
<!-- Prefab binds layout property to data -->
<StackView Name="XmlModelNode" Node.PaddingLeft="{Binding DataContext/Indent}">
  <ImageView Source="{Binding DataContext/Icon}"/>
  <TextBlock Text="{Binding DataContext/Title}"/>
</StackView>
```

```xml
<!-- Schema defines entity columns (Example/Data/ApplicationData.Schema.xml) -->
<Schema>
  <Entity Name="XmlModelLine">
    <Column Name="Icon"         Type="string"/>
    <Column Name="Title"        Type="string"/>
    <Column Name="PrimaryColor" Type="string"/>
    <Column Name="Indent"       Type="string"/>
  </Entity>
</Schema>
```

```xml
<!-- Data file — compact attribute syntax (no nested DataObject/DataObjectString) -->
<XmlModel>
  <XmlModelLine name="Line1" Icon="blocks.svg" Title="Structure" Indent="0"/>
  <XmlModelLine name="Line2" Icon="code.svg"   Title="Bindings"  Indent="16"/>
</XmlModel>
```

### When to use

- **Lists of items** where each item varies in layout properties (padding, color, visibility).
- **Tree or nested structures** where depth is a property of data, not of layout.
- **Data that may change** — binding keeps the layout static and lets data drive appearance.

### When to keep explicit layout

- **Unique, non-repeating sections** (hero, header, footer) — the layout *is* the structure, not a template.
- **Fixed-position overlays** where placement is absolute and unrelated to data.

## Good follow-up reads

- [Application architecture and state](app/index.md)
- [App styling](app/styling.md)
- `docs/architecture/message-actions.md`
- `docs/app/xml/triggers-actions-messages.md`
