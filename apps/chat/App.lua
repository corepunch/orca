local ui = require "orca.UIKit"
local Application = require "orca.core.application"
local Chat = require "api.chat"
local system = require "orca.system"
local renderer = require "orca.renderer"
local C = require "Chat/views/components"
local colors = C.colors
local auto = 0/0

local App = Application:extend {
    layout = function(self, ctx) return ctx.slots.inner end,

    __init = function(self)
        Application.__init(self)
        local config = require "Chat/config"
        self.remote = config.url ~= nil or config.publishableKey ~= nil
        if self.remote then self.client, self.configurationError = Chat.supabase(config)
        else self.client = Chat.demo() end
        self.drafts, self.readRooms, self.pinned = {}, {}, { alex = true, weekend = true }
        self.filter, self.query = "All", ""
    end,

    frame = function(self, content)
        local screen = ui.Screen {
            Width = 390, Height = 844, ResizeMode = "NoResize", ClearColor = colors.background,
            StyleSheet = "Chat/Styles/app.css",
        }
        screen:addChild(content)
        return screen
    end,

    show = function(self, route) self:activate_route(route or "/") end,

    notice = function(self, text, error)
        if self.status then
            self.status:setTextContent(text or "")
            self.status.Foreground = error and colors.error or colors.dim
        end
    end,

    navigation = function(self, active)
        local bar = ui.Grid {
            Columns = "1fr 1fr 1fr", Spacing = 14, Padding = "22 12 22 16",
            Background = colors.surface, BorderWidthTop = 1, BorderColor = colors.line,
        }
        bar:addChild(C.navItem("Chats", "message-circle", function() self:show() end, active == "Chats"))
        bar:addChild(C.navItem("New chat", "plus", function() self:show("/new") end, active == "New chat"))
        bar:addChild(C.navItem("Profile", "user-round", function() self:show("/profile") end, active == "Profile"))
        return bar
    end,

    openRoom = function(self, room)
        self.room, self.readRooms[room.id] = room, true
        self:show("/thread")
    end,

    unread = function(self, room)
        return not self.readRooms[room.id] and (room.unread or 0) or 0
    end,

    roomTime = function(self, room)
        if self.remote then return C.time(room.updated_at) end
        local messages = self.client:listMessages(room.id)
        return messages and #messages > 0 and C.time(messages[#messages].created_at) or ""
    end,

    updateSearch = function(self)
        self.query = self.searchInput.Text or ""
        self:renderRoomList()
    end,

    renderRoomList = function(self)
        local list = self.roomList
        while list:getFirstChild() do list:getFirstChild():removeFromParent() end
        for label, button in pairs(self.filterButtons) do
            button.Background = self.filter == label and colors.accent or colors.surface
            button.Foreground = self.filter == label and "white" or colors.muted
        end
        if self.query == "" and self.filter == "All" then
            local pinned = C.stack({ Spacing = 14, MarginBottom = 26 })
            local circle = C.stack({ Direction = "Horizontal", Spacing = 18 })
            for _, room in ipairs(self.rooms) do
                if self.pinned[room.id] then
                    local shortcut = C.button("", function() self:openRoom(room) end, { Width = 68, Height = 84 })
                    local content = C.stack({ Spacing = 9, AlignItems = "Center", IgnoreHitTest = true }, {
                        C.avatar(room.name, 52, room.id),
                        C.text(room.name:match("^%S+") or room.name, {
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
                    Name = "Room" .. index, Height = 88, Background = colors.surface, BorderRadius = "20",
                    class = "conversation", Click = function() self:openRoom(room) end,
                }
                local content = ui.Grid { Columns = "52px 1fr 42px", Padding = "13 14", Spacing = 11, IgnoreHitTest = true }
                content:addChild(C.avatar(room.name, 52, room.id))
                content:addChild(C.stack({ JustifyContent = "Center", Spacing = 6, IgnoreHitTest = true }, {
                    C.text(room.name, { FontSize = 15, FontWeight = "Bold", TextOverflow = "Ellipsis" }),
                    C.text(room.subtitle or "Start the conversation", {
                        FontSize = 12, Foreground = unread > 0 and colors.muted or colors.dim, TextOverflow = "Ellipsis",
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
        if not self.client then
            return self:frame(C.stack({ Padding = "26 56", Spacing = 18 }, {
                C.brand(), C.text("Connect your chat", { FontSize = 30, FontWeight = "Bold" }),
                C.text(self.configurationError.message, { WordWrap = true, Foreground = colors.error }),
                C.text("Set SUPABASE_URL and SUPABASE_PUBLISHABLE_KEY, then restart the app.", { WordWrap = true }),
            }))
        end
        if not self.client:getSession() then return self:auth() end
        local rooms, err = self.client:listRooms()
        self.rooms = rooms or {}
        self.status = C.text(err and err.message or (self.remote and "Refresh to sync your conversations" or "Demo session · make yourself at home"), {
            FontSize = 10, Foreground = err and colors.error or colors.dim, TextHorizontalAlignment = "Center",
        })
        local brand = ui.Grid { Columns = "1fr 36px", Height = 36 }
        brand:addChild(C.brand())
        local profile = C.button("", function() self:show("/profile") end, { Width = 36, Height = 36 })
        profile:addChild(C.avatar("You", 34, "me")); brand:addChild(profile)
        local title = ui.Grid { Columns = "1fr 46px", Height = 52, MarginTop = 18 }
        title:addChild(C.text("Messages", { FontSize = 34, FontWeight = "Bold", LetterSpacing = -0.7 }))
        title:addChild(C.iconButton("plus", function() self:show("/new") end, {
            Name = "NewChat", Background = colors.accent, Foreground = "white", BorderRadius = "16",
        }))
        self.searchInput = C.input("Search", "Search conversations", {
            Text = self.query, Height = 46, Padding = "12 0", Background = "#00000000",
            Submit = function() self:updateSearch() end,
            KeyUp = function() self:updateSearch() end,
        })
        self.searchInput.BorderWidth = "0"
        local search = ui.Grid { Columns = "22px 1fr 28px", Height = 48, Spacing = 4, Padding = "12 0", MarginTop = 12,
            Background = colors.surface, BorderWidth = "1", BorderColor = colors.line, BorderRadius = "16" }
        search:addChild(C.icon("search", 18, colors.dim)); search:addChild(self.searchInput)
        search:addChild(C.iconButton("x", function()
            self.query, self.searchInput.Text = "", ""; self:renderRoomList(); self.searchInput:setFocus()
        end, { Width = 28, Height = 44, Name = "ClearSearch" }))
        local filters = C.stack({ Direction = "Horizontal", Spacing = 8, Height = 34, MarginTop = 14 })
        self.filterButtons = {}
        for _, label in ipairs { "All", "Unread", "Pinned" } do
            local button = C.button(label, function() self.filter = label; self:renderRoomList() end, {
                Width = label == "All" and 66 or 84, Height = 34, FontSize = 12, BorderRadius = "17", Name = "Filter" .. label,
            })
            self.filterButtons[label] = button; filters:addChild(button)
        end
        local header = C.stack({ Padding = "22 22 22 12" }, { brand, title, search, filters })
        self.roomList = C.stack({ Name = "ConversationList", Padding = "22 12 22 20", Spacing = 9, Overflow = "hidden scroll" })
        local footer = C.stack({ Spacing = 8 }, { self.status, self:navigation("Chats") })
        local grid = ui.Grid { Rows = "236px 1fr 108px", Background = colors.background }
        grid:addChild(header); grid:addChild(self.roomList); grid:addChild(footer)
        local screen = self:frame(grid)
        self:renderRoomList()
        return screen
    end,

    renderMessages = function(self)
        local messages, err = self.client:listMessages(self.room.id, { limit = 100 })
        if not messages then self:notice(err.message, true); return end
        local me = self.client:getSession().user.id
        local node = self.messageList
        while node:getFirstChild() do node:getFirstChild():removeFromParent() end
        node:addChild(C.text(#messages > 0 and C.date(messages[1].created_at) or "A FRESH START", {
            FontSize = 10, Foreground = colors.dim, LetterSpacing = 1, TextHorizontalAlignment = "Center", MarginTop = 16, MarginBottom = 20,
        }))
        for _, message in ipairs(messages) do
            local mine = message.sender_id == me
            local bubble = C.stack({
                Name = "MessageBubble" .. message.id, Width = 284, Padding = "15 12", Spacing = 8, BorderRadius = "19",
                Background = mine and colors.sent or colors.raised,
                MarginLeft = mine and auto or 0, MarginRight = mine and 0 or auto,
            })
            if not mine and (self.remote or message.sender_id ~= self.room.id) then
                local sender = message.sender_name or (self.remote and ("Member " .. message.sender_id:sub(1, 8)))
                    or (message.sender_id:gsub("^%l", string.upper))
                bubble:addChild(C.text(sender, { FontSize = 10, FontWeight = "Bold", Foreground = colors.accentText }))
            end
            bubble:addChild(C.text(message.body, { WordWrap = true, FontSize = 14, Foreground = mine and "white" or colors.ink }))
            bubble:addChild(C.text((mine and "Sent · " or "") .. (C.time(message.created_at) or ""), {
                FontSize = 9, Foreground = mine and "#e1d4ff" or colors.dim, TextHorizontalAlignment = "Right",
            }))
            node:addChild(bubble)
        end
        if #messages == 0 then
            node:addChild(C.stack({ Spacing = 12, Padding = "16 32" }, {
                C.text("Every good chat starts\nwith a hello.", { FontSize = 27, FontWeight = "Bold", WordWrap = true }),
                C.text("Send the first message below.", { Foreground = colors.muted, FontSize = 13 }),
            }))
        end
        node:getRoot():UpdateLayout(390, 844)
        node.ContentOffset = { x = 0, y = math.min(0, node.ActualHeight - node.ScrollHeight) }
        self:notice(self.remote and "Synced · refresh for new messages" or "Demo conversation · messages stay in this session")
    end,

    sendMessage = function(self)
        local text = self.composer.Text or ""
        if not text:match("%S") then return end
        local message, err = self.client:sendMessage(self.room.id, text)
        if not message then self:notice(err.message, true); return end
        self.composer.Text, self.drafts[self.room.id] = "", nil
        self:renderMessages()
        self.composer:setFocus()
    end,

    thread = function(self)
        if not self.room then return self:home() end
        local header = ui.Grid {
            Columns = "38px 40px 1fr 38px 38px", Padding = "12 20", Spacing = 8,
            Background = colors.surface, BorderWidthBottom = 1, BorderColor = colors.line,
        }
        header:addChild(C.iconButton("arrow-left", function()
            self.drafts[self.room.id] = self.composer.Text; self:show()
        end, { Width = 38, Name = "BackToChats" }))
        header:addChild(C.avatar(self.room.name, 40, self.room.id))
        header:addChild(C.stack({ Spacing = 5, JustifyContent = "Center" }, {
            C.text(self.room.name, { FontSize = 15, FontWeight = "Bold", TextOverflow = "Ellipsis" }),
            C.text(self.remote and "Conversation" or "Demo conversation", { FontSize = 10, Foreground = colors.dim }),
        }))
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
        local compose = ui.Grid { Columns = "1fr 50px", Spacing = 10 }
        compose:addChild(self.composer)
        compose:addChild(C.iconButton("arrow-up", function() self:sendMessage() end, {
            Name = "SendMessage", Background = colors.accent, Foreground = "white", Width = 50, Height = 50, BorderRadius = "17",
        }))
        local footer = C.stack({ Padding = "20 12 20 22", Spacing = 12, Background = colors.surface,
            BorderWidthTop = 1, BorderColor = colors.line }, { self.status, compose })
        local grid = ui.Grid { Rows = "90px 1fr 112px", Background = colors.background }
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

    newRoom = function(self)
        local name = C.input("RoomName", "e.g. The weekend crew")
        local members = C.input("Members", "Member IDs, separated by commas")
        self.status = C.text("Invite your people with their account IDs. Leave this empty for a personal chat.", {
            WordWrap = true, FontSize = 12, Foreground = colors.muted,
        })
        local body = C.stack({ Padding = "26 20", Spacing = 20, Overflow = "hidden scroll" }, {
            C.iconTile("message-circle", 60, 27),
            C.text("Make space for\na good conversation.", { FontSize = 29, FontWeight = "Bold", WordWrap = true }),
            C.text("A quick catch-up. A big idea. Your next adventure.", { FontSize = 13, Foreground = colors.muted, WordWrap = true }),
            C.stack({ Spacing = 10, MarginTop = 10 }, { C.eyebrow("CONVERSATION NAME"), name }),
            C.stack({ Spacing = 10 }, { C.eyebrow("INVITE MEMBERS"), members, self.status }),
            C.stack({ Spacing = 8, Padding = "16 14", Background = colors.surface, BorderRadius = "16" }, {
                C.eyebrow("YOUR ACCOUNT ID"), C.text(self.client:getSession().user.id, { FontSize = 11, WordWrap = true, Foreground = colors.muted }),
            }),
            C.button("Create conversation", function()
                local ids = {}
                for id in (members.Text or ""):gmatch("[^,%s]+") do ids[#ids + 1] = id end
                if #ids == 0 then ids[1] = self.client:getSession().user.id end
                local room, err = self.client:createRoom(name.Text, ids)
                if not room then self:notice(err.message, true); return end
                self:openRoom(room)
            end, { Name = "CreateConversation", Background = colors.accent, Foreground = "white", Height = 52 }),
        })
        local grid = ui.Grid { Rows = "76px 1fr 86px", Background = colors.background }
        grid:addChild(self:pageHeader("New conversation")); grid:addChild(body); grid:addChild(self:navigation("New chat"))
        return self:frame(grid)
    end,

    profile = function(self)
        if not self.client:getSession() then return self:auth() end
        local user = self.client:getSession().user
        local name = user.user_metadata and user.user_metadata.display_name or "You"
        if name == "" then name = "You" end
        local body = C.stack({ Padding = "24 22", Spacing = 24, Overflow = "hidden scroll" }, {
            C.stack({ AlignItems = "Center", Spacing = 12 }, {
                C.avatar(name, 76, user.id), C.text(name, {
                    Width = 340, FontSize = 27, FontWeight = "Bold", WordWrap = true, TextHorizontalAlignment = "Center",
                }),
                C.text(self.remote and user.email or "Your local demo profile", {
                    Width = 320, FontSize = 12, Foreground = colors.muted, TextOverflow = "Ellipsis", TextHorizontalAlignment = "Center",
                }),
                C.badge(self.remote and "CONNECTED" or "DEMO SESSION", {
                    Width = 120, Height = 26, Background = colors.soft, Foreground = colors.accentText,
                }),
            }),
        })
        for _, item in ipairs {
            { "settings-2", "Appearance", "Midnight violet" },
            { "refresh-cw", "New messages", "Refresh to sync" },
            { "pin", "Pinned conversations", "Saved for this session" },
        } do
            local row = ui.Grid { Columns = "36px 1fr", Height = 70, Padding = "14 14", Spacing = 12,
                Background = colors.surface, BorderRadius = "18" }
            row:addChild(C.icon(item[1], 20, colors.accentText))
            row:addChild(C.stack({ Spacing = 5 }, {
                C.text(item[2], { FontSize = 13, FontWeight = "Bold" }),
                C.text(item[3], { FontSize = 11, Foreground = colors.muted }),
            }))
            body:addChild(row)
        end
        body:addChild(C.button(self.remote and "Sign out" or "Start a fresh demo", function()
            if self.remote then
                local ok, err = self.client:signOut(); self:show()
                if not ok then self:notice(err.message, true) end
            else
                self.client = Chat.demo(); self.drafts, self.readRooms, self.query, self.filter = {}, {}, "", "All"
                self.pinned = { alex = true, weekend = true }
                self:show()
            end
        end, { Foreground = colors.error, Background = colors.surface, Height = 48 }))
        local grid = ui.Grid { Rows = "76px 1fr 86px", Background = colors.background }
        grid:addChild(self:pageHeader("Your profile")); grid:addChild(body); grid:addChild(self:navigation("Profile"))
        return self:frame(grid)
    end,

    auth = function(self)
        local email = C.input("Email", "you@example.com")
        local password = C.input("Password", "Your password", { Type = "Password" })
        local displayName = C.input("DisplayName", "Your name, for a new account")
        self.status = C.text("Your people are just a message away.", { WordWrap = true, FontSize = 13, Foreground = colors.muted })
        local function submit(signup)
            local session, err
            if signup then session, err = self.client:signUp(email.Text, password.Text, displayName.Text)
            else session, err = self.client:signIn(email.Text, password.Text) end
            if not session then self:notice(err.message, true); return end
            if not self.client:getSession() then self:notice("Check your email to confirm your account, then sign in."); return end
            self:show()
        end
        local emblem = C.iconTile("message-circle", 68, 30, 22)
        emblem.MarginTop = 26
        return self:frame(C.stack({ Padding = "28 38", Spacing = 22, Overflow = "hidden scroll", Background = colors.background }, {
            C.brand(),
            emblem,
            C.text("Good to\nsee you again.", { FontSize = 35, FontWeight = "Bold", WordWrap = true }),
            self.status,
            C.stack({ Spacing = 10, MarginTop = 8 }, { C.eyebrow("EMAIL ADDRESS"), email }),
            C.stack({ Spacing = 10 }, { C.eyebrow("PASSWORD"), password }),
            C.button("Sign in", function() submit(false) end, { Background = colors.accent, Foreground = "white", Height = 52 }),
            C.stack({ Spacing = 12, Padding = "0 16", BorderWidthTop = 1, BorderColor = colors.line }, {
                C.eyebrow("NEW HERE? MAKE YOURSELF AT HOME."), displayName,
                C.button("Create an account", function() submit(true) end, { Background = colors.soft }),
            }),
        }))
    end,

    ["/"] = function(self) return self:home() end,
    ["/thread"] = function(self) return self:thread() end,
    ["/new"] = function(self) return self:newRoom() end,
    ["/profile"] = function(self) return self:profile() end,

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
