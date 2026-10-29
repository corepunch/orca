local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local ContactItems = require "Chat/views/contact_items"
local colors = C.colors
local Contacts = {}

local function publicName(person)
    return person.display_name and person.display_name ~= "" and person.display_name or person.username or "Member"
end

function Contacts.show(self)
    if not self.client or not self.client:getSession() then return self:home() end
    if self.shell and self.shell:has("Contacts") then
        self.status = self.contactsStatus
        local screen = self.shell:show("Contacts")
        self.contactsState.refresh()
        return screen
    end
    local epoch = self.sessionEpoch
    local state = { contacts = {}, rooms = {}, request = 0, query = "" }
    self.contactsState = state
    local panorama = ui.Panorama { Name = "ContactsPanorama", Title = "contacts", Foreground = colors.ink,
        ContentInsetAdjustment = "Always" }
    self.contactsPanorama = panorama
    local recent = ui.PanoramaItem { Name = "ContactsRecent", Header = "recent", Orientation = "Horizontal" }
    local all = ui.PanoramaItem { Name = "ContactsAll", Header = "all" }
    panorama:addChild(recent); panorama:addChild(all)
    local mosaic = ui.Grid { Name = "ContactsMosaic", Width = 720, AspectRatio = 2,
        Columns = "1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr", Rows = "1fr 1fr 1fr 1fr", Spacing = 5 }
    local recentEmpty = C.text("", { Name = "ContactsRecentEmpty", WordWrap = true,
        Width = 320, Visible = false, Foreground = colors.muted, FontSize = 18 })
    local status = C.text("", { Name = "ContactsStatus", Visible = false, WordWrap = true,
        Foreground = colors.muted, FontSize = 14 })
    self.contactsStatus, self.status = status, status
    local loading = ui.ActivityIndicator { Name = "ContactsLoading", Foreground = colors.muted }
    local loadingArea = ui.Node2D { Name = "ContactsLoadingArea", Height = 80 }
    loadingArea:addChild(loading)
    local opening = ui.ActivityIndicator { Name = "ContactsOpening", Foreground = colors.muted }
    local openingArea = ui.Node2D { Height = 40, Visible = false }
    openingArea:addChild(opening)
    -- The panorama alone pans horizontally. Perpendicular scrolling keeps
    -- the portrait wall accessible in compact viewports.
    local recentBody = C.stack({ Name = "ContactsRecentBody", Width = 720, Spacing = 18,
        Overflow = "hidden scroll" }, { mosaic, recentEmpty })
    recent:addChild(recentBody)
    local list = C.stack({ Name = "ContactsList", Spacing = 12 })
    local empty = C.text("Loading contacts…", { Name = "ContactsEmpty", WordWrap = true,
        FontSize = 16, Foreground = colors.muted })
    local search = C.input("ContactsSearch", "search contacts")
    local findNew = C.button("find someone new", function() self:show("/new") end,
        { Name = "ContactsFindNew", TextHorizontalAlignment = "Left", FontSize = 18 })
    local allBody = C.stack({ Name = "ContactsAllBody", Spacing = 16, Overflow = "hidden scroll" }, {
        search, list, empty, findNew,
    })
    all:addChild(allBody)
    -- Feedback belongs to the page, so a failure on the wall's right half
    -- stays visible without panning back to its first column.
    local feedback = C.stack({ Name = "ContactsFeedback", Spacing = 8, Padding = "20 0" }, {
        status, loadingArea, openingArea,
    })
    local page = ui.Grid { Name = "ContactsPage", Rows = "1fr auto", ContentInsetAdjustment = "Always" }
    page:addChild(panorama); page:addChild(feedback)
    local screen = self:frame(page, "Contacts")
    local shell = self.shell
    local function current()
        return self.sessionEpoch == epoch and self.contactsState == state and self.shell == shell
    end
    local function active()
        return current() and self.screen == shell.screen and shell.active == "Contacts"
    end
    local function updateFeedback()
        feedback.Visible = status.Visible or loadingArea.Visible or openingArea.Visible
    end
    local function notice(text, isError)
        status:setTextContent(text or "")
        status.Visible = text ~= nil and text ~= ""
        status.Foreground = isError and colors.error or colors.muted
        updateFeedback()
    end
    local function openPerson(person)
        if state.opening or not active() then return end
        local navigation = self.navigationEpoch
        state.opening, openingArea.Visible = true, true
        notice("")
        local room, err = self.models.Rooms:create { name = publicName(person), member_ids = { person.id },
            kind = "direct", visibility = "private" }
        if not current() then return end
        if not self.client:getSession() then self:resetSessionViews(); self:show("/"); return end
        state.opening, openingArea.Visible = false, false
        updateFeedback()
        if not active() or self.navigationEpoch ~= navigation then return end
        if not room then notice(err and err.message or "Could not open this conversation. Try again.", true); return end
        self:openRoom(room)
    end
    local portraits = ContactItems.new(self, mosaic, "ContactPortrait", openPerson)
    local rows = ContactItems.new(self, list, "ContactRow", openPerson)
    state.portraits, state.rows = portraits, rows
    local function renderContacts()
        local values, query = {}, state.query:lower()
        for _, person in ipairs(state.contacts) do
            local searchable = publicName(person) .. " " .. (person.username or "")
            if query == "" or searchable:lower():find(query, 1, true) then values[#values + 1] = person end
        end
        rows:refresh(values)
        empty.Visible = #values == 0
        empty:setTextContent(state.contactsLoaded and (query ~= "" and "No matching contacts." or "Your contacts will appear here.") or "Loading contacts…")
    end
    local function renderRecent()
        local ordered, byId, seen, rooms = {}, {}, {}, {}
        for _, person in ipairs(state.contacts) do byId[person.id] = person end
        for _, room in ipairs(state.rooms) do rooms[#rooms + 1] = room end
        table.sort(rooms, function(a, b) return (a.updated_at or a.created_at or "") > (b.updated_at or b.created_at or "") end)
        for _, room in ipairs(rooms) do
            local person = room.kind == "direct" and byId[room.peer_id]
            if person and not seen[person.id] then ordered[#ordered + 1], seen[person.id] = person, true end
        end
        -- Recent conversations lead; other saved contacts complete the wall.
        -- No fabricated status or global directory is used in the app.
        for _, person in ipairs(state.contacts) do
            if not seen[person.id] then ordered[#ordered + 1], seen[person.id] = person, true end
        end
        while #ordered > 32 do table.remove(ordered) end
        local wide = #ordered > 16
        local width = wide and 720 or 320
        mosaic.Width, recentBody.Width = width, width
        mosaic.Columns = wide and "1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr" or "1fr 1fr 1fr 1fr"
        mosaic.AspectRatio = wide and 2 or 1
        portraits:refresh(ordered)
        mosaic.Visible, recentEmpty.Visible = #ordered > 0, #ordered == 0 and state.contactsLoaded == true
        recentEmpty:setTextContent("Bring your people together. Find someone to start a conversation.")
    end
    local function updateSearch()
        local query = search.Text or ""
        if query ~= state.query then state.query = query; renderContacts() end
    end
    search.KeyUp = updateSearch
    search.onTextChanged = updateSearch
    findNew.Click = function() if active() then self:show("/new") end end
    local retry
    state.refresh = function()
        if not current() or state.loading then return end
        state.request = state.request + 1
        local request = state.request
        state.loading, loadingArea.Visible = true, true
        notice("")
        if retry then retry.Visible = false end
        self:load(function()
            local function alive() return current() and state.request == request end
            if not alive() then return end
            local people, contactError = self.client:listContacts()
            if not alive() then return end
            if not self.client:getSession() then self:resetSessionViews(); self:show("/"); return end
            if people then
                local user = self.client:getSession().user.id
                state.contacts = {}
                for _, person in ipairs(people) do if person.id ~= user then state.contacts[#state.contacts + 1] = person end end
                table.sort(state.contacts, function(a, b) return publicName(a):lower() < publicName(b):lower() end)
                state.contactsLoaded = true
            end
            renderContacts(); renderRecent()
            local rooms, roomError = self.models.Rooms:select()
            if not alive() then return end
            if not self.client:getSession() then self:resetSessionViews(); self:show("/"); return end
            if rooms then state.rooms = rooms end
            state.loading, loadingArea.Visible = false, false
            renderRecent()
            local err = contactError or roomError
            if err then notice(err.message or "Could not refresh contacts. Try again.", true) end
            if not people and not state.contactsLoaded then
                empty:setTextContent("Your contacts could not be loaded.")
            end
            retry.Visible = err ~= nil
            updateFeedback()
        end)
    end
    retry = C.button("retry", state.refresh, { Name = "ContactsRetry", Visible = false, TextHorizontalAlignment = "Left" })
    feedback:addChild(retry)
    renderContacts(); renderRecent()
    state.refresh()
    return screen
end

return Contacts
