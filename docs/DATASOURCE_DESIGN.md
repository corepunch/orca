# Datasource Views, Mutations, and Relationships

Status: proposed design for the remaining work in [issue 360](https://github.com/corepunch/orca/issues/360).

## Summary

ORCA should keep a provider gateway between UI binding and storage, but it should not materialize every field as a child `DataObject`. `ListBox` and `GridBox` bind directly to a provider table path. Resolving that path creates an internal fetched, selectable collection of lightweight records. A record identifies a provider row and resolves column values through the provider contract.

This is the useful common ground between DBKit's `DBFetchGroup`/`DBRecordList`, EOF's `EODisplayGroup`, and the `socialfeed.orion` example:

- the provider hides XML, SQL, REST, CLI, or another backend;
- the resolved collection owns selection, edit, and refresh state;
- bindings name columns or relationship key paths;
- a detail collection is scoped by a master's current record through reference-column metadata;
- mutations are messages sent through the provider gateway;
- one save operation can commit changes from several related views.

`DataSource` remains the package declaration (`Name`, `Type`, `Params`). At runtime it resolves to a provider session. `ItemsSource="ProjectName/DataProvider/TableName"` resolves a table collection from that session. There is no separate declaration for each view and no runtime object for every column value.

## Goals

- Bind controls to provider data without a `DataObject` child per field.
- Support flat lists, editable forms, and master-detail interfaces with the same abstraction.
- Keep backend behavior replaceable and message-driven.
- Allow optional schemas and relationships without requiring an ORM.
- Track and save changes across related views through one provider session.
- Close provider-owned resources reliably when a project is unloaded.

## Non-goals

- Expose SQL, URLs, CLI arguments, or storage paths in view markup.
- Require generated C classes for every entity.
- Make schemas mandatory for read-only or self-describing providers.
- Add transparent object faulting or a full EOF-style ORM in the first version.

## Runtime model

```text
package.lua DataSource declaration
              |
              v
       DataProvider session
       (backend gateway and unit of work)
          |               |
          v               v
  master collection --> related detail collection
       | current row       | reference-column rows
       +-------- controls bind by column/key path --------+
```

### DataProvider

A provider session is created lazily for one named project datasource. It owns backend handles, schema metadata, identity mapping if needed, and the pending change set. Providers implement behavior by handling datasource messages.

The provider is deliberately not a table. One provider may expose several entities or query endpoints, and one transaction may span several of them.

### Provider table collection

A provider table path resolves lazily to an internal record collection. This runtime object is an implementation detail rather than something applications declare in XML. It contains:

- provider session and entity/query identity;
- fetch parameters, sort, and filter state;
- lightweight record handles returned by the provider;
- current selection;
- optional relationship scope derived from a reference column;
- loading, error, and dirty state.

`ListBox` and `GridBox` consume the collection's rows. A record handle contains stable identity and provider ownership; it does not contain child objects for its columns. Resolving `title` or `author.name` sends a value-read message to the provider. Providers may cache values in a row buffer, but that cache is an implementation detail rather than an ORCA object tree.

This requires the binding evaluator to recognize provider-backed records as data contexts and resolve a path through the provider's value message. Existing object-property binding remains unchanged for normal ORCA objects.

### Schema and model metadata

Schema remains optional. When available, it can declare:

- entities and columns;
- column types, nullability, and editability;
- primary or stable identity keys;
- to-one and to-many relationships and their joins;
- validation and editor metadata.

A schema-less provider can report equivalent metadata dynamically or support plain column-name access. Master-detail requires a reference column reported by the provider. The relationship and join belong to that column's metadata, never to a control or separately declared view.

## Declarative shape

The canonical source address is:

```text
ProjectName/DataProvider/TableName
```

For example, both existing collection controls bind directly to the posts table:

```xml
<ListBox Name="Posts"
         ItemsSource="SocialFeed/db/posts"
         ItemTemplate="SocialFeed/Prefabs/PostRow" />

<GridBox Columns="3"
         ItemsSource="SocialFeed/db/posts"
         ItemTemplate="SocialFeed/Prefabs/PostCard" />
```

`ProjectName` selects the project resource scope, `DataProvider` selects the named datasource declaration, and `TableName` is interpreted by that provider. No `DataView`, `Master`, or `Relationship` objects are declared alongside the controls.

Item templates bind columns through their row `DataContext`:

```xml
<StackView Name="PostRow">
  <TextBlock Text="{Binding DataContext/title}" />
  <TextBlock Text="{Binding DataContext/author.name}" />
</StackView>
```

Relationships are reference columns in table metadata. A provider schema may describe them in the same compact form as the Orion database model:

```xml
<table name="posts">
  <field name="id" type="integer" key="YES" />
  <field name="author_id" type="integer" relation="authors.id" />
  <field name="comments" type="reference"
         relation="comments.post_id" multiple="YES" />
  <field name="title" type="string" length="256" />
</table>

<table name="comments">
  <field name="id" type="integer" key="YES" />
  <field name="post_id" type="integer" relation="posts.id" />
  <field name="author_id" type="integer" relation="authors.id" />
  <field name="text" type="string" length="1024" />
</table>
```

Here `posts.comments` is a to-many reference column. Reading it for a post record returns a provider collection scoped by `posts.id = comments.post_id`. Likewise, the `author_id` relation can expose a to-one reference path such as `author.name`; the provider schema controls the public reference-column name.

For a separate master-detail control pair, `ListBox` needs a selected-record property in addition to its current string-only `SelectedValue`. The detail source then binds through the selected row's reference column:

```xml
<ListBox Name="Posts"
         ItemsSource="SocialFeed/db/posts"
         ItemTemplate="SocialFeed/Prefabs/PostRow" />

<ListBox ItemsSource="{Binding #Posts/SelectedItem/comments}"
         ItemTemplate="SocialFeed/Prefabs/CommentRow" />
```

There is no join or foreign-key syntax in this UI. `comments` already carries that information as a reference column. A nested comments list inside the post item template can simply use `ItemsSource="{Binding DataContext/comments}"`.

## 2. Data mutations through messages

Reads and writes use the same provider gateway. Controls and application code must not mutate provider row buffers directly.

### Message contract

The initial contract should cover these operations conceptually; concrete field types belong in `.cgen` when implementation begins:

| Message | Target | Purpose |
|---|---|---|
| `DataProvider.Open` | provider | Open the provider session from declaration params. |
| `DataProvider.Fetch` | provider | Fetch records for an entity/query, filter, sort, and optional relationship scope. |
| `DataProvider.GetValue` | provider | Read a column or key path for a record handle. |
| `DataProvider.SetValue` | provider | Stage a column change for a record handle. |
| `DataProvider.Insert` | provider | Create a pending record, including relationship defaults. |
| `DataProvider.Delete` | provider | Mark a record for deletion. |
| `DataProvider.SaveChanges` | provider | Validate and atomically persist the session change set where supported. |
| `DataProvider.RevertChanges` | provider | Discard pending changes and restore fetched values. |
| `DataProvider.Close` | provider | Release all provider resources. |

The resolved collection exposes corresponding UI-level messages and forwards them with its provider, table, and record context filled in. This keeps common controls independent of provider details while still allowing a provider to override every operation.

### Edit flow

1. A control edits `DataContext/title`.
2. The binding layer sends `SetValue(record, "title", value)` to the record's provider collection.
3. The collection validates known column metadata and forwards `DataProvider.SetValue`.
4. The provider stages the value in its session change set and returns success or a structured error.
5. The collection marks the record dirty and notifies bindings for `title`, dirty state, and validation state.
6. `SaveChanges` is sent to the provider session, not independently to each field or control.
7. On success, generated identities and normalized server values are merged into affected records and collections are notified. On failure, pending edits remain available for correction or revert.

Providers may auto-save, but that is a provider policy. The default model is explicit save/revert because it supports forms and transactions predictably.

### Unit of work

The provider session is the transaction boundary. All collections resolved from the same named datasource share it. Therefore editing a post and inserting a related comment can be committed by one `SaveChanges` message when the backend supports transactions.

For backends without transactions, `SaveChanges` still provides a single semantic gateway. The provider must report whether the operation was atomic and return per-change errors. ORCA must not pretend a REST or CLI provider has rollback semantics it cannot supply.

Messages should return structured status rather than only a boolean: operation, record identity, column when applicable, error code/message, and whether retry or revert is possible.

## 3. Master-detail relationships

A detail collection is the value of a reference column on the master record. It is the same kind of provider collection as a root table source, only scoped by that record and the reference column's relationship metadata. It is not a special control and does not require application selection callbacks.

### Selection flow

1. The master `ListBox` current record changes.
2. It emits a selection-change message containing old and new record identities.
3. Each binding that reads a reference column from the selection clears or marks its old result stale.
4. If the new selection is non-null, resolving that column sends `DataProvider.Fetch` with the column metadata and master identity.
5. The provider resolves the reference using schema metadata or provider-native logic.
6. The resulting detail collection replaces its records, restores selection where meaningful, and notifies bound controls.

The detail collection should use a generation token for fetches. Results from an older master selection are discarded, which makes the design safe for asynchronous REST and CLI providers.

### Relationship behavior

- A null master selection produces an empty detail collection without querying.
- Changing the master with dirty detail edits must follow an explicit policy: `Prevent`, `AutoSave`, or `Revert`; default to `Prevent`.
- Inserting through a reference-column collection automatically supplies its relationship foreign key or provider-native parent identity.
- Deleting a master does not imply cascading client-side deletes; cascade behavior comes from relationship/provider metadata.
- To-one key paths such as `author.name` are resolved by the provider. Providers may join, batch, cache, or lazily fetch them.
- Several detail bindings may depend on one master, and selected detail records may expose further reference collections.
- Cycles encountered while resolving reference paths are rejected.

This follows EOF's useful division: the model owns the relationship, the master owns selection, and the detail collection merely follows both.

## 4. Deprecate direct `DataContextSource="file:child"`

`DataContextSource` currently tries the provider registry and falls back to `FS_LoadObject(value)`; a colon then selects a named child. That makes the same attribute mean either a datasource name or a filesystem path and keeps the old `DataObject` tree shape alive.

The replacement is name-based datasource/view resolution:

- `DataContextSource="ApplicationData"` may remain temporarily for a provider's default/root view.
- New collection UI should use `ItemsSource="ProjectName/DataProvider/TableName"` rather than `ApplicationData:Signals`.
- A provider-specific path belongs in `package.lua` `Params`, never in screen markup.
- Table selection is the third path segment; relationships are resolved through reference columns, not colon parsing.

Migration should be staged:

1. Add provider table-path resolution and provider-backed record binding.
2. Port `samples/Example` and `samples/Adventure` from file/child paths.
3. Emit a deprecation warning whenever provider lookup fails and the direct-file fallback is taken.
4. Remove colon child selection for provider-backed sources after all in-tree users migrate.
5. Remove the direct `FS_LoadObject` fallback in the next compatibility-breaking release.

If loading an arbitrary object file is still useful outside datasource binding, it should receive a clearly named resource-loading attribute/API rather than remain hidden inside `DataContextSource`.

## 5. Provider lifecycle cleanup

The current filesystem registry stores process-global provider types and datasource entries in fixed arrays. Datasource entries accumulate, and `FS_Shutdown` does not clear them or release fetched roots.

Provider types and provider sessions need different lifetimes:

- provider type registrations belong to the loaded module and normally live until module shutdown;
- datasource declarations, provider sessions, views, cached records, and pending changes belong to one project/workspace generation.

### Ownership

```text
filesystem module
  owns provider type registrations

workspace/project
  owns datasource declarations
  owns one lazy provider session per resolved datasource name
    owns backend handle, cache, and change set
    referenced by resolved table/reference collections
```

### Cleanup order

On project unload or replacement:

1. Stop views and detach master-detail subscriptions.
2. Cancel or invalidate in-flight fetches using the project generation token.
3. Resolve dirty sessions according to application policy; never silently save during destruction.
4. Send `DataProvider.Close` once to every opened session.
5. Release view records, provider caches, and provider instances.
6. Clear datasource declarations for that project.
7. Increment the registry generation so stale handles cannot resolve into a new project.

Module shutdown then unregisters provider types and clears remaining registry storage. Close and cleanup operations must be idempotent so a partially initialized provider is safe to tear down.

Provider callbacks must not return unowned roots with ambiguous lifetime. Resolution should return a retained session/view handle owned by the project registry, and callers should hold explicit references while using it.

## Compatibility and implementation sequence

1. Introduce provider sessions and lifecycle ownership without changing existing XML sample behavior.
2. Add provider collections, record handles, and provider-backed binding path resolution.
3. Implement XML provider fetch/get-value behavior without per-field `DataObject` proxies.
4. Add staged mutation messages, save/revert, and structured errors.
5. Add reference-column metadata, selected-record binding, and master-detail dependencies.
6. Port samples and deprecate direct file/child `DataContextSource` resolution.
7. Add another provider (CLI or SQL is a stronger contract test than a second file provider).

The XML provider may temporarily adapt an old `DataObject` source internally during migration, but that adapter must be behind the provider contract. Controls and bindings should see records and columns, not that compatibility tree.

## Required tests

- A row with many columns creates one record handle, not one object per field.
- `GetValue` and `SetValue` dispatch to overridden provider handlers.
- Insert, update, delete, save, failure, and revert preserve correct dirty state.
- Two views on one provider commit through one `SaveChanges` operation.
- Master selection scopes a detail fetch through a declared relationship.
- Rapid master changes discard stale asynchronous detail results.
- Detail insertion inherits the master relationship identity.
- Dirty-detail master changes enforce the configured policy.
- Provider-reported reference columns work without join metadata in UI markup.
- Project unload closes each provider session once and clears declarations and caches.
- Repeated project load/unload does not grow registry entry counts.
- Deprecated direct file/child syntax warns while migrated name-based sources remain warning-free.

## Decisions

- Use a provider session as the backend gateway and transaction boundary.
- Resolve provider table paths to internal collection objects; do not require application-declared `DataView` objects.
- Represent rows with lightweight handles and resolve fields through messages; do not create a `DataObject` per field.
- Declare relationships as reference-column metadata and make detail collections follow master selection automatically.
- Keep schemas optional; schema-less providers must report equivalent reference-column metadata to support relationships.
- Make provider cleanup project-scoped and deterministic.

## Open implementation questions

- Whether the record handle is a small `DataRecord` component or an opaque value understood directly by the binding VM. A component is easier to integrate initially; it must still avoid child field objects.
- Whether pending values live in the generic session layer or entirely inside each provider. A generic change-set envelope with provider-owned payloads is likely the best balance.
- Whether provider paths should allow an omitted `ProjectName` when resolving within the current project; the canonical form remains `ProjectName/DataProvider/TableName`.
- Whether XML remains editable in the first mutation milestone or is initially read-only while a transactional test provider proves the write contract.
