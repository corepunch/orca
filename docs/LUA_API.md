# Lua API

Use this file when changing Lua-visible behavior.

## Where the Lua API comes from

- Generated bindings from module `.cgen` files
- Handwritten object/Lua bridge files under `source/core/object/`
- Runtime Lua code under `share/orca/`

## Module loading rules

- Built-in modules are registered in `package.preload` and execute on first `require`.
- `orca.<name>` auto-requires the matching module through the `orca` table.
- Native builds load plugin modules from shared libraries.
- WebGL builds preload plugin modules into the binary.

## Change workflow

1. If the API shape changes, edit the module `.cgen` first.
2. Run `make modules`.
3. Implement the runtime behavior in handwritten C or Lua.
4. Add a focused Lua test.

## Important boundaries

- Object/Lua bridge files live in `source/core/object/`, not `source/object/`.
- If module init order matters, call `require` explicitly.
- Prefer messages and properties over tight plugin-to-plugin coupling.

## DataContext and datasource API

- `DataContext` is the base class for data-providing objects. It handles the `DataContext.GetData` message.
- `DataSource` (parent: DataContext) owns a root `DataObject` tree loaded by a concrete subclass.
- `XmlDataSource` (parent: DataSource) loads data from XML, optionally backed by a `Schema.xml`.
- `DataObject` (parent: DataContext) is the runtime representation of a schema entity instance.
- Schema files (`DS_ParseSchema`, `DS_ParseSchemaFromString` in `source/data/data_schema.c`) register `ClassDesc` entries with typed column properties.
- Binding engine walks the `DataContext` chain: DataSource returns its root DataObject, DataObject returns itself. Root and child collection bindings export Object pointers; missing object sources clear the destination.
- DataObject child changes notify ItemsControl automatically. Use `addChild`, `removeFromParent`, `clear`, and `moveBefore(sibling)`; moving within a parent preserves lifecycle and references. `beginUpdate()` / `endUpdate()` batch membership notifications, including nested batches.
- `orca.data` declares schemas, scopes, queries, views, commands, edits, media and routes for data-driven screens; `orca.data.lint` checks page bindings and `orca.data.fixtures` previews providers offline. See [data-driven screens](app/datasources.md).

## HTTP responses and private sessions

The handwritten `orca.network` response userdata provides `text()`, `json()`,
`size()`, `code()` and `headers()`. Native `headers()` returns lowercase names
mapped to arrays of values, preserving repeated `set-cookie` lines. Each call
returns a fresh table and only the final HTTP response's headers are retained.
The WebGL binding currently returns an empty header table; browser restrictions
also prevent exposing session cookies through response headers.

Use `network.fetch(url, {nocookies=true, ...})` with an explicit per-client
`Cookie` header when a service needs isolated sessions. `nocookies` disables
both loading and saving the legacy shared cookie jar. Session cookies must not
be logged or stored in app assets. Chat's native Appwrite Account adapter
demonstrates this boundary.

## Native photo preparation

`require "orca.media"` exposes `pickPhoto()` (modal macOS file selection) and
`processPhoto(encodedBytes)` (Apple ImageIO conversion). Both return a processed
photo table or `nil, {code,message}`; picker cancellation uses code `cancelled`.
The result contains binary JPEG/thumbnail strings, dimensions, byte counts,
MIME type and quality. Input and output bounds, metadata behavior, and platform
support are declared in `source/media/media.cgen`. These functions prepare
bytes; storage and messaging remain application API responsibilities.

## Native push notifications

`require "orca.notifications"` exposes `supported()`,
`requestAuthorization()` and `poll()`. On iOS and macOS, authorization is
asynchronous: a successful request means it was queued, not that permission
or APNs registration succeeded. Poll returns one queued event or `nil`.
Token events contain `{type="token", token=...}`; notification events contain
`type="open"` or `"received"` and the application's `conversationId`,
`messageId` and `userId`. Errors arrive as `{type="error", code=..., message=...}`;
codes distinguish denied permission from retryable registration failures.
Native callbacks queue data without calling Lua from another thread.

Command-line tools report unsupported. Delivery requires a signed application
bundle with APNs capabilities and a matching
provider. Apps own authenticated device-target registration, token updates,
logout cleanup and authorization when navigating from a notification. Other
platforms report unsupported. The API contract is declared in
`source/notifications/notifications.cgen`.

## Secure credentials

`require "orca.credentials"` exposes `supported()`, `get(key)`,
`set(key, value)` and `delete(key)`. Apple platforms use the Keychain, scoped
to the application bundle identifier. Writes complete synchronously and use
device-only, unlocked-device accessibility; credentials are not synchronized.
An absent key returns `nil`; failures return `nil, {code,message}`. Unsupported
platforms return `unsupported` and never store credentials in plaintext.
`supported()` only reports platform capability; a locked or inaccessible
Keychain still returns an error. Apps must namespace keys by backend/project,
validate restored sessions with their service, and delete credentials on logout.

Runtime vector userdata uses exported `X`/`Y` field names, for example
`node.ContentOffset = { X = 0, Y = -100 }`. These differ from lowercase C struct
members; verify runtime field names when constructing values for property setters.

## Event arguments

Native event callbacks receive `(owner, args, sender)`. `args` keeps its generated
userdata type and field names. It is an owned snapshot: strings and reflected
nested structs are copied, and Object/component fields retain their objects.
Handlers may keep the snapshot across a yield; mutating it does not alter the
producer's original payload. Replacing or clearing an event callback releases
its previous Lua registry reference.

Opaque native pointers remain borrowed, including the separate native queue
target. Retaining an Event does not retain an arbitrary native target; process
peeked events promptly while their target is valid. These payloads are runtime
messages, not a portable wire format. Persist model values and stable IDs rather
than pointer addresses. Reflected fields preserve unsigned 32-bit values and
reject values outside their range.

Snapshots support fixed arrays and acyclic reflected structs up to 32 levels.
Cycles and arrays without length metadata are rejected instead of shallow copied.

## Asynchronous errors

Load `orca.core.async` and call `set_error_handler(fn)` before starting an app
to receive failures from `async.run` and native event callbacks. Native failures
include a Lua traceback. Handlers run synchronously and must not yield.
Use `async.report_error(err)` for failures caught by application code, and
`set_error_handler(nil)` to restore stderr reporting. A failing or reentrant
handler falls back to stderr so later events and coroutine cleanup can proceed.

## Timers and foreground work

Use `core.Timer { Tick = function(owner, args) ... end }` as a nonvisual timer
owner. `owner:setTimer(milliseconds)` starts a repeating native timer and returns
its handle. Call `owner:cancelTimer(handle)` before a one-shot callback performs
work or before rescheduling. Handlers receive `args.timerId` through the normal
event coroutine dispatcher, so they may yield for network requests. Cancellation
does not recall queued events: compare the ID and an application generation
before starting or applying asynchronous work. Destroying the owner cancels its
native timers; retain it while scheduled.

On iOS, `Window.Deactivated` is queued before the display loop pauses and
`Window.Activated` before it resumes. Lua does not advance while inactive; both
messages are consumed in order on resumption. Cancel foreground work on
deactivation and refresh on activation. These are scene events, independent of
keyboard focus.

## Good follow-up reads

- `docs/lua-scripting.md`
- `docs/app/moonscript/scenes-and-rebuild.md`
- `docs/app/xml/triggers-actions-messages.md`
