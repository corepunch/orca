package.path = "apps/chat/?.lua;" .. package.path
local Chat = require "api.chat"
local Models = require "models"
local client = Chat.mock { clock = function() return 1700000000 end }
local models = Models.new(client)
local rows, err = models.Rooms:select()
assert(not rows and err.code == "unauthenticated", "Models must preserve API failures")
assert(client:signIn("demo@example.invalid", "password123"))

local alex = assert(models.Contacts:find { display_name = "Alex Morgan" })
assert(alex.id == "alex" and alex:get_display_name() == "Alex Morgan")
assert(models.Contacts:table_name() == "contacts")
assert(models.Contacts:find("missing") == nil and models.Contacts:find(nil) == nil)
local room = assert(models.Rooms:find("alex"))
local messages = assert(room:get_messages { limit = 2 })
assert(#messages == 2 and messages[1].id == "a3" and messages[2].id == "a4")
assert(messages[1]:get_sender():get_display_name() == "Alex Morgan", "Rows must expose relations")
local sent = assert(models.Messages:create { room_id = room.id, body = "From a model" })
assert(sent:get_sender():get_display_name() == "You", "Current-user relations use the API session")
assert(room:get_messages { limit = 1 }[1].id == sent.id)
local created = assert(models.Rooms:create { name = "Our model chat", member_ids = { alex.id } })
assert(#created:get_messages() == 0)
assert(#created:get_members() == 2)
assert(created:create_invite().room_id == created.id)
assert(created:send_message("Room relation write"):get_sender():get_display_name() == "You")
local value, failure = models.Messages:create { room_id = created.id, body = " " }
assert(not value and failure.code == "validation", "Model writes must share the normal client's validation")

local contactFetches, listContacts = 0, client.listContacts
client.listContacts = function(self)
    contactFetches = contactFetches + 1
    return listContacts(self)
end
local found = assert(models.Contacts:find_all { alex.id, alex.id, "missing" })
assert(#found == 1 and found[1].id == alex.id and contactFetches == 1, "find_all fetches once and deduplicates keys")
assert(#models.Contacts:find_all({ "Alex Morgan" }, { key = "display_name" }) == 1)
local before = contactFetches
assert(#models.Contacts:find_all {} == 0 and contactFetches == before, "An empty key set needs no fetch")
assert(#models.Messages:find_all({ sent.id }, { room_id = room.id, limit = 1 }) == 1, "Provider options must reach the fetch")

local unknown = models.Messages:load { id = "unknown", sender_id = "missing" }
local batch = { messages[1], sent, unknown }
assert(models.Messages:preload_senders(batch) == batch)
before = contactFetches
assert(batch[1]:get_sender():get_display_name() == "Alex Morgan")
assert(sent:get_sender():get_display_name() == "You")
assert(unknown:get_sender() == nil and unknown:get_sender() == nil)
assert(contactFetches == before, "Preloaded sender relations, including misses, must avoid N+1 fetches")
assert(models.Messages:preload_senders(batch) == batch and contactFetches == before + 1, "A new preload refreshes authorized contacts")

-- A second model registry cannot inherit the first account or its API state.
local otherClient = Chat.mock { clock = function() return 1700000000 end }
local other = Models.new(otherClient)
assert(otherClient:signIn("alex@example.invalid", "password123"))
assert(other.Rooms:find(created.id) == nil, "Independent backends must not leak created rows")
assert(other.Contacts:find("me"):get_display_name() == "You")
assert(client:getSession().user.id == "me" and otherClient:getSession().user.id == "alex")
assert(client:signOut())
rows, err = room:get_messages()
assert(not rows and err.code == "unauthenticated", "Existing rows must use the current session, not captured credentials")
rows, err = messages[1]:get_sender()
assert(not rows and err.code == "unauthenticated", "Sign-out must invalidate preloaded relations")
rows, err = models.Contacts:find_all { alex.id }
assert(not rows and err.code == "unauthenticated", "Bulk lookups preserve API errors")
assert(client:signIn("demo@example.invalid", "password123"))
assert(models.Messages:preload_senders(batch))
assert(client:signOut())
assert(client:signIn("alex@example.invalid", "password123"))
before = contactFetches
assert(sent:get_sender():get_display_name() == "You" and contactFetches == before + 1, "An account switch must discard a previous account's preload")

local interrupted = Models.new {
    session = { user = { id = "first" }, access_token = "one" },
    getSession = function(self) return self.session end,
    listContacts = function(self)
        self.session = { user = { id = "second" }, access_token = "two" }
        return { { id = "private-first-contact", display_name = "Private" } }
    end,
}
local stale = interrupted.Messages:load { id = "stale", sender_id = "private-first-contact" }
rows, err = interrupted.Messages:preload_senders { stale }
assert(not rows and err.code == "session_changed" and not stale._sender_relation, "A session change during a fetch cannot cache the old directory")
print("PASS: chat models (bulk queries, domain methods, batched relations, errors and session isolation)")
