# ORCA Editor

A live editor for ORCA projects, laid out like Kanzi Studio and Unity:
Hierarchy and Library on the left, document tabs above a rendered Preview
with a Console below it, and the Inspector on the right. The edited project runs in a separate
`orca -server=true` process; every change is applied to that running scene
and the preview is re-rendered from it. Panels, menus and dialogs use native
UIKit controls authored in XML. Buttons, inputs and checkboxes use the
engine's default stylesheet with semantic light/dark palettes from
`package.lua`. Appearance follows the operating system and updates retained
controls immediately, preserving focus, selection and unfinished text.

The editor starts in a compact 1040 × 680 window with original 3D splash artwork, a
searchable project list, and recent projects. Select a project and choose
Open project, double-click a row, or press Enter. Open project… (Ctrl+O)
opens an in-app folder browser; paste a folder or `package.lua` path, or
navigate through folders. The project runtime starts only after a selection.
File > Choose Project returns here; Back to editor keeps the current
workspace. Switching projects with unsaved tabs offers Save All, Don't Save
or Cancel. Small windows hide
the artwork to preserve space for project controls.

Recents are stored as literal paths in `~/.orca-editor-recent-projects`;
`ORCA_EDITOR_RECENTS` overrides the history file. Discovery never evaluates
package files. The splash was made with the built-in ImageGen tool; its
reproduction prompt is in `Images/welcome-sculpture.prompt.txt`. Its runtime
export is 832 × 1360, exactly twice the 416 × 680 panel, with explicit linear
texture filtering and mipmaps. The original ImageGen master is retained.

![Project welcome screen](docs/welcome.png)

![Editor with document tabs and the Library panel](docs/editor.png)

Panels never build rows themselves. The runtime answers each panel's
`XmlDataSource` with typed records, including the icon path for every node
class and file kind (the request names the icon folder with `icons=`), and
the item templates bind straight to them. Hierarchy and Project use nested
`TreeViewItem` containers: every item owns its `Header` and the child
`ItemsPresenter` generated from its own `ItemsSource`.

## Running

```bash
make editor                       # builds ORCA and shows the project welcome screen
make editor PROJECT=apps/Weather  # preselects a project in the welcome screen
cd apps/Editor && make run PROJECT=../Example
```

`ORCA_EDITOR_PROJECT` adds and preselects a project when launching
`orca apps/Editor` directly. Remembered projects retain their recent order.

## Common tasks

| Task | How |
|---|---|
| Open another project | File > Choose Project (Ctrl+O), then select or browse to a folder |
| Open a screen or prefab | Double-click it in the Library (it opens in a new tab), or File > Open Document |
| Switch documents | Click a tab, or Ctrl+Tab / Ctrl+Shift+Tab (also Ctrl+PageDown / PageUp) |
| Close a document | Ctrl+W, the tab's × button, or the tab's context menu (Close, Close Others). Unsaved tabs ask Save / Don't Save / Cancel. |
| Find a resource | Ctrl+P focuses the Library search field; Enter applies the filter |
| Browse folders instead of libraries | The folder button in the Library header, or View > Group by Library |
| Select a node | Click it in Hierarchy or in the Preview (hover shows the node under the pointer) |
| Find a node | Ctrl+F focuses the Hierarchy search field; Enter applies the filter |
| Create a child | Create menu, Hierarchy + button, or right-click > Create Child (common classes first, the rest under More) |
| Rename, duplicate, delete | F2 / Ctrl+D / Delete, or the Hierarchy context menu |
| Edit a value | Checkboxes toggle; enums open a keyboard-accessible dropdown; strings and numbers edit inline. Enter or leaving a field commits; Escape restores its value. |
| Edit Margin, Padding, SafeArea or BorderWidth | Edit labelled L/T/R/B fields in one compact row; expand the disclosure to access individual bindings. |
| Edit a mixed compound such as Font | Expand its concise summary to edit family, size, weight and other fields. |
| Choose an object/resource | Click the resource chip to browse compatible objects. The picker shows canonical paths, supports filtering and includes None. |
| Bind a property | Click the binding button beside a property, press Ctrl+B, or use its context menu |
| Undo / redo | Ctrl+Z / Ctrl+Shift+Z (also Ctrl+Y) |
| Save | Ctrl+S writes the active document back to its XML file; Ctrl+Shift+S saves every unsaved tab |

Help > Keyboard Shortcuts (F1) lists every shortcut. Menus open from the menu
bar, with Alt+letter, or by right-clicking; they support submenus, check marks,
disabled items, shortcuts, arrow-key navigation and first-letter jumps.

## Library

The Library panel groups the project's resources by what they are, after
Kanzi Studio's Library, instead of by folder: Screens, Prefabs, Scenes,
Animations, Materials, Material Types, Mesh Data, Textures, Images, Fonts,
Styles, Data Sources, Themes, Scripts and Resource Files. Each library shows
its item count, and each item shows the folder (or library file) it lives in.

- XML files are classified by the class of their root element, so a page
  under `Pages/` and a template under `Templates/` are both Prefabs.
- A `<Library>` file contributes each named child as its own item; a mesh
  library file lists its meshes, and an item opens in place
  (`Project/Widgets/Card`).
- `package.lua` declarations keep a library visible while it is empty
  (`ScreenLibrary`, `PrefabLibrary`, `ImageLibrary`, …). Inline
  `DataSourceLibrary` objects and `ThemeLibrary` values are listed too.
- Screens, prefabs and scenes open as documents; other resources are listed
  for reference. The folder button switches to the on-disk Files view.

## Document tabs

Every opened screen, prefab or scene gets a tab above the Preview. Each tab
keeps its own selection, undo/redo history and unsaved state, so Ctrl+Z
never reaches into another document. The tab title gains `*` while it has
unsaved edits, and the status bar counts unsaved documents.

All tabs share the one project runtime rather than starting a process per
tab. Documents in a project depend on each other (screens instantiate
prefabs, and all of them read the same themes, fonts, plugins and data
sources), so a process per document would duplicate that state, render
stale prefabs after an edit in another tab, and multiply startup time and
memory. The runtime already caches each opened document by path, so
switching tabs is a lookup plus a preview render. Don't Save on close asks
the runtime to forget the document's pending edits (`post/revert`); it is
reloaded from disk the next time it opens.

## Binding editor

The binding editor places a syntax-colored code surface inside a native UIKit
dialog for ORCA binding expressions:

- syntax colors for functions, `{property}` paths, strings, numbers and
  comments, with unknown functions and syntax errors marked in color;
- bracket matching around the caret and diagnostics for unbalanced brackets
  and unknown functions;
- signature help for the call around the caret (`STEP(edge, x)`, argument 2);
- completion of functions and of `{` property paths (Ctrl+Space, Tab or Enter
  to accept), plus a clickable function palette;
- auto-closed brackets and quotes, multi-line editing with indentation;
- a binding mode selector (Expression, OneWay, TwoWay, OneWayToSource,
  OneTime) with mode-specific checks;
- diagnostics before Apply, and the server's error if it rejects the binding.

The binding VM reads a newline as a statement separator, so the editor joins
the lines (dropping `#` comments) into one expression when it applies it.

Only the code surface uses `CodeInput`, a narrowly scoped ConsoleView wrapper
that preserves the original syntax colors, line numbers, bracket highlights
and caret editing. Menus, panels, lists, diagnostics, completion, the function
palette and dialog buttons use native XML/UIKit controls. The palette and
completion viewports are clipped and scroll without covering the dialog footer.

## Architecture

| File | Role |
|---|---|
| `Screens/Editor.xml`, `App.lua` | XML shell, shared runtime connection, global commands |
| `Pages/*.xml`, `Templates/*.xml` | Hierarchy, Project and Inspector layouts and row templates |
| `Views/*.xml`, `Styles/editor.css` | Native menus, headers, logs, inputs, dialogs and editor layout styling |
| `package.lua` `ThemeLibrary`, `Icons/*.svg` | Semantic light/dark palettes for the engine default stylesheet; Lucide icons named by runtime records |
| `Data/Editor.Schema.xml` | Typed runtime node, file and property records |
| `Scripts/Workspace.lua` | Open documents (tabs) with per-document selection, undo/redo and unsaved state; edit/save commands and events |
| `Scripts/DocumentTabs.lua`, `Views/DocumentTab*.xml` | Tab strip above the Preview; close and context menu |
| `Scripts/Hierarchy.lua`, `ProjectView.lua`, `Inspector.lua` | Panel actions and source request parameters; ProjectView switches Library and Files views |
| `Scripts/Native.lua`, `ChromeController.lua`, `DataPanel.lua` | XML event dispatch and local chrome records |
| `Scripts/Preview.lua` | Snapshot image, selection and hover outlines, click-to-select |
| `Scripts/BindingEditor.lua`, `CodeInput.lua`, `Syntax.lua` | Syntax-colored code input, native completion and validation |
| `Scripts/Xml.lua` | Reader for command replies and preview metadata; panel records bypass it |

`Screens/Editor.xml` declares one `ProcessConnection` named `Runtime`. Each
panel declares an `XmlDataSource` with `ConnectionPath="Runtime"` and the
editor schema. The root `TreeView` binds directly to `Source.Items`:

```xml
<XmlDataSource Name="Source" ConnectionPath="Runtime"
               Schema="Editor/Data/Editor.Schema.xml" AutoLoad="false"/>
<TreeView ItemsSource="{Binding ../Source/Items}"
          ItemTemplate="Editor/Templates/SceneNodeRow"/>
```

Its template is a `TreeViewItem` whose `Header` holds an icon and name.
The native container supplies the disclosure, header selection and child
`ItemsPresenter`; each item's `ItemsSource` binds its record's `Children`.
Descendants inherit the parent's item template, so the template does not
load its own resource recursively:

```xml
<TreeViewItem ItemsSource="{Binding DataContext/Children}">
  <TreeViewItem.Header>
    <TextBlock Text="{Binding DataContext/Label}"/>
  </TreeViewItem.Header>
</TreeViewItem>
```

Only root records belong to the root `TreeView`. Child containers belong
to their parent item and remain attached while collapsed. `IsExpanded`
is local control state, initially false, retained through runtime refresh;
disclosure and arrow keys never rebuild or request the flattened tree.
Selecting a descendant reveals its ancestors. Search preserves matching
ancestor chains and expands the filtered results. The Inspector continues
to use a flat property list, which is a different presentation.

The workspace sets the request in `Source` and sends `XmlDataSource.Reload`
when a document, selection or filter changes. Runtime response XML becomes
native typed records directly; no Lua table conversion, row construction or
per-row event wiring occurs. Reload retains records by XML `name` and class,
which keeps generated control identity and selection. Failed responses keep
the last successful records and expose `HasError`/`ErrorMessage`.

The connection uses literal process arguments and synchronous pipe requests
with a configurable deadline (`Timeout`, 10 seconds by default). This is a
native process transport, not an HTTP data-source implementation.

The server side is `plugins/EditorKit/server.c`. Requests are single lines,
`<method>/<route><path>?<args>`, with percent-encoded paths and values; each
reply is an XML `<response status="success|error">` document.

| Route | Purpose |
|---|---|
| `get/data-libraries?filter=...` | Library records grouped by resource type, each owning its items in `Children` |
| `get/data-project` | Nested native file records for the Files view, including each record's `Children` |
| `get/data-scene/<doc>?filter=...` | Nested native node records, retaining ancestors of filter matches |
| `get/data-properties/<path>` | Typed property records and enum options for the Inspector |
| `get/data-resources/<path>?property=<qualified>&filter=...` | Compatible typed resource records for the native picker |
| `get/data-functions` | Native function catalog for the binding palette |
| `get/project-overview` | Project metadata for document commands |
| `get/startup-screen`, `get/scene-hierarchy/<doc>` | Node tree |
| `get/node/<path>` | Properties grouped by class, with `data-id`, `data-set` and `data-program` (binding) |
| `put/node/<path>?Prop=value` | Set values; the reply carries the previous values for undo |
| `put/binding/<path>?property=&expression=&mode=` | Set or (empty expression) remove a binding |
| `post/node/<parent>?class=&name=`, `post/duplicate/<path>`, `delete/node/<path>` | Structure |
| `get/snapshot/<doc>?width=&height=&file=` | Render offscreen to PNG, plus each node's rectangle |
| `get/binding-functions`, `get/class/<base>` | Completion data |
| `post/save/<doc>` | Replay this session's edits onto the source XML |
| `post/revert/<doc>` | Forget unsaved edits and reload the document from disk on next use |

Inspector compounds come from generated shorthand/struct metadata; explicit
`inspector-group` and `inspector-label` metadata covers independently declared
groups such as SafeArea. Numeric compounds include native `Fields` records;
mixed compounds expose a summary and only emit detail rows when expanded or
matched by a filter. Each leaf keeps its qualified property identifier, value,
binding and read-only state. Lua handles actions without converting runtime
records to tables. The same native list picker displays enum options and
resource results directly.

The interaction follows compact spacing controls in
[Figma](https://help.figma.com/hc/en-us/articles/31289464393751-Use-the-horizontal-and-vertical-flows-in-auto-layout),
expandable property editors and binding markers in
[Visual Studio](https://learn.microsoft.com/en-us/visualstudio/xaml-tools/creating-a-ui-by-using-xaml-designer-in-visual-studio),
and typed object fields in
[Unity](https://docs.unity3d.com/6000.0/Documentation/Manual/UIE-uxml-element-ObjectField.html).
Collapsible groups also follow the rollout approach used in
[3ds Max](https://help.autodesk.com/cloudhelp/2024/ENU/3DSMax-Basics/files/GUID-FD2F49B2-B86F-445B-AE4B-03F020671485.htm).

## Saving

Saving does not serialize the runtime tree, which would inline resolved
styles and theme values. The server records each successful edit and replays
it onto the document's source file with libxml2: attributes are set,
`<BindingExpression>` children replaced, nodes created, renamed, duplicated or
removed, and everything else (comments, style references, `$theme` values,
the DOCTYPE) is kept. Edits under elements without a `Name` in the source
cannot be located; Save reports them in the Console and keeps them pending.

Known limitation: libxml2 writes each element's attributes on one line, so
attribute lists that were wrapped across lines in the source are re-flowed on
the first save.
