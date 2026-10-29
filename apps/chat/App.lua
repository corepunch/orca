local ui = require "orca.UIKit"
local Footer = require "orca.UIKit.Footer"
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
local Contacts = require "Chat/views/contacts"
local Timeline = require "Chat/views/timeline"
local PhotoViewer = require "Chat/views/photo_viewer"
local Diagnostics = require "Chat/diagnostics"
local async = require "orca.core.async"
local orca = require "orca"
local AvatarLoader = require "orca.UIKit.AvatarLoader"
local PageTransition = require "orca.UIKit.PageTransition"
local data = require "orca.data"
local Visibility = require "orca.data.visibility"
local filesystem = require "orca.filesystem"
local ChatProvider = require "Chat/providers/chat"
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
                settings.pushStore = require("Chat/push_store").new(config)
            end
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
        if self.remote and self.client and self.client.mode == "appwrite" then
            self.push = require("Chat/push").new(self, config, options.push)
        end
    end,

    resetSessionViews = function(self)
        PhotoViewer.close(self)
        if self.polling then self.polling:stop() end
        -- Records belong to one account: the next session gets a new store.
        if self.scope then self.scope:dispose() end
        self.scope, self.visibility = nil, Visibility.new()
        self.sessionEpoch = (self.sessionEpoch or 0) + 1
        if self.avatars then self.avatars:clear() end
        self.avatarPaths = {}
        self.avatars = AvatarLoader.new(self, {
            lookup = function(ids) return self.client:getAvatars(ids) end,
            fetch = function(id) return self.client:getAvatar(id) end,
            cached = function(id) return self.media.keys and self.media.keys["avatar-" .. id] end,
            store = function(bytes, id) return self.media:storePreview(bytes, "avatar-" .. id) end,
            apply = C.photo,
            schedule = function(fn) orca.async(fn) end,
            repaint = function() if self.screen then self.screen:post("Window.Paint", renderer.getSize()) end end,
        })
        if self.messagePhotos then self.messagePhotos:clear() end
        self.messagePhotos = require("Chat/photo_loader").new(self)
        if self.media and self.media.clear then self.media:clear() end
        self.shell, self.room, self.contacts, self.contactsById = nil, nil, nil, nil
        self.pivot = nil
        self.messageList, self.composer, self.threadBody = nil, nil, nil
        self.timelineSnapshot, self.timelinePage, self.messageRefreshQueued, self.editing = nil, nil, nil, nil
        self.status, self.homeStatus, self.newStatus = nil, nil, nil
        self.contactsState, self.contactsPanorama, self.contactsStatus = nil, nil, nil
        self.messageLoading = nil
        self.authDraft, self.loginIdentifier, self.loginReturnRoute = nil, nil, nil
        self.drafts, self.photoDrafts, self.readRooms, self.pinned = {}, {}, {}, { alex = true, weekend = true }
        self.newSelection, self.inviteCodes, self.profileStatus, self.pendingIdentity = {}, {}, nil, nil
    end,

    -- The signed-in account's data scope (Data/Inbox.xml): rooms, contacts,
    -- local views and avatars. Late results from another session never apply.
    dataScope = function(self)
        local session = self.client and self.client:getSession()
        if not session then return nil end
        if self.scope and not self.scope.disposed then return self.scope end
        local epoch, user = self.sessionEpoch, session.user.id
        local provider = ChatProvider.new {
            client = self.client, models = self.models, media = self.media, avatarPaths = self.avatarPaths,
            read = self.readRooms, pinned = self.pinned,
            openRoom = function(room) self:openRoom(room) end,
            sessionLost = function()
                if self.sessionEpoch == epoch then self:resetSessionViews(); self:show("/") end
            end,
        }
        provider.repaint = function() self:repaint() end
        local host = data.Scope.defaultHost()
        host.run = function(fn) self:load(fn) end
        host.report = function(err, stage) self:reportError(err, stage) end
        self.scope = data.scope("Chat/Data/Inbox.xml", { provider = provider, identity = "chat:" .. user, host = host,
            valid = function()
                local active = self.client:getSession()
                return self.sessionEpoch == epoch and active ~= nil and active.user.id == user
            end })
        self.scope:member("Rooms"):listen(function(query) self:inboxStatus(query) end)
        self.scope:member("Contacts"):listen(function(query)
            self.contacts, self.contactsById = {}, {}
            for _, record in ipairs(query:records()) do
                self.contacts[#self.contacts + 1] = record.raw
                self.contactsById[record.key] = record.raw
            end
        end)
        return self.scope
    end,

    -- The inbox status line reports a failed refresh; rows stay visible.
    inboxStatus = function(self, query)
        local status = self.homeStatus
        if not status or query.mode ~= nil and not query.quiet then return end
        local err = query.error
        status:setTextContent(err and err.message or "")
        status.Foreground = err and colors.error or colors.dim
        status.Visible = err ~= nil
    end,

    -- A room changed elsewhere (renamed, left, pinned): update its record.
    roomChanged = function(self, id, values)
        local store = self.scope and self.scope.store
        if not store then return end
        if values == false then return store:remove("ChatRoom", id) end
        if values then values.id = id; store:put("ChatRoom", values, { partial = true }) end
        local record = store:get("ChatRoom", id)
        if record then store:refresh(record) end
    end,

    repaint = function(self)
        if self.screen then self.screen:post("Window.Paint", renderer.getSize()) end
    end,

    -- Route constructors attach their controls before any request can yield.
    load = function(self, work)
        orca.async(function()
            local ok, err = xpcall(work, Diagnostics.traceback)
            if not ok then self:reportError(err, "Loading content") end
            if self.screen then self.screen:post("Window.Paint", renderer.getSize()) end
        end)
    end,

    loadingPage = function(self, title, tab, request, render, page)
        local host = ui.Node2D { Name = "LoadingContent" }
        local screen = self:frame(C.page(title, host, page), tab)
        local epoch, navigation, route = self.sessionEpoch, self.navigationEpoch, self.currentRoute
        local function current()
            return self.sessionEpoch == epoch and (tab ~= nil or self.navigationEpoch == navigation)
        end
        local function load()
            while host:getFirstChild() do host:getFirstChild():removeFromParent() end
            host:addChild(ui.ActivityIndicator { Name = "ContentLoading", Foreground = colors.muted })
            self:load(function()
                if not current() then return end
                local value, err = request()
                if not current() then return end
                if not self.client:getSession() then self:resetSessionViews(); self:show("/"); return end
                while host:getFirstChild() do host:getFirstChild():removeFromParent() end
                if value then
                    local status = self.status
                    host:addChild(render(value))
                    if tab and self.currentRoute ~= route then self.status = status end
                else
                    host:addChild(C.stack({ Spacing = 16, JustifyContent = "Center" }, {
                        C.text(err and err.message or "Could not load this page.", { WordWrap = true, Foreground = colors.error }),
                        C.button("Retry", load, { Name = "RetryContent" }),
                    }))
                end
            end)
        end
        load()
        return screen
    end,

    avatarPhoto = function(self, tile, userId)
        if self.client and self.media then self.avatars:watch(tile, userId) end
    end,

    afterPaint = function(self)
        if self.visibility then self.visibility:update() end
        if self.avatars then self.avatars:loadVisible() end
        if self.messagePhotos then self.messagePhotos:loadVisible() end
        if self.currentRoute == "/thread" then Timeline.loadNearTop(self) end
    end,

    activate_controller = function(self, controller)
        self:installErrorHandler()
        local result = Application.activate_controller(self, controller)
        if self.polling then self.polling:activate() end
        if self.push then self.push:activate() end
        return result
    end,

    installErrorHandler = function(self)
        async.set_error_handler(function(err) self:reportError(err, "Background action") end)
    end,

    reportError = function(self, err, stage)
        self.diagnostics:show(err, stage)
    end,

    dispatch = function(self, request)
        PhotoViewer.close(self)
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
            StyleSheet = "Chat/Styles/app.css", class = "chat-" .. colors.appearance,
        }
        if content then screen:addChild(content) end
        return screen
    end,

    -- `direction` navigates as on Windows Phone: "Forward" going into a
    -- conversation or its info, "Backward" coming back out. The page in view goes out
    -- first and the next page comes in once it has gone.
    show = function(self, route, direction)
        PhotoViewer.close(self)
        local request = {}
        self.pendingShow = request
        local leaving = PageTransition.leave(self.screen, direction, function()
            if self.pendingShow == request then self:navigate(route, direction) end
        end)
        if not leaving then return self:navigate(route, direction) end
    end,

    navigate = function(self, route, direction)
        if self.polling then self.polling:stop() end
        -- Inbox polling follows the visible page; home() reactivates it.
        if self.scope then self.scope:deactivate() end
        self.navigationEpoch = (self.navigationEpoch or 0) + 1
        local epoch = self.navigationEpoch
        local controller = self:dispatch(route or "/")
        if self.navigationEpoch ~= epoch then return end
        local result = self:activate_controller(controller)
        if result then PageTransition.enter(self.screen, direction) end
        return result
    end,

    goBack = function(self, route) return self:show(route, "Backward") end,

    notice = function(self, text, error)
        if self.status then
            self.status:setTextContent(text or "")
            self.status.Foreground = error and colors.error or colors.dim
            self.status.Visible = text ~= nil and text ~= ""
        elseif error then
            self:reportError(text, "Request")
        end
    end,

    openRoom = function(self, room)
        self.room, self.readRooms[room.id] = room, true
        self:show("/thread", "Forward")
    end,

    unread = function(self, room)
        return not self.readRooms[room.id] and (room.unread or 0) or 0
    end,

    roomTime = function(self, room)
        return C.time(room.updated_at or room.created_at)
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
                C.text(self.logoutError or "Couldn't remove your saved login. Please retry signing out.", { WordWrap = true, Foreground = colors.muted }),
                C.button("Retry sign out", function() self:signOut() end, { Name = "RetrySignOut" }),
            })))
        end
        if self.restorePending then
            local loading = ui.Grid { Name = "SessionSplash", Background = C.brandBackground }
            loading:addChild(ui.ImageView { Name = "LoadingGradient", Src = "Chat/assets/splash-gradient.png",
                Stretch = "Fill", Foreground = "#ffffff", IgnoreHitTest = true })
            loading:addChild(ui.ImageView { Name = "LoadingLogo", Src = "Chat/assets/splash-mark.png",
                Width = 128, Height = 128, Stretch = "Uniform", Foreground = "#ffffff", IgnoreHitTest = true,
                MarginLeft = auto, MarginRight = auto, MarginTop = auto, MarginBottom = auto })
            loading:addChild(C.stack({ MarginLeft = auto, MarginRight = auto, MarginTop = auto,
                MarginBottom = auto, PaddingTop = 216, IgnoreHitTest = true }, {
                ui.ActivityIndicator { Name = "SessionLoading", Foreground = "#ffffff" },
            }))
            local screen = self:frame(loading)
            screen.RespectSafeArea, screen.ClearColor = false, C.brandBackground
            local epoch = self.sessionEpoch
            self:load(function()
                if self.navigationEpoch ~= navigation or self.sessionEpoch ~= epoch then return end
                local restored, err = self.client:restoreSession()
                if self.navigationEpoch ~= navigation or self.sessionEpoch ~= epoch then return end
                if restored or (err and (err.code == "unauthenticated" or err.status == 401 or err.status == 403)) then
                    self.restorePending = false
                    self:show("/")
                else
                    while screen:getFirstChild() do screen:getFirstChild():removeFromParent() end
                    screen.RespectSafeArea, screen.ClearColor = true, colors.background
                    screen:addChild(C.page("reconnect", C.stack({ Spacing = 16 }, {
                        C.text("Couldn't restore your session. Please try again.", { WordWrap = true, Foreground = colors.muted }),
                        C.button("Retry", function() self:show("/") end, { Name = "RetrySessionRestore" }),
                    })))
                end
            end)
            return screen
        end
        if not self.client:getSession() then self:resetSessionViews(); return self:auth() end
        local scope = self:dataScope()
        if self.shell and self.shell:has("Chats") then
            self.status = self.homeStatus
            -- Returning to the inbox refreshes it quietly; rows stay in place.
            scope:activate()
            scope:member("Rooms"):refresh { quiet = true }
            return self.shell:show("Chats")
        end
        local page = assert(filesystem.loadObject("Chat/Pages/Inbox"), "Missing Chat/Pages/Inbox")
        scope:attach(page)
        self.status = assert(page:findChild("InboxStatus"))
        self.homeStatus, self.pivot = self.status, page:findChild("Inbox")
        self.visibility:watch(page, scope)
        local screen = self:frame(page, "Chats")
        scope:activate()
        return screen
    end,

    signOut = function(self)
        if self.polling then self.polling:stop() end
        if self.push then self.push:suspend() end
        self.sessionEpoch = (self.sessionEpoch or 0) + 1
        local ok, err = self.client:signOut()
        self.restorePending = false
        self.logoutPending = not ok and (err.code == "session_storage" or err.code == "push_cleanup")
        self.logoutError = self.logoutPending and err.message or nil
        self:resetSessionViews(); self:show()
        if not ok and not self.logoutPending then self:notice(err.message, true) end
    end,

    renderMessages = function(self, options)
        local indicator, node, epoch = self.messageLoading, self.messageList, self.sessionEpoch
        local result, err = Timeline.refresh(self, options)
        if self.sessionEpoch == epoch and not self.client:getSession() then self:resetSessionViews(); self:show("/"); return result, err end
        if indicator and (err or (self.timelineSnapshot and self.timelineSnapshot.node == node)) then indicator.Visible = false end
        return result, err
    end,

    sendMessage = function(self)
        if self.editing then return self:saveEdit() end
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

    -- Message actions: tapping your own message (or any message, for group and
    -- channel staff) offers edit and delete; deletion asks once more.
    openMessageActions = function(self, message, canEdit, canDelete)
        local overlay, sheet = self.messageActions, self.messageActionSheet
        if not overlay or self.sending then return end
        local function show(items)
            while sheet:getFirstChild() do sheet:getFirstChild():removeFromParent() end
            for _, item in ipairs(items) do
                sheet:addChild(C.button(item[2], function() overlay.Visible = false; item[3]() end, { Name = item[1],
                    Height = 52, FontSize = 20, TextHorizontalAlignment = "Left", Padding = "16 0",
                    Foreground = item[4] or colors.ink, Background = colors.chrome, BorderWidth = "0" }))
            end
            sheet.Height, overlay.Visible = #items * 52 + 16, #items > 0
        end
        local items = {}
        if canEdit then items[#items + 1] = { "EditMessage", "edit", function() self:startEdit(message) end } end
        if canDelete then
            -- Deleting removes the message for everyone, so confirm it first.
            items[#items + 1] = { "DeleteMessage", "delete", function()
                show {
                    { "ConfirmDeleteMessage", "delete for everyone", function() self:deleteMessage(message) end, colors.error },
                    { "CancelDeleteMessage", "cancel", function() end },
                }
            end, colors.error }
        end
        show(items)
    end,

    startEdit = function(self, message)
        local room, composer = self.room, self.composer
        if not room or not composer or message.room_id ~= room.id then return end
        if self.photoDrafts[room.id] then self:notice("Send or remove the photo before editing a message.", true); return end
        -- The unsent draft returns once the edit is saved or cancelled.
        if not self.editing then self.drafts[room.id] = composer.Text ~= "" and composer.Text or nil end
        self.editing = { room = room, message = message }
        composer.Text = message.body or ""
        self.editBanner.Visible = true
        self:notice("")
        composer:setFocus()
    end,

    cancelEdit = function(self)
        local editing = self.editing
        if not editing then return end
        self.editing = nil
        if self.editBanner then self.editBanner.Visible = false end
        if self.room and self.room.id == editing.room.id and self.composer then self.composer.Text = self.drafts[editing.room.id] or "" end
    end,

    saveEdit = function(self)
        local editing, composer, epoch = self.editing, self.composer, self.sessionEpoch
        if self.sending or not editing then return end
        local text = composer.Text or ""
        if text == (editing.message.body or "") then self:cancelEdit(); return end
        self.sending = true
        local edited, err = editing.message:edit(text)
        self.sending = false
        if self.sessionEpoch ~= epoch or self.editing ~= editing then return end
        if not edited then self:notice(err.message, true); return end
        self:cancelEdit()
        self:renderMessages { preserveScroll = true }
    end,

    deleteMessage = function(self, message)
        local room, epoch = self.room, self.sessionEpoch
        if self.sending then return end
        self.sending = true
        local ok, err = message:delete()
        self.sending = false
        if self.sessionEpoch ~= epoch or not self.room or self.room.id ~= room.id then return end
        if not ok then self:notice(err.message, true); return end
        if self.editing and self.editing.message.id == message.id then self:cancelEdit() end
        local page = self.timelinePage
        if page then
            for i, row in ipairs(page.messages) do
                if row.id == message.id then table.remove(page.messages, i); break end
            end
        end
        self:renderMessages { preserveScroll = true, quiet = true }
        self:notice("Message deleted.")
    end,

    getMedia = function(self)
        return self.media
    end,

    openPhoto = function(self, attachment)
        return PhotoViewer.open(self, attachment)
    end,

    attachPhoto = function(self)
        if self.sending then self:notice("Finish sending before adding another photo."); return end
        if self.editing then self:notice("Finish editing before adding a photo."); return end
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
        self.timelinePage, self.timelineSnapshot = nil, nil
        -- Leaving the thread abandons an edit but keeps the unsent draft.
        local function keepDraft()
            if self.editing then self.editing = nil
            else self.drafts[room.id] = self.composer and self.composer.Text or "" end
        end
        if self.editing and self.editing.room.id ~= room.id then self.editing = nil end
        -- At the newest message, the list stays there as the keyboard and composer grow.
        self.messageList = C.stack({ Name = "Messages", Spacing = 0, PaddingBottom = 8, Overflow = "hidden scroll",
            ScrollAnchor = "Bottom" })
        self.status = C.text("", { FontSize = 11, Foreground = colors.dim, TextHorizontalAlignment = "Center",
            Name = "ThreadStatus", Visible = false, Margin = "24 8" })
        self.composer = C.input("Message", "type a message", {
            Text = self.drafts[room.id] or "", Submit = function() self:sendMessage() end, BorderRadius = "0",
        })
        -- Match the compact ApplicationBar's 58-point height, with breathing
        -- room around the 48-point controls. Footer adds system insets separately.
        local compose = ui.Grid { Name = "Composer", Columns = "48px 1fr 48px", Height = 58,
            PaddingTop = 5, PaddingBottom = 5 }
        compose:addChild(C.iconButton("plus", function() self:attachPhoto() end,
            { Name = "AttachPhoto", Width = 48, Height = 48, Foreground = colors.ink }))
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
        self.editBanner = ui.Grid { Name = "EditingMessage", Columns = "1fr 40px", Height = 40,
            Visible = self.editing ~= nil, Background = colors.soft, Padding = "12 0" }
        self.editBanner:addChild(C.text("Editing message", { FontSize = 13, Foreground = colors.accentText, TextVerticalAlignment = "Center" }))
        self.editBanner:addChild(C.iconButton("x", function() self:cancelEdit() end,
            { Name = "CancelEdit", Width = 36, Height = 36, Foreground = colors.ink }))
        if self.editing then self.composer.Text = self.editing.message.body or "" end
        local readOnly = room.can_post == false
        compose.Visible, self.photoPreview.Visible = not readOnly, not readOnly and self.photoDrafts[room.id] ~= nil
        local footer = Footer { Name = "ThreadFooter", class = "frosted bar-top" }
        footer:addChild(C.stack({ Spacing = 0 }, { self.status, self.editBanner, self.photoPreview, compose,
            C.text("You’re subscribed · Only admins can publish", { Name = "ChannelReadOnly", Visible = readOnly,
                FontSize = 13, Foreground = colors.muted, WordWrap = true, Padding = "24 12" }),
        }))
        -- After Messenger: no command row. The title line goes back, as does a
        -- swipe in from the left edge, and the conversation's name opens its info.
        self.threadBody = ui.Node2D {}
        local timeline = ui.Node2D {}
        timeline:addChild(self.messageList)
        self.messageLoading = ui.ActivityIndicator { Name = "MessagesLoading", Foreground = colors.muted }
        timeline:addChild(self.messageLoading)
        self.threadBody:addChild(timeline)
        -- Keep the rectangular composer outside the page's horizontal content margins.
        local screen = self:frame()
        local kind = room.kind == "group" and "Messenger · group" or room.kind == "channel" and "Messenger · channel" or nil
        Footer.layout(screen, C.page(room.name, self.threadBody, { Name = "ThreadPage", Title = kind, ExtendBodyBehindHeader = true,
            back = function() keepDraft(); self:goBack() end,
            details = function() keepDraft(); self:show("/details", "Forward") end }), footer,
            { Background = colors.background, Foreground = colors.ink })
        -- A bottom sheet for one message's actions, dismissed by tapping outside.
        self.messageActions = ui.Node2D { Name = "MessageActionsMenu", Visible = false }
        self.messageActions:addChild(C.button("", function() self.messageActions.Visible = false end,
            { Name = "DismissMessageActions", Height = auto, BorderWidth = "0", Background = "#00000000" }))
        self.messageActionSheet = C.stack({ Name = "MessageActionSheet", Height = 120, MarginTop = auto,
            Padding = "0 8", Background = colors.chrome })
        self.messageActions:addChild(self.messageActionSheet)
        screen:addChild(self.messageActions)
        local node, epoch, navigation = self.messageList, self.sessionEpoch, self.navigationEpoch
        self:load(function()
            if self.messageList == node and self.sessionEpoch == epoch and self.navigationEpoch == navigation then
                self:renderMessages()
            end
        end)
        return screen
    end,

    newRoom = Conversations.newRoom,
    contactsView = Contacts.show,
    profile = Identity.profile,
    auth = Identity.auth,
    details = Conversations.details,

    ["/"] = function(self) return self:home() end,
    ["/thread"] = function(self) return self:thread() end,
    ["/new"] = function(self) return self:newRoom() end,
    ["/contacts"] = function(self) return self:contactsView() end,
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
                if self.push then self.push:suspend() end
                async.set_error_handler(nil)
                return
            end
            if self.polling then
                if message:is("Window.Deactivated") then self.polling:setForeground(false)
                elseif message:is("Window.Activated") then self.polling:setForeground(true) end
            end
            if self.scope then
                if message:is("Window.Deactivated") then self.scope:setForeground(false)
                elseif message:is("Window.Activated") then self.scope:setForeground(true) end
            end
            if self.push then
                if message:is("Window.Deactivated") then self.push:setForeground(false)
                elseif message:is("Window.Activated") then self.push:setForeground(true) end
            end
            local ok, failure = xpcall(function()
                local result = system.dispatchMessage(self.screen, message)
                if message:is("Window.Paint") then self.diagnostics:checkLayout(); self:afterPaint()
                elseif result then self.screen:post("Window.Paint", renderer.getSize()) end
                system.translateMessage(message)
            end, Diagnostics.traceback)
            if not ok then self:reportError(failure, "Event dispatch") end
        end
    end,
}

return App
