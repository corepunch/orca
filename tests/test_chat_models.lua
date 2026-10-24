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
local value, failure = models.Messages:create { room_id = created.id, body = " " }
assert(not value and failure.code == "validation", "Model writes must share the normal client's validation")

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
print("PASS: chat models (query, create, row methods, relations, errors and client isolation)")
