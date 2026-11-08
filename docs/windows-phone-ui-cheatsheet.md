# Windows Phone UI cheatsheet for ORCA

Reviewed 2 November 2026. Use this for Windows Phone / Metro applications;
it is not the default visual style for every ORCA app. For Chat, also read
[its product decisions](../apps/chat/DESIGN.md) and the
[screen review](../apps/chat/docs/ui-review-2026-11-02.md).

## Choose the reference generation first

| Reference | Use it for | Keep separate |
| --- | --- | --- |
| Phone 7 / 7.1, Silverlight | Content-led hierarchy, large type, Panorama hubs, hardware Back | Original screen dimensions and prerelease guidance are not modern phone points |
| Phone 8, Silverlight | Pivot rules, theme resources, familiar Metro controls | An SDK margin resource is not a universal page-margin prescription |
| Phone 8.1, WinJS / WinRT | Adaptive fields and compact/minimal command bars | Identify the framework; not every template matches Silverlight |
| Windows 10 Mobile / UWP | Effective pixels, adaptive layouts, updated type ramp and control semantics | UWP includes desktop/tablet guidance; this is not a Phone 8 skin |
| Current Windows / WinUI | Accessibility and general layout checks | Current pages can redirect to Windows 11; do not import its type ramp, NavigationView preference or rounded styling as historical Phone rules |

Microsoft's [WP7 guide announcement][wp7-announcement] establishes the original
design reference. Its old download now redirects away; a
[Microsoft-authored July 2010 v2 guide mirror][wp7-guide] remains available.
Treat that guide as prerelease historical material. Microsoft's
[Silverlight-to-UWP migration article][migration] explicitly maps Panorama
to Hub, showing that the generations are not interchangeable.

Labels used below: **Historical** is sourced period guidance; **Current
Microsoft** is a maintained cross-platform reference; **ORCA** is engine
behavior; **Chat** is an established product decision; **Recommendation**
is our design judgment.

## Which navigation container?

| Need | Choose | Avoid / reason |
| --- | --- | --- |
| Views of related data, e.g. all / unread / pinned mail | Swipeable `Pivot` + `PivotItem` | Unrelated tasks or sequential form steps |
| A browsable overview with different kinds of content, e.g. recent people + directory | `Panorama` + `PanoramaItem`, then drill into a page | Making the entire app one giant Panorama |
| A detail, edit, confirmation or creation flow | Separate page with Back | Giving each step a swipeable Pivot tab |
| Consistent Metro title on a single Chat page | Existing `C.page` / locked, single-item `Pivot` | Calling this a historical recommendation for editable Pivot pages |
| Root destinations that retain independent state | Existing Chat `PageHost` shell | Putting root destinations in a back-and-forth task stack |
| Reusable declarative push/pop navigation in a new ORCA app | `NavigationHost`, `Push`, `Pop`, `CanGoBack` | Replacing Chat's established router just to adopt another API |
| Short blocking decision | `Popup`, with explicit outcome actions | A modal for ordinary browsing or long forms |
| Full photo inspection | Immersive fitted image + clear Back | A Panorama or ordinary form-card layout |

**Historical Phone 8 Pivot:** use four or fewer related views, short one- or
two-word headers, and retain an empty view when it can become populated.
Do not nest Pivot/Panorama, use Pivot for a task sequence, or put editable or
competing horizontal-gesture controls in a swipeable Pivot.
Vertical lists remain appropriate. See [Pivot design guidelines][wp8-pivot].

**Historical Panorama:** it is a broad horizontal canvas, unlike Pivot's
switching between views. Our recommendation is a curated overview linking
to focused tasks; this is an interpretation of the
[Panorama API][wp8-panorama] and Microsoft's
[design-principles discussion][wp-design-principles]. A horizontal
`PanoramaItem` can contain wider content and stop within that section before
advancing ([orientation contract][panorama-orientation]).

**ORCA gesture ownership:** let Panorama own its horizontal movement. Use
`Orientation="Horizontal"` on a wide section rather than adding a nested
horizontal scroller. Keep ordinary directory lists vertical. A search/form
inside a hub deserves a specific gesture and keyboard review; route complex
editing to another page. Chat's Contacts is an intentional recent/all hub;
its auth, Settings and creation flows are not candidates for Panorama.

Windows 10 used a different control vocabulary. Current
[UWP Pivot documentation][uwp-pivot] also contains Windows 11 migration
advice; its warning is not a reason to remove Pivot from a deliberate Phone
7/8 design.

## Units, gutters, padding and spacing

**Do not copy screenshot pixels into layout.** Historical Phone canvases,
UWP effective pixels, ORCA logical units and physical display pixels are
different. `390 / 480` is a possible visual scaling decision, not a touch
target conversion or a Microsoft rule. ORCA's Metro metrics are maintained
constants, not a uniform scale applied to every SDK resource.

### Historical values, with their actual scope

| Phone 8 SDK resource | Value |
| --- | --- |
| `PhoneHorizontalMargin` / `PhoneVerticalMargin` | `12,0` / `0,12` |
| `PhoneMargin` | `12` |
| `PhoneTouchTargetOverhang` / `PhoneTouchTargetLargeOverhang` | `12` / `12,20` |
| `PhoneTextBoxInnerMargin` / `PhonePasswordBoxInnerMargin` | `1,2` / `3,2` |
| `PhoneBorderThickness` / `PhoneStrokeThickness` | `3` / `3` |

These are [theme resource values][wp8-theme], not an instruction to add 12
at every nested level. A control's own margin and a parent's inset can add.
Verify the resulting visible edge.

**Current Microsoft layout guidance:** use a four-effective-pixel increment
for measured dimensions; use 12 epx gutters below 640 epx and 24 for larger
windows. This maintained [layout page][layout] is a useful adaptive reference,
not proof of the precise Phone 7 template. Chat's six-point photo gap is an
explicit exception, not an error to round to eight.

### Current ORCA / Chat metrics

| Pattern | Logical units | Owner / interpretation |
| --- | --- | --- |
| Metro leading/trailing inset | 12 | `plugins/UIKit/Metro.h`; Pivot arranges the item inside it |
| Pivot title / selected header | 15 / 44 font size | `Pivot.c`; title top 14, title-to-header offset 2, header-to-hairline 8, hairline-to-content 20; the Pivot owns that 20 for page content and docked bars, as the PivotItem template margin did |
| Pivot header spacing | 18 | Engine header-strip metric, not general form spacing |
| Panorama title / section header | 110 / 36 font size | `Panorama.c`; section width factor 0.875 creates a next-section hint |
| ApplicationBar normal / minimized height | 58 / 24 | `ApplicationBar.c`; system inset is additional |
| App-bar button row / glyph | 38 / 28 | `ApplicationBar.c`; ORCA decision: plain glyphs, no Windows Phone circle |
| Header Back ring / glyph / minimum target | 30 / 16 / 44 | Visual artwork is smaller than its hit area |
| Ordinary Chat button / input | 48 outer height; 15 text | `views/components.lua`; border counts outside the padding box |
| Ordinary icon action | 44 target; 20 glyph | `C.iconButton` |
| Command row | 56 high; 52 icon column; 14 gap; 20 label | `C.command`; whole command is actionable |
| New-message / search result row | 64 high; 52 avatar; 14 gap | Six-point top/bottom inset; 20/14 name/detail text |
| Contacts directory row / list gap | 56 high; 52 avatar; 12 list gap | Current Contacts template, distinct from New message |
| Inbox row | 76 high; 56 avatar; 14 column gap | 22/14/12 title/preview/time text |
| Settings header | 96 avatar; 16 gap; 28/15 text | Established Chat hierarchy |
| Settings grouping | 14 between groups; 6 within groups | Current implementation, not a global spacing token |
| Auth | 24 horizontal / 28 vertical inset; 16 form gap; 20 group gap | Branded sign-in exception; 52 outer controls, 16 input text |
| Section label / footnote | 18 / 13 font size | Current shared components |
| Character grid | Four equal columns; 6 horizontal and vertical gaps; 4 body top/bottom padding | User decision, 2 November 2026; header owns the 12-point side inset |
| Viewer | 48 header/back target; 22 title | Immersive exception; image fits its available area |
| Visible rectangular corner | 6 radius | Chat choice; bordered controls declare 4 with outside 2-point border |
| Composer | 58 content height; 48 main controls; 5 above/below | Safe-area and keyboard insets are added independently |

The table records current ownership. It does **not** endorse every existing
number as a default: the [review](../apps/chat/docs/ui-review-2026-11-02.md)
records inconsistent row density and smaller auxiliary hit targets.

**Recommendation for new work:** reuse the closest established pattern.
Use the six-point gap for the character grid; use the shared contact row for
people; use one form spacing rhythm throughout a flow. Do not halve every
screen's spacing because one photo grid was made denser. Give extra space
between groups rather than between every label and its value.

### ORCA box-model traps

| Task | Correct ORCA expression / check |
| --- | --- |
| XML/Lua edge shorthand | `Padding="12 0"` means horizontal 12, vertical 0; four values are left/top/right/bottom |
| CSS edge shorthand | `padding: 12 0` means vertical 12, horizontal 0; four values are top/right/bottom/left |
| A 48-point outside box with a two-point border | Declare height 44; ORCA counts its outside border as margin |
| Equal photo columns | `GridBox Columns="1fr 1fr 1fr 1fr" Spacing="6"`; child `AspectRatio="1"` |
| Container already inset by Pivot | Add only genuinely additional inset; do not accidentally add another 12 |
| Full-bleed toolbar | Separate background extension from foreground alignment; avoid unexplained negative margins |
| Visible gap | Parent spacing + neighboring margins + child padding; measure artwork edges too |
| Text aligned to an image top | Consider `TrimLeading="true"`; compare painted glyph bounds, not just node origins |
| Content-sized image action | Borderless `Button` containing `ImageView`; reserve the child's measured height |

See [App styling](app/styling.md) for the authoritative box model. Avoid
manual coordinates, fixed heights to hide measurement defects, or browser
CSS properties that ORCA does not support.

## Typography and alignment

**Historical Phone 8 font-size resources:** small 18.667, normal 20, medium
22.667, medium-large 25.333, large 32, extra-large 42.667, extra-extra-large
72, huge 186.667. The SDK used Segoe WP families and theme brushes
([theme resources][wp8-theme]). Those resource names and numbers describe
that platform; they are not ORCA font registrations.

**Windows 10 period type ramp:** Header 46, Subheader 34, Title 24,
Subtitle 20, SubtitleAlt 18, Base/Body 15, CaptionAlt 13, Caption 12, in
effective pixels ([Microsoft Design Language, March 2016, p. 30][mdl2016]).
The [August 2015 UWP guidelines][uwp2015] specify line spacings of 56, 40,
28, 24, 20 and 14 for Header through Caption. Do not substitute the
Windows 11 Segoe UI Variable ramp that current typography links may show.

**ORCA / Chat recommendations:**

- Align headings, body text, form fields and lists to a shared leading edge;
  for RTL, adapt the leading edge and test glyphs and navigation direction.
- Align paired labels/values by a shared baseline or put the label above
  the value. Right-align short trailing values only when the pair remains
  readable at narrow widths.
- Use the container's text metrics. A larger name beside a smaller footnote
  can have a different apparent top despite equal `ActualY`.
- Wrap explanations and errors. Reserve ellipsis for bounded titles,
  previews and short row values; do not hide the essential part of an error.
- Use large display type for navigation hierarchy, not for long paragraphs.
  Preserve user names and user text rather than case-transforming them.
- Use a font available to the package/host and inspect its actual metrics.
  Selecting a Segoe family name does not install or license a font.

## Which control?

| User intent | ORCA choice | Behavior to preserve |
| --- | --- | --- |
| Read text | `TextBlock` | No editing affordance or unnecessary tab stop |
| Enter text | `Input` + persistent label where needed | Placeholder is an example/hint; retain clear field identity after typing |
| Enter password | `Input Type="Password"` | Let the control mask display; submit the real value |
| Perform an action or open a detail | `Button` / shared command row | One action, full hit area, clear feedback |
| Tap a picture to save immediately | Borderless `Button` → `ImageView` | `Click` belongs to the action; image is its presentation |
| Pick one value and retain selection | `ListBox`, or `RadioGroup` + direct `RadioButton` children for a short set | Visible selected state and one source of truth |
| Pick several independent values | Native `CheckBox` rows or an explicit multi-select presentation | Checkmark/state as well as colour; ORCA `ListBox` is not a UWP multi-select API |
| Apply an on/off setting immediately | `ToggleSwitch` | Explain effect and save failure/reversion |
| Edit independent draft options, saved later | `CheckBox` | A clear Save outcome |
| Short mutually exclusive options, e.g. public/private | `RadioGroup` + `RadioButton` | Show both choices; avoid a button that cycles unexplained states |
| A long set of alternatives | Dedicated picker/list page | Avoid a screen of dozens of radios |
| Show retained service rows without automatic selection | `ItemsStack` + `ItemTemplate` | Each row's ordinary actions remain ordinary actions |
| Show a photo collection | `GridBox` + image item template | Equal gutters and reserved aspect ratios; implement interaction explicitly |
| Display a short loading state | `ActivityIndicator` in the affected region | Keep navigation usable and healthy content visible |
| Display recoverable failure | Wrapped status + applicable Retry | Keep draft, last image or loaded rows when useful |

These choices combine [ORCA's selection-control contract](selection-controls.md)
with Microsoft's [2015 switch/checkbox/radio guidance][uwp2015],
[text-box guidance][text-box] and [list/grid distinction][lists]. The
borderless image action follows [WPF's image-content pattern][image-button];
this is a content-model reference, not proof of identical ORCA keyboard or
accessibility implementation. `GridBox` does not automatically provide
UWP `GridView` selection, virtualization or `ItemClick` properties.

**Chat exception:** native checkboxes in Settings are an explicit user
decision. Preserve them; do not mechanically replace them with switches
because another generation recommends a different control.

## Back, Cancel, Save and app bars

Historical Back dismisses transient UI before leaving a page and goes
backward through history ([navigation basics][back]). A simple read-only
subpage does not need an additional Cancel just to leave it
([Microsoft's Phone/iOS comparison][cancel]). Original phones supplied
hardware Back; Chat's header Back is a deliberate iOS/desktop adaptation.

**Chat rules:** use the header Back for the character picker; no bottom
Cancel. Keep explicit Cancel/Discard where it abandons an edit or answers a
dialog, and make that outcome clear. Preserve drafts and scroll position
when returning. Never use a button labelled Back to perform Save or send.
Do not promise keyboard-dismissal or unsaved-data behavior without testing
the app's actual navigation owner.

**Phone 8.1 WinJS:** compact bars show up to four primary commands; minimal
bars expose the ellipsis; expansion reveals labels and secondary commands
([Microsoft's 2014 guidance][wp81]). Keep the four-command limit specific
to that reference; Windows 10's `CommandBar` is a different API.
Microsoft's [UWP AppBar reference][uwp-appbar] recommends `CommandBar` for
new Windows 10 apps. In ORCA, use the implemented `ApplicationBar`; do not
invent a `CommandBar` alias or assume every UWP property is supported.

**Chat adaptation:** the root bar contains chats, contacts, new chat and
settings; using the bar for root navigation and disabling the active
destination is a product choice. The thread has attach/input/send, without
another root command bar. Put infrequent actions in the menu, disclose icon
labels when expanded, and keep destructive actions clearly named. Do not
draw a second circle inside an icon: `ApplicationBar` draws its ring.

## Touch, state and accessibility

**Historical WP7 guide:** distinguish visible artwork from its hit target.
Its prerelease target advice is 9 mm / 34 source pixels with 2 mm / 8 source
pixels between targets; 7 mm / 26 pixels is an exception, not a new modern
minimum ([guide, printed pp. 74–75][wp7-guide]).

**Current Microsoft:** target roughly 7.5 mm / 40 pixels at the documented
135-PPI, 1× plateau ([targeting][targeting]); current touch guidance suggests
44×44 epx with at least four epx between touch-optimized targets
([touch interactions][touch]). These physical/density assumptions do not
make 34 historical pixels equal to 34 ORCA points.

**Recommendation for Chat:** keep 48-point primary controls/rows and at
least 44×44 for independent icon actions. A small glyph may sit inside a
larger target. Dense photo grids still need distinct hit boxes; pressing
near a gap must not activate an overlapped neighbor.

Do not communicate selected membership by colour alone. Inspect light,
dark, pressed, disabled and keyboard-focus states. Test text against its
actual background: aim for at least 4.5:1 for ordinary text
([accessible text][contrast]); translucent backdrops require composited
background checks. This is a readability target, not a claim of full app
accessibility conformance. Native control names alone do not prove screen
reader support, focus order, keyboard activation or host text scaling.

## Screen recipes and review checklist

| Screen | Starting point | Specific check |
| --- | --- | --- |
| Inbox | Related-data Pivot + retained rows | Active search remains visible/clearable in every filtered pane |
| Contacts | Curated Panorama overview + directory | One owner for horizontal gestures; directory search and keyboard work |
| New message / discovery | Commands then a vertical results list | Whole-row actions, stable labels, explicit multi-selection for groups |
| Settings | Grouped fields and preferences | Labels remain visible; an edit opened below the fold is revealed; current save behavior is evident |
| Group/channel/invitation flow | Focused child page | Explicit choice controls; Back preserves drafts; validation near its field |
| Thread/details | Timeline or vertical details + relevant commands | Error text wraps; auxiliary touch targets remain usable; rows share density |
| Character picker | Four square columns, six-point gaps | No overlap, header Back only, saved image updates Settings |
| Photo viewer | Fit image on black | Keep preview during loading/failure; Back stays accessible |
| Authentication | Existing branded form shell | Clear field labels, keyboard-safe scrolling, understandable route back to sign-in options |

For each changed screen, check 320 and 390 logical widths, a wider/landscape
viewport, both themes, long/localized labels, safe areas and keyboard.
Check loading, empty, failure, retry, and populated data; compare actual
global bounds and painted edges. Test through screen-level pointer routing,
not only direct event sends. An image can paint outside a zero-height parent
and still be untappable—the character picker regression demonstrated this.

Reuse the focused offline suites named in the
[Chat review](../apps/chat/docs/ui-review-2026-11-02.md). Do not start backend,
Keychain, device or live checks for routine UI review. Native geometry tests
and decoded image properties establish their assertions; they do not replace
visual inspection or establish historical fidelity.

## Source register

Period sources are primary Microsoft pages/documents. The WP7 PDF is a mirror
of the Microsoft-authored guide; current Microsoft pages are explicitly
marked here because their content can move to newer Windows generations.

- [WP7 design resources, 27 July 2010][wp7-announcement]; [v2 guide mirror][wp7-guide].
- [WP8 theme resources][wp8-theme]; [WP8 Pivot design rules][wp8-pivot].
- [WP8 Panorama API][wp8-panorama]; [horizontal item orientation][panorama-orientation].
- [Phone design principles, December 2011][wp-design-principles].
- [Phone navigation basics, March 2011][back]; [Phone/iOS comparison, October 2012][cancel].
- [Phone 8.1 WinJS, April 2014][wp81].
- [UWP app design guidelines, August 2015][uwp2015]; [Microsoft Design Language, March 2016][mdl2016].
- [Silverlight-to-UWP migration, April 2016][migration].
- Current: [layout][layout], [Pivot][uwp-pivot], [text boxes][text-box], [collections][lists],
  [targets][targeting], [touch][touch], [accessible text][contrast].
- Windows 10 API distinction: [UWP AppBar / CommandBar][uwp-appbar].
- Cross-framework content model: [WPF image button][image-button].

[wp7-announcement]: https://blogs.windows.com/windowsdeveloper/2010/07/27/windows-phone-7-design-resources-ui-guide-and-design-templates/
[wp7-guide]: https://www.pocketpc.ch/attachments/windows-phone-7-entwicklung/36554d1283328945-windows-phone-7-ui-design-guide-wichtigste-checkliste-ui-design-interaction-guide-windows-phone-7-v2.0.pdf
[wp8-theme]: https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff769552(v=vs.105)
[wp8-pivot]: https://learn.microsoft.com/en-us/previous-versions/windows/apps/hh202919(v=vs.105)
[wp8-panorama]: https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff941269(v=vs.105)
[panorama-orientation]: https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff941126(v=vs.105)
[wp-design-principles]: https://learn.microsoft.com/en-us/archive/msdn-magazine/2011/december/windows-phone-how-to-translate-common-design-principles-to-the-windows-phone
[back]: https://learn.microsoft.com/en-us/archive/msdn-magazine/2011/march/msdn-magazine-mobile-matters-windows-phone-navigation-the-basics
[cancel]: https://learn.microsoft.com/en-us/archive/msdn-magazine/2012/october/windows-phone-building-an-app-for-both-windows-phone-and-ios
[wp81]: https://blogs.windows.com/windowsdeveloper/2014/04/18/winjs-on-windows-phone-8-1/
[uwp2015]: https://download.microsoft.com/download/2/4/A/24A81A29-77CF-4AA5-967E-64E42554F21B/UWP%20app%20design%20guidelines%20v1509.pdf
[mdl2016]: https://download.microsoft.com/download/F/2/C/F2C19EC6-03E2-4D8C-B417-0265B808CD06/Microsoft-Design-Language-1603.pdf
[migration]: https://blogs.windows.com/windowsdeveloper/2016/04/14/tips-for-porting-apps-from-windows-phone-silverlight-to-uwp/
[layout]: https://learn.microsoft.com/en-us/windows/apps/develop/ui/alignment-margin-padding
[uwp-pivot]: https://learn.microsoft.com/en-us/windows/uwp/design/controls-and-patterns/pivot
[uwp-appbar]: https://learn.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.appbar?view=winrt-15063
[text-box]: https://learn.microsoft.com/en-au/windows/apps/design/controls/text-box
[lists]: https://learn.microsoft.com/en-us/windows/uwp/controls-and-patterns/listview-and-gridview
[targeting]: https://learn.microsoft.com/en-us/windows/apps/design/input/guidelines-for-targeting
[touch]: https://learn.microsoft.com/en-us/windows/apps/develop/input/touch-interactions
[contrast]: https://learn.microsoft.com/en-us/windows/apps/design/accessibility/accessible-text-requirements
[image-button]: https://learn.microsoft.com/en-us/dotnet/desktop/wpf/controls/how-to-create-a-button-that-has-an-image
