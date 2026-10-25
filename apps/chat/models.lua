local Model = require "api.model"

local function sessionScope(client)
    local session = client:getSession()
    if not session then return nil end
    return { user_id = session.user.id, token = session.access_token, active = client.session }, session
end

local function sameScope(left, right)
    return left and right and left.user_id == right.user_id and left.token == right.token and left.active == right.active
end

local Contacts = Model:extend("contacts", {
    fetch = function(api) return api:listContacts() end,
    get_display_name = function(self) return self.display_name ~= "" and self.display_name or "Member" end,
})

local Rooms = Model:extend("rooms", {
    fetch = function(api) return api:listRooms() end,
    insert = function(api, values) return api:createRoom(values.name, values.member_ids, { kind = values.kind, visibility = values.visibility }) end,
    get_messages = function(self, options)
        return self._model.models.Messages:select { room_id = self.id, limit = options and options.limit }
    end,
    get_members = function(self) return self._model.client:listMembers(self.id) end,
    create_invite = function(self) return self._model.client:createInvite(self.id) end,
    send_message = function(self, body, attachmentIds)
        return self._model.models.Messages:create { room_id = self.id, body = body, attachment_ids = attachmentIds }
    end,
})

local Messages = Model:extend("messages", {
    fetch = function(api, options) return api:listMessages(options.room_id, { limit = options.limit }) end,
    insert = function(api, values) return api:sendMessage(values.room_id, values.body, values.attachment_ids) end,
    get_sender = function(self)
        local contacts = self._model.models.Contacts
        local scope, session = sessionScope(self._model.client)
        if self._sender_relation then
            if sameScope(self._sender_relation.scope, scope) then
                return self._sender_relation.rows[self.sender_id] or nil
            end
            self._sender_relation = nil
        end
        if session and session.user.id == self.sender_id then
            return contacts:load { id = self.sender_id, display_name = (session.user.user_metadata or {}).display_name }
        end
        return contacts:find(self.sender_id)
    end,
})

-- Relations belong to this batch and session, including missing contacts. A new
-- batch refetches the authorized directory rather than retaining a global cache.
function Messages:preload_senders(messages)
    assert(type(messages) == "table", "preload_senders needs message rows")
    for _, message in ipairs(messages) do
        assert(message._model == self, "Preload rows through their own client-bound model")
    end
    if #messages == 0 then return messages end
    local scope, session = sessionScope(self.client)
    local contacts, err = self.models.Contacts:select()
    if not contacts then return nil, err end
    -- A yielding fetch may finish after sign-out or an account switch.
    if not sameScope(scope, sessionScope(self.client)) then
        return nil, { code = "session_changed", message = "The active account changed while loading senders." }
    end
    local byId = {}
    for _, contact in ipairs(contacts) do byId[contact.id] = contact end
    if session then
        byId[session.user.id] = self.models.Contacts:load {
            id = session.user.id, display_name = (session.user.user_metadata or {}).display_name,
        }
    end
    local relation = { scope = scope, rows = byId }
    for _, message in ipairs(messages) do message._sender_relation = relation end
    return messages
end

local Models = {}
function Models.new(client)
    local models = {
        Contacts = Contacts:using(client), Rooms = Rooms:using(client), Messages = Messages:using(client),
    }
    for _, model in pairs(models) do model.models = models end
    return models
end

return Models
