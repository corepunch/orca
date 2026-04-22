# Link Component — Design Spec

## Problem

Navigation in UIKit/MoonScript samples currently requires threading a `navigate` closure
through every layout function and attaching `LeftButtonUp: -> navigate route` to each
individual node. This makes layouts verbose, non-declarative, and tightly coupled to the
app-level routing implementation.

Example of current boilerplate (Weather footer):

```moonscript
make_footer = (active_route, navigate) ->   -- navigate must be passed in
    StackView class: "flex-row justify-evenly", =>
        for item in *NAV_ITEMS
            ImageView
                Source: item.icon
                LeftButtonUp: -> navigate item.route   -- repeated on every nav item
```

## Design decision: standalone wrapper node, not attach-only component

Two approaches were considered:

### Option A — `Link` as a standalone wrapper node (chosen)

```moonscript
Link Destination: "/forecast", ->
    ImageView Source: "assets/icons/search.svg?width=48&mask=true"
```

- `Link` is a transparent `Node2D` subclass that wraps its children.
- Handles `Node.LeftButtonUp` internally — fires `Link.Navigate` message up the tree.
- App-level router (or `Application`) intercepts `Link.Navigate` and calls the routing
  implementation.
- No closure threading in layout code. Layouts become purely declarative.
- Mirrors the semantics of HTML `<a href>`: universal, immediately readable.

### Option B — `Link` as an attach-only component (rejected)

```moonscript
ImageView Source: "page-icon", ->
    Link Destination: "/some-page"     -- would be a child node, NOT a component on ImageView
```

This does not work with the current DSL. In the `->` / `=>` block, everything is a
**child scene node**, not an attached component. `attach-only` is enforced by
`OBJ_AddComponentByName`, which is never called by the DSL constructor path. `Link` would
become a child layout node inside `ImageView`, not an interceptor on it.

Attach-only makes sense for **orthogonal cross-cutting behaviors** (e.g. audio feedback on
any node type). Navigation intent is tightly bound to the tapped node, so wrapping is the
correct model.

## Specification

### Class

```xml
<class name="Link" parent="Node2D" xmlns="...">
    <summary>Transparent navigation wrapper. Fires Link.Navigate when tapped.</summary>
    <properties>
        <property name="Destination" type="string">Route path to navigate to</property>
    </properties>
    <handles>
        <handle message="Node.LeftButtonUp"/>
    </handles>
    <messages>
        <message name="Navigate" routing="Bubble">
            <summary>Fired when the Link is tapped. Bubbles up to app-level router.</summary>
            <fields>
                <field name="Destination" type="string">Target route path</field>
            </fields>
        </message>
    </messages>
</class>
```

### C handler (UIKit/Link.c)

```c
HANDLER(Link, Node, LeftButtonUp) {
    _SendMessage(hObject, Link, Navigate, .Destination = pLink->Destination);
    return TRUE;  // consume — don't also fire on parent nodes
}
```

### App-level router wiring

`Application` (or the sample router) listens for `Link.Navigate` via `SV_RegisterMessageProc`
or a top-level `LeftButtonUp`/message handler and dispatches to the routing implementation:

```lua
-- in application.lua or router
OBJ_AddHandler(screen, "Link.Navigate", function(msg)
    app:navigate(msg.Destination)
end)
```

### Usage after implementation

```moonscript
-- Weather footer — no navigate closure, fully declarative:
make_footer = (active_route) ->
    StackView class: "bg-violet-900 flex-row justify-evenly items-center p-2", =>
        for item in *NAV_ITEMS
            selected = active_route == item.route
            icon_color = selected and "text-foreground" or "text-muted-foreground"
            label_color = selected and "text-foreground" or "text-muted-foreground"
            Link Destination: item.route, ->
                StackView class: "w-12 flex-col items-center justify-center gap-1", ->
                    ImageView class: "align-middle-center #{icon_color}",
                        Source: "#{item.icon}?width=48&mask=true"
                    TextBlock class: "text-xs #{label_color}", item.label
```

## Naming rationale

| Name | Verdict |
|---|---|
| `Link` | ✅ universal, immediately understood |
| `Destination` | ✅ declarative, mirrors `href` semantics |
| `Link.Navigate` | ✅ clean, consistent with `PageHost.NavigateToPage` pattern |
| `Link.NavigateTo` | ❌ redundant — "Link" already implies navigation |

## Implementation checklist

Follow the mandatory component workflow:

1. Add `<class name="Link">` to `plugins/UIKit/UIKit.xml`
2. Run `cd tools && make` to regenerate `UIKit.h`, `UIKit_properties.h`, `UIKit_export.c`
3. Create `plugins/UIKit/Link.c` with `HANDLER(Link, Node, LeftButtonUp)`
4. Register `OBJ_RegisterClass(&_Link)` in `on_ui_module_registered`
5. Add `Link.c` to `orca.xcodeproj/project.pbxproj` (four places)
6. Wire app router to handle `Link.Navigate` in `share/orca/core/application.lua`
7. Replace `LeftButtonUp: -> navigate item.route` in Weather/Banking layouts with `Link Destination: item.route`
8. Add test: `tests/test_link.lua`
