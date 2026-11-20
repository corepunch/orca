local I18n = require "Chat/Scripts/i18n"
local function t(key, params) return I18n:t(key, params) end
local ui = require "orca.UIKit"
local data = require "orca.data"
local filesystem = require "orca.filesystem"
local C = require "Chat/views/components"
local colors = C.colors
local Conversations = {}
local discoveryQueries = { "People", "DeviceMatches", "Channels" }

function Conversations.deviceContacts(app)
    if app.contactsNative ~= nil then return app.contactsNative end
    local ok, native = pcall(require, "orca.contacts")
    return ok and native or false
end

function Conversations.leave(app)
    local scope = app.newScope
    if not scope then return end
    if app.unwatchNewRecipients then app.unwatchNewRecipients(); app.unwatchNewRecipients = nil end
    scope:member("Avatars"):lookup {}
    for _, name in ipairs(discoveryQueries) do
        local query = scope:member(name)
        if query.mode then query:clear() end
    end
    scope:deactivate()
end

function Conversations.dispose(app)
    if app.unwatchNewRecipients then app.unwatchNewRecipients(); app.unwatchNewRecipients = nil end
    if app.newScope then app.newScope:dispose(); app.newScope = nil end
    app.newScreen, app.newStatus, app.selectNewRecipient = nil, nil, nil
end

function Conversations.newRoom(self)
    if not self:dataScope() then return self:home() end
    if self.newScreen then
        self.status = self.newStatus
        self.unwatchNewRecipients = self.visibility:watch(self.newScreen, self.newScope)
        self.newScope:activate()
        return self.newScreen
    end
    local scope = data.scope("Chat/Data/NewMessage.xml", self.sessionData)
    local lists = assert(filesystem.loadObject("Chat/Pages/NewMessageLists"), "Missing Chat/Pages/NewMessageLists")
    self.newScope = scope
    local epoch = self.sessionEpoch
    local mode, visibility, busy, groupDetails = "direct", "private", false, false
    local page
    self.newSelection = {}
    local query = C.input("FindPerson", t("Search name, username or email"), { class = "joined-left" })
    -- One pale field: the search command shares the input's fill, as in the inbox.
    local searchRow = ui.Grid { Columns = "1fr auto auto", Height = 48 }
    local searchBar = C.toolbar("Top", { searchRow }, { Name = "PeopleSearch", Padding = "20 0 20 12" })
    local name = C.input("RoomName", t("Group name"))
    local inviteCode = C.input("InviteCode", t("Invitation code"))
    local status = C.text("", { Name = "NewChatStatus", WordWrap = true, FontSize = 14, StateClasses = "chat-muted" })
    self.status, self.newStatus = status, status
    local function notice(message, isError)
        status:setTextContent(t(message or ""))
        status.Visible = message ~= nil and message ~= ""
        status.StateClasses = isError and "chat-error" or "chat-muted"
    end
    local function active(navigation)
        return self.sessionEpoch == epoch and self.currentRoute == "/new"
            and (navigation == nil or self.navigationEpoch == navigation)
    end
    local actions = C.stack({ Name = "NewChatActions", Spacing = 0 })
    local results, people = lists:findChild("DiscoveryResults"), lists:findChild("Contacts")
    local selected = C.text("", { Name = "SelectedPeople", FontSize = 15, class = "chat-muted", WordWrap = true })
    local setup = C.stack({ Name = "ConversationSetup", Spacing = 14, MarginTop = 8 })
    local invitation = C.stack({ Name = "InvitationSetup", Spacing = 14, MarginTop = 8 })
    local findPeople, findChannels, nextButton, createButton, visibilityButton
    local function clearResults(except)
        for _, name in ipairs(discoveryQueries) do
            if name ~= except then scope:member(name):clear() end
        end
    end
    query.onTextChanged = function(_, text)
        for _, name in ipairs { "People", "Channels" } do
            local source = scope:member(name)
            if not source.state.IsIdle and source:get("Search") ~= text then source:clear() end
        end
    end
    local function updateSelection()
        local names = {}
        for _, name in pairs(self.newSelection) do names[#names + 1] = name end
        table.sort(names)
        selected:setTextContent(#names > 0 and table.concat(names, ", ") or t("Select members or continue with an empty group."))
    end
    local function refreshRecipient(id)
        for _, source in ipairs { "contact", "search", "device" } do
            local record = scope.store:get("ChatRecipient", source .. ":" .. id)
            if record then scope.store:refresh(record) end
        end
    end
    local function clearSelection()
        local previous = self.newSelection
        self.newSelection = {}
        scope.store:batch(function()
            for id in pairs(previous) do refreshRecipient(id) end
        end)
        updateSelection()
    end
    local function open(room)
        clearSelection()
        if active() then self:openRoom(room) end
    end
    self.selectNewRecipient = function(record)
        if busy or not active() then return end
        local person, navigation = record.values, self.navigationEpoch
        if person.IsChannel then
            local searched = scope:member("Channels"):get("Search")
            if mode ~= "channels" or query.Text ~= searched then return end
            busy = true
            local room, err = self.client:joinPublicChannel(person.Id)
            busy = false
            if not active(navigation) or query.Text ~= searched then return end
            if not room then notice(err.message, true); return end
            open(room)
        elseif mode == "direct" then
            busy = true
            local room, err = self.models.Rooms:create { name = person.Title, member_ids = { person.Id },
                kind = "direct", visibility = "private" }
            busy = false
            if not active(navigation) then return end
            if not room then notice(err.message, true); return end
            open(room)
        elseif mode == "group" and not groupDetails then
            self.newSelection[person.Id] = not self.newSelection[person.Id] and person.Title or nil
            scope.store:batch(function() refreshRecipient(person.Id) end)
            updateSelection()
        end
    end
    local function header(value)
        if page then page:getFirstChild().Header = value end
    end
    local function changeMode(value)
        if busy then return end
        mode, visibility, groupDetails = value, "private", false
        clearResults(); notice("")
        query.Text, name.Text, inviteCode.Text = "", "", ""
        clearSelection()
        local browsing = mode == "direct" or mode == "group" or mode == "channels"
        actions.Visible = mode == "direct"
        searchBar.Visible, results.Visible = browsing, browsing
        people.Visible = mode == "direct" or mode == "group"
        findPeople.Visible = people.Visible
        findChannels.Visible = mode == "channels"
        header(mode == "group" and t("new group") or mode == "channel" and t("new channel")
            or mode == "channels" and t("channels") or mode == "invite" and t("join chat") or t("new message"))
        setup.Visible, invitation.Visible = mode == "channel", mode == "invite"
        selected.Visible, nextButton.Visible = mode == "group", mode == "group"
        name.PlaceholderText = mode == "channel" and t("Channel name") or t("Group name")
        query.PlaceholderText = mode == "channels" and t("Search public channels") or t("Search name, username or email")
        visibilityButton.Visible = mode == "channel"
        visibilityButton:setTextContent(t("Private channel · invitation required"))
        createButton:setTextContent(mode == "channel" and t("Create channel") or t("Create group"))
    end
    local function showGroupDetails(show)
        clearResults(); notice("")
        groupDetails = show
        setup.Visible, people.Visible, searchBar.Visible, results.Visible = show, not show, not show, not show
        selected.Visible, findPeople.Visible, nextButton.Visible = not show, not show, not show
    end
    local function goBack()
        if busy then return end
        if mode == "group" and groupDetails then
            showGroupDetails(false)
        elseif mode == "direct" then self:goBack("/")
        else changeMode("direct") end
    end
    -- Friends from the device's contacts: their email addresses are matched
    -- once against people who opted in to "find me by email" (Data: none kept).
    local contacts = Conversations.deviceContacts(self)
    if contacts and contacts.supported() then
        actions:addChild(C.command("FromContacts", "user-round-plus", t("Find friends from contacts"), function()
            if busy or not active() or mode ~= "direct" then return end
            local matches = scope:member("DeviceMatches")
            if matches.mode then return end
            clearResults(); notice("")
            matches:load()
        end))
    end
    for _, item in ipairs {
        { "KindGroup", "users-round", t("New group"), "group" },
        { "KindChannel", "megaphone", t("New channel"), "channel" },
        { "SearchChannels", "compass", t("Browse channels"), "channels" },
        { "ShowInvite", "ticket", t("Join with invitation"), "invite" },
    } do
        local target = item[4]
        actions:addChild(C.command(item[1], item[2], item[3], function() changeMode(target) end))
    end
    nextButton = C.button(t("Next"), function()
        if mode == "group" and not busy then showGroupDetails(true) end
    end, { Name = "GroupNext", class = "chat-primary" })
    visibilityButton = C.button(t("Private channel · invitation required"), function()
        if busy then return end
        visibility = visibility == "private" and "public" or "private"
        visibilityButton:setTextContent(visibility == "private" and t("Private channel · invitation required") or t("Public channel · anyone can join"))
    end, { Name = "ConversationVisibility", Background = colors.surface })
    createButton = C.button(t("Create group"), function()
        if busy or not active() or (mode ~= "channel" and not groupDetails) then return end
        if not name.Text:match("%S") then notice(mode == "channel" and t("Enter a channel name.") or t("Enter a group name."), true); name:setFocus(); return end
        local ids = {}
        for id in pairs(self.newSelection) do ids[#ids + 1] = id end
        table.sort(ids)
        local navigation = self.navigationEpoch
        busy = true
        local room, err = self.models.Rooms:create { name = name.Text, member_ids = ids, kind = mode,
            visibility = mode == "channel" and visibility or "private" }
        busy = false
        if not active(navigation) then return end
        if not room then notice(err.message, true); return end
        changeMode("direct"); open(room)
    end, { Name = "CreateConversation", class = "chat-primary" })
    setup:addChild(name); setup:addChild(visibilityButton); setup:addChild(createButton)
    invitation:addChild(inviteCode)
    invitation:addChild(C.button(t("Join"), function()
        if busy or mode ~= "invite" or not active() then return end
        local navigation = self.navigationEpoch
        busy = true
        local room, err = self.client:joinInvite(inviteCode.Text)
        busy = false
        if not active(navigation) then return end
        if not room then notice(err.message, true); return end
        changeMode("direct"); open(room)
    end, { Name = "JoinInvite", class = "chat-primary" }))
    local function searchCommand(name)
        return { Name = name, Width = 48, Height = 48, Background = colors.surface, class = "chat-field joined-right" }
    end
    local function search(member)
        clearResults(member); notice("")
        local source = scope:member(member)
        local searchText = query.Text
        if member == "People" and not searchText:match("%S") then source:clear(); return end
        if source:get("Search") ~= searchText then
            source:clear()
            source:set("Search", searchText)
        elseif not source.mode then
            source:refresh()
        end
    end
    local function searchPeople()
        if busy or not active() or groupDetails or (mode ~= "direct" and mode ~= "group") then return end
        search("People")
    end
    local function searchChannels()
        if busy or mode ~= "channels" or not active() then return end
        search("Channels")
    end
    findPeople = C.iconButton("search", searchPeople, searchCommand("FindPeople"), "chat-field-ink")
    findChannels = C.iconButton("search", searchChannels, searchCommand("FindChannels"), "chat-field-ink")
    searchRow:addChild(query); searchRow:addChild(findPeople); searchRow:addChild(findChannels)
    query.Submit = function()
        if mode == "channels" then searchChannels() else searchPeople() end
    end
    local list = C.stack({ Spacing = 8, Padding = "0 10 0 16", Overflow = "hidden scroll" }, {
        selected, nextButton, actions, status, lists, setup, invitation,
    })
    local body = ui.Node2D {}
    body:addChild(list); body:addChild(searchBar)
    page = C.page(t("new message"), body, { Name = "NewMessagePage", class = "toolbar-page",
        ExtendBodyBehindHeader = true, back = goBack })
    changeMode("direct")
    local screen = self:frame(page)
    self.newScreen = screen
    scope:attach(lists)
    self.unwatchNewRecipients = self.visibility:watch(screen, scope)
    scope:activate()
    return screen
end

return Conversations
