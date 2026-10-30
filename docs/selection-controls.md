# Selection controls

Use `CheckBox` for an independent choice, `RadioButton` inside `RadioGroup`
for one choice from a set, and `ToggleSwitch` for an on/off setting. They
share native interaction and CSS styling, but represent different decisions.
The group is named **`RadioGroup`**, not `RadioButtonGroup`.

| Control | Use when | State to read or bind | Change event |
| --- | --- | --- | --- |
| `CheckBox` | A user selects any number of independent options, accepts a condition, or edits a draft preference | `IsChecked` (`bool`); optional `IsIndeterminate` (`bool`) | `CheckedChanged` |
| `RadioButton` + `RadioGroup` | A user chooses one mutually exclusive option, such as a sort order or delivery speed | Group `SelectedValue` (`string`), matching an option's `Value`; option `IsChecked` reflects selection | Group `SelectionChanged` |
| `ToggleSwitch` | A setting takes effect when switched, such as notifications on/off | `IsOn` (`bool`) | `Toggled` |
| `SelectionControl` | Shared engine implementation and inherited properties | `Text`, `IsEnabled`, `IndicatorSize`, `IndicatorSpacing` | No selection event of its own |
| Legacy `Input Type="Checkbox"` | Maintaining an existing input-based form | `Checked` (`bool`) | Existing Input/property binding behavior |

A checkbox can be left unselected independently of its neighbors. Selecting
a radio option clears the other options in its group; activating the selected
option does not clear it. A switch communicates an immediate setting change.
The engine updates the control state immediately in all three cases; the app
decides whether to save a draft later or apply a setting now. No control
persists data or calls a backend by itself.

For long lists of alternatives, use a more suitable list or picker rather
than presenting dozens of radios. Use a `Button` for a command and navigation
controls for changing pages; selection controls represent values.

## XML usage

Application pages and templates are authored in XML. These fragments belong
inside the app's existing `Screen`, `Page`, or template.

```xml
<StackView Spacing="12">
  <CheckBox Name="RememberMe" Text="Remember me" IsChecked="true"/>
  <CheckBox Name="Folders" Text="Some folders selected" IsIndeterminate="true"/>
  <ToggleSwitch Name="Notifications" Text="Notifications" IsOn="true"/>
  <RadioGroup Name="Delivery" SelectedValue="standard" Spacing="8">
    <RadioButton Text="Standard delivery" Value="standard"/>
    <RadioButton Text="Express delivery" Value="express"/>
    <RadioButton Text="Collection unavailable" Value="collection" IsEnabled="false"/>
  </RadioGroup>
</StackView>
```

`Text` is the built-in label: tapping the label or indicator activates the
same control. Do not add a second label with its own click handler. Use a
clear description of the setting or option. `IsEnabled="false"` prevents user
activation while retaining the displayed value; programmatic and bound
updates still work.

### Checkboxes and mixed values

`IsChecked` and `IsIndeterminate` are separate booleans. The mixed state is
useful for a parent selection such as “All folders” when some children are
selected. CSS displays the mixed bar in preference to the checkmark. The app
must compute the parent state and update the children; a checkbox does not
discover or aggregate child checkboxes automatically.

Activating a mixed checkbox sets `IsChecked=true` and clears
`IsIndeterminate`, producing one `CheckedChanged` event with the final pair.
Subsequent activations toggle the checked value. There is no automatic
unchecked → checked → mixed cycle.

### Radio groups

Put each `RadioButton` directly under its `RadioGroup`, and give each option a
stable, unique, nonempty `Value`. The group inherits `StackView` layout:
`Direction="Horizontal"` lays options in a row; the default direction is
vertical, and `Spacing` controls the gap. Give the set a heading using a
separate `TextBlock` outside the group.

```xml
<StackView Spacing="8">
  <TextBlock Text="Sort messages"/>
  <RadioGroup SelectedValue="newest" Direction="Horizontal" Spacing="20">
    <RadioButton Text="Newest first" Value="newest"/>
    <RadioButton Text="Oldest first" Value="oldest"/>
  </RadioGroup>
</StackView>
```

Use group `SelectedValue` as the application-facing source of selection.
Avoid binding the group value and each child's `IsChecked` to competing
sources. Separate groups own separate selections. A standalone radio has no
group to coordinate it; use a checkbox for an independent yes/no choice.

Setting `SelectedValue` checks matching direct children and clears the rest.
An unknown value, an empty value with the recommended nonempty option values,
or `nil` leaves no radio checked; the group does not automatically choose the
first option. Set an explicit valid initial value when a choice is required.
Disabled options can still be selected by the model: `IsEnabled` restricts
user input, not the stored value.

String bindings can normalize a model's `nil` to `""` when evaluating the
control value. Both clear the selection with nonempty option values; do not
use the distinction to represent separate application choices.

An initially checked radio attached to a group with no selected value adopts
its `Value`. An explicit group selection wins when a radio is attached or a
view is rebuilt. Setting a child's `IsChecked=true` selects it and updates
the group; explicitly clearing the selected child's `IsChecked` clears the
group value. Renaming a selected child's `Value` updates the group value;
changing an unselected value rechecks it against the current group selection.
Keep values unique so one value cannot select several options.

For dynamic options, attach radios directly and let the destination group
selection synchronize them. Before removing or moving the selected option,
clear its source group's `SelectedValue` (or select a replacement): removal
does not automatically clear the source value. A destination's explicit
selection wins over the moved option's previous checked state.

## Bindings and events

Bind editing controls with `Mode=TwoWay`; ordinary `{Binding …}` defaults to
`OneWay`, which reads data but does not write user changes back. The nearest
`DataContext` must provide writable fields of matching types: booleans for
checkboxes and switches, strings for the group value. Plain Lua tables are
not observable data sources. See [form bindings](app/xml/triggers-actions-messages.md#form-bindings)
and [data-driven screens](app/datasources.md).

```xml
<StackView>
  <CheckBox Text="Send me updates"
            IsChecked="{Binding DataContext/OptIn, Mode=TwoWay}"/>
  <ToggleSwitch Text="Notifications"
                IsOn="{Binding DataContext/NotificationsEnabled, Mode=TwoWay}"/>
  <RadioGroup SelectedValue="{Binding DataContext/DeliveryMethod, Mode=TwoWay}">
    <RadioButton Text="Standard" Value="standard"/>
    <RadioButton Text="Express" Value="express"/>
  </RadioGroup>
</StackView>
```

Use bindings to synchronize values and handlers for effects such as applying
a setting or validation. `CheckBox.CheckedChanged` carries `IsChecked` and
`IsIndeterminate`; `ToggleSwitch.Toggled` carries `IsOn`;
`RadioGroup.SelectionChanged` carries `SelectedValue` and `OldValue`.
These are direct events: attach the handler to the checkbox, switch, or
group itself, rather than expecting the event to bubble to an outer screen.

`SelectionChanged` reports a radio-led change, including user activation,
programmatically checking or clearing a radio, and changing the selected
radio's `Value`. Directly assigning or refreshing group `SelectedValue`
synchronizes the children without emitting this event. Its old/new strings
are snapshots of the change. Activating an already selected radio produces
no selection change event. Use the group's bound value for model state and
the event when reacting to a radio-led change.

For an XML controller, set `Controller` on the containing view and name its
handler in the attribute:

```xml
<ToggleSwitch Text="Notifications" Toggled="NotificationsToggled"/>
```

```lua
return {
    NotificationsToggled = function(self, args, sender)
        -- Apply args.IsOn through the app's settings/service layer.
    end,
}
```

### Lua API

Lua controllers can read and assign the same properties. Existing
programmatic views and focused tests can construct the same native objects:

```lua
local ui = require "orca.UIKit"

local check = ui.CheckBox {
    Text = "Remember me",
    CheckedChanged = function(sender, args)
        print(args.IsChecked, args.IsIndeterminate)
    end,
}
local toggle = ui.ToggleSwitch {
    Text = "Notifications",
    Toggled = function(sender, args)
        print(args.IsOn)
    end,
}
local group = ui.RadioGroup {
    SelectedValue = "standard",
    SelectionChanged = function(sender, args)
        print(args.OldValue, args.SelectedValue)
    end,
}
group:addChild(ui.RadioButton { Text = "Standard", Value = "standard" })
group:addChild(ui.RadioButton { Text = "Express", Value = "express" })

check.IsChecked = true
toggle.IsOn = false
group.SelectedValue = "express"
```

Setting properties uses the same state and binding machinery as user input.
Checkbox and switch events also fire for property changes, including model
refreshes; do not assume every callback came from a user. Their messages are
queued, so a headless test must pump messages before asserting callbacks.
Read event arguments for the event's state rather than assuming the sender
has not changed again before delivery. Avoid a handler that blindly writes
the same value back or starts persistence during initial view setup.

## Pointer and keyboard behavior

| Input | Checkbox | Switch | Radio option |
| --- | --- | --- | --- |
| Tap/click indicator or label | Toggle; mixed becomes checked | Toggle on/off | Select option |
| Space or Enter while focused | Same activation | Same activation | Select option |
| Right or Down arrow | No selection behavior | No selection behavior | Focus and select next enabled direct sibling, wrapping |
| Left or Up arrow | No selection behavior | No selection behavior | Focus and select previous enabled direct sibling, wrapping |
| Tab | Move to next tab stop | Move to next tab stop | Move to next tab stop |

Disabled controls reject pointer and keyboard activation and are removed
from ordinary tab navigation; radio arrow navigation skips disabled options.
Changing a disabled control through a binding or code remains possible.
Radios are individual tab stops; a group does not currently implement a
single roving Tab stop. A switch activates on release or keyboard input; it
does not provide a draggable or animated thumb gesture.

`:focus` identifies actual focus, while `:focus-visible` identifies keyboard
focus presentation. Prefer `:focus-visible` for the surrounding focus ring.
Touch has no persistent `:hover`; use `:active` for press feedback.

## CSS skins and generated parts

The native control owns behavior and builds an ordinary child tree; CSS owns
its appearance. This provides state-based skins without a WPF-style
`ControlTemplate` replacement API. Keep the native object and its bindings;
style the generated parts rather than replacing the control with a picture
and app-written click logic.

Normal startup loads `share/plugins/selection-controls.lua`, which registers
the shared `share/orca/UIKit/selection-controls.css` before application sheets
and assigns the bundled Lucide SVG to the checkmark. Custom startup/test
harnesses must load the startup plugins too if they need this default skin.

| Control | Part class | Role |
| --- | --- | --- |
| All three | `control-indicator` | Box, radio outline, or switch track |
| All three | `control-label` | Generated `TextBlock` displaying root `Text` |
| CheckBox | `control-check control-mark` | SVG checkmark (`VectorView`) |
| CheckBox | `control-indeterminate` | Mixed-state bar (`Node2D`) |
| RadioButton | `control-mark` | Selected circular dot (`Node2D`) |
| ToggleSwitch | `control-track` | Additional class on its indicator |
| ToggleSwitch | `control-thumb` | Moving circular thumb inside track |

The generated children ignore hit testing, so the labeled root remains the
activation target. Their names (`Indicator`, `Label`, `CheckMark`,
`Indeterminate`, `Mark`, `Thumb`) support inspection and focused tests. Treat
their classes as the styling interface; do not remove or reparent parts, add
application content inside them, or bind application state to them.

Supported states are `:checked`, `:indeterminate`, `:disabled`, `:enabled`,
`:hover`, `:active`, `:focus`, `:focus-visible`, and `:dark`. Switch `IsOn`
maps to `:checked`, so there is no `:on` pseudo-state. State changes restyle
descendants, and leaving a state restores the normal cascade. Mixed and
checked can both be true: keep mixed mark rules after checked mark rules.

```css
CheckBox, RadioButton, ToggleSwitch {
    min-height: 44;
    color: #111111;
    -orca-indicator-spacing: 12;
}
CheckBox .control-indicator {
    border: 2 solid #111111;
    border-radius: 4;
    background-color: transparent;
}
CheckBox:checked .control-indicator { background-color: #0078d4; }
CheckBox:checked .control-check { color: #ffffff; }
CheckBox:indeterminate .control-check { opacity: 0; }
CheckBox:disabled, RadioButton:disabled, ToggleSwitch:disabled { opacity: 0.4; }
CheckBox:focus-visible, RadioButton:focus-visible, ToggleSwitch:focus-visible {
    ring: 0 2 #0078d4;
}
RadioButton .control-mark { background-color: #0078d4; }
```

Scope a skin with an application class, such as
`CheckBox.settings-option .control-indicator`. Root constructor classes are
`selection-control` plus `checkbox`, `radio-button`, or `toggle-switch`, but
XML/class assignments can replace them. Type selectors remain stable. ORCA
matches types exactly, so `SelectionControl` does not match its subclasses;
list the three concrete types when styling all controls.

The shared skin uses checkboxes with 6-point outer corners and circular radios,
plus a Windows-style pill switch. Its switch has the same 2-point outline as
a radio, a 16-point thumb, and symmetric insets at both ends. Change the skin
through CSS, including dark and disabled states, without changing native
state behavior. Radio and switch geometry use styled nodes, so they need no
separate SVGs for each state. Replace a checkbox glyph when necessary with:

```css
CheckBox .control-check { -orca-source: url("MyApp/Icons/check.svg"); }
```

`VectorView` tints an SVG with its inherited foreground/CSS `color`; use a raw
Lucide SVG with `stroke="currentColor"`. There is no required
`-orca-tint-mode` declaration. Stylesheet ordering is global defaults then
application sheets; within a sheet, matching state rules override base rules
in source order. This is sheet/state priority, not full browser specificity.
See [CSS compatibility](css-compatibility.md) for supported values.

### Layout and sizing

The root retains TextBlock's `Text` and typography API; the generated label
draws the text. The control arranges the indicator on the left, vertically
centered, and the label in the remaining width. An empty label removes
indicator-to-label spacing. Use container layout and the root's margins for
row placement, rather than manually shifting the indicator and label.

The default root minimum height is 44 points. The shared indicator has a
24-point padding box and a 2-point border outside it, occupying 28 points.
The checkbox declares a 4-point radius; its border makes the outer radius 6.
The switch has a 52 × 24 padding box and the same border, occupying 56 × 28.
Its thumb uses auto margins and equal end insets. These measurements follow
ORCA's [box model](app/styling.md#layout-and-sizing), where borders add to
occupied size. CSS numeric lengths are bare numbers.

Prefer part `width`, `height`, `border-width`, `border-radius`, and `padding`
for a complete skin. Explicitly assigning `IndicatorSize` overrides CSS
indicator width/height during layout; on switches it also derives a wider
track. It does not proportionally scale every mark or thumb. For reliable
custom proportions, size the affected parts together in CSS and leave
`IndicatorSize` unset. `IndicatorSpacing` (default 12) controls the label gap
and maps to CSS `-orca-indicator-spacing`.

## Compatibility and verification

New interfaces should use the dedicated controls. Legacy
`Input Type="Checkbox"` uses `Checked`, a fixed input draw path and separate
input behavior. It does not create `control-indicator`/`control-label` parts,
offer mixed state, or gain the dedicated control's events simply by matching
`:checked`. `Input Type="Radio"` is also not a substitute for a native
`RadioGroup`. Preserve old forms when necessary, or migrate their `Checked`
binding to `CheckBox.IsChecked` and use its built-in `Text` label.

`RadioButton.AccentColor` is retained for source compatibility; CSS
`RadioButton .control-mark` now owns the visible dot color. Explicit legacy
`RadioButton.IndicatorSize` still overrides indicator sizing. Do not
instantiate `SelectionControl` as a selectable widget: concrete subclasses
build the parts and implement activation.

The native dark/light gallery is [apps/Controls](../apps/Controls/README.md):
run `build/bin/orca apps/Controls` after building. It shows normal, checked/on,
mixed, disabled and radio-group states. Check pointer and keyboard behavior
there, plus narrow widths and long labels in the consuming app. Focused
selection-control and XML gallery tests verify state, events and layout;
a native rendered frame is required to verify SVG artwork and appearance.

Run `make test-radio-group` for exclusive selection, initialization, dynamic
attachment, keyboard wrapping, disabled options and event snapshots.
`make test-selection-controls` checks the shared control behavior and CSS
parts; `make test-selection-gallery-xml` checks XML initialization, two-way
radio bindings and gallery layout. `make test-string-properties` includes the
native nullable-string regression used when clearing a radio selection.
