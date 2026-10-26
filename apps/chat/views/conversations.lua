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
    local epoch = self.sessionEpoch
    local mode, visibility, generation, busy = "direct", "private", 0, false
    local page
    self.newSelection = {}
    local query = C.input("FindPerson", "Search username, email or phone")
    local searchRow = ui.Grid { Name = "PeopleSearch", Columns = "1fr auto auto", Spacing = 8, Height = 48 }
    local name = C.input("RoomName", "Group name")
    local inviteCode = C.input("InviteCode", "Invitation code")
    local status = C.text("", { Name = "NewChatStatus", WordWrap = true, FontSize = 14, Foreground = colors.muted })
    self.status, self.newStatus = status, status
    local function notice(message, isError)
        status:setTextContent(message or "")
        status.Visible = message ~= nil and message ~= ""
        status.Foreground = isError and colors.error or colors.muted
    end
    local function active()
        return self.sessionEpoch == epoch and (not self.shell or
            self.screen == self.shell.screen and self.shell.active == "New")
    end
    local actions = C.stack({ Name = "NewChatActions", Spacing = 0 })
    local results = C.stack({ Name = "DiscoveryResults", Spacing = 8 })
    local people = C.stack({ Name = "Contacts", Spacing = 4 })
    local selected = C.text("", { Name = "SelectedPeople", FontSize = 15, Foreground = colors.muted, WordWrap = true })
    local heading = C.eyebrow("ADD MEMBERS")
    local setup = C.stack({ Name = "ConversationSetup", Spacing = 14 })
    local invitation = C.stack({ Name = "InvitationSetup", Spacing = 14 })
    local personButtons = {}
    local findPeople, findChannels, nextButton, backButton, createButton, visibilityButton
    local function clearResults()
        generation = generation + 1
        while results:getFirstChild() do results:getFirstChild():removeFromParent() end
        for _, buttons in pairs(personButtons) do
            for i = #buttons, 1, -1 do
                if buttons[i].Name:match("^FoundPerson") then table.remove(buttons, i) end
            end
        end
    end
    local function discoveryLoading()
        local pending = ui.Node2D { Height = 80 }
        pending:addChild(ui.ActivityIndicator { Name = "DiscoveryLoading", Foreground = colors.muted })
        results:addChild(pending)
        return pending
    end
    local function updateSelection()
        local names = {}
        for _, person in pairs(self.newSelection) do names[#names + 1] = publicName(person) end
        table.sort(names)
        selected:setTextContent(#names > 0 and table.concat(names, ", ") or "Select members or continue with an empty group.")
        for id, buttons in pairs(personButtons) do
            for _, button in ipairs(buttons) do button.Background = self.newSelection[id] and colors.soft or colors.surface end
        end
    end
    local function open(room)
        self.newSelection = {}; updateSelection()
        if active() then self:openRoom(room) end
    end
    local function addPerson(parent, person, prefix)
        local label = publicName(person) .. (person.username and "  ·  @" .. person.username or "")
        local button = C.button(label, function()
            if busy or not active() then return end
            if mode == "direct" then
                busy = true
                local room, err = self.models.Rooms:create { name = publicName(person), member_ids = { person.id },
                    kind = "direct", visibility = "private" }
                busy = false
                if not active() then return end
                if not room then notice(err.message, true); return end
                open(room)
            elseif mode == "group" then
                self.newSelection[person.id] = not self.newSelection[person.id] and person or nil
                updateSelection()
            end
        end, { Name = prefix .. person.id, Background = self.newSelection[person.id] and colors.soft or colors.surface, Height = 52, FontSize = 18,
            FontWeight = "Normal", BorderWidth = "0", TextHorizontalAlignment = "Left" })
        personButtons[person.id] = personButtons[person.id] or {}
        table.insert(personButtons[person.id], button)
        local row = ui.Grid { Columns = "52px 1fr", Spacing = 10, Height = 52 }
        row:addChild(C.avatar(publicName(person), 52, person.id, self, person.id)); row:addChild(button)
        parent:addChild(row)
    end
    local function changeMode(value)
        if busy then return end
        mode, visibility = value, "private"
        clearResults(); notice("")
        query.Text, name.Text, inviteCode.Text = "", "", ""
        self.newSelection = {}; updateSelection()
        local browsing = mode == "direct" or mode == "group" or mode == "channels"
        actions.Visible = mode == "direct"
        searchRow.Visible, query.Visible, results.Visible = browsing, browsing, browsing
        people.Visible = mode == "direct" or mode == "group"
        findPeople.Visible = people.Visible
        findChannels.Visible = mode == "channels"
        heading.Visible, backButton.Visible = mode == "group", mode ~= "direct"
        heading:setTextContent("add members")
        if page then page:getFirstChild().Header = mode == "group" and "new group" or mode == "channel" and "new channel"
            or mode == "channels" and "channels" or mode == "invite" and "join chat" or "new message" end
        setup.Visible, invitation.Visible = mode == "channel", mode == "invite"
        selected.Visible, nextButton.Visible = mode == "group", mode == "group"
        name.PlaceholderText = mode == "channel" and "Channel name" or "Group name"
        query.PlaceholderText = mode == "channels" and "Search public channels" or "Search username, email or phone"
        visibilityButton.Visible = mode == "channel"
        visibilityButton:setTextContent("Private channel · invitation required")
        createButton:setTextContent(mode == "channel" and "Create channel" or "Create group")
    end
    backButton = C.button("Back", function()
        if busy then return end
        if mode == "group" and setup.Visible then
            clearResults(); notice("")
            heading.Visible, setup.Visible, people.Visible, searchRow.Visible, results.Visible = true, false, true, true, true
            findPeople.Visible, nextButton.Visible = true, true
        else changeMode("direct") end
    end, { Name = "NewChatBack", TextHorizontalAlignment = "Left" })
    for _, item in ipairs { { "KindGroup", "New Group", "group" }, { "KindChannel", "New Channel", "channel" },
        { "SearchChannels", "Browse Channels", "channels" }, { "ShowInvite", "Join with Invitation", "invite" } } do
        local target = item[3]
        actions:addChild(C.button(item[2], function() changeMode(target) end, {
            Name = item[1], TextHorizontalAlignment = "Left", FontSize = 18 }))
    end
    nextButton = C.button("Next", function()
        if mode ~= "group" or busy then return end
        clearResults(); notice("")
        heading.Visible = false
        setup.Visible, people.Visible, searchRow.Visible, results.Visible = true, false, false, false
        findPeople.Visible, nextButton.Visible = false, false
    end, { Name = "GroupNext", Background = colors.accent, Foreground = colors.onAccent })
    visibilityButton = C.button("Private channel · invitation required", function()
        if busy then return end
        visibility = visibility == "private" and "public" or "private"
        visibilityButton:setTextContent(visibility == "private" and "Private channel · invitation required" or "Public channel · anyone can join")
    end, { Name = "ConversationVisibility", Background = colors.surface })
    createButton = C.button("Create group", function()
        if busy or not setup.Visible or not active() then return end
        if not name.Text:match("%S") then notice(mode == "channel" and "Enter a channel name." or "Enter a group name.", true); name:setFocus(); return end
        local ids = {}
        for id in pairs(self.newSelection) do ids[#ids + 1] = id end
        table.sort(ids)
        busy = true
        local room, err = self.models.Rooms:create { name = name.Text, member_ids = ids, kind = mode,
            visibility = mode == "channel" and visibility or "private" }
        busy = false
        if not active() then return end
        if not room then notice(err.message, true); return end
        changeMode("direct"); open(room)
    end, { Name = "CreateConversation", Background = colors.accent, Foreground = colors.onAccent })
    setup:addChild(name); setup:addChild(visibilityButton); setup:addChild(createButton)
    invitation:addChild(inviteCode)
    invitation:addChild(C.button("Join", function()
        if busy or mode ~= "invite" or not active() then return end
        busy = true
        local room, err = self.client:joinInvite(inviteCode.Text)
        busy = false
        if not active() then return end
        if not room then notice(err.message, true); return end
        changeMode("direct"); open(room)
    end, { Name = "JoinInvite", Background = colors.accent, Foreground = colors.onAccent }))
    findPeople = C.iconButton("search", function()
        if busy or not active() or mode ~= "direct" and mode ~= "group" then return end
        clearResults(); notice("")
        if not query.Text:match("%S") then return end
        local request, searched, pending = generation, query.Text, discoveryLoading()
        local found, err = self.client:findPeople(searched)
        pending.Visible = false
        if not active() or generation ~= request or query.Text ~= searched then return end
        if not found then notice(err.message, true); return end
        if #found == 0 then results:addChild(C.text("No results. Ask them to set a public username in Profile, or enable Find me by email or phone for a verified login detail. You don't need to add a contact first.", {
            Name = "NoPeopleResults", FontSize = 14, Foreground = colors.muted, WordWrap = true })) end
        for _, person in ipairs(found) do addPerson(results, person, "FoundPerson") end
    end, { Name = "FindPeople", Width = 44, Height = 48 })
    findChannels = C.iconButton("search", function()
        if busy or mode ~= "channels" or not active() then return end
        clearResults(); notice("")
        local request, searched, pending = generation, query.Text, discoveryLoading()
        local found, err = self.client:findChannels(searched)
        pending.Visible = false
        if not active() or generation ~= request or query.Text ~= searched then return end
        if not found then notice(err.message, true); return end
        if #found == 0 then results:addChild(C.text("No channels found.", { FontSize = 14, Foreground = colors.muted })) end
        for _, channel in ipairs(found) do
            results:addChild(C.button(channel.name, function()
                if busy or not active() or generation ~= request or query.Text ~= searched then return end
                busy = true
                local room, joinError = self.client:joinPublicChannel(channel.id)
                busy = false
                if not active() or generation ~= request or query.Text ~= searched then return end
                if not room then notice(joinError.message, true); return end
                open(room)
            end, { Name = "JoinChannel" .. channel.id, TextHorizontalAlignment = "Left", FontSize = 18 }))
        end
    end, { Name = "FindChannels", Width = 44, Height = 48 })
    searchRow:addChild(query); searchRow:addChild(findPeople); searchRow:addChild(findChannels)
    query.Submit = function()
        if mode == "channels" then findChannels:send("Node.LeftButtonUp") else findPeople:send("Node.LeftButtonUp") end
    end
    people:addChild(C.eyebrow("CONTACTS"))
    local body = C.stack({ Spacing = 14, Overflow = "hidden scroll" }, {
        backButton, heading, searchRow, actions, status, results, people,
        selected, nextButton, setup, invitation,
    })
    changeMode("direct")
    page = C.page("new message", body)
    local screen = self:frame(page, "New")
    local pending = ui.Node2D { Height = 80 }
    pending:addChild(ui.ActivityIndicator { Name = "ContactsLoading", Foreground = colors.muted })
    people:addChild(pending)
    local function loadContacts()
        pending.Visible = true
        self:load(function()
            if self.sessionEpoch ~= epoch then return end
            local contacts, err = self.client:listContacts()
            if self.sessionEpoch ~= epoch then return end
            if not self.client:getSession() then self:resetSessionViews(); self:show("/"); return end
            pending.Visible = false
            if err then
                notice(err.message, true)
                local retry
                retry = C.button("Retry contacts", function() retry:removeFromParent(); loadContacts() end, { Name = "RetryContacts" })
                people:addChild(retry)
            elseif #contacts == 0 then
                people:addChild(C.text("No contacts yet. Search for someone by their exact username to start a chat.", {
                    FontSize = 14, WordWrap = true, Foreground = colors.dim }))
            else
                table.sort(contacts, function(a, b) return publicName(a):lower() < publicName(b):lower() end)
                for _, person in ipairs(contacts) do addPerson(people, person, "Contact") end
            end
        end)
    end
    loadContacts()
    return screen
end

local function detailsBody(self, room, members)
    local canManage = room.role == "owner" or room.role == "admin"
    local body = C.stack({ Spacing = 14, Overflow = "hidden scroll" }, {
        C.avatar(room.name, 72, room.id, self, room.kind == "direct" and room.peer_id or nil),
        C.badge((room.kind or "group"):upper(), { Width = 100, Background = colors.soft, Foreground = colors.accentText }),
    })
    self.status = C.text((room.kind == "channel" and "Only owners and admins can publish here." or "A space for everyone in this conversation."), {
        WordWrap = true, FontSize = 14, Foreground = colors.muted })
    body:addChild(self.status)
    body:addChild(C.eyebrow("PEOPLE"))
    for _, member in ipairs(members or {}) do
        local id = member.user_id or member.id
        local info = member.profile or member
        local person = ui.Grid { Columns = "52px 1fr", Spacing = 12 }
        person:addChild(C.avatar(publicName(info), 52, id, self, id))
        person:addChild(C.stack({ Spacing = 2, JustifyContent = "Center" }, {
            C.text(publicName(info), { FontSize = 22, TextOverflow = "Ellipsis" }),
            C.text(member.role or "member", { FontSize = 14, Foreground = colors.dim }),
        }))
        local row = C.stack({ Name = "Member" .. id, Spacing = 6, Padding = "0 6" }, { person })
        if canManage and id ~= self.client:getSession().user.id and member.role ~= "owner" then
            if room.role == "owner" then
                row:addChild(C.button(member.role == "admin" and "Make member" or "Make admin", function()
                    local ordinary = room.kind == "channel" and "subscriber" or "member"
                    local ok, roleError = self.client:setMemberRole(room.id, id, member.role == "admin" and ordinary or "admin")
                    if not ok then self:notice(roleError.message, true) else self:show("/details") end
                end, { Name = "Role" .. id, Background = colors.surface }))
            end
            row:addChild(C.button("Remove", function()
                local ok, removeError = self.client:removeMember(room.id, id)
                if not ok then self:notice(removeError.message, true) else self:show("/details") end
            end, { Name = "Remove" .. id, Foreground = colors.error }))
        end
        body:addChild(row)
    end
    if canManage and room.kind ~= "direct" then
        local invite = self.inviteCodes[room.id]
        local inviteText = C.text(invite and "Invitation: " .. invite or "Invitations expire and can be revoked.", {
            Name = "Invitation", FontSize = 14, WordWrap = true, Foreground = colors.muted })
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
        end, { Name = "CreateInvite", Background = colors.surface }))
        body:addChild(C.button("Revoke invitation", function()
            if not invite then self:notice("Create an invitation first."); return end
            local ok, revokeError = self.client:revokeInvite(room.id, invite)
            if not ok then self:notice(revokeError.message, true); return end
            inviteText:setTextContent("Invitation revoked."); invite, self.inviteCodes[room.id] = nil, nil
            inviteField.Text, inviteField.Visible = "", false
        end, { Name = "RevokeInvite", Foreground = colors.error }))
    end
    return body
end

function Conversations.details(self)
    if not self.room then return self:home() end
    local room = self.room
    local screen = self:loadingPage(room.name, nil, function() return self.client:listMembers(room.id) end,
        function(members) return detailsBody(self, room, members) end)
    local content = screen:getFirstChild()
    content:removeFromParent()
    local grid = ui.Grid { Rows = "1fr 58px", Background = colors.background, Foreground = colors.ink }
    grid:addChild(content)
    grid:addChild(C.appBar({ { "BackToThread", "arrow-left", "back", function() self:show("/thread") end } }))
    screen:addChild(grid)
    return screen
end

return Conversations
