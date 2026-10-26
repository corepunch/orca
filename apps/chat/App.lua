local ui = require "orca.UIKit"
local Application = require "orca.core.application"
local Chat = require "api.chat"
local Models = require "Chat/models"
local Sync = require "Chat/sync"
local system = require "orca.system"
local renderer = require "orca.renderer"
local C = require "Chat/views/components"
local Shell = require "Chat/views/shell"
local Identity = require "Chat/views/identity"
local Conversations = require "Chat/views/conversations"
local Timeline = require "Chat/views/timeline"
local Diagnostics = require "Chat/diagnostics"
local async = require "orca.core.async"
local orca = require "orca"
local colors = C.colors
local auto = 0/0

local App = Application:extend {
    layout = function(self, ctx) return ctx.slots.inner end,

    __init = function(self, options)
        Application.__init(self)
        self.diagnostics = Diagnostics.new(self)
        options = options or {}
        local config = require "Chat/config"
        local backend = config.backend or "appwrite"
        if options.client then self.client = options.client
        elseif backend == "appwrite" then
            local settings = {}
            for key, value in pairs(config) do settings[key] = value end
            settings.sessionStore, self.configurationError = require("Chat/session_store").new(config)
            if not self.configurationError then
                self.client, self.configurationError = Chat.appwrite(settings)
                self.restorePending = self.client ~= nil and settings.sessionStore ~= nil
            end
        elseif backend == "mock" then self.client = Chat.mock()
        else self.configurationError = { message = "Choose CHAT_BACKEND=appwrite or mock." } end
        self.media = options.media or require("Chat/media").new()
        self.remote = self.client ~= nil and self.client.mode ~= "mock"
        if self.client then self.models = Models.new(self.client) end
        self:resetSessionViews()
        if self.remote then self.polling = Sync.new(self) end
    end,

    resetSessionViews = function(self)
        if self.polling then self.polling:stop() end
        self.sessionEpoch = (self.sessionEpoch or 0) + 1
        if self.media and self.media.clear then self.media:clear() end
        -- Clearing media removes the cached photo files, so forget their paths.
        self.avatarPaths, self.avatarWaiting = {}, {}
        self.shell, self.room, self.contacts, self.contactsById = nil, nil, nil, nil
        self.roomLists, self.searchInput, self.pivot, self.rooms = nil, nil, nil, nil
        self.messageList, self.composer, self.threadBody = nil, nil, nil
        self.timelineSnapshot, self.messageRefreshQueued = nil, nil
        self.status, self.homeStatus, self.newStatus = nil, nil, nil
        self.authDraft, self.loginIdentifier, self.loginReturnRoute = nil, nil, nil
        self.drafts, self.photoDrafts, self.readRooms, self.pinned = {}, {}, {}, { alex = true, weekend = true }
        self.newSelection, self.inviteCodes, self.profileStatus, self.pendingIdentity = {}, {}, nil, nil
        self.filter, self.query = "All", ""
    end,

    -- Show a person's profile photo in an avatar tile. Tiles built in one pass
    -- wait for a single task, so a screen costs one profile photo lookup.
    avatarPhoto = function(self, tile, userId)
        local path = self.avatarPaths[userId]
        if path then return C.photo(tile, path) end
        if path == false or not self.client or not self.media then return end
        self.avatarWaiting[userId] = self.avatarWaiting[userId] or {}
        table.insert(self.avatarWaiting[userId], tile)
        if self.avatarTask then return end
        self.avatarTask = true
        local function load()
            while next(self.avatarWaiting) do
                local waiting, ids = self.avatarWaiting, {}
                self.avatarWaiting = {}
                for id in pairs(waiting) do ids[#ids + 1] = id end
                table.sort(ids)
                -- Failures leave the initials and allow a retry on the next screen.
                local avatars = self.client:getAvatars(ids) or {}
                for _, id in ipairs(ids) do
                    local avatarId = avatars[id]
                    path = avatarId and self.media.keys and self.media.keys["avatar-" .. avatarId]
                    local unusable = avatarId == false
                    if avatarId and not path then
                        local bytes = self.client:getAvatar(avatarId)
                        path = bytes and self.media:storePreview(bytes, "avatar-" .. avatarId)
                        -- Previews are JPEG only; keep initials for other formats.
                        unusable = bytes ~= nil and not path
                    end
                    if unusable then self.avatarPaths[id] = false end
                    if path then
                        self.avatarPaths[id] = path
                        -- The run loop repaints after the resumed task, as for timeline photos.
                        for _, waitingTile in ipairs(waiting[id]) do C.photo(waitingTile, path) end
                    end
                end
            end
        end
        orca.async(function()
            local ok, err = pcall(load)
            self.avatarTask = nil
            if not ok then io.stderr:write("chat: profile photos: " .. tostring(err) .. "\n") end
        end)
    end,

    activate_controller = function(self, controller)
        self:installErrorHandler()
        local result = Application.activate_controller(self, controller)
        if self.polling then self.polling:activate() end
        return result
    end,

    installErrorHandler = function(self)
        async.set_error_handler(function(err) self:reportError(err, "Background action") end)
    end,

    reportError = function(self, err, stage)
        self.diagnostics:show(err, stage)
    end,

    dispatch = function(self, request)
        self:installErrorHandler()
        self.currentRoute = type(request) == "string" and request
            or type(request) == "table" and request.path or "/"
        local ok, controller = xpcall(Application.dispatch, Diagnostics.traceback, self, request)
        if ok then return controller end
        return self.diagnostics:controller(controller, "Opening screen")
    end,

    frame = function(self, content, tab)
        if tab then
            if not self.shell then
                self.shell = Shell.new(self:frame(), function(route) self:show(route) end, {
                    { "AppBarRefresh", "refresh", function() self:show(self.shell.routes[self.shell.active]) end },
                    { "AppBarSignOut", "sign out", function() self:signOut() end },
                })
            end
            return self.shell:show(tab, content)
        end
        local screen = ui.Screen {
            Width = 390, Height = 844, ResizeMode = PLATFORM == "ios" and "CanResize" or "NoResize",
            ClearColor = colors.background,
            StyleSheet = "Chat/Styles/app.css",
        }
        if content then screen:addChild(content) end
        return screen
    end,

    show = function(self, route)
        if self.polling then self.polling:stop() end
        self.navigationEpoch = (self.navigationEpoch or 0) + 1
        local epoch = self.navigationEpoch
        local controller = self:dispatch(route or "/")
        if self.navigationEpoch == epoch then return self:activate_controller(controller) end
    end,

    notice = function(self, text, error)
        if self.status then
            self.status:setTextContent(text or "")
            self.status.Foreground = error and colors.error or colors.dim
        elseif error then
            self:reportError(text, "Request")
        end
    end,

    openRoom = function(self, room)
        self.room, self.readRooms[room.id] = room, true
        self:show("/thread")
    end,

    unread = function(self, room)
        return not self.readRooms[room.id] and (room.unread or 0) or 0
    end,

    roomTime = function(self, room)
        return C.time(room.updated_at or room.created_at)
    end,

    updateSearch = function(self)
        local query = self.searchInput.Text or ""
        if query == self.query then return end
        self.query = query
        self:renderRoomList()
    end,

    renderRoomList = function(self)
        for _, list in pairs(self.roomLists) do
            while list:getFirstChild() do list:getFirstChild():removeFromParent() end
        end
        local index = ({ All = 0, Unread = 1, Pinned = 2 })[self.filter]
        if self.pivot.SelectedIndex ~= index then self.pivot.SelectedIndex = index end
        -- Prepare neighboring pages before a swipe; selection only changes the viewport.
        local query = self.query:lower()
        for filter, list in pairs(self.roomLists) do
            local visible = 0
            for index, room in ipairs(self.rooms) do
                local unread = self:unread(room)
                local matches = (room.name .. " " .. (room.subtitle or "")):lower():find(query, 1, true)
                local inFilter = filter == "All" or (filter == "Unread" and unread > 0)
                    or (filter == "Pinned" and self.pinned[room.id])
                if matches and inFilter then
                    visible = visible + 1
                    local row = ui.Button {
                        Name = "Room" .. index, Height = 76, Background = colors.background,
                        class = "conversation", Click = function() self:openRoom(room) end,
                    }
                    local content = ui.Grid { Columns = "56px 1fr 44px", Spacing = 14, Padding = "0 10", IgnoreHitTest = true }
                    content:addChild(C.avatar(room.name, 56, room.id, self, room.kind == "direct" and room.peer_id or nil))
                    content:addChild(C.stack({ JustifyContent = "Center", Spacing = 2, IgnoreHitTest = true }, {
                        C.text(room.name, { FontSize = 22, TextOverflow = "Ellipsis" }),
                        C.text(room.subtitle or "Start the conversation", {
                            FontSize = 14, Foreground = unread > 0 and colors.accentText or colors.dim, TextOverflow = "Ellipsis",
                        }),
                    }))
                    local meta = C.stack({ AlignItems = "End", Spacing = 8, Padding = "0 6", IgnoreHitTest = true }, {
                        C.text(self:roomTime(room), { FontSize = 12, Foreground = unread > 0 and colors.accentText or colors.dim }),
                    })
                    if unread > 0 then meta:addChild(C.badge(unread, { Width = 20, Height = 20 }))
                    elseif self.pinned[room.id] then meta:addChild(C.icon("pin", 14, colors.dim)) end
                    content:addChild(meta); row:addChild(content); list:addChild(row)
                end
            end
            if visible == 0 then
                list:addChild(C.stack({ Spacing = 8, Padding = "0 8" }, {
                    C.text(self.query ~= "" and "no conversations found" or "you're all caught up", { FontSize = 22, Foreground = colors.muted }),
                    C.text(self.query ~= "" and "Try another name or message." or "Start a new chat, or swipe to another list.", {
                        FontSize = 14, Foreground = colors.dim, WordWrap = true,
                    }),
                }))
            end
        end
    end,

    home = function(self)
        local navigation = self.navigationEpoch
        if not self.client then
            return self:frame(C.page("connect", C.stack({ Spacing = 16 }, {
                C.text(self.configurationError.message, { WordWrap = true, Foreground = colors.error }),
                C.text("Check your backend settings, then restart. Appwrite uses APPWRITE_ENDPOINT, APPWRITE_PROJECT_ID and APPWRITE_CHAT_FUNCTION_ID.", { WordWrap = true }),
            })))
        end
        if self.logoutPending then
            return self:frame(C.page("sign out", C.stack({ Spacing = 16 }, {
                C.text("Couldn't remove your saved login. Please retry signing out.", { WordWrap = true, Foreground = colors.muted }),
                C.button("Retry sign out", function() self:signOut() end, { Name = "RetrySignOut" }),
            })))
        end
        if self.restorePending then
            local restored, restoreError = self.client:restoreSession()
            if self.navigationEpoch ~= navigation then return self.screen end
            if restored or restoreError.code == "unauthenticated"
                or restoreError.status == 401 or restoreError.status == 403 then
                self.restorePending = false
            else
                return self:frame(C.page("reconnect", C.stack({ Spacing = 16 }, {
                    C.text("Couldn't restore your session. Please try again.", { WordWrap = true, Foreground = colors.muted }),
                    C.button("Retry", function() self:show("/") end, { Name = "RetrySessionRestore" }),
                })))
            end
        end
        if not self.client:getSession() then self:resetSessionViews(); return self:auth() end
        local epoch = self.sessionEpoch
        local function current()
            return self.navigationEpoch == navigation and self.sessionEpoch == epoch and self.client:getSession() ~= nil
        end
        local _, err = Sync.refreshInbox(self, current, true)
        if self.navigationEpoch ~= navigation then return self.screen end
        if not self.client:getSession() then self:resetSessionViews(); return self:auth() end
        self.rooms, self.contacts, self.contactsById = self.rooms or {}, self.contacts or {}, self.contactsById or {}
        if self.shell and self.shell:has("Chats") then
            self.status = self.homeStatus
            self:renderRoomList()
            self:notice(err and err.message or (self.remote and "Conversations update automatically" or "Offline mock · make yourself at home"), err ~= nil)
            return self.shell:show("Chats")
        end
        self.status = C.text(err and err.message or (self.remote and "Conversations update automatically" or "Offline mock · make yourself at home"), {
            FontSize = 11, Foreground = err and colors.error or colors.dim, TextHorizontalAlignment = "Center",
        })
        self.homeStatus = self.status
        self.searchInput = C.input("Search", "search", {
            Text = self.query, Submit = function() self:updateSearch() end, KeyUp = function() self:updateSearch() end,
        })
        local search = ui.Grid { Columns = "1fr 44px", Height = 48, MarginBottom = 12 }
        search:addChild(self.searchInput)
        search:addChild(C.iconButton("x", function()
            self.query, self.searchInput.Text = "", ""; self:renderRoomList(); self.searchInput:setFocus()
        end, { Name = "ClearSearch", Height = 48, Foreground = colors.textboxInk, Background = colors.textbox }))
        -- The filters are pivot items: swipe or tap a header to switch lists.
        local filters = { "All", "Unread", "Pinned" }
        self.pivot = ui.Pivot { Name = "Inbox", Title = "Messenger", Foreground = colors.ink,
            SelectionChanged = function(pivot)
                local filter = filters[pivot.SelectedIndex + 1]
                self.filter = filter
            end,
        }
        self.roomLists = {}
        for _, filter in ipairs(filters) do
            local item = ui.PivotItem { Name = "Filter" .. filter, Header = filter }
            self.roomLists[filter] = C.stack({ Name = "ConversationList" .. filter, Overflow = "hidden scroll" })
            if filter == "All" then
                local body = ui.Grid { Rows = "60px 1fr" }
                body:addChild(search); body:addChild(self.roomLists[filter])
                item:addChild(body)
            else
                item:addChild(self.roomLists[filter])
            end
            self.pivot:addChild(item)
        end
        local grid = ui.Grid { Rows = "1fr 24px", Background = colors.background }
        grid:addChild(self.pivot); grid:addChild(self.status)
        local screen = self:frame(grid, "Chats")
        self:renderRoomList()
        return screen
    end,

    signOut = function(self)
        if self.polling then self.polling:stop() end
        self.sessionEpoch = (self.sessionEpoch or 0) + 1
        local ok, err = self.client:signOut()
        self.restorePending = false
        self.logoutPending = not ok and err.code == "session_storage"
        self:resetSessionViews(); self:show()
        if not ok and not self.logoutPending then self:notice(err.message, true) end
    end,

    renderMessages = Timeline.refresh,

    sendMessage = function(self)
        if self.room.can_post == false then self:notice("Only channel admins can publish.", true); return end
        if self.sending then return end
        local room, composer, epoch = self.room, self.composer, self.sessionEpoch
        local text = composer.Text or ""
        local draft = self.photoDrafts[room.id]
        if not text:match("%S") and not draft then return end
        self.sending = true
        local attachments
        if draft then
            self:notice("Uploading photo…")
            if not draft.attachment then
                local attachment, uploadError = self.client:uploadPhoto(room.id, draft)
                if self.sessionEpoch ~= epoch then self.sending = false; return end
                if not attachment then
                    self.sending = false
                    if self.room == room and self.composer == composer then
                        self:notice(uploadError.message .. " · Your draft is safe. Try sending again.", true)
                    end
                    return
                end
                draft.attachment = attachment
            end
            attachments = { draft.attachment.id }
        end
        local message, err = self.models.Messages:create { room_id = room.id, body = text, attachment_ids = attachments }
        self.sending = false
        if self.sessionEpoch ~= epoch then return end
        if not message then
            if self.room == room and self.composer == composer then self:notice(err.message, true) end
            return
        end
        if self.photoDrafts[room.id] == draft then self.photoDrafts[room.id] = nil end
        if self.room == room and self.composer ~= composer then return end
        local remaining = composer.Text ~= text and composer.Text or ""
        self.drafts[room.id] = remaining ~= "" and remaining or nil
        if self.room ~= room or self.composer ~= composer then return end
        composer.Text = remaining
        if self.photoPreview then self.photoPreview.Visible = false end
        self:renderMessages()
        if self.room == room and self.composer == composer and self.sessionEpoch == epoch then composer:setFocus() end
    end,

    getMedia = function(self)
        return self.media
    end,

    attachPhoto = function(self)
        if self.sending then self:notice("Finish sending before adding another photo."); return end
        local media = self:getMedia()
        if not media then self:notice("Photo selection is unavailable on this platform.", true); return end
        self:notice("Preparing photo…")
        local room, composer = self.room, self.composer
        local photo, err = media:pick()
        if self.room ~= room or self.composer ~= composer then return end
        if not photo then
            self:notice(type(err) == "table" and err.message or err or "Photo selection cancelled.",
                err ~= nil and not (type(err) == "table" and err.code == "cancelled"))
            return
        end
        local previous = self.photoDrafts[self.room.id]
        if previous and previous.attachment then self.client:discardPhoto(previous.attachment.id) end
        self.photoDrafts[self.room.id] = photo
        self.photoPreview.Visible = true
        self.photoLabel:setTextContent("Photo ready · " .. math.ceil(#photo.jpeg / 1000) .. " KB")
        self:notice("Photo ready. Add a caption or send it as it is.")
    end,

    thread = function(self)
        if not self.room then return self:home() end
        local room = self.room
        local function keepDraft() self.drafts[room.id] = self.composer and self.composer.Text or "" end
        self.messageList = C.stack({ Name = "Messages", Spacing = 0, PaddingBottom = 8, Overflow = "hidden scroll" })
        self.status = C.text("", { FontSize = 11, Foreground = colors.dim, TextHorizontalAlignment = "Center",
            Margin = "24 8" })
        self.composer = C.input("Message", "type a message", {
            Text = self.drafts[room.id] or "", Submit = function() self:sendMessage() end, BorderRadius = "0",
        })
        local compose = ui.Grid { Name = "Composer", Columns = "48px 1fr 48px", Height = 48,
            Background = colors.textbox }
        compose:addChild(C.iconButton("plus", function() self:attachPhoto() end,
            { Name = "AttachPhoto", Width = 48, Height = 48, Foreground = colors.textboxInk }))
        compose:addChild(self.composer)
        compose:addChild(C.iconButton("arrow-up", function() self:sendMessage() end,
            { Name = "SendMessage", Width = 48, Height = 48, Foreground = colors.accentText }))
        self.photoLabel = C.text("Photo ready", { FontSize = 13, Foreground = colors.accentText, TextVerticalAlignment = "Center" })
        self.photoPreview = ui.Grid { Name = "PhotoDraft", Columns = "1fr 40px", Height = 40,
            Visible = self.photoDrafts[room.id] ~= nil, Background = colors.soft, Padding = "12 0" }
        self.photoPreview:addChild(self.photoLabel)
        self.photoPreview:addChild(C.iconButton("x", function()
            if self.sending then self:notice("The photo is being sent. Please wait."); return end
            local photo = self.photoDrafts[room.id]
            if photo and photo.attachment then
                local ok, err = self.client:discardPhoto(photo.attachment.id)
                if not ok then self:notice(err.message, true); return end
            end
            self.photoDrafts[room.id], self.photoPreview.Visible = nil, false
            self:notice("Photo removed. Your message is still here.")
        end, { Name = "RemovePhoto", Width = 36, Height = 36, Foreground = colors.ink }))
        local readOnly = room.can_post == false
        compose.Visible, self.photoPreview.Visible = not readOnly, not readOnly and self.photoDrafts[room.id] ~= nil
        local footer = C.stack({ Name = "ThreadFooter", Spacing = 0,
            Background = readOnly and colors.background or colors.textbox }, { self.status, self.photoPreview, compose,
            C.text("You’re subscribed · Only admins can publish", { Name = "ChannelReadOnly", Visible = readOnly,
                FontSize = 13, Foreground = colors.muted, WordWrap = true, Padding = "24 12" }),
        })
        local pinned = self.pinned[room.id]
        local screen, menuButton
        local overflow = ui.Node2D { Name = "ThreadOverflow", Visible = false }
        local function closeMenu() overflow.Visible = false end
        overflow:addChild(C.button("", closeMenu, { Name = "DismissConversationMenu", Height = auto,
            BorderWidth = "0", Background = "#00000000" }))
        local menu = C.stack({ Name = "ThreadOverflowMenu", Width = 220, Height = 144,
            MarginRight = auto, MarginBottom = auto, Background = colors.chrome })
        overflow:addChild(menu)
        local commands = ui.Grid { Name = "ThreadCommands", Columns = "44px 1fr 44px", Height = 44 }
        commands:addChild(C.iconButton("arrow-left", function() keepDraft(); self:show() end,
            { Name = "BackToChats", Foreground = colors.ink }))
        commands:addChild(ui.Node2D { IgnoreHitTest = true })
        menuButton = C.button("…", function()
            if overflow.Visible then closeMenu(); return end
            screen:UpdateLayout(screen.Width, screen.Height)
            local x, y, node = 0, 0, menuButton
            while node and node ~= screen do
                x, y, node = x + node.ActualX, y + node.ActualY, node:getParent()
            end
            menu.MarginLeft = math.max(screen.SafeAreaLeft, x + menuButton.ActualWidth - menu.Width)
            menu.MarginTop = math.max(screen.SafeAreaTop,
                math.min(y + menuButton.ActualHeight, screen.Height - screen.SafeAreaBottom - menu.Height))
            overflow.Visible = true
        end, { Name = "ConversationMenu", Width = 44, Height = 44, FontSize = 28, Foreground = colors.ink })
        commands:addChild(menuButton)
        local actions = {
            { "PinConversation", pinned and "unpin" or "pin", function()
                keepDraft(); self.pinned[room.id] = not self.pinned[room.id]; self:show("/thread")
            end },
            { "ConversationInfo", "conversation info", function() keepDraft(); self:show("/details") end },
            { "RefreshMessages", "refresh", function() self:renderMessages() end },
        }
        for _, action in ipairs(actions) do
            menu:addChild(C.button(action[2], function() closeMenu(); action[3]() end,
                { Name = action[1], Height = 48, FontSize = 18, TextHorizontalAlignment = "Left",
                    Padding = "16 0", Foreground = colors.ink, Background = colors.chrome, BorderWidth = "0" }))
        end
        self.threadBody = ui.Grid { Rows = "44px 1fr" }
        self.threadBody:addChild(commands); self.threadBody:addChild(self.messageList)
        -- Keep the rectangular composer outside the page's horizontal content margins.
        local grid = ui.Grid { Rows = "1fr auto", Background = colors.background, Foreground = colors.ink }
        grid:addChild(C.page(room.name, self.threadBody)); grid:addChild(footer)
        screen = self:frame(grid)
        screen:addChild(overflow)
        screen.RespectSafeArea = false
        for _, edge in ipairs { "Left", "Top", "Right" } do
            grid:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
        end
        footer:attachPropertyProgram("Node.PaddingBottom", "{Screen.SafeAreaBottom}", "OneWay", true)
        self:renderMessages()
        return screen
    end,

    newRoom = Conversations.newRoom,
    profile = Identity.profile,
    auth = Identity.auth,
    details = Conversations.details,

    ["/"] = function(self) return self:home() end,
    ["/thread"] = function(self) return self:thread() end,
    ["/new"] = function(self) return self:newRoom() end,
    ["/profile"] = function(self) return self:profile() end,
    ["/auth"] = function(self) return self:auth() end,
    ["/auth/email"] = function(self) return Identity.auth(self, "email") end,
    ["/auth/signup"] = function(self) return Identity.auth(self, "signup") end,
    ["/auth/code"] = function(self) return Identity.auth(self, "code") end,
    ["/details"] = function(self) return self:details() end,

    run = function(self)
        self:installErrorHandler()
        self.screen:post("Window.Paint", renderer.getSize())
        for message in system.getMessage do
            if message:is("Window.Closed") then
                if self.polling then self.polling:stop() end
                async.set_error_handler(nil)
                return
            end
            if self.polling then
                if message:is("Window.Deactivated") then self.polling:setForeground(false)
                elseif message:is("Window.Activated") then self.polling:setForeground(true) end
            end
            local ok, failure = xpcall(function()
                local result = system.dispatchMessage(self.screen, message)
                if message:is("Window.Paint") then self.diagnostics:checkLayout()
                elseif result then self.screen:post("Window.Paint", renderer.getSize()) end
                system.translateMessage(message)
            end, Diagnostics.traceback)
            if not ok then self:reportError(failure, "Event dispatch") end
        end
    end,
}

return App
