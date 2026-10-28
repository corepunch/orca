# Windows Phone chat architecture references

Research recorded 2026-10-28. Start with Windows Phone chat applications as
the behavior and authoring reference, then critique their choices and improve
them where there is a concrete benefit. This complements the
[visual direction](../DESIGN.md). It does not introduce a new renderer or
claim that Chat has already migrated to XML.

## Verified implementation reference: Telegram for Windows Phone

[Telegram's own applications/source page](https://telegram.org/apps#source-code)
links its Windows Phone client to
[evgeny-nadymov/telegram-wp](https://github.com/evgeny-nadymov/telegram-wp).
The inspected revision is `fd98ac6d18637019218679b81cb92d983019014b`.
This is historical application source, not evidence that its revision matches
the archived screenshot or that every Windows Phone messenger used this design.

The [conversation XAML](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views/Dialogs/DialogDetailsView.xaml#L515)
contains a `MultiTemplateLazyListBox` whose `ItemsSource` binds to `Items`.
It selects among outgoing, incoming, group and service-message templates,
uses a `VirtualizingStackPanel`, and invokes `LoadNextSlice` near the list
boundary. Inside templates, bindings supply message text, media, date,
delivery status and selection state. Media has its own template selector.
There are also code-behind event handlers and Caliburn action declarations;
this is not a claim that every interaction is expressed solely in XAML.

Useful source locations:

- [Message-type template selection, lines 416–421](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views/Dialogs/DialogDetailsView.xaml#L416).
- [Collection binding, virtualization and paging, lines 515–535](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views/Dialogs/DialogDetailsView.xaml#L515).
- [Message/media/status bindings, lines 104–142](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views/Dialogs/DialogDetailsView.xaml#L104).
- [Conversation view-model source](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/ViewModels/Dialogs/DialogDetailsViewModel.cs)
  is a follow-up reference. Its body could not be retrieved during this pass;
  the observations above come from the XAML, not assumptions about its exact
  collection implementation.

For the platform's MVVM pattern, Microsoft's
[Windows Phone 7 MVVM sample](https://devblogs.microsoft.com/vbteam/visual-basic-windows-phone-7-series-4-how-to-implement-a-model-view-viewmodel-pattern-in-a-windows-phone-application/)
(28 February 2011) explicitly separates models, view models and views. It
uses an `ObservableCollection`, a `ListBox.ItemsSource` binding and a
`DataTemplate` containing property bindings. This verifies that collection
and per-item binding were ordinary Windows Phone authoring mechanisms;
it is not a messenger-specific implementation.

## Static values, collection data and bindings

These are separate concerns:

| Concern | Chat example | What it needs |
| --- | --- | --- |
| Initial property values | An existing message's body and timestamp | Assign or bind fields when the row is created; ongoing observation is optional for immutable fields. |
| Changes to one row | Editing, delivery state, loaded photo, selection | Update the affected properties through a binding or explicit update path. |
| Changes to the collection | New message, older page, deletion, reordering | Add/remove/move rows while preserving their identity and scroll position. |
| Structure by message type | Text, captioned photo, photo-only, service row | Reusable templates with selection or conditional structure. |

A data source can be a fixed array in a tiny test scene. It need not be a
network service or a continuously changing observable object. Likewise,
bindings are useful for initial population of repeated rows; their usefulness
does not depend on every message field changing continuously.

## Where etlua fits

[etlua](https://github.com/leafo/etlua) compiles a template into a Lua function
that renders text from input data. It supports loops and conditionals, so it
can generate XML for different message kinds. Rendering again does not itself
retain native rows, subscribe to property changes or reconcile a collection.
[Lapis](https://leafo.net/lapis/reference/etlua_templates.html) uses etlua for
views and partials and recommends caching compiled templates.

The local `lua-objc` implementation adds the missing retained layer:
`docs/retained-templates.md` documents `template:update(data)`, matching by
tag and `id`/`key`, in-place property updates and child insertion/movement/
removal. `demo/mail/views/MessageRow.etlua` demonstrates declarative row
markup. Those capabilities belong to its template host, not etlua alone;
ORCA would need an equivalent integration rather than just importing etlua.

## Proposed direction and critique

Use Telegram's collection-plus-declarative-template separation as the starting
point. Keep the view's input independent of Appwrite, so production data and
small fixtures pass through the same message presentation path. Prefer XML
for layout and Lua for actions and presentation calculations.

Critique and possible improvements for ORCA:

- Avoid duplicating large row trees for every variation. Share text, media and
  metadata sections, with clear variants where the structure actually differs.
- Compute adjacency-dependent decoration once in the presentation layer:
  sender names, first/last-in-run, tails and date separators. Inserting or
  deleting a message must also refresh its affected neighbors.
- Keep message identity stable. Update affected rows and preserve the reading
  anchor, composer draft and photo state instead of rebuilding the conversation.
- Begin with tiny fixture scenes: text runs, incoming/outgoing captionless
  photos, captioned photos, empty history and loading/error states. Use local
  images and fixed dates, users, viewport sizes and themes. These scenes should
  require no authentication, Appwrite accounts or live checks.
- Test presentation decisions with focused unit tests; inspect native scenes
  for actual geometry and rendering. A fake-node test cannot establish visual
  correctness, and a backend integration test cannot establish bubble layout.

Plain XML with bindings and XML generated by etlua remain implementation
options. Decide after a small retained-row prototype, based on clarity,
supported bindings and update cost, rather than assuming static message values
make one mandatory. The user prefers XML and wants the `lua-objc` approach
considered; that preference does not yet select or implement a migration.

ORCA's current [collection support](../../../docs/app/index.md#state-bindings-and-collections)
needs to be accounted for: `ListBox`/`GridBox` create source rows at startup
and do not automatically reconcile later membership changes. Plain Lua tables
also do not automatically become observable `DataContext` objects. A tested
adapter or engine work is needed before promising the same collection behavior
as the Windows Phone reference.
