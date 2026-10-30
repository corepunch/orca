local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local colors = C.colors
local Conversations = {}

local function publicName(person)
    return person.display_name and person.display_name ~= "" and person.display_name or person.username or "Member"
end

-- One list row after the Windows Phone People hub: a 52-point tile or avatar,
-- a title and an optional muted detail line. The whole row is the target.
local function row(name, leading, title, detail, action, props)
    props = props or {}
    props.Name, props.Height, props.BorderWidth, props.FontWeight = name, 64, "0", "Normal"
    props.Background, props.class = colors.surface, "chat-row"
    local button = C.button("", action, props)
    local lines = C.stack({ JustifyContent = "Center", Spacing = 1, IgnoreHitTest = true },
        { C.text(title, { FontSize = 20, TextOverflow = "Ellipsis" }) })
    if detail then lines:addChild(C.text(detail, { FontSize = 14, class = "chat-dim", TextOverflow = "Ellipsis" })) end
    local grid = ui.Grid { Columns = "52px 1fr", Spacing = 14, Padding = "0 6", IgnoreHitTest = true }
    grid:addChild(leading); grid:addChild(lines)
    button:addChild(grid)
    return button
end

-- A command row after Telegram's New Message: an accent outline icon in the
-- avatar column and an accent label, so commands read apart from people.
local function command(name, icon, label, action)
    local button = C.button("", action, { Name = name, Height = 56, BorderWidth = "0", FontWeight = "Normal",
        Background = colors.surface, class = "chat-row" })
    local grid = ui.Grid { Columns = "52px 1fr", Spacing = 14, IgnoreHitTest = true }
    grid:addChild(C.icon(icon, 26, "chat-accent-text"))
    grid:addChild(C.text(label, { FontSize = 20, class = "chat-accent-text", TextVerticalAlignment = "Center",
        TextOverflow = "Ellipsis" }))
    button:addChild(grid)
    return button
end

-- The letter a contact is filed under; names without a letter go under "#".
local function initial(person)
    local first = publicName(person):match(utf8.charpattern) or "#"
    if #first > 1 then return first end
    return first:match("%a") and first:upper() or "#"
end

function Conversations.newRoom(self)
    if not self.client or not self.client:getSession() then return self:home() end
    if self.shell and self.shell:has("New") then self.status = self.newStatus; return self.shell:show("New") end
    local epoch = self.sessionEpoch
    local mode, visibility, generation, busy = "direct", "private", 0, false
    local page
    self.newSelection = {}
    local query = C.input("FindPerson", "Search username, email or phone")
    -- One pale field: the search command shares the input's fill, as in the inbox.
    local searchRow = ui.Grid { Columns = "1fr auto auto", Height = 48 }
    local searchBar = C.toolbar("Top", { searchRow }, { Name = "PeopleSearch", Padding = "20 0 20 12" })
    local name = C.input("RoomName", "Group name")
    local inviteCode = C.input("InviteCode", "Invitation code")
    local status = C.text("", { Name = "NewChatStatus", WordWrap = true, FontSize = 14, StateClasses = "chat-muted" })
    self.status, self.newStatus = status, status
    local function notice(message, isError)
        status:setTextContent(message or "")
        status.Visible = message ~= nil and message ~= ""
        status.StateClasses = isError and "chat-error" or "chat-muted"
    end
    local function active()
        return self.sessionEpoch == epoch and (not self.shell or
            self.screen == self.shell.screen and self.shell.active == "New")
    end
    local actions = C.stack({ Name = "NewChatActions", Spacing = 0 })
    local results = C.stack({ Name = "DiscoveryResults", Spacing = 0 })
    local people = C.stack({ Name = "Contacts", Spacing = 0 })
    local selected = C.text("", { Name = "SelectedPeople", FontSize = 15, class = "chat-muted", WordWrap = true })
    local setup = C.stack({ Name = "ConversationSetup", Spacing = 14, MarginTop = 8 })
    local invitation = C.stack({ Name = "InvitationSetup", Spacing = 14, MarginTop = 8 })
    local personButtons = {}
    local findPeople, findChannels, nextButton, createButton, visibilityButton
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
        pending:addChild(ui.ActivityIndicator { Name = "DiscoveryLoading", class = "chat-muted" })
        results:addChild(pending)
        return pending
    end
    local function updateSelection()
        local names = {}
        for _, person in pairs(self.newSelection) do names[#names + 1] = publicName(person) end
        table.sort(names)
        selected:setTextContent(#names > 0 and table.concat(names, ", ") or "Select members or continue with an empty group.")
        for id, buttons in pairs(personButtons) do
            for _, button in ipairs(buttons) do button.StateClasses = self.newSelection[id] and "chat-soft" or "" end
        end
    end
    local function open(room)
        self.newSelection = {}; updateSelection()
        if active() then self:openRoom(room) end
    end
    local function addPerson(parent, person, prefix)
        local button = row(prefix .. person.id, C.avatar(publicName(person), 52, person.id, self, person.id),
            publicName(person), person.username and "@" .. person.username, function()
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
        end, { StateClasses = self.newSelection[person.id] and "chat-soft" or "" })
        personButtons[person.id] = personButtons[person.id] or {}
        table.insert(personButtons[person.id], button)
        parent:addChild(button)
    end
    local function header(value)
        if page then page:getFirstChild().Header = value end
    end
    local function showBack(visible)
        if page then page.BackIcon = visible and "Chat/assets/icons/arrow-left.svg" or "" end
    end
    local function changeMode(value)
        if busy then return end
        mode, visibility = value, "private"
        clearResults(); notice("")
        query.Text, name.Text, inviteCode.Text = "", "", ""
        self.newSelection = {}; updateSelection()
        local browsing = mode == "direct" or mode == "group" or mode == "channels"
        actions.Visible = mode == "direct"
        searchBar.Visible, results.Visible = browsing, browsing
        people.Visible = mode == "direct" or mode == "group"
        findPeople.Visible = people.Visible
        findChannels.Visible = mode == "channels"
        header(mode == "group" and "new group" or mode == "channel" and "new channel"
            or mode == "channels" and "channels" or mode == "invite" and "join chat" or "new message")
        showBack(mode ~= "direct")
        setup.Visible, invitation.Visible = mode == "channel", mode == "invite"
        selected.Visible, nextButton.Visible = mode == "group", mode == "group"
        name.PlaceholderText = mode == "channel" and "Channel name" or "Group name"
        query.PlaceholderText = mode == "channels" and "Search public channels" or "Search username, email or phone"
        visibilityButton.Visible = mode == "channel"
        visibilityButton:setTextContent("Private channel · invitation required")
        createButton:setTextContent(mode == "channel" and "Create channel" or "Create group")
    end
    local function goBack()
        if busy then return end
        if mode == "group" and setup.Visible then
            clearResults(); notice("")
            setup.Visible, people.Visible, searchBar.Visible, results.Visible = false, true, true, true
            selected.Visible, findPeople.Visible, nextButton.Visible = true, true, true
        else changeMode("direct") end
    end
    for _, item in ipairs {
        { "KindGroup", "users-round", "New group", "group" },
        { "KindChannel", "megaphone", "New channel", "channel" },
        { "SearchChannels", "compass", "Browse channels", "channels" },
        { "ShowInvite", "ticket", "Join with invitation", "invite" },
    } do
        local target = item[4]
        actions:addChild(command(item[1], item[2], item[3], function() changeMode(target) end))
    end
    nextButton = C.button("Next", function()
        if mode ~= "group" or busy then return end
        clearResults(); notice("")
        setup.Visible, people.Visible, searchBar.Visible, results.Visible = true, false, false, false
        selected.Visible, findPeople.Visible, nextButton.Visible = false, false, false
    end, { Name = "GroupNext", class = "chat-primary" })
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
    end, { Name = "CreateConversation", class = "chat-primary" })
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
    end, { Name = "JoinInvite", class = "chat-primary" }))
    local function searchCommand(name)
        return { Name = name, Width = 48, Height = 48, Background = colors.surface, class = "chat-field" }
    end
    findPeople = C.iconButton("search", function()
        if busy or not active() or mode ~= "direct" and mode ~= "group" then return end
        clearResults(); notice("")
        if not query.Text:match("%S") then return end
        local request, searched, pending = generation, query.Text, discoveryLoading()
        local found, err = self.client:findPeople(searched)
        if self.sessionEpoch ~= epoch or generation ~= request then return end
        pending:removeFromParent()
        if not active() or query.Text ~= searched then return end
        if not found then notice(err.message, true); return end
        if #found == 0 then results:addChild(C.text("No results. Ask them to set a public username in Profile, or enable Find me by email or phone for a verified login detail. You don't need to add a contact first.", {
            Name = "NoPeopleResults", FontSize = 14, class = "chat-muted", WordWrap = true, Padding = "0 8" })) end
        for _, person in ipairs(found) do addPerson(results, person, "FoundPerson") end
    end, searchCommand("FindPeople"), "chat-field-ink")
    findChannels = C.iconButton("search", function()
        if busy or mode ~= "channels" or not active() then return end
        clearResults(); notice("")
        local request, searched, pending = generation, query.Text, discoveryLoading()
        local found, err = self.client:findChannels(searched)
        if self.sessionEpoch ~= epoch or generation ~= request then return end
        pending:removeFromParent()
        if not active() or query.Text ~= searched then return end
        if not found then notice(err.message, true); return end
        if #found == 0 then results:addChild(C.text("No channels found.", { FontSize = 14, class = "chat-muted", Padding = "0 8" })) end
        for _, channel in ipairs(found) do
            results:addChild(row("JoinChannel" .. channel.id, C.avatar(channel.name, 52, channel.id), channel.name, "Public channel", function()
                if busy or not active() or generation ~= request or query.Text ~= searched then return end
                busy = true
                local room, joinError = self.client:joinPublicChannel(channel.id)
                busy = false
                if not active() or generation ~= request or query.Text ~= searched then return end
                if not room then notice(joinError.message, true); return end
                open(room)
            end))
        end
    end, searchCommand("FindChannels"), "chat-field-ink")
    searchRow:addChild(query); searchRow:addChild(findPeople); searchRow:addChild(findChannels)
    query.Submit = function()
        if mode == "channels" then findChannels:send("Node.LeftButtonUp") else findPeople:send("Node.LeftButtonUp") end
    end
    local list = C.stack({ Spacing = 8, Padding = "0 10 0 16", Overflow = "hidden scroll" }, {
        selected, nextButton, actions, status, results, people, setup, invitation,
    })
    local body = ui.Node2D {}
    body:addChild(list); body:addChild(searchBar)
    page = C.page("new message", body, { Name = "NewMessagePage", class = "toolbar-page",
        ExtendBodyBehindHeader = true, back = goBack })
    changeMode("direct")
    local screen = self:frame(page, "New")
    local shell = self.shell
    local pending = ui.Node2D { Height = 80 }
    pending:addChild(ui.ActivityIndicator { Name = "ContactsLoading", class = "chat-muted" })
    people:addChild(pending)
    local function loadContacts()
        pending.Visible = true
        self:load(function()
            if self.sessionEpoch ~= epoch or self.shell ~= shell then return end
            local contacts, err = self.client:listContacts()
            if self.sessionEpoch ~= epoch or self.shell ~= shell then return end
            if not self.client:getSession() then self:resetSessionViews(); self:show("/"); return end
            pending.Visible = false
            if err then
                notice(err.message, true)
                local retry
                retry = C.button("Retry contacts", function() retry:removeFromParent(); loadContacts() end, { Name = "RetryContacts" })
                people:addChild(retry)
            elseif #contacts == 0 then
                people:addChild(C.text("No contacts yet. Search for someone by their exact username to start a chat.", {
                    FontSize = 14, WordWrap = true, class = "chat-dim" }))
            else
                table.sort(contacts, function(a, b) return publicName(a):lower() < publicName(b):lower() end)
                local letter
                for _, person in ipairs(contacts) do
                    if initial(person) ~= letter then
                        letter = initial(person)
                        people:addChild(C.eyebrow(letter))
                    end
                    addPerson(people, person, "Contact")
                end
            end
        end)
    end
    loadContacts()
    return screen
end

return Conversations
