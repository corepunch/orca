# Building ORCA applications

This is the shared guide for application architecture, state, navigation, and
delivery. Read [App styling](styling.md) for visual conventions and supported
layout patterns. Engine ownership and paths are mapped in
[Project Map](../PROJECT_MAP.md); public API names come from generated references,
as described in [Lua API](../LUA_API.md).

## Recommended direction and current support

For a new application with stable screens, prefer a declarative object tree:
XML pages and prefabs, CSS presentation, Lua controllers, and separate state
and service modules. This is an authoring convention built on existing APIs,
not a new framework or a completed migration of the samples.

Lua/MoonScript object composition remains a supported alternative for dynamic
components and code-oriented applications. Both authoring forms use ORCA's
object, property, message, layout, and rendering systems. Preserve an existing
project's approach or an explicit user choice; a feature request is not an
instruction to rewrite its presentation language.

| Approach | Use it for | Tradeoff |
| --- | --- | --- |
| XML + CSS + Lua controllers | Stable app shells, pages, reusable templates, and binding-driven UI | Structure and behavior are separate; bindings and asset paths require runtime validation. |
| `Application` + Lua/MoonScript widgets | Code-composed apps, route-based screens, and dynamic components | Composition is concise; state preservation and rebuilding need explicit ownership. |
| Small all-in-one Lua prototype | Exploring one interaction or design | Useful for discovery; separate services, state, and screens before making it a reference architecture. |

The file extension does not establish a frame-rate advantage. Measure tree
construction, allocation, layout, and rendering before making performance claims.

## Ownership boundaries

| Owner | Responsibility | Keep elsewhere |
| --- | --- | --- |
| `package.lua` / bootstrap | Declare project roots, startup entry, resources, plugins, and theme values | Network requests and application behavior |
| Screen / page / component | Describe structure, properties, bindings, and simple message actions | Authentication, HTTP, and shared application state |
| Controller | Translate UI events into state changes, service calls, and navigation | Duplicated page layout and backend protocol details |
| State module / store | Own selected records, drafts, collections, and loading/error state | References to destroyed views and rendering details |
| Service module | Handle transport, session protocol, response conversion, and errors | UI nodes, colors, and navigation |
| Theme / stylesheet | Own semantic visual values and reusable presentation | Business data encoded as layout classes |

A store is an application convention, not a built-in `Store` or `ChatStore`
engine class. Keep a small app small: separate modules when they establish an
actual ownership boundary, rather than creating empty layers.

An illustrative XML-first project might grow into:

```text
package.lua
Screens/App.xml
Pages/Inbox.xml
Pages/Conversation.xml
Prefabs/Message.xml
Controllers/Conversation.lua
State/Chat.lua
Services/Chat.lua
Styles/app.css
Data/Chat.Schema.xml
```

Create only the files the app uses. Existing lowercase directories and module
names do not need to be renamed to match this example.

## Startup and module loading

ORCA currently supports two entry styles through the same startup module:

- `StartupScreen = "MyApp/Screens/App"` loads the resource-owned view. This is
  the normal entry for an XML-first app. Declare the resource libraries it uses.
- `StartupViewController = "MyApp/App"`, with `StartupRoute`, constructs an
  `Application` subclass and dispatches its initial route. This is the pattern
  used by Banking and Weather.

Set one entry style deliberately. If both are present, the current startup
module chooses `StartupViewController` first.

Use `ProjectReferences` to expose additional script/resource roots. Follow the
chosen project's module paths and require built-in modules explicitly when
initialization order matters. A package declaration should describe the project;
put runtime initialization in its application/controller modules.

Avoid copying a sample's event loop into each app. Shared keyboard policy,
reload behavior, and window dispatch belong in the framework when generalized.
Check typing in editable controls: a sample-level quit shortcut such as `q`
must not consume ordinary text entry in an application.

## Navigation has one owner

For persistent application shells, prefer the engine controls documented in
[UI System](../UI_SYSTEM.md):

- `PageHost.ActivePage` selects peer pages such as tabs. Bind it to the tab
  selection. Switching visibility retains the existing child pages and their
  instances; retain shared app state independently of the selected page.
- `NavigationHost` owns hierarchical push/pop navigation. Its pushed templates
  have a `Page` root. Previous pages remain attached and hidden. Pop releases
  host ownership of the popped page; external references can keep it alive.
  Use `CanGoBack` for back-button state.
- `Popup` is modal content, opened through `Screen.ShowModal` and closed through
  `Popup.ClosePopup`. It does not replace the page navigation stack.

Put a tab bar in the persistent application shell, alongside its content host;
individual tab pages supply only their content. Declare the destinations once,
bind `PageHost.ActivePage` to the selection source, and translate URL routes
through that same selection owner. Keep inactive tab labels and icons visible.
Test every destination and repeated switches for one bar instance, matching
selection, visible content, and retained form drafts. The chat app's
[shared shell](../../apps/chat/views/shell.lua) demonstrates this in Lua with
native `TabBar`/`Tab` selection and retained `Page` instances.

The existing Lua `Application` router is also supported. In its current dispatch
path, widget views and layouts are constructed when a route is activated;
switching routes does not by itself preserve a native back stack, scroll
position, or a form draft. Store those values deliberately.

If an app needs URL routes/deep links together with native page navigation,
choose which mechanism owns the navigation state and translate requests into
it. A shared router-to-`NavigationHost` adapter is a possible future addition;
do not assume one already exists or maintain two unrelated histories.

## State, bindings, and collections

Services return application data. State modules own that data, while the view
displays it through an explicit update path. Keep selection, search, drafts,
and the lifetime of cached data out of transient view construction.

Plain Lua tables do not automatically become observable binding sources. Use
the existing `DataObject`/schema mechanism or an explicitly implemented adapter
when exposing store values through `DataContext`. Application data schemas
describe record properties; changes to native engine API shape still belong
in `.cgen` and require generation.

For forms, bind controls to an XML-schema-defined **draft** through
`DataContext`. `Mode=TwoWay` initializes each editor from its draft field and
writes edits back through the shared property system. A controller validates
and submits a snapshot on Save; Cancel restores or discards the draft. Services
and committed application state should not be the direct target of keystrokes.
See the runnable [MVVM form sample](../../apps/MvvmForms/README.md) and
[binding modes](xml/triggers-actions-messages.md#form-bindings).

For repeated content, prefer `ItemsSource` + `ItemTemplate`. A reusable row
template binds its fields through `DataContext`; values such as indentation or
visibility belong to data when they vary by item. Use ordinary explicit layout
for unique headers and other non-repeating sections.

Property changes and collection membership changes are separate contracts.
Currently `ListBox` and `GridBox` instantiate source children at `Object.Start`;
automatic reconciliation after appends, removals, reordering, or source
replacement is not implemented. Configure the source/template before attaching
a newly constructed control; starting it is one-shot. Use one explicit,
tested refresh/adapter path for changing collections, rather than promising
automatic observability. Verify insertion, removal, ordering, and selection
preservation in that path. Avoid a controller adding a data record and separately
reimplementing the same row's layout as a second source of truth.

Use partial updates or a component `rebuild()` at the intended boundary when
needed. Calling `rebuild()` on an existing list does not replay `Object.Start`
or reconcile its source. A refresh must recreate the configured list within its
owning component or explicitly instantiate/update row templates through the
adapter. Preserve focus, drafts, selection, and scroll position deliberately;
do not assume recreating the entire screen is a reactive update mechanism.

## Requests and view lifetimes

Start requests from application/controller actions and expose loading, loaded,
empty, and error states to presentation. Keep HTTP side effects out of layout
and `content()` construction where possible. Rendering a page should not
silently start duplicate requests.

`orca.network.fetch` yields; invoke service operations in the supported ORCA
coroutine context. Start an asynchronous controller action with
`local async = require "orca.core.async"` and `async.run(function() ... end)`.
The [async helper](../../share/orca/core/async.lua) catches thrown errors and
routes them to `async.set_error_handler`; its default handler writes to stderr.
Install application-level reporting deliberately, and handle expected service
failures in the operation so loading/error state is updated. Starting a
coroutine alone does not provide that UI state or cancel stale requests.

When a request completes, check that its result still belongs to the current
record/request and that any target view is alive. A request generation or record
ID can prevent a slow earlier response from replacing a newer selection. Choose
a lifecycle owner for timers and subscriptions and release them at that owner's
end; do not retain detached nodes in a global service.

Keep transport errors structured and preserve user input after a failed send or
save. Use the [models and offline testing pattern](data.md): keep one client and
model layer and swap its transport for an isolated in-process mock. Test login,
contacts, and messages through that same contract, and identify mock limitations.
Do not present simulated replies,
presence, read receipts, or unsupported actions as real backend capabilities.

Client configuration may contain public project identifiers/keys. Keep backend
secrets out of app assets, and define session persistence deliberately rather
than inheriting a sample's incidental storage behavior.

## Using the samples

Treat samples as references for specific mechanisms, not complete standards.

| Reference | Useful evidence | Boundary |
| --- | --- | --- |
| [Adventure shell](../../apps/Adventure/Screens/Adventures.xml) | XML shell, tab bindings, native page navigation | Its play controller also constructs message rows manually; this is not the preferred collection ownership pattern. |
| [Banking app](../../apps/Banking/App.moon) | `Application`, separate views/layout, authentication routing | Banking was migrated to Widget/Application in commit `2071831f`; its old routing plugin was removed. |
| [Weather app](../../apps/Weather/App.moon) | The same Widget/Application entry; separate model/service modules | Some views obtain remote data during content construction; prefer explicit request state for new work. |
| [Example package](../../apps/Example/package.lua) | Semantic theme values, CSS, resources, and reusable presentation | Its dark/violet palette is one app design, not a universal ORCA theme. |
| [Chat app](../../apps/chat/App.lua) | A Lua service adapter, working app states, and a native portrait preview | The app is still a prototype with much of its UI/state in one module; the recommended separation has not been applied to it. |

Read one relevant example for the mechanism being implemented. The XML and
MoonScript-specific entry points remain under [XML](xml/index.md) and
[MoonScript](moonscript/index.md).

## Delivery and verification

Build one useful vertical slice first: load a screen, display state, perform an
action, and return through navigation. Establish styling and update behavior
there before multiplying pages.

Use [Build](../BUILD.md) and [Testing](../TESTING.md) to choose focused checks:

- Verify resource loading, controller event arguments, and a binding update.
- Exercise back/tab navigation and preservation of drafts/selection where the
  product requires it. Test list updates without duplicate rows.
- Exercise a successful service operation, an error, and stale/out-of-order
  completion when concurrent requests are possible. Inject a fake service so
  ordinary UI tests do not require a live backend.
- Set `ResizeMode = "NoResize"` for deterministic fixed-viewport tests. Flush
  queued rebuilds before assertions when the test uses deferred construction.
- Inspect the actual native rendering at the intended viewport: text wrapping,
  fonts, icons, focus, hit areas, overflow, and loading/empty/error states.
  A headless property/layout assertion does not prove that pixels render.

Run `make modules` after native API-shape changes. Broaden to shared engine
tests when shared behavior changes; documentation-only edits do not require
rebuilding the engine. Report what was tested and what still needs a configured
service or target platform.

## Maintaining the conventions

Add a best practice when a real application, failure, or test demonstrates its
value. State whether it is supported behavior, a recommended convention, or an
unimplemented direction; include the smallest useful example or evidence.
Keep application-specific choices out of framework-wide rules.

Maintain the human-facing explanation here and in the styling guide. Skills
should route to it and describe the implementation workflow, rather than
carrying copies of the architecture. Keep public API reference material in
generated documentation and edit its `.cgen` source when that API changes.
