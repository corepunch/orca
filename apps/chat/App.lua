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
        local backend = config.backend or "appwrite"
        if options.client then self.client = options.client
        elseif backend == "appwrite" then self.client, self.configurationError = Chat.appwrite(config)
        elseif backend == "supabase" then self.client, self.configurationError = Chat.supabase(config)
        elseif backend == "mock" then self.client = Chat.mock()
        else self.configurationError = { message = "Choose CHAT_BACKEND=appwrite, mock or supabase." } end
        self.media = options.media or require("Chat/media").new()
        self.remote = self.client ~= nil and self.client.mode ~= "mock"
        if self.client then self.models = Models.new(self.client) end
        self:resetSessionViews()
    end,

    resetSessionViews = function(self)
        self.sessionEpoch = (self.sessionEpoch or 0) + 1
        if self.media and self.media.clear then self.media:clear() end
        self.shell, self.room, self.contacts, self.contactsById = nil, nil, nil, nil
        self.roomLists, self.searchInput, self.pivot, self.rooms = nil, nil, nil, nil
        self.messageList, self.composer, self.threadBody = nil, nil, nil
        self.status, self.homeStatus, self.newStatus = nil, nil, nil
        self.drafts, self.photoDrafts, self.readRooms, self.pinned = {}, {}, {}, { alex = true, weekend = true }
        self.newSelection, self.inviteCodes, self.profileStatus, self.pendingIdentity = {}, {}, nil, nil
        self.filter, self.query = "All", ""
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
        for _, list in pairs(self.roomLists) do
            while list:getFirstChild() do list:getFirstChild():removeFromParent() end
        end
        local index = ({ All = 0, Unread = 1, Pinned = 2 })[self.filter]
        if self.pivot.SelectedIndex ~= index then self.pivot.SelectedIndex = index end
        local list, visible = self.roomLists[self.filter], 0
        for index, room in ipairs(self.rooms) do
            local unread = self:unread(room)
            local matches = (room.name .. " " .. (room.subtitle or "")):lower():find(self.query:lower(), 1, true)
            local inFilter = self.filter == "All" or (self.filter == "Unread" and unread > 0)
                or (self.filter == "Pinned" and self.pinned[room.id])
            if matches and inFilter then
                visible = visible + 1
                local row = ui.Button {
                    Name = "Room" .. index, Height = 76, Background = colors.background,
                    class = "conversation", Click = function() self:openRoom(room) end,
                }
                local content = ui.Grid { Columns = "56px 1fr 44px", Spacing = 14, Padding = "0 10", IgnoreHitTest = true }
                content:addChild(C.avatar(room.name, 56, room.id))
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
    end,

    home = function(self)
        local navigation = self.navigationEpoch
        if not self.client then
            return self:frame(C.page("connect", C.stack({ Spacing = 16 }, {
                C.text(self.configurationError.message, { WordWrap = true, Foreground = colors.error }),
                C.text("Check your backend settings, then restart. Appwrite uses APPWRITE_ENDPOINT, APPWRITE_PROJECT_ID and APPWRITE_CHAT_FUNCTION_ID.", { WordWrap = true }),
            })))
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
        end, { Name = "ClearSearch", Foreground = colors.textboxInk, Background = colors.textbox }))
        -- The filters are pivot items: swipe or tap a header to switch lists.
        local filters = { "All", "Unread", "Pinned" }
        self.pivot = ui.Pivot { Name = "Inbox", Title = "Messenger", Foreground = colors.ink,
            SelectionChanged = function(pivot)
                local filter = filters[pivot.SelectedIndex + 1]
                if filter ~= self.filter then self.filter = filter; self:renderRoomList() end
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
        local ok, err = self.client:signOut()
        if not ok then self:notice(err.message, true); return end
        self:resetSessionViews(); self:show()
    end,

    renderMessages = function(self)
        local room, node = self.room, self.messageList
        local messages, err = self.models.Messages:select { room_id = room.id, limit = 100 }
        if self.room ~= room or self.messageList ~= node then return end
        if not messages then self:notice(err.message, true); return end
        local me = self.client:getSession().user.id
        while node:getFirstChild() do node:getFirstChild():removeFromParent() end
        node:addChild(C.text(#messages > 0 and C.date(messages[1].created_at) or "A FRESH START", {
            FontSize = 12, Foreground = colors.dim, TextHorizontalAlignment = "Center", MarginTop = 4, MarginBottom = 6,
        }))
        for _, message in ipairs(messages) do
            local mine = message.sender_id == me and self.room.kind ~= "channel"
            local bubble = C.stack({
                Name = "MessageBubble" .. message.id, Width = 272, Padding = "12 14", Spacing = 6,
                Background = mine and colors.sent or colors.raised,
                MarginLeft = mine and auto or 0, MarginRight = mine and 0 or auto,
            })
            if not mine then
                local person = self.contactsById and self.contactsById[message.sender_id]
                local sender = self.room.kind == "channel" and self.room.name
                    or (person and person:get_display_name() or "Member " .. tostring(message.sender_id or ""):sub(1, 8))
                bubble:addChild(C.text(sender, { FontSize = 12, FontWeight = "Bold", Foreground = colors.accentText }))
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
                bubble:addChild(C.text(message.body, { WordWrap = true, FontSize = 16, Foreground = mine and colors.sentText or colors.ink }))
            end
            bubble:addChild(C.text((mine and "Sent · " or "") .. (C.time(message.created_at) or ""), {
                FontSize = 11, Foreground = mine and colors.sentMeta or colors.dim, TextHorizontalAlignment = "Right",
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
            node:addChild(C.stack({ Spacing = 8, Padding = "0 8" }, {
                C.text(channel and (readOnly and "no posts yet" or "your channel starts here") or "every good chat starts with a hello", {
                    Name = "EmptyTimeline", FontSize = 22, Foreground = colors.muted, WordWrap = true }),
                C.text(channel and (readOnly and "New posts will appear here when the channel publishes."
                    or "Publish your first update below.") or "Send the first message below.", {
                    Foreground = colors.dim, FontSize = 14, WordWrap = true }),
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
        if self.photoPreview then self.photoPreview.Visible = false; self:fitComposer() end
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
        self:fitComposer()
        self.photoLabel:setTextContent("Photo ready · " .. math.ceil(#photo.jpeg / 1000) .. " KB")
        self:notice("Photo ready. Add a caption or send it as it is.")
    end,

    fitComposer = function(self)
        local readOnly = self.room.can_post == false
        local photo = not readOnly and self.photoDrafts[self.room.id] ~= nil
        self.threadBody.Rows = ("1fr %dpx"):format(48 + (readOnly and 44 or 48) + (photo and 48 or 0))
    end,

    thread = function(self)
        if not self.room then return self:home() end
        local room = self.room
        local function keepDraft() self.drafts[room.id] = self.composer and self.composer.Text or "" end
        self.messageList = C.stack({ Name = "Messages", Spacing = 14, Overflow = "hidden scroll" })
        self.status = C.text("", { FontSize = 11, Foreground = colors.dim, TextHorizontalAlignment = "Center" })
        self.composer = C.input("Message", "type a message", {
            Text = self.drafts[room.id] or "", Submit = function() self:sendMessage() end,
        })
        local compose = ui.Grid { Name = "Composer", Height = 48 }
        compose:addChild(self.composer)
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
            self:fitComposer()
            self:notice("Photo removed. Your message is still here.")
        end, { Name = "RemovePhoto", Width = 36, Height = 36, Foreground = colors.ink }))
        local readOnly = room.can_post == false
        compose.Visible, self.photoPreview.Visible = not readOnly, not readOnly and self.photoDrafts[room.id] ~= nil
        local footer = C.stack({ Spacing = 8, Padding = "0 8" }, { self.status, self.photoPreview, compose,
            C.text("You’re subscribed · Only admins can publish", { Name = "ChannelReadOnly", Visible = readOnly,
                FontSize = 13, Foreground = colors.muted, WordWrap = true }),
        })
        -- Grid tracks are fixed or flexible, so the composer row is sized here.
        self.threadBody = ui.Grid {}
        self.threadBody:addChild(self.messageList); self.threadBody:addChild(footer)
        self:fitComposer()
        local pinned = self.pinned[room.id]
        -- Windows Phone puts the conversation commands in the application bar.
        local bar = C.appBar({
            { "BackToChats", "arrow-left", "back", function() keepDraft(); self:show() end },
            { "AttachPhoto", "plus", "attach", function() self:attachPhoto() end },
            { "SendMessage", "arrow-up", "send", function() self:sendMessage() end },
            { "PinConversation", "pin", pinned and "unpin" or "pin", function()
                keepDraft(); self.pinned[room.id] = not self.pinned[room.id]; self:show("/thread")
            end },
        }, {
            { "ConversationInfo", "conversation info", function() keepDraft(); self:show("/details") end },
            { "RefreshMessages", "refresh", function() self:renderMessages() end },
        })
        bar:findChild("AttachPhoto").Visible, bar:findChild("SendMessage").Visible = not readOnly, not readOnly
        local grid = ui.Grid { Rows = "1fr 58px", Background = colors.background, Foreground = colors.ink }
        grid:addChild(C.page(room.name, self.threadBody)); grid:addChild(bar)
        local screen = self:frame(grid)
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
