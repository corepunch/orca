local ui = require "orca.UIKit"
local Application = require "orca.core.application"
local Chat = require "api.chat"
local Models = require "Chat/models"
local system = require "orca.system"
local renderer = require "orca.renderer"
local C = require "Chat/views/components"
local Shell = require "Chat/views/shell"
local Identity = require "Chat/views/identity"
local Conversations = require "Chat/views/conversations"
local colors = C.colors
local auto = 0/0

local App = Application:extend {
    layout = function(self, ctx) return ctx.slots.inner end,

    __init = function(self, options)
        Application.__init(self)
        options = options or {}
        local config = require "Chat/config"
        local backend = config.backend or ((config.url or config.publishableKey) and "supabase" or "mock")
        if options.client then self.client = options.client
        elseif backend == "supabase" then self.client, self.configurationError = Chat.supabase(config)
        elseif backend == "mock" then self.client = Chat.mock()
        else self.configurationError = { message = "Choose CHAT_BACKEND=mock or CHAT_BACKEND=supabase." } end
        self.media = options.media or require("Chat/media").new()
        self.remote = self.client ~= nil and self.client.mode == "supabase"
        if self.client then self.models = Models.new(self.client) end
        self:resetSessionViews()
    end,

    resetSessionViews = function(self)
        self.sessionEpoch = (self.sessionEpoch or 0) + 1
        if self.media and self.media.clear then self.media:clear() end
        self.shell, self.room, self.contacts, self.contactsById = nil, nil, nil, nil
        self.roomList, self.searchInput, self.filterButtons, self.rooms = nil, nil, nil, nil
        self.messageList, self.composer, self.pinButton = nil, nil, nil
        self.status, self.homeStatus, self.newStatus = nil, nil, nil
        self.drafts, self.photoDrafts, self.readRooms, self.pinned = {}, {}, {}, { alex = true, weekend = true }
        self.newSelection, self.inviteCodes, self.profileStatus, self.pendingIdentity = {}, {}, nil, nil
        self.filter, self.query = "All", ""
    end,

    frame = function(self, content, tab)
        if tab then
            if not self.shell then
                self.shell = Shell.new(self:frame(), function(route) self:show(route) end)
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
        self.navigationEpoch = (self.navigationEpoch or 0) + 1
        local epoch = self.navigationEpoch
        local controller = self:dispatch(route or "/")
        if self.navigationEpoch == epoch then return self:activate_controller(controller) end
    end,

    notice = function(self, text, error)
        if self.status then
            self.status:setTextContent(text or "")
            self.status.Foreground = error and colors.error or colors.dim
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
        self.query = self.searchInput.Text or ""
        self:renderRoomList()
    end,

    renderRoomList = function(self)
        local list = self.roomList
        while list:getFirstChild() do list:getFirstChild():removeFromParent() end
        for label, button in pairs(self.filterButtons) do
            button.Foreground = self.filter == label and colors.ink or colors.faint
        end
        if self.query == "" and self.filter == "All" then
            local pinned = C.stack({ Spacing = 14, MarginBottom = 26 })
            local circle = C.stack({ Direction = "Horizontal", Spacing = 18 })
            for _, room in ipairs(self.rooms) do
                if self.pinned[room.id] then
                    local shortcut = C.button("", function() self:openRoom(room) end, { Width = 68, Height = 84 })
                    local content = C.stack({ Spacing = 9, AlignItems = "Center", IgnoreHitTest = true }, {
                        C.avatar(room.name, 52, room.id),
                        C.text((room.name:match("^%S+") or room.name):lower(), {
                            Width = 68, FontSize = 11, Foreground = colors.muted,
                            TextHorizontalAlignment = "Center", TextOverflow = "Ellipsis",
                        }),
                    })
                    shortcut:addChild(content)
                    circle:addChild(shortcut)
                end
            end
            if circle:getFirstChild() then
                local label = C.stack({ Direction = "Horizontal", Spacing = 8, AlignItems = "Center" }, {
                    C.icon("pin", 12, colors.dim), C.eyebrow("PINNED CONVERSATIONS"),
                })
                pinned:addChild(label); pinned:addChild(circle); list:addChild(pinned)
            end
        end
        local heading = ui.Grid { Columns = "1fr 44px", Height = 36, MarginBottom = 8 }
        heading:addChild(C.eyebrow(self.query ~= "" and "SEARCH RESULTS" or "CONVERSATIONS"))
        heading:addChild(C.iconButton("refresh-cw", function() self:show() end, { Name = "RefreshChats", Height = 32 }))
        list:addChild(heading)
        local visible = 0
        for index, room in ipairs(self.rooms) do
            local unread = self:unread(room)
            local matches = (room.name .. " " .. (room.subtitle or "")):lower():find(self.query:lower(), 1, true)
            local inFilter = self.filter == "All" or (self.filter == "Unread" and unread > 0)
                or (self.filter == "Pinned" and self.pinned[room.id])
            if matches and inFilter then
                visible = visible + 1
                local row = ui.Button {
                    Name = "Room" .. index, Height = 88, Background = colors.background,
                    class = "conversation", Click = function() self:openRoom(room) end,
                }
                local content = ui.Grid { Columns = "52px 1fr 42px", Padding = "13 14", Spacing = 11, IgnoreHitTest = true }
                content:addChild(C.avatar(room.name, 52, room.id))
                content:addChild(C.stack({ JustifyContent = "Center", Spacing = 2, IgnoreHitTest = true }, {
                    C.text(room.name:lower(), { FontSize = 21, TextOverflow = "Ellipsis" }),
                    C.text(room.subtitle or "Start the conversation", {
                        FontSize = 13, Foreground = unread > 0 and colors.accentText or colors.dim, TextOverflow = "Ellipsis",
                    }),
                }))
                local meta = C.stack({ AlignItems = "End", JustifyContent = "Center", Spacing = 9, IgnoreHitTest = true }, {
                    C.text(self:roomTime(room), { FontSize = 10, Foreground = unread > 0 and colors.accentText or colors.dim }),
                })
                if unread > 0 then meta:addChild(C.badge(unread, { Width = 20, Height = 20 }))
                elseif self.pinned[room.id] then meta:addChild(C.icon("pin", 12, colors.dim)) end
                content:addChild(meta); row:addChild(content); list:addChild(row)
            end
        end
        if visible == 0 then
            list:addChild(C.stack({ Spacing = 12, Padding = "10 24" }, {
                C.text(self.query ~= "" and "No conversations found" or "You're all caught up", { FontSize = 21, FontWeight = "Bold" }),
                C.text(self.query ~= "" and "Try another name or message." or "Start a new chat, or choose another filter.", {
                    FontSize = 13, Foreground = colors.muted, WordWrap = true,
                }),
                C.button("Start a conversation", function() self:show("/new") end, { Background = colors.soft }),
            }))
        end
    end,

    home = function(self)
        local navigation = self.navigationEpoch
        if not self.client then
            return self:frame(C.stack({ Padding = "26 56", Spacing = 18 }, {
                C.brand(), C.text("Connect your chat", { FontSize = 30, FontWeight = "Bold" }),
                C.text(self.configurationError.message, { WordWrap = true, Foreground = colors.error }),
                C.text("Set SUPABASE_URL and SUPABASE_PUBLISHABLE_KEY, then restart the app.", { WordWrap = true }),
            }))
        end
        if not self.client:getSession() then self:resetSessionViews(); return self:auth() end
        local rooms, err = self.models.Rooms:select()
        self.rooms = rooms or {}
        local contacts, contactsError = self.models.Contacts:select()
        if self.navigationEpoch ~= navigation then return self.screen end
        if not self.client:getSession() then self:resetSessionViews(); return self:auth() end
        self.contacts, self.contactsById = contacts or {}, {}
        for _, contact in ipairs(self.contacts) do self.contactsById[contact.id] = contact end
        err = err or contactsError
        if self.shell and self.shell:has("Chats") then
            self.status = self.homeStatus
            self:renderRoomList()
            self:notice(err and err.message or (self.remote and "Refresh to sync your conversations" or "Offline mock · make yourself at home"), err ~= nil)
            return self.shell:show("Chats")
        end
        self.status = C.text(err and err.message or (self.remote and "Refresh to sync your conversations" or "Offline mock · make yourself at home"), {
            FontSize = 10, Foreground = err and colors.error or colors.dim, TextHorizontalAlignment = "Center",
        })
        self.homeStatus = self.status
        local brand = ui.Grid { Columns = "1fr 36px", Height = 36 }
        brand:addChild(C.brand())
        local profile = C.button("", function() self:show("/profile") end, { Width = 36, Height = 36 })
        profile:addChild(C.avatar("You", 34, "me")); brand:addChild(profile)
        -- Pivot headers: the filters double as the page title.
        local title = ui.Grid { Columns = "1fr 46px", Height = 52, MarginTop = 8 }
        local filters = C.stack({ Direction = "Horizontal", Spacing = 16, Height = 52 })
        title:addChild(filters)
        title:addChild(C.iconButton("plus", function() self:show("/new") end, {
            Name = "NewChat", Background = colors.accent, Foreground = colors.onAccent,
        }))
        self.searchInput = C.input("Search", "Search conversations", {
            Text = self.query, Height = 46, Padding = "12 0", Background = "#00000000",
            Submit = function() self:updateSearch() end,
            KeyUp = function() self:updateSearch() end,
        })
        self.searchInput.BorderWidth = "0"
        local search = ui.Grid { Columns = "22px 1fr 28px", Height = 48, Spacing = 4, Padding = "12 0", MarginTop = 12,
            Background = colors.surface, BorderWidth = "2", BorderColor = colors.line }
        search:addChild(C.icon("search", 18, colors.dim)); search:addChild(self.searchInput)
        search:addChild(C.iconButton("x", function()
            self.query, self.searchInput.Text = "", ""; self:renderRoomList(); self.searchInput:setFocus()
        end, { Width = 28, Height = 44, Name = "ClearSearch" }))
        self.filterButtons = {}
        for _, pivot in ipairs { { "All", 46 }, { "Unread", 104 }, { "Pinned", 100 } } do
            local label = pivot[1]
            local button = C.button(label:lower(), function() self.filter = label; self:renderRoomList() end, {
                Width = pivot[2], Height = 52, FontSize = 32, FontWeight = "Normal", Name = "Filter" .. label,
            })
            button.TextHorizontalAlignment = "Left"
            self.filterButtons[label] = button; filters:addChild(button)
        end
        local header = C.stack({ Padding = "22 22 22 12" }, { brand, title, search })
        self.roomList = C.stack({ Name = "ConversationList", Padding = "22 12 22 20", Overflow = "hidden scroll" })
        local footer = C.stack({ Padding = "0 2", Spacing = 8 }, { self.status })
        local grid = ui.Grid { Rows = "190px 1fr 22px", Background = colors.background }
        grid:addChild(header); grid:addChild(self.roomList); grid:addChild(footer)
        local screen = self:frame(grid, "Chats")
        self:renderRoomList()
        return screen
    end,

    renderMessages = function(self)
        local room, node = self.room, self.messageList
        local messages, err = self.models.Messages:select { room_id = room.id, limit = 100 }
        if self.room ~= room or self.messageList ~= node then return end
        if not messages then self:notice(err.message, true); return end
        local me = self.client:getSession().user.id
        while node:getFirstChild() do node:getFirstChild():removeFromParent() end
        node:addChild(C.text(#messages > 0 and C.date(messages[1].created_at) or "A FRESH START", {
            FontSize = 10, Foreground = colors.dim, LetterSpacing = 1, TextHorizontalAlignment = "Center", MarginTop = 16, MarginBottom = 20,
        }))
        for _, message in ipairs(messages) do
            local mine = message.sender_id == me and self.room.kind ~= "channel"
            local bubble = C.stack({
                Name = "MessageBubble" .. message.id, Width = 284, Padding = "15 12", Spacing = 8,
                Background = mine and colors.sent or colors.raised,
                MarginLeft = mine and auto or 0, MarginRight = mine and 0 or auto,
            })
            if not mine then
                local person = self.contactsById and self.contactsById[message.sender_id]
                local sender = self.room.kind == "channel" and self.room.name
                    or (person and person:get_display_name() or "Member " .. tostring(message.sender_id or ""):sub(1, 8))
                bubble:addChild(C.text(sender, { FontSize = 10, FontWeight = "Bold", Foreground = colors.accentText }))
            end
            for _, attachment in ipairs(message.attachments or {}) do
                local bytes, photoError = self.client:getPhoto(attachment.id, true)
                if self.room ~= room or self.messageList ~= node then return end
                local media = self:getMedia()
                local path = bytes and media and media:storePreview(bytes, attachment.id)
                if path then
                    bubble:addChild(ui.ImageView { Name = "Photo" .. attachment.id, Src = path,
                        Width = 250, Height = 180, Stretch = "Uniform", IgnoreHitTest = true })
                else
                    bubble:addChild(C.text(photoError and photoError.message or "Photo preview unavailable", {
                        FontSize = 12, Foreground = colors.muted, WordWrap = true }))
                end
            end
            if message.body and message.body ~= "" then
                bubble:addChild(C.text(message.body, { WordWrap = true, FontSize = 14, Foreground = mine and colors.sentText or colors.ink }))
            end
            bubble:addChild(C.text((mine and "Sent · " or "") .. (C.time(message.created_at) or ""), {
                FontSize = 9, Foreground = mine and colors.sentMeta or colors.dim, TextHorizontalAlignment = "Right",
            }))
            -- Incoming tails point up from the top-left; outgoing ones hang from the bottom-right.
            local entry, tone = C.stack({ Spacing = 0 }), mine and colors.sent or colors.raised
            if not mine then entry:addChild(C.tail(true, tone, false)) end
            entry:addChild(bubble)
            if mine then entry:addChild(C.tail(false, tone, true)) end
            node:addChild(entry)
        end
        if #messages == 0 then
            local channel, readOnly = self.room.kind == "channel", self.room.can_post == false
            node:addChild(C.stack({ Spacing = 12, Padding = "16 32" }, {
                C.text(channel and (readOnly and "No posts yet." or "Your channel\nstarts here.") or "Every good chat starts\nwith a hello.", {
                    Name = "EmptyTimeline", FontSize = 27, FontWeight = "Bold", WordWrap = true }),
                C.text(channel and (readOnly and "New posts will appear here when the channel publishes."
                    or "Publish your first update below.") or "Send the first message below.", {
                    Foreground = colors.muted, FontSize = 13, WordWrap = true }),
            }))
        end
        local root = node:getRoot()
        root:UpdateLayout(root.Width, root.Height)
        -- Property structs use exported X/Y fields; lowercase keys are ignored.
        node.ContentOffset = { X = 0, Y = math.min(0, node.ActualHeight - node.ScrollHeight) }
        self:notice(self.remote and "Synced · refresh for new messages" or "Offline mock · messages stay in this session")
    end,

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
        self.composer:setFocus()
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
        local header = ui.Grid {
            Columns = "38px 40px 1fr 38px 38px", Padding = "12 20", Spacing = 8,
            Background = colors.background, BorderWidthBottom = 1, BorderColor = colors.line,
        }
        header:addChild(C.iconButton("arrow-left", function()
            self.drafts[self.room.id] = self.composer.Text; self:show()
        end, { Width = 38, Name = "BackToChats" }))
        header:addChild(C.avatar(self.room.name, 40, self.room.id))
        local info = C.button("", function()
            self.drafts[self.room.id] = self.composer and self.composer.Text or ""
            self:show("/details")
        end, { Name = "ConversationInfo", Height = 52 })
        info:addChild(C.stack({ Spacing = 5, JustifyContent = "Center", IgnoreHitTest = true }, {
            C.text(self.room.name:lower(), { FontSize = 22, TextOverflow = "Ellipsis" }),
            C.text((self.room.kind or "group") .. " · Tap for info", { FontSize = 10, Foreground = colors.dim }),
        }))
        header:addChild(info)
        self.pinButton = C.iconButton("pin", function()
            self.drafts[self.room.id] = self.composer.Text
            self.pinned[self.room.id] = not self.pinned[self.room.id]
            self:show("/thread")
        end, { Width = 38, Name = "PinConversation", Background = self.pinned[self.room.id] and colors.soft or "#00000000",
            Foreground = self.pinned[self.room.id] and colors.accentText or colors.dim })
        header:addChild(self.pinButton)
        header:addChild(C.iconButton("refresh-cw", function() self:renderMessages() end, { Width = 38, Name = "RefreshMessages" }))
        self.messageList = C.stack({ Name = "Messages", Padding = "20 8", Spacing = 12, Overflow = "hidden scroll" })
        self.status = C.text("", { FontSize = 9, Foreground = colors.dim, TextHorizontalAlignment = "Center" })
        self.composer = C.input("Message", "Write a message…", {
            Text = self.drafts[self.room.id] or "", Height = 50, Submit = function() self:sendMessage() end,
        })
        local compose = ui.Grid { Name = "Composer", Columns = "40px 1fr 50px", Spacing = 8 }
        compose:addChild(C.iconButton("plus", function() self:attachPhoto() end, {
            Name = "AttachPhoto", Width = 40, Height = 50, Background = colors.soft }))
        compose:addChild(self.composer)
        compose:addChild(C.iconButton("arrow-up", function() self:sendMessage() end, {
            Name = "SendMessage", Background = colors.accent, Foreground = colors.onAccent, Width = 50, Height = 50,
        }))
        self.photoLabel = C.text("Photo ready", { FontSize = 11, Foreground = colors.accentText, TextVerticalAlignment = "Center" })
        self.photoPreview = ui.Grid { Name = "PhotoDraft", Columns = "1fr 40px", Height = 38,
            Visible = self.photoDrafts[self.room.id] ~= nil, Background = colors.soft, Padding = "10 0" }
        self.photoPreview:addChild(self.photoLabel)
        self.photoPreview:addChild(C.iconButton("x", function()
            if self.sending then self:notice("The photo is being sent. Please wait."); return end
            local photo = self.photoDrafts[self.room.id]
            if photo and photo.attachment then
                local ok, err = self.client:discardPhoto(photo.attachment.id)
                if not ok then self:notice(err.message, true); return end
            end
            self.photoDrafts[self.room.id], self.photoPreview.Visible = nil, false
            self:notice("Photo removed. Your message is still here.")
        end, { Name = "RemovePhoto", Width = 36, Height = 36 }))
        local readOnly = self.room.can_post == false
        compose.Visible, self.photoPreview.Visible = not readOnly, not readOnly and self.photoDrafts[self.room.id] ~= nil
        local footer = C.stack({ Padding = "16 10 16 14", Spacing = 8, Background = colors.surface,
            BorderWidthTop = 1, BorderColor = colors.line }, { self.status, self.photoPreview, compose,
                C.text("You’re subscribed · Only admins can publish", { Name = "ChannelReadOnly", Visible = readOnly,
                    FontSize = 12, Foreground = colors.muted, TextHorizontalAlignment = "Center", WordWrap = true }),
            })
        local grid = ui.Grid { Rows = "90px 1fr 146px", Background = colors.background }
        grid:addChild(header); grid:addChild(self.messageList); grid:addChild(footer)
        local screen = self:frame(grid)
        self:renderMessages()
        return screen
    end,

    pageHeader = function(self, title)
        local header = ui.Grid { Columns = "44px 1fr 44px", Padding = "14 16", Spacing = 10 }
        header:addChild(C.iconButton("arrow-left", function() self:show() end, { Name = "BackToChats" }))
        header:addChild(C.text(title, { FontSize = 16, FontWeight = "Bold", TextVerticalAlignment = "Center" }))
        header:addChild(C.icon("message-circle", 21, colors.accentText))
        return header
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
    ["/details"] = function(self) return self:details() end,

    run = function(self)
        self.screen:post("Window.Paint", renderer.getSize())
        for message in system.getMessage do
            if message:is("Window.Closed") then return end
            local ok, result = pcall(system.dispatchMessage, self.screen, message)
            if not ok then io.stderr:write(tostring(result) .. "\n")
            elseif result and not message:is("Window.Paint") then self.screen:post("Window.Paint", renderer.getSize()) end
            system.translateMessage(message)
        end
    end,
}

return App
