# Models, API clients, and offline tests

Keep data ownership outside views. For networked apps, use this dependency
direction:

```text
View/controller → client-bound models → one application API → chosen transport
                                                         ├─ HTTP service
                                                         └─ in-process mock + fixtures
```

Choose the backend at app construction. Pass the resulting client into models
and controllers; they use the same operations in both modes. Keep mode checks
for user-facing status/configuration, rather than alternate data paths inside
pages. Never silently fall back to mock data when a real service fails.

## Lapis-inspired model boundary

[Lapis](https://github.com/leafo/lapis) and its
[model manual](https://leafo.net/lapis/reference/models.html) provide a useful
Lua convention: a named model represents a collection, returned rows expose
fields and domain methods, and relations connect those rows. Adapt that
interface to the application's service contract. ORCA client apps should not
connect directly to the server database or ship database credentials.

The chat app demonstrates this in [models.lua](../../apps/chat/models.lua) and
its small [model adapter](../../apps/chat/api/model.lua):

```lua
local models = require("Chat/models").new(client)
local contacts, err = models.Contacts:select()
local alex = models.Contacts:find { display_name = "Alex Morgan" }
local room = models.Rooms:create { name = "Weekend", member_ids = { alex.id } }
local messages = room:get_messages { limit = 20 }
local sent = models.Messages:create { room_id = room.id, body = "See you soon" }
```

Handle `nil, error` before using results in production actions. Scope a registry
to one client/account; avoid mutable global model configuration. Give row
methods only the operations the backend actually supports. Make query limits,
ordering, missing rows, and relation-fetch costs explicit. Load related data in
one action and index it for repeated presentation when possible.

The chat adapter is an application example, not an installed Lapis ORM or a
new engine binding. Its `find` matches fields within a fetched result window.
It does not provide SQL, update/delete operations, automatic collection
reconciliation, or observable `DataContext` objects. Continue to use the
explicit view-update rules in [the app guide](index.md).

## Interchangeable mock API

Test the same API client used by the real app. Replace its transport with an
in-process handler that accepts the same request shape and returns the same
response/error shape. Keep session handling, input validation, parsing, models,
and presentation above this seam. This follows the useful part of Lapis's
[request simulation](https://leafo.net/lapis/reference/testing.html): exercise
application behavior without an HTTP server.

The chat [mock transport](../../apps/chat/api/mock_transport.lua) owns accounts,
tokens, contacts, memberships, rooms, and messages. Its
[fixtures](../../apps/chat/api/mock_fixtures.lua) are copied per instance. Use an
injected clock for repeatable timestamps and expiry. Start signed out and
exercise login; don't bypass it by supplying an authenticated UI fixture.
Signing out clears account-specific views and drafts, while the backend's data
remains available to a later login in that same mock instance.

For tests and previews:

- Select mock mode explicitly and avoid importing the network transport.
- Reject unsupported mock endpoints rather than supplying plausible data.
- Exercise auth failures, membership restrictions, ordering, sends, and signup.
- Verify fresh instances cannot leak fixture mutations, sessions, or writes.
- Drive UI login, contact selection, message sending, and logout with real
  control events. Keep API method stubs out of these flow tests.
- Use separate protocol tests for network errors and malformed responses.
- Keep live backend checks for SQL/RLS, real JWTs, email confirmation, and
  multi-user behavior; a mock does not prove those integrations.

See [the chat API](../../apps/chat/API.md) for the supported contract and
`make test-chat` for the executable example.

## Photos and binary transport

Keep native file selection/encoding, the media upload API, and timeline views
separate. The chat [media adapter](../../apps/chat/media.lua) owns preview files
per app instance and removes them at sign-out. Preserve private temporary-file
permissions when adding a decoder-required filename extension; do not remove a
secure temporary file and recreate a predictable sibling with default permissions.

The generated `orca.media` API provides synchronous Apple still-image processing
and a modal macOS picker. Handle cancellation separately from invalid data.
The application uploads the processed JPEG and thumbnail through the same API
and mock transport. Storage reservations and message publication have explicit
states so a failed send can reuse its upload without duplicating the message.

Native transport tests must preserve embedded NUL bytes in both directions.
An in-process mock cannot detect a libcurl string-length bug, so retain a
separate loopback transport regression. Likewise, test the actual JSON encoder
for empty RPC arrays: an empty Lua table may serialize to `{}` while the server
expects `[]`. The chat adapter marks array-valued request fields explicitly.

For image UI verification, assert decoded texture dimensions and inspect a
native frame. A non-nil `ImageView.Source` alone does not prove the resource is
initialized or visible. For timeline scrolling, use the runtime's exported
vector fields: `ContentOffset = { X = 0, Y = offset }`. Check that the entire
latest bubble, image, caption, and time fit inside the viewport above the composer.
