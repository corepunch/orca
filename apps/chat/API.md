# Chat Lua API

`require "api.chat"` is an application Lua API, following ORCA's package
`ProjectReferences` convention. It uses the existing `orca.network` and
`orca.parsers.json` bindings; it does not add engine classes.

```lua
local Chat = require "api.chat"
local client = Chat.demo() -- in-memory sample conversations; no network
local rooms, err = client:listRooms()
local messages = client:listMessages(rooms[1].id)
local sent = client:sendMessage(rooms[1].id, "Hello!")
```

Demo messages last for this app process. Demo mode never sends a message to a
real person and never invents replies. Start a fresh `Chat.demo()` to reset it.

For a real backend:

```lua
local client, err = Chat.supabase {
  url = "https://YOUR_PROJECT.supabase.co",
  publishableKey = "sb_publishable_YOUR_KEY",
}
local session, err = client:signIn("you@example.com", "your password")
local room, err = client:createRoom("Weekend", { "OTHER_USER_UUID" })
```

Remote methods yield through `network.fetch`, so invoke them inside ORCA's
event coroutine context. The UI can keep showing a loading state while they
run. The implementation catches thrown HTTP response userdata and maps it to
the same error contract as transport failures.

All methods return `value, nil` on success and `nil, error` on failure. Errors
have `code`, `message`, and an optional numeric HTTP `status`.

| Method | Result |
| --- | --- |
| `getSession()` | Session copy or nil; no HTTP request |
| `signIn(email, password)` | Session with `user`, access/refresh tokens and `expires_at` |
| `signUp(email, password, displayName)` | Session, or `{confirmationRequired=true}` if email confirmation is enabled |
| `refreshSession()` | Rotated session; automatically called near expiry or once after HTTP 401 |
| `signOut()` | Clears local credentials and requests local-session revocation; can return an HTTP/network error |
| `listRooms()` | Rooms `{id,name,created_at}` visible to the signed-in member |
| `createRoom(name, memberIds)` | New room; include the other participants' Supabase Auth UUIDs |
| `listMessages(roomId, {limit=100})` | Latest 1–100 messages in chronological order |
| `sendMessage(roomId, body)` | Persisted message `{id,room_id,sender_id,body,created_at}` |

Passwords, sessions, and tokens are held in memory only. Restarting requires
sign-in. There is no insecure file persistence. Do not log session objects.
The client accepts only publishable keys, so secret/service-role keys cannot
be accidentally used in app configuration. Protected requests use the user's
JWT; the publishable key identifies the app, and database RLS authorizes rows.

The first version refreshes conversation history on request. It does not yet
implement WebSocket subscriptions, push notifications, attachments, read
receipts, member discovery, older-history pagination, or end-to-end encryption.

## Supabase setup

1. Create a Supabase project. No cloud project is created by this repository.
2. Run `backend/schema.sql` once in its SQL editor. The migration creates
   `chat_rooms`, `chat_members`, `chat_messages`, membership RLS policies, and
   an atomic authenticated `create_chat_room` function. Anonymous access is
   revoked. Members can read their rooms and send messages under their own ID.
   A user cannot join an existing room by inserting a membership row.
3. Enable email/password sign-in in Authentication. Keep email confirmation
   enabled for real users; configure email delivery before inviting users.
4. Copy the project URL and **publishable** key from the Connect dialog into
   the app's local configuration. Never copy a secret or service-role key.
5. Create and confirm two accounts. Copy the other account's UUID from the
   project's Authentication users table to start a conversation. There is no
   public user directory in this prototype.
6. Sign into both accounts in separate clients and refresh to receive the
   other user's messages. Test a third account: it must see no private rooms
   and must be rejected when sending to a room it does not belong to.

The schema intentionally allows a signed-in user who knows another user's
UUID to invite them into a newly created room. Production invitation consent,
abuse limits, retention, and account management remain future product work.
The SQL migration has not been applied against a live account automatically.

## Testing and transport injection

Pass `transport = function(request) ... end` to `Chat.supabase` to substitute
HTTP in tests. Requests contain `method`, `url`, `headers`, and an optional
Lua table `body`; responses contain numeric `status` and a decoded table
`body`. Pass `clock = function() return unixSeconds end` for expiry tests.

From the repository root, run `lua tests/test_chat_api.lua` with Lua 5.3+, or
use the ORCA headless runner's `-test=tests/test_chat_api.lua` option. The suite
does not access the network or need credentials.

Backend choice and protocol references: Supabase combines [Auth and JWT-backed
database authorization](https://supabase.com/docs/guides/auth) with
[Postgres row-level security](https://supabase.com/docs/guides/database/postgres/row-level-security).
Its [Auth API specification](https://github.com/supabase/auth/blob/master/openapi.yaml)
documents password/refresh token grants. The [API key guide](https://supabase.com/docs/guides/getting-started/api-keys)
specifies publishable keys for desktop and mobile clients and secret keys for servers.
