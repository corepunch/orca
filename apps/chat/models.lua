local Model = require "api.model"

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
})

local Messages = Model:extend("messages", {
    fetch = function(api, options) return api:listMessages(options.room_id, { limit = options.limit }) end,
    insert = function(api, values) return api:sendMessage(values.room_id, values.body, values.attachment_ids) end,
    get_sender = function(self)
        local contacts = self._model.models.Contacts
        local session = self._model.client:getSession()
        if session and session.user.id == self.sender_id then
            return contacts:load { id = self.sender_id, display_name = (session.user.user_metadata or {}).display_name }
        end
        return contacts:find(self.sender_id)
    end,
})

local Models = {}
function Models.new(client)
    local models = {
        Contacts = Contacts:using(client), Rooms = Rooms:using(client), Messages = Messages:using(client),
    }
    for _, model in pairs(models) do model.models = models end
    return models
end

return Models
