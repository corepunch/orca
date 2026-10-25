local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local colors = C.colors
local Conversations = {}

local function publicName(person)
    return person.display_name and person.display_name ~= "" and person.display_name or person.username or "Member"
end

function Conversations.newRoom(self)
    if not self.client or not self.client:getSession() then return self:home() end
    if self.shell and self.shell:has("New") then self.status = self.newStatus; return self.shell:show("New") end
    local navigation = self.navigationEpoch
    self.newSelection = self.newSelection or {}
    local kind, visibility = "direct", "private"
    local name = C.input("RoomName", "Give your group or channel a name")
    local query = C.input("FindPerson", "@username, phone number or email")
    local inviteCode = C.input("InviteCode", "Paste an invitation code")
    self.status = C.text("Find someone by their exact username, or a phone/email they allow people to find.", {
        WordWrap = true, FontSize = 12, Foreground = colors.muted,
    })
    self.newStatus = self.status
    local status = self.newStatus
    local function notice(message, isError)
        status:setTextContent(message or "")
        status.Foreground = isError and colors.error or colors.muted
    end
    local epoch = self.sessionEpoch
    local selected = C.text("No one selected", { Name = "SelectedPeople", FontSize = 11, Foreground = colors.accentText, WordWrap = true })
    local results = C.stack({ Name = "DiscoveryResults", Spacing = 8 })
    local findPeople, findChannels, peopleMode, channelsMode
    local personButtons, discoveryGeneration = {}, 0
    local function clearResults()
        discoveryGeneration = discoveryGeneration + 1
        while results:getFirstChild() do results:getFirstChild():removeFromParent() end
        for _, buttons in pairs(personButtons) do
            for i = #buttons, 1, -1 do
                if buttons[i].Name:match("^FoundPerson") then table.remove(buttons, i) end
            end
        end
    end
    local function searchMode(channels)
        clearResults()
        findPeople.Visible, findChannels.Visible = not channels, channels
        peopleMode.Background = channels and colors.surface or colors.soft
        channelsMode.Background = channels and colors.soft or colors.surface
        query.PlaceholderText = channels and "Search public channels" or "@username, phone number or email"
        notice(channels and "Explore public broadcasts. Subscribe to read new posts from a channel."
            or "Find someone by their exact username, or a phone/email they allow people to find.")
    end
    peopleMode = C.button("People", function() searchMode(false) end,
        { Name = "SearchPeople", Height = 38, Background = colors.soft, FontSize = 12 })
    channelsMode = C.button("Channels", function() searchMode(true) end,
        { Name = "SearchChannels", Height = 38, Background = colors.surface, FontSize = 12 })
    local modes = ui.Grid { Columns = "1fr 1fr", Spacing = 8, Height = 38 }
    modes:addChild(peopleMode); modes:addChild(channelsMode)
    local people = C.stack({ Name = "Contacts", Spacing = 8 }, { C.eyebrow("YOUR PEOPLE") })
    local kindButtons = {}
    local visibilityButton
    local function updateSelection()
        local names = {}
        for _, person in pairs(self.newSelection) do names[#names + 1] = publicName(person) end
        table.sort(names)
        selected:setTextContent(#names > 0 and table.concat(names, ", ") or "No one selected")
        for id, buttons in pairs(personButtons) do
            for _, button in ipairs(buttons) do button.Background = self.newSelection[id] and colors.soft or colors.surface end
        end
    end
    local function addPerson(parent, person, prefix)
        local label = publicName(person) .. (person.username and "  ·  @" .. person.username or "")
        local button
        button = C.button(label, function()
            if self.newSelection[person.id] then self.newSelection[person.id] = nil
            else self.newSelection[person.id] = person end
            if kind == "direct" and self.newSelection[person.id] then self.newSelection = { [person.id] = person } end
            updateSelection()
        end, { Name = prefix .. person.id, Background = self.newSelection[person.id] and colors.soft or colors.surface, Height = 48, FontSize = 12 })
        personButtons[person.id] = personButtons[person.id] or {}
        table.insert(personButtons[person.id], button)
        parent:addChild(button)
    end
    local kindRow = ui.Grid { Columns = "1fr 1fr 1fr", Spacing = 8, Height = 44 }
    for _, item in ipairs { { "direct", "Direct" }, { "group", "Group" }, { "channel", "Channel" } } do
        kindButtons[item[1]] = C.button(item[2], function()
            kind = item[1]
            for key, button in pairs(kindButtons) do button.Background = key == kind and colors.soft or colors.surface end
            name.Visible = kind ~= "direct"
            visibilityButton.Visible = kind ~= "direct"
            if kind == "direct" then self.newSelection = {}; updateSelection() end
            notice(kind == "channel" and "A broadcast space. Only owners and admins can publish."
                or kind == "group" and "A shared space where members can talk." or "Choose one person to start a private chat.")
        end, { Name = "Kind" .. item[2], Background = item[1] == kind and colors.soft or colors.surface, FontSize = 12 })
        kindRow:addChild(kindButtons[item[1]])
    end
    name.Visible = false
    visibilityButton = C.button("Private · invitation required", function()
        visibility = visibility == "private" and "public" or "private"
        visibilityButton:setTextContent(visibility == "private" and "Private · invitation required" or "Public · share with everyone")
    end, { Name = "ConversationVisibility", Background = colors.surface, FontSize = 12, Visible = false })
    local contacts, contactsError = self.client:listContacts()
    if self.navigationEpoch ~= navigation or self.sessionEpoch ~= epoch then return self.screen end
    if contactsError then people:addChild(C.text(contactsError.message, { WordWrap = true, Foreground = colors.error }))
    elseif #contacts == 0 then people:addChild(C.text("Your conversations will bring your people here.", { FontSize = 12, WordWrap = true, Foreground = colors.dim }))
    else for _, person in ipairs(contacts) do addPerson(people, person, "Contact") end end
    findPeople = C.button("Find person", function()
        clearResults()
        local generation = discoveryGeneration
        local found, err = self.client:findPeople(query.Text)
        if self.sessionEpoch ~= epoch or generation ~= discoveryGeneration then return end
        if not found then notice(err.message, true); return end
        if #found == 0 then results:addChild(C.text("No matching person. Try a username or ask for an invite.", {
            FontSize = 12, Foreground = colors.muted, WordWrap = true })) end
        for _, person in ipairs(found) do addPerson(results, person, "FoundPerson") end
    end, { Name = "FindPeople", Background = colors.soft })
    findChannels = C.button("Find channels", function()
        clearResults()
        local generation = discoveryGeneration
        local found, err = self.client:findChannels(query.Text)
        if self.sessionEpoch ~= epoch or generation ~= discoveryGeneration then return end
        if not found then notice(err.message, true); return end
        if #found == 0 then results:addChild(C.text("No public channels found. Try another name.", {
            FontSize = 12, Foreground = colors.muted, WordWrap = true })) end
        for _, channel in ipairs(found) do
            local item = C.stack({ Name = "FoundChannel" .. channel.id, Spacing = 10, Padding = "14 12",
                Background = colors.surface }, {
                C.text(channel.name, { FontSize = 15, FontWeight = "Bold", WordWrap = true }),
                C.text("Public broadcast channel", { FontSize = 11, Foreground = colors.muted }),
                C.button(channel.role and "Open channel" or "Subscribe", function()
                    local room, joinError = self.client:joinPublicChannel(channel.id)
                    if self.sessionEpoch ~= epoch or generation ~= discoveryGeneration then return end
                    if not room then notice(joinError.message, true); return end
                    if self.shell and self.screen == self.shell.screen and self.shell.active == "New" then self:openRoom(room) end
                end, { Name = "JoinChannel" .. channel.id, Background = colors.soft }),
            })
            results:addChild(item)
        end
    end, { Name = "FindChannels", Background = colors.soft, Visible = false })
    local body = C.stack({ Padding = "24 18", Spacing = 16, Overflow = "hidden scroll" }, {
        C.text("A space for\nyour people.", { FontSize = 30, FontWeight = "Bold", WordWrap = true }),
        kindRow, name, visibilityButton, status,
        C.eyebrow("FIND PEOPLE & CHANNELS"), modes, query, findPeople, findChannels, results, people,
        C.eyebrow("SELECTED PEOPLE"), selected,
        C.button("Start conversation", function()
            local ids, selectedName = {}, nil
            for id, person in pairs(self.newSelection) do ids[#ids + 1] = id; selectedName = publicName(person) end
            table.sort(ids)
            if kind == "direct" and #ids ~= 1 then notice("Choose exactly one person for a direct chat.", true); return end
            local room, err = self.models.Rooms:create { name = kind == "direct" and selectedName or name.Text, member_ids = ids,
                kind = kind, visibility = kind == "direct" and "private" or visibility }
            if self.sessionEpoch ~= epoch then return end
            if not room then notice(err.message, true); return end
            name.Text, self.newSelection = "", {}; updateSelection()
            if self.shell and self.screen == self.shell.screen and self.shell.active == "New" then self:openRoom(room) end
        end, { Name = "CreateConversation", Background = colors.accent, Foreground = colors.onAccent, Height = 52 }),
        C.eyebrow("HAVE AN INVITATION?"), inviteCode,
        C.button("Join with invitation", function()
            local room, err = self.client:joinInvite(inviteCode.Text)
            if self.sessionEpoch ~= epoch then return end
            if not room then notice(err.message, true); return end
            inviteCode.Text = ""
            if self.shell and self.screen == self.shell.screen and self.shell.active == "New" then self:openRoom(room) end
        end, { Name = "JoinInvite", Background = colors.surface }),
    })
    local grid = ui.Grid { Rows = "76px 1fr", Background = colors.background }
    grid:addChild(self:pageHeader("New conversation")); grid:addChild(body)
    return self:frame(grid, "New")
end

function Conversations.details(self)
    if not self.room then return self:home() end
    local room, navigation, epoch = self.room, self.navigationEpoch, self.sessionEpoch
    local members, err = self.client:listMembers(room.id)
    if self.navigationEpoch ~= navigation or self.sessionEpoch ~= epoch then return self.screen end
    local canManage = room.role == "owner" or room.role == "admin"
    local body = C.stack({ Padding = "24 20", Spacing = 14, Overflow = "hidden scroll" }, {
        C.avatar(room.name, 72, room.id), C.text(room.name, { FontSize = 28, FontWeight = "Bold", WordWrap = true }),
        C.badge((room.kind or "group"):upper(), { Width = 100, Background = colors.soft, Foreground = colors.accentText }),
    })
    self.status = C.text(err and err.message or (room.kind == "channel" and "Only owners and admins can publish here." or "A space for everyone in this conversation."), {
        WordWrap = true, FontSize = 12, Foreground = err and colors.error or colors.muted })
    body:addChild(self.status)
    body:addChild(C.eyebrow("PEOPLE"))
    for _, member in ipairs(members or {}) do
        local id = member.user_id or member.id
        local info = member.profile or member
        local row = C.stack({ Name = "Member" .. id, Spacing = 8, Padding = "14 12", Background = colors.surface }, {
            C.text(publicName(info), { FontWeight = "Bold", FontSize = 14 }),
            C.text(member.role or "member", { FontSize = 11, Foreground = colors.muted }),
        })
        if canManage and id ~= self.client:getSession().user.id and member.role ~= "owner" then
            if room.role == "owner" then
                row:addChild(C.button(member.role == "admin" and "Make member" or "Make admin", function()
                    local ordinary = room.kind == "channel" and "subscriber" or "member"
                    local ok, roleError = self.client:setMemberRole(room.id, id, member.role == "admin" and ordinary or "admin")
                    if not ok then self:notice(roleError.message, true) else self:show("/details") end
                end, { Name = "Role" .. id, Background = colors.soft, Height = 40, FontSize = 12 }))
            end
            row:addChild(C.button("Remove", function()
                local ok, removeError = self.client:removeMember(room.id, id)
                if not ok then self:notice(removeError.message, true) else self:show("/details") end
            end, { Name = "Remove" .. id, Foreground = colors.error, Height = 40, FontSize = 12 }))
        end
        body:addChild(row)
    end
    if canManage and room.kind ~= "direct" then
        local invite = self.inviteCodes[room.id]
        local inviteText = C.text(invite and "Invitation: " .. invite or "Invitations expire and can be revoked.", {
            Name = "Invitation", FontSize = 12, WordWrap = true, Foreground = colors.muted })
        local inviteField = C.input("InvitationCode", "Invitation code", { Text = invite or "", Visible = invite ~= nil })
        body:addChild(C.eyebrow("INVITE PEOPLE")); body:addChild(inviteText)
        body:addChild(inviteField)
        body:addChild(C.button("Create invitation", function()
            if invite then self:notice("Revoke the current invitation before creating a new one."); return end
            local result, inviteError = self.client:createInvite(room.id)
            if not result then self:notice(inviteError.message, true); return end
            invite = result.code or result.token
            self.inviteCodes[room.id] = invite
            inviteField.Text, inviteField.Visible = tostring(invite), true
            inviteText:setTextContent("Invitation: " .. tostring(invite) .. "\nShare this code with someone you trust.")
        end, { Name = "CreateInvite", Background = colors.soft }))
        body:addChild(C.button("Revoke invitation", function()
            if not invite then self:notice("Create an invitation first."); return end
            local ok, revokeError = self.client:revokeInvite(room.id, invite)
            if not ok then self:notice(revokeError.message, true); return end
            inviteText:setTextContent("Invitation revoked."); invite, self.inviteCodes[room.id] = nil, nil
            inviteField.Text, inviteField.Visible = "", false
        end, { Name = "RevokeInvite", Foreground = colors.error }))
    end
    local header = ui.Grid { Columns = "44px 1fr", Padding = "14 16", Spacing = 10 }
    header:addChild(C.iconButton("arrow-left", function() self:show("/thread") end, { Name = "BackToThread" }))
    header:addChild(C.text("Conversation info", { FontSize = 16, FontWeight = "Bold", TextVerticalAlignment = "Center" }))
    local grid = ui.Grid { Rows = "76px 1fr", Background = colors.background }
    grid:addChild(header); grid:addChild(body)
    return self:frame(grid)
end

return Conversations
