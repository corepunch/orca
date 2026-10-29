# Data-driven screens: schemas, scopes, queries and commands

A standard list, detail page or form is a **schema**, a **scope declaration**,
an **XML page** and **CSS** over a **provider**. The page binds query state,
records and drafts; its buttons name commands. Nothing in the app creates rows,
wires per-row callbacks, reconciles lists or toggles loading and error views.
The runtime lives in `share/orca/data/` (`require "orca.data"`).

```text
XML/CSS pages and templates
  ├─ bindings      → records, query/view/command/edit state
  ├─ ItemsSource   → a query or view collection → ItemsStack + ItemTemplate
  └─ Click/Submit  → handler names → commands, edits, member actions
Scope (Data/*.xml)            one owner's queries, views, commands, drafts, media
Store                         one record per entity key, per account
Provider (providers/*.lua)    operations, computed fields, filters, validators…
Service API / models          unchanged transport and authorization
```

The runnable reference is [apps/ledger](../../apps/ledger): a master–detail
screen with paging and a validated form, whose only Lua is the provider's
domain rules and a 30-line app shell. Chat and Octopus use the same runtime for
their inbox, conversations, feeds, profiles, search, post pages and forms.

## Application layout

```text
package.lua                  resources and startup
Data/App.Schema.xml          entities: columns, keys, relations
Data/<Screen>.xml            one scope declaration per screen or session
Data/Routes.xml              routes: page + scope + parameters (optional)
Pages/*.xml, Templates/*.xml composition and bindings
Styles/*.css                 colours, type, spacing and state classes
providers/<app>.lua          operations and small computed functions
```

## Schema

```xml
<Schema>
  <Entity Name="LedgerEntry" Key="Id" ObjectName="Id" Description="One movement on an account.">
    <Column Name="Id" Type="string" Source="id"/>
    <Column Name="Amount" Type="int" Source="stats.amount" Nullable="true"/>
    <Column Name="Pending" Type="bool" Local="true"/>
    <Column Name="AmountText" Type="string" Computed="money" DependsOn="Amount"/>
    <Column Name="AuthorName" Type="string" From="Author.DisplayName"/>
    <Relation Name="Author" Entity="Person" Key="AuthorId" Embed="true"/>
    <Relation Name="Tags" Entity="Tag" Many="true" Source="tags"/>
  </Entity>
</Schema>
```

- `Type` is `string`, `int`, `float` or `bool`. `Source` maps a service field,
  dotted for nested tables. `Default` fills a missing field on a full put.
- `Key` is the entity's identity. `ObjectName` names the native record object,
  which names generated rows (`ItemNamePrefix="Post"` gives `Post<id>`).
- `Local` columns are client state a refresh keeps (photo paths, pending
  reactions). `Computed` columns call a provider function `fn(values, record,
  context)`; all computed columns re-run in declared order on every change, so
  later ones may read earlier ones. `DependsOn` documents the inputs.
- `From="Relation.Column"` projects a related record's column; `Embed="true"`
  also exposes the related record under the row (`DataContext/Post/Title`).
  `Many="true"` relations are child collections that templates can list.
- `Name`, `id`, `class`, `StyleClass` and `DataContext` are reserved: they are
  native object properties. Loading fails with the resource and line.
- Schemas register native typed DataObject classes; `Schema:describe()` lists
  fields, types, nullability, keys, relations and writability for tools.

**Identity.** A store holds one record per entity key and creates it from the
first row that names it. A post shown in three feeds is one record: a like
updates every row that shows it. Rows of a list can have their own keys (a
feed entry is `feed:post`) and embed the shared record. Stores are per account
(`identity = "chat:" .. userId`), so equal IDs of two accounts never meet.
`put(entity, row, { partial = true })` changes only the fields present;
`Schema.null` clears a nullable field; a missing field is left alone. Records
that nothing shows are evicted beyond the store's capacity.

## Scope declarations

```xml
<DataScope Name="Ledger" Schema="Ledger/Data/Ledger.Schema.xml">
  <Include Path="Ledger/Data/Shared.xml"/>
  <Query Name="Accounts" Entity="LedgerAccount" Operation="accounts"/>
  <View Name="AccountList" Source="Accounts" Selection="first" Filter="active"/>
  <Query Name="Entries" Entity="LedgerEntry" Operation="entries" Require="AccountId" PageSize="20">
    <Param Name="AccountId" Type="string" Bind="AccountList.Selected.Id"/>
  </Query>
  <Edit Name="NewEntry" Entity="LedgerEntryDraft" Submit="AddEntry" Validate="entry"/>
  <Command Name="AddEntry" Operation="addEntry" Invalidates="Entries Accounts"/>
  <Handler Name="ClearSearch"/>
  <Expose Name="Account" Entity="LedgerAccount" Key="AccountId"/>
</DataScope>
```

Every name refers to the provider (`operations`, `computed`, `filters`,
`sorts`, `validators`, `conditions`, `hooks`, `handlers`, `media`, `caches`).
Unknown names, entities, columns, sources and duplicate members fail when the
scope loads, with resource and line, before any request runs.
`Declarations.describe(spec)` returns members, parameters, entities, states
and descriptions for tools.

### Query

| Attribute | Meaning |
| --- | --- |
| `Operation` | `fn(params, page, context, query) -> rows \| { items, cursor, hasMore } \| nil, err` |
| `Entity` | entity of the rows; rows may also be records or `{ key, record }` |
| `Require` | parameters that must be set before it loads |
| `PageSize`, `Prefetch` | cursor paging; `Prefetch="true"` loads more near the list's end |
| `PrefetchEdge="start"` | prefetch near the start instead (older history) |
| `Order="newest"` | the first page is newest; more pages go *before* the rows |
| `Window="latest"` | refresh re-reads the newest page and keeps loaded history |
| `Poll`, `PollWhen` | refresh every N ms while active and in the foreground; failures back off |
| `AutoLoad="false"` | wait for an explicit load |
| `Cached` | show a cache function's rows at once while the first request runs |
| `Arrange`, `Loaded` | hooks before the collection changes (neighbour fields) and after |

State, bindable at `DataContext/<Query>/<State>`: `IsLoading`, `IsRefreshing`,
`IsLoadingMore`, `IsBusy`, `HasLoaded`, `IsEmpty`, `HasItems`, `HasError`,
`ErrorMessage`, `ErrorCode`, `HasMore`, `CanLoadMore`, `Count`, `ShowLoading`,
`ShowEmpty`, `ShowError`, `ShowProgress`, `IsStale`, `IsIdle`,
`HasMoreError`, `MoreErrorMessage`, `MoreErrorCode`, `HasGap`, `ShowHistory`.
Rows are `DataContext/<Query>/Items`; parameters are
`DataContext/<Query>/Params/<Name>` and accept `Mode=TwoWay`.

Rules: each request carries a generation; a response for older parameters, a
replaced session (`valid()`), a disposed scope or an older generation is
discarded. A transport that cannot cancel still runs to completion; nothing is
rolled back. Refreshing the same parameters coalesces. A parameter change
starts a new result: old rows stay on screen until it arrives and are dropped
if it fails. `Debounce` on a parameter waits for typing to pause. A failed
refresh keeps the rows (`ShowError` is for a list that never loaded). A failed
older page is `HasMoreError` and waits for `LoadMore`/`Retry`; polling the
newest page cannot hide it. With `Window="latest"`, a newest page that no
longer overlaps the loaded history sets `HasGap` instead of creating a hole;
`reset()` drops history and shows the newest page.

### View, selection and master–detail

`<View Source="Query" Filter="fn" Sort="fn">` filters and sorts loaded rows
without a request (`fn(record, params, context)`); views chain. Filtering one
page of a server result is a local view, not a search: use a query parameter
for a server search. `Selection="clear" | "keep" | "first"` sets what happens
when the selected record leaves. The selection follows identity across
refresh and reorder and is bindable as `DataContext/<View>/Selected/<Column>`
with `HasSelection` and `SelectedKey`. `<Param Bind="View.Selected.Column">`
makes a dependent query follow the selection.

### Command

`fn(params, context, command) -> result | nil, err` with `params.item` (the
record the sender presents), `params.draft`, `params.target`, `params.key` and
declared `<Argument>`s. `Parameter="Item"` makes a missing item an error.
`Concurrency` is `block` (default), `queue`, `parallel` or `latest`.
`Invalidates` refreshes queries after success; `Succeeded`/`Failed` hooks run
after the outcome; `CanExecute` names a condition. State: `CanExecute`,
`IsExecuting`, `IsAvailable`, `HasError`, `ErrorMessage`, `ErrorCode`,
`Succeeded`, `Reason`. A command without an operation is unavailable, so its
button can be disabled rather than pretend to work. Work queued for a replaced
session or disposed page never starts.

### Edit (forms and drafts)

An edit is a draft record (`Entity` lists its fields) that inputs bind with
`Mode=TwoWay`, plus `Status` (`IsDirty`, `IsValid`, `HasError`,
`ErrorMessage`, `IsSubmitting`, `CanSubmit`, `IsEditing`) and `Errors/<Field>`.
`Validate` returns `{ Field = message, _ = form message }`; field errors clear
as the field changes. `Submit` hands a **snapshot** to its command: a failure
keeps the input; text typed while a submit is pending survives its success.

- `switch(key)` keeps one draft per key (one per kind of post, per room); each
  key submits independently, and a result settles its own draft even when
  another is showing.
- `begin(record)` with `<Copy Field From>` edits a copy of a committed record;
  the unsent new draft returns after save or cancel. Until a field changes, the
  copy follows the record, so a form opened before its record loads fills in.
  `Begin="Room"` starts it from an exposed record when a route opens.
- `Submitted` is a hook `(ok, result, err, info, edit)` with `info.key`,
  `info.visible`, `info.cleared` (nothing was typed meanwhile) and
  `info.target`.

### Media

```xml
<Media Name="Photos" Fetch="photoFetch" Store="photoStore" Cached="photoCached"
       Validate="photoValidate" Stages="preview full">
  <Target Entity="ChatAttachment" Id="Id" Revision="UpdatedAt" Path="PhotoPath" State="PhotoState"/>
</Media>
```

Templates bind `PhotoPath` and state-derived fields; the media member loads
for records the presentation reports as visible (`orca.data.visibility`),
shares one request between records with the same id, runs at most
`MaxActive` downloads, shows a preview stage before the full file, validates
decoding, and records `error` for an explicit `Photos.Retry` (or
`retryFailed()` after a refresh). Results for a replaced session, a changed
revision or a scrolled-away request are discarded. `Lookup` batches id →
file resolution (avatars); `Missing` remembers ids without media. Picking,
encoding, upload and cleanup stay in the provider.

### Handlers, exposures and includes

`<Handler Name="X"/>` names a provider function `fn(scope, params)` for UI
actions that are not data operations (focus a field, open a picker).
`<Expose Name="Room" Entity="ChatRoom"/>` puts one record at
`DataContext/Room` (`scope:expose(name, record)`; `Key="RoomId"` takes it from
a route parameter). `<Include Path>` shares declarations, such as the actions
every list of posts offers.

## XML

```xml
<ItemsStack Name="AccountItems" ItemNamePrefix="Account"
            ItemsSource="{Binding DataContext/AccountList/Items}"
            ItemTemplate="Ledger/Templates/AccountRow"/>
<Grid Visible="{Binding DataContext/Entries/ShowLoading}"><ActivityIndicator/></Grid>
<Button Click="Entries.LoadMore" Visible="{Binding DataContext/Entries/CanLoadMore}"/>
<Input Text="{Binding DataContext/NewEntry/Memo, Mode=TwoWay}" Submit="NewEntry.Submit"/>
```

- `ItemsStack` is a StackView with an ItemsControl. Rows retain their native
  node while their record stays in the collection; field changes update
  bindings in place. `ItemNamePrefix` names rows by record.
- `StateClasses="{Binding DataContext/State}"` applies data-driven style
  classes (`liked pending`) and restyles the subtree; colours stay in CSS.
- Handler names resolve through the nearest data controller: a declared
  handler, a command or edit by name, or `Member.Action` with `Load`,
  `Refresh`, `Retry`, `LoadMore`, `Invalidate`, `Reset`, `Submit`, `Cancel`,
  `Clear`, `Select` or `Begin`. The handler receives the record the sender's
  row presents.
- `scope:attach(page)` sets the page's DataContext and controller.
  `scope:present(node, "Post")` makes a subtree present one exposed record, as
  a row presents its item.
- Bindings evaluate on paint. Expressions join and choose values
  (`{DataContext/A} + " · " + {DataContext/B}`, `IF(...)`, `AND`, `OR`);
  keep algorithms in provider operations.

Pages and templates that the Lisp build loads need `.lisp` twins with the same
structure.

## Routes

```xml
<Routes>
  <Route Path="/details" Name="DetailsPage" Page="Chat/Pages/Details" Scope="Chat/Data/Details.xml"
         HeaderFrom="Room.Title" Back="/thread" Status="DetailsStatus">
    <Param Name="RoomId" From="room.id"/>
    <Present Node="Header" Record="Room"/>
  </Route>
</Routes>
```

`Routes.load(path)` reads a registry; `routes:open(path, app, options)` builds
the route's scope, opens it with parameters read from the app (`From` is a
dotted path), loads and attaches the page and presents records. The app keeps
navigation and page chrome: Chat registers each route with its router, docks a
page's `ApplicationBar`, and disposes the scope when the route changes. A new
standard screen is a route, a scope and a page.

## Lifetimes

A scope belongs to one owner: Chat's inbox scope lives for the session; a
conversation, details page or post page gets a scope that ends with the page.
`scope:activate()`/`deactivate()` start and stop polling with visibility;
`setForeground(false)` pauses it with the app. `dispose()` stops requests,
media and drafts and releases records. `valid()` (for example "the same
account is signed in") rejects late results of a replaced session.
`Scope.defaultHost()` runs operations in ORCA coroutines and timers on native
Timers; tests pass hosts whose work and timers they step.

## Conversations and scrolling

A conversation query uses `Order="newest" Window="latest" PrefetchEdge="start"`.
Its `Arrange` hook sets neighbour-dependent fields (sender runs, tails) on the
final rows before they show. `orca.data.scroll.keep(list, rows, collection)` is
the presentation policy: a reader at the end follows new rows; history
prepended above, edits and removals keep the first visible row in place.

## Diagnostics, previews and tools

- Declarations and schemas fail at load with resource and line.
- `orca.data.lint` checks a page without running it: binding paths, handler
  names and the templates its lists use, against the scope and schema.
  `Lint.page("Chat/Pages/Inbox", "Chat/Data/Inbox.xml")` returns problems with
  resource, line, node, property, path and reason; `tests/test_data_lint.lua`
  keeps every migrated page clean.
- `orca.data.fixtures` serves the provider contract from deterministic rows
  (parameters filter by matching snake_case fields; `PageSize` pages them), so
  pages preview without a network and with the same declarations.

## What stays code

Providers keep domain rules: timeline merging, comment threading, upload with
retry, permissions the service enforces. Page chrome (Pivot headers, app bars,
transitions), the shell, authentication flows, Chat's multi-step new
conversation and the photo viewer remain Lua; they own no records and call
the same providers and services.

Not provided: virtualization, an optimistic placeholder engine, offline
mutation queues, generic form generation and a visual editor. The metadata
above (`Schema:describe`, `Declarations.describe`, lint) is what an editor
would read and write; its output is ordinary ORCA assets.

## Tests

`tests/test_data_runtime.lua` covers identity, partial/null fields,
projections, stale responses, paging, conversations, debounce, polling,
selection, commands, drafts, keyed submissions, presentation and expression
regressions. `tests/test_data_lint.lua` and `tests/test_ledger_example.lua` run
in `make test-headless`. App tests drive pages through control events:
`tests/test_chat_inbox.lua`, `tests/test_chat_timeline.lua`,
`tests/test_octopus_templates.lua` and the app suites.
