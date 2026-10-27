# Windows Phone page transitions

How Windows Phone animated navigation between pages, what each implementation
specified, and how ORCA reproduces it with `Pivot.NavigationTransition`,
`Pivot.TransitionMode` and `orca.UIKit.PageTransition`.

Labels on the findings:

- **[V]** read from a source fetched while researching (quoted or taken from code);
- **[D]** derived directly from verified code, such as a timeline assembled from constants;
- **[I]** inference or recollection, not established by a source.

## Summary

- Windows Phone navigation is **out, then in**. The page you leave plays its
  out animation, and only then does the page you reach play its in animation.
  The navigation frame does this for every navigation, so a page cannot forget
  its transition.
- The default phone transition is the **turnstile**: pages turn in perspective
  about a vertical hinge just left of the screen. Forward, the old page's right
  edge swings towards you (0° → 50°) and the new page swings in from 80°, its
  right edge first deep behind the screen. Back runs the other way. Both phases
  continue the same rotation, like a turnstile.
- The **feather** variant turns a page's elements one after another, from top
  to bottom, instead of the page as one plane.
- **Slide**, **continuum**, **swivel** and plain fades are the other navigation
  transitions.
- There are four independent implementations, and their numbers differ. They
  are compared below.

## Implementations compared

| Implementation | Stagger | Out (rotation, duration) | In (rotation, duration) | Easing | Hinge |
|---|---|---|---|---|---|
| WP7/WP8 **Windows Phone Toolkit** (Silverlight) | 40/50 ms | 0→+50°, 250 ms | -80°→0, 350 ms | ExponentialEase, exponent 6 | page: `CenterOfRotationX=0`; feathered elements: `-0.2` |
| **WP 8.1 Windows.UI.Xaml** (the OS framework; open-sourced in WinUI 3 `dxaml/phone`) | 33 ms | 0→+50°, 128 ms | -80°→0 over 428 ms after a 128 ms hold | ExponentialEase 6 | `CenterOfRotationX=-0.1`, `CenterOfRotationZ=-100` |
| **WinJS** 3/4, including WinJS on phone | 50 ms, capped at 1000 ms | `rotateY(0)`→`rotateY(-50deg)`, 128 ms | `rotateY(80deg)`→0, 300 ms | cubic-béziers | 40 px left of the viewport, at its vertical centre |
| Kinnara's fork of the toolkit | 33 ms | toolkit values | toolkit values | toolkit values | toolkit values |

XAML `PlaneProjection.RotationY = θ` is CSS `rotateY(-θ)` (see
[Projection math](#projection-math)). The toolkit's -80/+50 and WinJS's
80deg/-50deg therefore describe the same motion. [D]

## Sequencing

- **Toolkit `TransitionFrame`** [V], from `Transitions/TransitionFrame.cs`:
  1. `OnNavigating` starts the old page's out transition, marks the old
     presenter `IsHitTestVisible=false` and subscribes
     `OnExitTransitionCompleted`.
  2. `OnContentChanged` puts the new page into a second presenter at opacity 0.
  3. `OnExitTransitionCompleted` collapses and empties the old presenter, and
     only then begins the new page's in transition.

  A forward turnstile takes about 250 + 350 = 600 ms plus layout. [D]
- **WinJS phone navigator** [V], from the WP 8.1 HTML sample in
  `oldnewthing/old-Windows8-samples`:
  `animations.exit(host.children).then(render).then(() => animations.entrance(host.children))`.
  This is also strictly sequential.
- **WP 8.1 XAML** [V for the keyframes, D/I for the timing]:
  - The out storyboard is built on Unload and the in storyboard on Load, both
    from the **new** page's `NavigationThemeTransition`. `LayoutTransition.cpp`
    says the transition on the "from" page is never used.
  - Each in storyboard holds invisible for exactly the out duration: 128 ms for
    turnstile, 250 ms for slide, 267 ms for continuum. The result is still
    out, then in, with the handoff fixed in advance.
  - Every storyboard sets `IsHitTestVisible=false` at time 0. A page in
    transition takes no input.

## Turnstile

### Windows Phone Toolkit (WP7.1, WP8)

Page storyboards in `Microsoft.Phone.Controls.Toolkit/Transitions/Storyboards/`
animate `(UIElement.Projection).(PlaneProjection.RotationY)`. The hinge is
`new PlaneProjection { CenterOfRotationX = 0 }`, the page's left edge. [V]

| Storyboard | RotationY | Easing | Opacity |
|---|---|---|---|
| TurnstileForwardIn | -80 → 0 in 0.35 s | ExponentialEase EaseOut, exponent 6 | 0, then 1 at 0.01 s |
| TurnstileForwardOut | 0 → 50 in 0.25 s | EaseIn, 6 | 1 until 0.24 s, 0 at 0.25 s |
| TurnstileBackwardIn | 50 → 0 in 0.35 s | EaseOut, 6 | 0, then 1 at 0.01 s |
| TurnstileBackwardOut | 0 → -80 in 0.25 s | EaseIn, 6 | 1 until 0.24 s, 0 at 0.25 s |

### WP 8.1 (Windows.UI.Xaml, `CommonNavigationTransitionInfo`)

Constants from WinUI's `dxaml/phone/lib/NavigateTransitionHelper.h`: [V]

```cpp
TURNSTILE_EASE = 6;  TURNSTILE_OFFSET_IN = 80;  TURNSTILE_OFFSET_OUT = 50;
TURNSTILE_AXIS_X = -0.1;  TURNSTILE_AXIS_Z = -100.0;
TURNSTILE_MID_TIME = 128;  TURNSTILE_END_TIME = 128 + 428;
TURNSTILE_FEATHER_OFFSET_IN = 33;  TURNSTILE_FEATHER_OFFSET_OUT = 33;
TURNSTILE_FEATHER_SCALE_IN = 0.5;  TURNSTILE_FEATHER_SCALE_OUT = 0.5;
```

| Trigger | RotationY | Opacity |
|---|---|---|
| Forward in | -80 held to 128 ms, → 0 at 556 ms, ExpEaseOut(6) | 0 until 128, 1 at 129 |
| Forward out | 0 → +50 over 0–128 ms, ExpEaseIn(6) | 1 until 128, 0 at 129 |
| Back in | +50 held to 128 ms, → 0 at 556 ms, ExpEaseOut(6) | 0 until 128, 1 at 129 |
| Back out | 0 → -80 over 0–128 ms, ExpEaseIn(6) | 1 until 128, 0 at 129 |

### WinJS

From `src/js/WinJS/Animations.js`. The four functions are identical in WinJS
3.0 and master. [V]

| Function | Transform | Duration | Transform timing | Opacity |
|---|---|---|---|---|
| turnstileForwardIn | `perspective(600px) rotateY(80deg)` → `rotateY(0deg)` | 300 ms | `cubic-bezier(0.01,0.975,0.4775,0.9775)` | 0→1, `cubic-bezier(0,2,0,2)` |
| turnstileForwardOut | `perspective(600px) rotateY(0deg)` → `rotateY(-50deg)` | 128 ms | `cubic-bezier(0.4925,0.01,0.7675,-0.01)` | 1→0, `cubic-bezier(1,-0.42,0.995,-0.425)` |
| turnstileBackwardIn | `perspective(600px) rotateY(-50deg)` → 0 | 300 ms | as ForwardIn | as ForwardIn |
| turnstileBackwardOut | `perspective(800px) rotateY(0deg)` → `rotateY(80deg)` | 128 ms | as ForwardOut | as ForwardOut |

- The opacity béziers behave as steps [D]:
  - `(0,2,0,2)` overshoots at once and clamps, so it is a step to 1 at the start.
  - `(1,-0.42,0.995,-0.425)` is a step to 0 at the end.

  These are the CSS form of the XAML discrete opacity keyframes.
- All four use `delay: staggerDelay(0, 50, 1, 1000)`, which means
  `delay(i) = min(50·i, 1000)` for the i-th element of the array passed in. [V]
- The transform origin is computed per element so that every element hinges on
  one line: [V]

  ```js
  var offsetLeftLTR = -(40 + itemBoundingBox.left);
  var totalOffsetY = (_Global.innerHeight / 2) - itemBoundingBox.top;
  ```

  The hinge is a vertical line 40 px left of the viewport (right of it in RTL),
  and the vanishing point is at the viewport's vertical centre.
- `createPageNavigationAnimations(current, next, movingBackwards)` in 3.0 [V]:
  - On phone it pairs `exit = <type><Forward|Backward>Out` with
    `entrance = <type>…In` for turnstile and continuum.
  - Slide is paired specially: forward is `exit: empty, entrance: slideUp`, and
    back is `exit: slideDown, entrance: empty`.
  - Desktop, and master (4.x), always use `enterPage`.

The 128 ms out duration equals WP 8.1's `TURNSTILE_MID_TIME`. WinJS's 300 ms in
is shorter than the OS's 428 ms. [D]

## Feather (staggered turnstile)

Shawn Oster announced the feather in the toolkit's September 2012 release [V]:

> "FeatherTransition - Another common effect seen on Windows Phone is a
> 'feathering' in of the controls on a page such as when you launch the mail
> app. Notice how the items on the page feather in from top to bottom."

### Toolkit `TurnstileFeatherEffect`

From `Effects/TurnstileFeatherEffect.cs` [V; timeline D]:

| Mode | Start of element k | RotationY | Duration | Easing | Opacity |
|---|---|---|---|---|---|
| ForwardIn | 40·k ms | -80 → 0 | 350 ms | ExpEaseOut(6) | appears at its start |
| ForwardOut | 50·k ms | 0 → +50 | 250 ms | ExpEaseIn(6) | disappears at its end |
| BackwardIn | 50·k ms | +50 → 0 | 350 ms | ExpEaseOut(6) | appears at its start |
| BackwardOut | 40·k ms | 0 → -80 | 250 ms | ExpEaseIn(6) | disappears at its end |

- **No cap.** The total is `delay·(n-1) + duration`. The page itself is not
  rotated.
- **Order:**
  - Elements are sorted by ascending attached `FeatheringIndex` (default -1,
    which means not feathered), keeping only those on screen.
  - A `ListBox` or `LongListSelector` expands into its items in the viewport,
    top to bottom. A `Pivot` expands into its title, then its header strip.
  - `k` counts animated elements, not index values.
- **Axis:**
  - The hinge is `CenterOfRotationX = -0.2`, i.e. 20% of the element's own
    width to the left of its left edge.
  - Each element is translated to the screen's vertical centre and moved back
    with `GlobalOffsetY`. All elements therefore share the screen's vertical
    vanishing point, as if they were parts of one turning page.

### WP 8.1 (`IsStaggeringEnabled` + `IsStaggerElement`)

[V for the code; D for the timeline]

| Trigger | Element k | Easing actually used |
|---|---|---|
| Forward in | -80 until 64+33k ms, → 0 at 278+33k ms (214 ms) | ExpEaseOut(6) |
| Forward out | 0 at 33k ms, → +50 at 64+33k ms (64 ms) | linear |
| Back in | +50 until 64+33k ms, → 0 at 278+33k ms | linear |
| Back out | 0 at 33k ms, → -80 at 64+33k ms | ExpEaseIn(6) |

- The stagger is **33 ms with no cap**. Base times are halved
  (`FEATHER_SCALE = 0.5`).
- The page itself is not rotated. Its opacity steps at 129 ms, which cuts off
  later out-elements.
- Two of the four branches pass a null easing, so they run linear. This looks
  like a bug.
- Stagger elements run in the order `IsStaggerElement` was set, filtered to
  those intersecting the page.
- Kinnara's toolkit fork changed all four toolkit delays to 33 ms in 2014,
  matching WP 8.1.

## Slide

- **WP 8.1 `SlideNavigationTransitionInfo`** (vertical) [V]:
  - In: TranslateY 200 held to 250 ms, → 0 at 600 ms, ExpEaseOut(6). Opacity 1
    at 250 ms.
  - Forward out: no motion; opacity 0 at 250 ms.
  - Back out: TranslateY 0 → 200 over 600 ms, ExpEaseIn(6); opacity 0 at
    250 ms.
  - Back in: opacity only.
- **WinJS** [V]:
  - `slideUp`: `translate(0,200px)` → 0, 350 ms,
    `cubic-bezier(0.17,0.79,0.215,1.0025)`. Opacity is a step-in, staggered
    34 ms.
  - `slideDown`: 0 → `translate(0,200px)`, 250 ms,
    `cubic-bezier(0.3825,0.0025,0.8775,-0.1075)`. Opacity uses
    `cubic-bezier(1,-0.42,0.995,-0.425)`, i.e. step-out.
- **Toolkit `SlideTransition`** [V]. ExpEaseOut(6) on translate and opacity:

  | Storyboard | Translate | Duration |
  |---|---|---|
  | SlideUpFadeIn | Y 200→0 | 350 ms |
  | SlideUpFadeOut | Y 0→-200 | 250 ms |
  | SlideDownFadeIn | Y -200→0 | 350 ms |
  | SlideDownFadeOut | Y 0→200 | 250 ms |
  | SlideLeftFadeIn | X 200→0 | 500 ms |
  | SlideLeftFadeOut | X 0→-200 | 300 ms |
  | SlideRightFadeIn | X -200→0 | 500 ms |
  | SlideRightFadeOut | X 0→200 | 300 ms |

## Fade

WinJS [V]:

- `fadeIn`: opacity 0 → 1, 250 ms, linear.
- `fadeOut`: opacity → 0, 167 ms, linear.

Windows Phone had no navigation transition that was only a fade.

## Continuum

The title of a tapped item flies out, and the next page's title flies in.

- **WP 8.1 `ContinuumNavigationTransitionInfo`** [V]:
  - Forward in starts at 267 ms:
    - The page scales 0.9 → 1 over 267–617 ms (CircleEase).
    - The entrance element turns RotationX -50 → 0 (PowerEase 2) and moves
      TranslateY 200 → -40 at 367 ms (ExpEaseOut 8), then → 0 at 617 ms. It
      scales 1.5 → 1, and its opacity goes 0 → 1 over 267–434 ms.
  - Forward out: the exit element turns RotationX 0 → 80, moves TranslateY
    0 → 200 and scales 1 → 1.5 by 120 ms (ExpEaseIn). The page fades from
    120 to 250 ms.
  - Back in: RotationX -90 → 0 and TranslateY -20 → 0 over 267–467 ms
    (ExpEaseOut 4).
  - Back out: the page moves TranslateY 0 → 200 by 250 ms (ExpEaseIn 6).
- **WinJS** `continuumForwardIn` [V]:
  - The page goes `scale(0.5)` → 1 over 350 ms.
  - The item root goes `translate(0,225px)` → 0.
  - The item content goes `rotateX(80deg) scale(1.5)` → identity, with origin
    `0px 50%`.

  The out and back variants are 120–250 ms (see `Animations.js`).
- The toolkit has no continuum. Colin Eberhardt's "flying titles" reconstructs
  it: 300 ms out, 200 ms in, SineEase, a curved path from X +500 and Y +50.

## Swivel

Swivel is for dialogs and pickers, not pages. Toolkit values [V]:

| Storyboard | RotationX | Easing | Opacity |
|---|---|---|---|
| SwivelForwardIn | -45 → 0, 0.35 s | ExpEaseOut 6 | 1 |
| SwivelForwardOut | 0 → 90, 0.25 s | ExpEaseIn **15** | off at 0.25 s |
| SwivelBackwardIn | -45 → -30 at 0.2 s (linear), → 0 at 0.35 s | ExpEaseOut 6 | 1 from 0.21 s |
| SwivelBackwardOut | 0 → 60, 0.25 s | ExpEaseIn 6 | off at 0.25 s |

Kinnara's fork uses `SwivelIn` (-45 → 0, 0.3 s) and `SwivelOut` (0 → 60, 0.15 s).

## Projection math

- **ExponentialEase** [V, WPF source]:
  - EaseIn: `f(t) = (e^(a·t) − 1) / (e^a − 1)`.
  - EaseOut: `1 − f(1 − t)`.
- **PlaneProjection** [V, WinUI `PlaneProjection.cpp`]:
  - Field of view 57°, near plane 1, far plane 1001.
  - The camera looks at the centre of the projected element's layout.
  - Chain: `CenterOfRotation`, then RotateX, RotateY, RotateZ, then undo the
    centre, then GlobalOffset, then the perspective.
  - That is roughly CSS `perspective(1000px)` about the element's centre. [I]
- **Sign** [V/D]:
  - WinUI composes RotationY as `CreateFromAxisAngle((0, -1, 0), θ)`, so XAML
    `RotationY = θ` is CSS `rotateY(-θ)`.
  - `CenterOfRotationX` is relative to the element's width (0 = left edge).
    `CenterOfRotationZ` is in pixels; negative values are behind the plane.
- **CSS composition**:
  - `perspective()` inside `transform` applies about `transform-origin`. WinJS
    moves the origin to the shared hinge; the toolkit moves each element to the
    screen's centre instead.
  - Both put every part on one turning page.

## Third-party investigations

- **Colin Eberhardt, "Metro In Motion"** (Scott Logic, 2011):
  - Part 2, "Peel", is the WP7 app-exit effect: 50 ms per element, 0.3 s
    SineEase, `CenterOfRotationX = -0.1`, RotationY to
    `-(180 − atan(1000/(w/2)))`, RotationZ 23. He wrote: "I think what I have
    come up with is close, but noticeably different."
  - Part 3, "Flying titles", is a WP7 continuum.
  - Parts 1 and 4–8 have no turnstile values.
- **CodeProject, "Windows Phone 7 Turnstile Control for Silverlight"** (2010).
  Only search snippets were reachable. It covers the Start-screen tile
  turnstile, makes the perspective relative to the whole screen and adds
  randomness.
- **`Chinmay-KB/wp_pivot_flutter`**, a recent Flutter reconstruction. It took
  its structure from Windows Phone Toolkit 4.2013.8.16 and notes its own
  emulator captures are "not a fitted timing curve".
- No frame-accurate measurement of the native OS feather was found. Telerik,
  Kevin Marshall and Jeff Wilcox turned up nothing with numbers.

## ORCA implementation

### API

- **`TransitionType`** (UIKit) is `None`, `Slide`, `Fade` or `Turnstile`: the
  kind of motion.
- **`TransitionMode`** is the phase: `ForwardOut`, `ForwardIn`, `BackwardOut`,
  `BackwardIn`, or `None`.
- **`Pivot`**, ORCA's Windows Phone page, has:
  - `NavigationTransition`, a `TransitionType`. It defaults to `Turnstile`, the
    phone's default.
  - `TransitionMode`. Setting it plays that phase from the next frame:
    - In modes return to `None` once the page has arrived.
    - Out modes hold the page away until the mode changes.
    - `TransitionCompleted` fires at the end.
    - While a mode plays, the page takes no input, as on Windows Phone.
- **`orca.UIKit.PageTransition`** plays the frame's role:
  - `leave(screen, direction, done)` sets the visible page's mode to
    `<direction>Out` and calls `done` on `TransitionCompleted`.
  - `enter(screen, direction)` sets the next page to `<direction>In`. Without a
    direction it sets `None`, which returns a retained page that was left
    turned away.
- **Apps** navigate with `App:show(route, direction)`:

  ```lua
  show = function(self, route, direction)
      local request = {}
      self.pendingShow = request
      local leaving = PageTransition.leave(self.screen, direction, function()
          if self.pendingShow == request then self:navigate(route, direction) end
      end)
      if not leaving then return self:navigate(route, direction) end
  end,
  ```

  Chat and Octopus go `"Forward"` into a post, profile, conversation or
  conversation info, and `"Backward"` out of one. Tab switches have no
  direction. A newer navigation supersedes one still waiting for its page to go
  out.

### Values

The motion is in `plugins/UIKit/PageTransition.c`.

- **Turnstile.** The WinJS values, for curves that CSS and our matrices share:
  - In: 300 ms, `rotateY(80deg)` (forward) or `-50deg` (back) to 0.
  - Out: 128 ms, to `-50deg` (forward) or `80deg` (back).
  - Perspective 600 points, or 800 for back-out.
  - Hinge 40 points left of the page, at its vertical centre.
  - The WinJS béziers for motion and opacity.
- **Feather.** Windows Phone 8.1's 33 ms per element, mapped onto position:
  - A part waits 33 ms for every 80 points it starts below the top of the page,
    roughly one list row.
  - The lowest part of an 844-point page therefore starts about 350 ms after
    the top, close to the six to ten elements a phone page staggered.
  - The y-proportional lag keeps a row's avatar and text together, which an
    element count cannot.
- **Slide.** WinJS's navigation pairing:
  - Forward in slides up from 200 points (`slideUp`) over an instant exit.
  - Back out slides down 200 points (`slideDown`), and the previous page
    returns at once.
- **Fade.** WinJS `fadeIn` (250 ms) and `fadeOut` (167 ms), linear.

### Parts

Windows Phone feathered elements the app marked, and expanded lists into their
visible items. ORCA finds parts itself:

- A node with a background, border or backdrop filter is one part, and its
  content goes with it.
- So is a node without child nodes.
- An undecorated container is split into its children.

Rows, cards, frosted bars and the Pivot's title, back button and header each
turn as one piece. Each part turns about the page's shared hinge, so the parts
still read as one page.

The Pivot lays its subtree out in place, then applies each part's perspective
matrix and opacity to that part and its descendants. Node2D hit testing solves
for the point on a node's plane, so it stays exact under perspective.

### Deviations

- In comes strictly after out, by `TransitionCompleted`, as the Toolkit and
  WinJS do. WP 8.1 instead started both at once with a fixed hold; the result
  looks the same.
- The turnstile uses WinJS's 300 ms in, not the OS's 428 ms, and WinJS's
  béziers rather than ExponentialEase 6. Each part's motion is not halved as in
  WP 8.1's feather.
- The out page is not cut off at 129 ms. Every part finishes its own exit.
- The application bar and other content outside the Pivot do not move, as the
  Windows Phone application bar did not.

### Testing

- `tests/test_metro.lua` covers the default type, every type's out and in
  phases, holding, completion and input.
- The Chat and Octopus UI tests check that navigation plays the leaving page
  out and the next page in.
- Test pumps run the animation frames in real time, so each navigation adds its
  transition's duration to the test.

## Sources

Windows Phone Toolkit:

- <https://github.com/microsoftarchive/WindowsPhoneToolkit> (master `4e4fd5d`):
  - `Microsoft.Phone.Controls.Toolkit/Effects/TurnstileFeatherEffect.cs`
  - `Microsoft.Phone.Controls.Toolkit/Transitions/` (`TransitionFrame.cs`, `Transitions.cs`, `Storyboards/*.xaml`)
  - `PhoneToolkitSample/Samples/TurnstileFeatherEffectSample{1,2}.xaml`
- <https://raw.githubusercontent.com/jesperll/PhoneToolkit/master/WindowsPhone71/Microsoft.Phone.Controls.Toolkit/Transitions/Storyboards/TurnstileForwardIn.xaml>
- <https://github.com/Kinnara/WPToolkit> (branch `custom`; delays changed in `c382543`)
- <https://raw.githubusercontent.com/shawnoster/whiskeyandcode/master/content/posts/windows-phone-toolkit-september-2012-release.md>

WP 8.1 Windows.UI.Xaml, in WinUI 3:

- <https://raw.githubusercontent.com/microsoft/microsoft-ui-xaml/main/dxaml/phone/lib/ThemeTransitions.cpp>
- <https://raw.githubusercontent.com/microsoft/microsoft-ui-xaml/main/dxaml/phone/lib/NavigateTransitionHelper.h>
- <https://raw.githubusercontent.com/microsoft/microsoft-ui-xaml/main/dxaml/phone/lib/NavigateTransitionHelper.cpp>
- <https://raw.githubusercontent.com/microsoft/microsoft-ui-xaml/main/dxaml/xcp/core/core/elements/LayoutTransition.cpp>
- <https://raw.githubusercontent.com/microsoft/microsoft-ui-xaml/main/dxaml/xcp/components/transforms/PlaneProjection.cpp>
- <https://raw.githubusercontent.com/microsoft/microsoft-ui-xaml/main/dxaml/xcp/components/graphics/ExpressionHelper.cpp>
- <https://raw.githubusercontent.com/MicrosoftDocs/winrt-api/docs/windows.ui.xaml.media.animation/commonnavigationtransitioninfo.md>

WinJS:

- <https://github.com/winjs/winjs> (`src/js/WinJS/Animations.js`, `Animations/_Constants.js`, `Animations/_TransitionAnimation.js`)
- <https://raw.githubusercontent.com/winjs/winjs/release/3.0/src/js/WinJS/Animations.js>
- <https://raw.githubusercontent.com/oldnewthing/old-Windows8-samples/master/Tiles/js/WindowsPhone/js/default.js>

Easing:

- <https://raw.githubusercontent.com/dotnet/wpf/main/src/Microsoft.DotNet.Wpf/src/PresentationCore/System/Windows/Media/Animation/ExponentialEase.cs>

Third-party write-ups:

- <https://raw.githubusercontent.com/ScottLogic/blog/gh-pages/_posts/2011-03-27-metro-in-motion-part-2-peel-animations.html>
- <https://raw.githubusercontent.com/ScottLogic/blog/gh-pages/_posts/2011-04-04-metro-in-motion-3-flying-titles.html>
- <https://raw.githubusercontent.com/Chinmay-KB/wp_pivot_flutter/master/research/components/application-bar/README.md>
