local fs = require "orca.filesystem"
local Items = require "orca.UIKit.ItemsView"
local C = require "Chat/views/components"
local colors = C.colors
local ContactItems = {}
local columns = { Label = "string", Username = "string", Initials = "string",
    Tone = "string", Ink = "string", Text = "string", Muted = "string" }

function ContactItems.new(app, node, template, activate)
    local portrait = template == "ContactPortrait"
    return Items.new {
        node = node, template = assert(fs.loadObject("Chat/Templates/" .. template)),
        record = Items.record("ChatContactPresentation", columns), key = function(person) return person.id end,
        fields = function(person)
            local name = person.display_name and person.display_name ~= "" and person.display_name or person.username or "Member"
            local initials = (name:match("^([%w])") or "C") .. (name:match("%s+([%w])[%w]*$") or "")
            local hash = 0
            for i = 1, #person.id do hash = hash + person.id:byte(i) * i end
            local tone = colors.avatar[hash % #colors.avatar + 1]
            return { Label = name, Username = person.username and "@" .. person.username or "",
                Initials = initials:upper(), Tone = tone.background, Ink = tone.foreground, Text = colors.ink, Muted = colors.dim }
        end,
        update = function(entry, person, created)
            entry.node.Name = (portrait and "ContactsTile" or "ContactsRow") .. person.id
            entry.node.Click = function() activate(entry.value) end
            if created then app:avatarPhoto(assert(entry.node:findChild("ContactAvatar", true)), person.id) end
        end,
    }
end

return ContactItems
