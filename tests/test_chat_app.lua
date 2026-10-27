local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
-- Avatar textures need a current graphics context on macOS as well as real font metrics.
require("orca.renderer").init(390, 844, true)
filesystem.init("apps/chat")
require("orca.core.startup").load_plugins()

local App = require "Chat/App"
local Chat = require "api.chat"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
-- This suite verifies application state and explicit layout. Keep native fonts
-- and avatar textures, but leave GPU painting to the rendering regressions.
local function flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then return end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(nil, message) end
    end
end
local function drain(app)
    flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(app.screen, message) end
    end
    flushQueue()
    if app.screen then
        app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
        app:afterPaint()
        flushQueue()
    end
end
local function mockApp(media, client)
    local app = App { client = client or Chat.mock { clock = function() return 1700000000 end }, media = media }
    -- Keep navigation real while the fixture controls layout and painting.
    app.show = function(self, route) self.screen = self:dispatch(route or "/").view end
    app:show("/")
    local login = app.screen
    assert(not app.client:getSession() and login:findChild("RequestCode", true), "Mock mode must start at real login")
    login:findChild("UsePassword", true):send("Node.LeftButtonUp")
    drain(app)
    login = app.screen
    login:findChild("Email", true).Text = "demo@example.invalid"
    login:findChild("Password", true).Text = "incorrect"
    login:findChild("SignIn", true):send("Node.LeftButtonUp")
    drain(app)
    assert(not app.client:getSession() and app.screen == login, "Wrong mock credentials must stay on login")
    login:findChild("Password", true).Text = "password123"
    login:findChild("SignIn", true):send("Node.LeftButtonUp")
    drain(app)
    assert(app.client:getSession() and app.screen:findChild("Room1", true),
        "Mock login must reach API-backed conversations: " .. tostring(app.client:getSession()) .. " / " .. app.status:getTextContent())
    return app
end
local app = mockApp()
local home = app.screen
assert(home.Width == 390 and home.Height == 844)
assert(home.ResizeMode == "NoResize")
assert(home:findChild("Room1", true), "Conversation list must be rendered")
home:UpdateLayout(390, 844)
assert(home:getFirstChild().ActualHeight >= 843, "App content must fill the portrait viewport")
local firstRoom = home:findChild("Room1", true)
local firstAvatar = assert(firstRoom:findChild("AvatarPhoto", true), "Bundled demo portraits must appear in chat rows")
assert(firstAvatar.Source and firstAvatar.Source.Width > 100, "Chat portraits must resolve real image resources")
assert(firstRoom.ActualWidth > 300 and firstRoom.ActualHeight > 60, "Conversation rows need usable hit areas")
assert(home.StyleSheet, "The shared interactive stylesheet must load")
local savedPlatform = PLATFORM
PLATFORM = "ios"
local mobile = app:frame(require("orca.UIKit").Grid { Rows = "1fr" })
assert(mobile.ResizeMode == "CanResize", "iOS screens must track the drawable size")
mobile:UpdateLayout(834, 1100)
assert(mobile.Width == 834 and mobile.Height == 1100, "iPad layout must use available points")
mobile.KeyboardInsetBottom = 384
mobile:UpdateLayout(390, 844)
assert(mobile.Width == 390 and mobile.Height == 844 and mobile:getFirstChild().ActualHeight == 460,
    "Keyboard avoidance must reserve content space while preserving the physical viewport")
PLATFORM = savedPlatform
assert(home:findChild("Search", true) and home:findChild("FilterUnread", true))
app.searchInput.Text = "Jules"
app:updateSearch()
assert(not home:findChild("Room1", true) and home:findChild("Room3", true), "Search must filter conversations")
app.searchInput.Text = ""
app:updateSearch()
app.filter = "Unread"
app:renderRoomList()
assert(app.roomLists.Unread:findChild("Room1", true) and not app.roomLists.Unread:findChild("Room2", true), "Unread filter must use unread counts")
app.readRooms.alex = true
app:renderRoomList()
assert(not app.roomLists.Unread:findChild("Room1", true), "Opening a chat must clear its local unread state")
app.filter = "Pinned"
app:renderRoomList()
assert(app.roomLists.Pinned:findChild("Room1", true) and app.roomLists.Pinned:findChild("Room2", true) and not app.roomLists.Pinned:findChild("Room3", true))
app.filter = "All"
app:renderRoomList()

-- Pivot selection must reuse prepared pages, without rebuilding any rows.
drain(app)
home:UpdateLayout(390, 844)
local retained = {}
for filter, list in pairs(app.roomLists) do retained[filter] = list:getFirstChild() end
assert(app.roomLists.All:findChild("Room3", true), "All conversations must stay prepared offscreen")
assert(app.roomLists.Pinned:findChild("Room1", true), "Pinned conversations must be prepared before swiping")
local renders, renderRoomList = 0, app.renderRoomList
app.renderRoomList = function(self) renders = renders + 1; return renderRoomList(self) end
for _, index in ipairs { 1, 2, 0 } do
    app.pivot.SelectedIndex = index
    drain(app)
    home:UpdateLayout(390, 844)
    assert(app.filter == ({ "All", "Unread", "Pinned" })[index + 1])
    for filter, list in pairs(app.roomLists) do
        assert(list:getFirstChild() == retained[filter], "Swiping must retain rows in " .. filter)
    end
end
app:updateSearch()
assert(renders == 0, "Selection and unchanged search input must not rebuild conversation rows")
app.renderRoomList = renderRoomList

app.room = app.client:listRooms()[1]
local screen = app:dispatch("/thread").view
app.screen = screen
assert(screen:findChild("MessageBubblea1", true), "Render history before the first thread paint")
flushQueue()
screen:UpdateLayout(390, 844)
assert(screen:findChild("Message", true), "Thread must have a composer")
assert(app.composer.ActualHeight >= 44 and app.composer.ActualWidth > 200,
    string.format("Composer dimensions: %.1f x %.1f", app.composer.ActualWidth, app.composer.ActualHeight))
local composerStrip = screen:findChild("Composer", true)
local footer = screen:findChild("ThreadFooter", true)
assert(math.abs(composerStrip.ActualWidth - 390) < 1 and math.abs(composerStrip.ActualX) < 1,
    "The rectangular composer must span the full screen width")
assert(math.abs(footer.ActualY + composerStrip.ActualY + composerStrip.ActualHeight - 844) < 1,
    "The composer must sit at the bottom without an application bar or padding beneath it")
assert(not screen:findChild("AppBar", true), "The conversation must not keep a second bottom command bar")
for _, name in ipairs { "AttachPhoto", "SendMessage" } do
    local button = screen:findChild(name, true)
    assert(button:getParent() == composerStrip and button.ActualWidth >= 44 and button.ActualHeight >= 44,
        name .. " must be a usable target inside the composer strip")
end
for _, name in ipairs { "BackToChats", "ConversationMenu" } do
    local button = screen:findChild(name, true)
    assert(button.Visible and button:getParent().Name == "ThreadCommands" and button.ActualHeight >= 44,
        name .. " must remain accessible above the messages")
end
local overflow = screen:findChild("ThreadOverflow", true)
assert(not overflow.Visible, "Conversation commands must stay hidden behind the ellipsis")
screen:findChild("ConversationMenu", true):send("Node.LeftButtonUp")
drain(app)
assert(overflow.Visible, "The ellipsis must open the conversation menu")
screen:UpdateLayout(390, 844)
for _, name in ipairs { "PinConversation", "ConversationInfo", "RefreshMessages" } do
    local item = screen:findChild(name, true)
    assert(item:getParent().Name == "ThreadOverflowMenu" and item.ActualHeight >= 44,
        name .. " must be accessible in the overflow menu")
end
screen:findChild("DismissConversationMenu", true):send("Node.LeftButtonUp")
drain(app)
assert(not overflow.Visible, "Tapping outside must dismiss the conversation menu")
screen:findChild("ConversationMenu", true):send("Node.LeftButtonUp")
drain(app)
screen:findChild("RefreshMessages", true):send("Node.LeftButtonUp")
drain(app)
assert(not overflow.Visible, "Choosing a menu command must dismiss the menu")
screen.ResizeMode = "NoResize"
screen.Width, screen.Height = 390, 844
screen:UpdateLayout(390, 844)
assert(not screen.RespectSafeArea, "The thread must paint behind the home gesture area")
screen.SafeAreaTop, screen.SafeAreaBottom = 59, 34
screen.SafeAreaLeft, screen.SafeAreaRight = 12, 18
flushQueue()
core.advanceFrame()
core.runAllPrograms(screen)
screen:UpdateLayout(390, 844)
local threadRoot = screen:getFirstChild()
assert(threadRoot.PaddingTop == 59 and threadRoot:getFirstChild().PaddingLeft == 12
    and threadRoot:getFirstChild().PaddingRight == 18,
    "Thread content must follow the system's safe top and side insets")
assert(footer.PaddingBottom == 34 and math.abs(composerStrip.ActualWidth - 360) < 1,
    "The composer must leave safe room for the home gesture and landscape edges")
assert(math.abs(footer.ActualY + composerStrip.ActualY + composerStrip.ActualHeight - (844 - 34)) < 1,
    "Composer controls must end immediately above the home gesture area")
assert(math.abs(footer.ActualY + footer.ActualHeight - 844) < 1 and
    math.abs(footer.BackgroundColor.R - composerStrip.BackgroundColor.R) < 0.001,
    "The composer background must continue to the physical bottom edge")
for _, inset in ipairs { 384, 0 } do
    screen.KeyboardInsetBottom = inset
    core.advanceFrame()
    core.runAllPrograms(screen)
    screen:UpdateLayout(390, 844)
    local bottom = math.max(34, inset)
    assert(screen.Height == 844 and footer.PaddingBottom == bottom,
        "Keyboard show and hide must preserve the viewport and restore the home gesture inset")
    assert(math.abs(footer.ActualY + composerStrip.ActualY + composerStrip.ActualHeight - (844 - bottom)) < 1,
        "Composer controls must stay immediately above the keyboard or home gesture area")
    assert(math.abs(footer.ActualY + footer.ActualHeight - 844) < 1,
        "The composer background must extend under the keyboard to the physical bottom")
end
screen.SafeAreaTop, screen.SafeAreaBottom = 0, 0
screen.SafeAreaLeft, screen.SafeAreaRight = 0, 0
flushQueue()
core.advanceFrame()
core.runAllPrograms(screen)
screen:UpdateLayout(390, 844)
assert(footer.PaddingBottom == 0 and threadRoot.PaddingTop == 0,
    "Safe-area bindings must update when the keyboard or orientation changes")

-- Compact bubbles keep the body verbatim and let layout reserve metadata space.
local layoutMessages = {}
for _, body in ipairs { "Hi", ("A longer message that needs to wrap. "):rep(8),
    "First line\nSecond line\n", "Привет · 你好 · café", ("unbroken"):rep(40) } do
    layoutMessages[#layoutMessages + 1] = assert(app.client:sendMessage(app.room.id, body))
end
assert(app:renderMessages())
for _, width in ipairs { 320, 390, 834 } do
    for _, fontSize in ipairs { 16, 24 } do
        screen.Width = width
        for _, message in ipairs(layoutMessages) do
            local bubble = assert(screen:findChild("MessageBubble" .. message.id, true))
            local body = bubble:getFirstChild()
            assert(body:getTextContent() == message.body, "Layout must not append padding characters to message text")
            body.FontSize = fontSize
            -- Metadata must remain separate even if its font metrics change.
            for child in bubble.children do
                if child ~= body then child.FontSize = fontSize - 3 end
            end
        end
        screen:UpdateLayout(width, 844)
        for _, message in ipairs(layoutMessages) do
            local bubble = screen:findChild("MessageBubble" .. message.id, true)
            local holder, body, time = bubble:getParent(), bubble:getFirstChild()
            local entry, x = holder:getParent(), holder.ActualX + bubble.ActualX
            for child in bubble.children do time = child end
            assert(x >= entry.PaddingLeft - 1 and x + bubble.ActualWidth <= entry.ActualWidth - entry.PaddingRight + 1,
                string.format("Message bubble %s must fit its gutter at %d/%d: x=%.1f width=%.1f entry=%.1f",
                    message.id, width, fontSize, x, bubble.ActualWidth, entry.ActualWidth))
            local actions = assert(holder:findChild("MessageActions" .. message.id), "Own messages offer actions")
            assert(math.abs(actions.ActualWidth - bubble.ActualWidth) < 1 and math.abs(actions.ActualHeight - bubble.ActualHeight) < 1,
                "The action target must cover exactly its bubble")
            assert(time.ActualY >= body.ActualY + body.ActualHeight,
                "Timestamp must not overlap the message at any tested width or font size")
            assert(time.ActualY + time.ActualHeight <= bubble.ActualHeight - bubble.PaddingBottom + 1,
                "Bubble height must include the timestamp")
        end
    end
end
screen.Width = 390
screen:UpdateLayout(390, 844)
assert(app:renderMessages())

-- Appending to a sender run moves its tail without moving an earlier reading anchor.
local reading = screen:findChild("MessageEntry" .. layoutMessages[2].id, true)
app.messageList.ContentOffset = { X = 0, Y = -reading.ActualY - 5 }
local readingY = reading.ActualY + app.messageList.ContentOffset.Y
assert(app.client:sendMessage(app.room.id, "Another message from the same sender"))
assert(app:renderMessages { preserveScroll = true, quiet = true })
reading = screen:findChild("MessageEntry" .. layoutMessages[2].id, true)
assert(math.abs(reading.ActualY + app.messageList.ContentOffset.Y - readingY) < 1,
    "Moving a grouped sender's tail must preserve the visible message anchor")

local incoming = app.client:listMessages(app.room.id)[1]
local savedContact, savedKind = app.contactsById[incoming.sender_id], app.room.kind
app.contactsById[incoming.sender_id] = { get_display_name = function() return ("LongName"):rep(20) end }
app.room.kind, screen.Width = "group", 320
assert(app:renderMessages())
local namedBubble = screen:findChild("MessageBubble" .. incoming.id, true)
assert(namedBubble.ActualWidth <= namedBubble:getParent():getParent().ActualWidth - 56 + 1,
    "An unbroken sender name must wrap inside a narrow group-chat bubble")
app.contactsById[incoming.sender_id], app.room.kind = savedContact, savedKind
screen.Width = 390
assert(app:renderMessages())

local before = #app.client:listMessages(app.room.id)
app.composer.Text = "A quick hello from the UI test"
app:sendMessage()
flushQueue()
local messages = app.client:listMessages(app.room.id)
assert(#messages == before + 1)
assert(messages[#messages].body == "A quick hello from the UI test")
assert(app.composer.Text == "", "Clear composer only after successful send")

app.composer.Text = "   "
app:sendMessage()
assert(#app.client:listMessages(app.room.id) == before + 1, "Do not send whitespace")
app.composer.Text = string.rep("x", 4001)
app:sendMessage()
assert(#app.composer.Text == 4001, "An API-rejected send must retain input")

local newRoom = app:dispatch("/new").view
assert(newRoom:findChild("RoomName", true))
assert(newRoom:findChild("FindPerson", true), "New conversations must offer privacy-aware discovery")
assert(newRoom:findChild("Contactalex", true), "Contacts must come from the API")
newRoom:findChild("Contactalex", true):send("Node.LeftButtonUp")
drain(app)
assert(app.room.kind == "direct" and app.room.peer_id == "alex", "Tapping a contact opens its direct chat immediately")
app:show("/new")
newRoom:findChild("KindGroup", true):send("Node.LeftButtonUp")
drain(app)
newRoom:findChild("Contactalex", true):send("Node.LeftButtonUp")
drain(app)
assert(app.newSelection.alex, "Group member selection must use the API ID")
newRoom:findChild("Contactalex", true):send("Node.LeftButtonUp")
drain(app)
assert(not app.newSelection.alex, "Selecting a member again removes it")
local profile = app:dispatch("/profile").view
profile:UpdateLayout(390, 844)
assert(profile:findChild("AppBarChats", true), "Profile must offer a return to chats")

-- Root pages keep one shell and one ApplicationBar through real Click events.
local tabApp = mockApp()
local shell = tabApp.screen
local bar = assert(shell:findChild("AppBar", true), "Root screens need one shared application bar")
local tabs = {
    { name = "AppBarChats", value = "Chats" },
    { name = "AppBarNew", value = "New" },
    { name = "AppBarProfile", value = "Profile" },
}
for _, tab in ipairs(tabs) do tab.view = assert(bar:findChild(tab.name, true), "Missing button " .. tab.name) end

local function namedCount(node, name)
    local count = node.Name == name and 1 or 0
    for child in node.children do count = count + namedCount(child, name) end
    return count
end

local function isShown(node)
    while node do
        if node.Visible == false then return false end
        node = node:getParent()
    end
    return true
end

local function assertTabs(selected)
    flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(shell, message) end
    end
    flushQueue()
    core.advanceFrame()
    core.runAllPrograms(shell)
    shell:UpdateLayout(390, 844)
    assert(tabApp.screen == shell, "Page switches must keep the same screen")
    assert(shell.Name == "ChatShell" and namedCount(shell, "ChatShell") == 1,
        "Page switches must not nest or duplicate the application shell")
    assert(namedCount(shell, "AppBar") == 1 and shell:findChild("AppBar", true) == bar,
        "Page switches must retain one application bar")
    assert(tabApp.shell.active == selected, "The shell must show the requested page")
    for _, tab in ipairs(tabs) do
        local view = tab.view
        assert(bar:findChild(tab.name, true) == view, "Bar buttons must survive navigation")
        assert(view.IsEnabled == (tab.value ~= selected), "Only the current page's button is disabled")
        assert(isShown(view), "Every bar button must remain visible")
        assert(view.ActualWidth >= 44 and view.ActualHeight >= 44, "Bar buttons need usable hit areas")
        assert(type(view.IconUri) == "string" and view.IconUri ~= "", "Each bar button needs an SVG icon")
        assert(type(view.Text) == "string" and view.Text:match("%S"), "Each bar button needs a label")
        local page = shell:findChild("TabContent", true):findChild(tab.value)
        if tab.value == selected then assert(page, "The selected page must exist") end
        if page then
            assert(isShown(page) == (tab.value == selected), string.format(
                "Selection must display only its own page: selected=%s page=%s visible=%s active=%s",
                selected, tab.value, tostring(page.Visible), tostring(shell:findChild("TabContent", true).ActivePage)))
        end
    end
end

-- The current page's button is disabled, so selecting it again is a no-op.
local function selectTab(index)
    tabs[index].view:send("Node.LeftButtonUp")
    assertTabs(tabs[index].value)
end

assertTabs("Chats")
-- The chrome fills the home-indicator inset without moving controls into it.
local barFooter = assert(shell:findChild("AppBarFooter", true))
local shellRoot = shell:getFirstChild()
assert(not shell.RespectSafeArea, "The shell must paint behind the home gesture area")
shell.SafeAreaTop, shell.SafeAreaBottom = 59, 34
shell.SafeAreaLeft, shell.SafeAreaRight = 12, 18
assertTabs("Chats")
assert(shellRoot.PaddingTop == 59 and shellRoot:getFirstChild().PaddingLeft == 12
    and shellRoot:getFirstChild().PaddingRight == 18,
    "Shell content must follow the system's safe top and side insets")
assert(barFooter.PaddingBottom == 34 and math.abs(bar.ActualHeight - 58) < 1,
    "The home gesture inset must be separate from the fixed application bar row")
assert(math.abs(barFooter.ActualY + bar.ActualY + bar.ActualHeight - (844 - 34)) < 1,
    string.format("Application bar controls must end immediately above the home gesture area: " ..
        "shell=%.1fx%.1f root=(%.1f,%.1f %.1fx%.1f) footer=(%.1f,%.1f %.1fx%.1f) bar=(%.1f,%.1f %.1fx%.1f) inset=%.1f",
        shell.Width, shell.Height, shellRoot.ActualX, shellRoot.ActualY, shellRoot.ActualWidth, shellRoot.ActualHeight,
        barFooter.ActualX, barFooter.ActualY, barFooter.ActualWidth, barFooter.ActualHeight,
        bar.ActualX, bar.ActualY, bar.ActualWidth, bar.ActualHeight, barFooter.PaddingBottom))
assert(math.abs(barFooter.ActualY + barFooter.ActualHeight - 844) < 1 and
    math.abs(barFooter.BackgroundColor.R - bar.BackgroundColor.R) < 0.001 and
    math.abs(barFooter.BackgroundColor.G - bar.BackgroundColor.G) < 0.001 and
    math.abs(barFooter.BackgroundColor.B - bar.BackgroundColor.B) < 0.001,
    "The application bar background must continue to the physical bottom edge")
shell.SafeAreaTop, shell.SafeAreaBottom = 0, 0
shell.SafeAreaLeft, shell.SafeAreaRight = 0, 0
assertTabs("Chats")
assert(barFooter.PaddingBottom == 0 and shellRoot.PaddingTop == 0 and
    math.abs(barFooter.ActualHeight - 58) < 1,
    "Application bar safe-area bindings must update without leaving a bottom gap")
selectTab(2)
local roomName = assert(shell:findChild("RoomName", true))
local finder = assert(shell:findChild("FindPerson", true))
roomName.Text, finder.Text = "Keep our conversation draft", "@alex"
for _, index in ipairs({ 3, 1, 2, 3, 2, 1, 1, 3, 2 }) do selectTab(index) end
for _, tab in ipairs(tabs) do
    assert(shell:findChild("TabContent", true):findChild(tab.value), "Every visited page must remain retained")
end
assert(shell:findChild("RoomName", true) == roomName and shell:findChild("FindPerson", true) == finder,
    "Switching pages must retain the new-conversation form")
assert(roomName.Text == "Keep our conversation draft" and finder.Text == "@alex",
    "Switching pages must preserve unsent new-conversation fields")
for _, width in ipairs({ 320, 834 }) do
    shell.Width = width
    core.advanceFrame()
    core.runAllPrograms(shell)
    shell:UpdateLayout(width, 844)
    local previousRight = -1
    for _, tab in ipairs(tabs) do
        local view = tab.view
        assert(view.ActualX >= previousRight - 1 and view.ActualX + view.ActualWidth <= width + 1,
            "Bar buttons must remain ordered and fit inside narrow and tablet viewports")
        previousRight = view.ActualX + view.ActualWidth
    end
end
shell.Width = 390
core.advanceFrame()
core.runAllPrograms(shell)
assertTabs("New")
app.screen = profile
profile:findChild("SignOut", true):send("Node.LeftButtonUp")
drain(app)
local auth = app.screen
assert(not app.client:getSession() and app.shell == nil, "Signing out must discard protected views")
auth:findChild("UsePassword", true):send("Node.LeftButtonUp")
drain(app)
auth = app.screen
assert(auth:findChild("Email", true) and auth:findChild("Password", true))
assert(auth:findChild("Email", true).Type == "Email", "Email sign-in must request the email keyboard")
assert(auth:findChild("Password", true).Type == "Password")

-- Phone/email verification uses the same client and in-process mock protocol.
local otpApp = App { client = Chat.mock { clock = function() return 1700000000 end } }
otpApp.show = function(self, route) self.screen = self:dispatch(route or "/").view end
otpApp:show()
local function click(app, name)
    assert(app.screen:findChild(name, true), "Missing action " .. name):send("Node.LeftButtonUp")
    drain(app)
end
-- Authentication methods are distinct pages; switching preserves useful drafts
-- and never mounts a second, hidden form or retains a password.
local authApp = App { client = Chat.mock() }
authApp.show = function(self, route) self.screen = self:dispatch(route or "/").view end
authApp:show()
local phonePage = authApp.screen
assert(phonePage:findChild("PhonePage", true) and not phonePage:findChild("Email", true),
    "Phone entry must contain only the phone form")
assert(phonePage:findChild("Identifier", true).Type == "Phone", "Phone entry must request an international phone keypad")
assert(namedCount(phonePage, "AuthLogo") == 1, "Authentication needs exactly one logo")
local wordmark = phonePage:findChild("AuthWordmark", true)
assert(wordmark:getTextContent() == "ORCA / CHAT" and wordmark.FontSize == 14,
    "The auth wordmark must use the compact Metro capitals")
phonePage:UpdateLayout(390, 844)
local logo = phonePage:findChild("AuthLogo", true)
assert(math.abs(wordmark.ActualY + wordmark.ActualHeight / 2 - logo.ActualY - logo.ActualHeight / 2) < 0.5,
    "Wordmark and logo must share the same vertical center")
phonePage:findChild("Identifier", true).Text = "+4915110000001"
click(authApp, "UsePassword")
assert(authApp.screen ~= phonePage and authApp.screen:findChild("EmailPage", true),
    "Use email must navigate to a separate page")
assert(not authApp.screen:findChild("Identifier", true) and not authApp.screen:findChild("DisplayName", true),
    "Email sign-in must not append phone or registration fields")
authApp.screen:findChild("Email", true).Text = "demo@example.invalid"
authApp.screen:findChild("Password", true).Text = "do not retain this"
click(authApp, "CreateAccount")
assert(authApp.screen:findChild("SignUpPage", true) and not authApp.screen:findChild("SignIn", true),
    "Create account needs its own page and primary action")
assert(authApp.screen:findChild("Email", true).Text == "demo@example.invalid")
assert(authApp.screen:findChild("Email", true).Type == "Email", "Signup must also request the email keyboard")
assert(authApp.screen:findChild("Password", true).Text == "", "Passwords must not carry between pages")
authApp.screen:findChild("DisplayName", true).Text = "New friend"
click(authApp, "UsePhone")
assert(authApp.screen:findChild("Identifier", true).Text == "+4915110000001", "Returning to phone must preserve the number")
click(authApp, "CreateAccount")
assert(authApp.screen:findChild("DisplayName", true).Text == "New friend", "Returning to signup must preserve the name")
click(authApp, "BackToSignIn")
local emailPage = authApp.screen
local signIn = emailPage:findChild("SignIn", true)
for _, size in ipairs({ {320, 568}, {390, 844}, {390, 460}, {834, 1100}, {390, 460} }) do
    emailPage.Width, emailPage.Height = size[1], size[2]
    flushQueue()
    core.advanceFrame()
    core.runAllPrograms(emailPage)
    emailPage:UpdateLayout(size[1], size[2])
    local email, password = emailPage:findChild("Email", true), emailPage:findChild("Password", true)
    for _, control in ipairs({ email, password, signIn }) do
        assert(control.Visible and control.ActualHeight + control.BorderWidthTop + control.BorderWidthBottom >= 50
            and control.ActualWidth >= size[1] - 64,
            "Auth fields and Sign in must retain full hit areas across resize/keyboard changes")
    end
    assert(signIn.ActualY >= password:getParent().ActualY + password:getParent().ActualHeight,
        "Sign in must remain below the password field")
end
-- A failed request leaves the same page and primary action in place.
emailPage:findChild("Password", true).Text = "incorrect"
click(authApp, "SignIn")
assert(authApp.screen == emailPage and signIn.Visible and signIn:getTextContent() == "Sign in")
assert(emailPage:findChild("Email", true).Text == "demo@example.invalid")
click(authApp, "CreateAccount")
authApp.screen:findChild("Email", true).Text = "signup_ui@example.invalid"
authApp.screen:findChild("Password", true).Text = "password123"
click(authApp, "SignUp")
assert(authApp.client:getSession() and authApp.screen:findChild("ConversationListAll", true), "Signup must reach the real mock service: " .. authApp.status:getTextContent())
assert(authApp.authDraft == nil, "Successful authentication must clear form drafts")

otpApp.screen:findChild("Identifier", true).Text = "+4915110000001"
click(otpApp, "RequestCode")
assert(otpApp.screen:findChild("CodeStep", true).Visible, "Requesting a code must show the verification step")
assert(otpApp.screen:findChild("VerificationCode", true).Type == "Number", "Verification codes need a digits keyboard")
otpApp.screen:findChild("VerificationCode", true).Text = "000000"
assert(otpApp.screen:findChild("VerificationCode", true).Text == "000000", "Numeric entry must preserve leading zeroes")
click(otpApp, "VerifyCode")
assert(not otpApp.client:getSession(), "A wrong code must not log in")
otpApp.screen:findChild("VerificationCode", true).Text = "123456"
click(otpApp, "VerifyCode")
assert(otpApp.client:getSession() and otpApp.screen:findChild("Room1", true), "Verified OTP must reach the mock inbox")
otpApp:show("/profile")
otpApp.screen:findChild("Username", true).Text = "test_person"
click(otpApp, "DiscoverableEmail")
click(otpApp, "SaveProfile")
assert(otpApp.client:getProfile().username == "test_person", "Save profile must use the API")
otpApp.screen:findChild("LinkIdentifier", true).Text = "+447700900987"
click(otpApp, "RequestLink")
assert(otpApp.screen:findChild("LinkVerification", true).Visible, "Linking must require verification")
assert(otpApp.screen:findChild("LinkCode", true).Type == "Number", "Linked identities need the same digits keyboard")
otpApp.screen:findChild("LinkCode", true).Text = "123456"
click(otpApp, "VerifyLink")
assert(otpApp.client:getProfile().phone == "+447700900987", "Verified phone must attach to the existing account")
otpApp:show("/new")
otpApp.screen:findChild("FindPerson", true).Text = "@alex"
click(otpApp, "FindPeople")
assert(otpApp.screen:findChild("FoundPersonalex", true), "Exact username discovery must render API results")
click(otpApp, "FoundPersonalex")
assert(otpApp.room.kind == "direct", "The direct action must open a canonical direct chat")
otpApp:show("/new")
click(otpApp, "KindGroup")
otpApp.screen:findChild("RoomName", true).Text = "Our group"
click(otpApp, "Contactalex")
click(otpApp, "GroupNext")
click(otpApp, "CreateConversation")
assert(otpApp.room.kind == "group" and otpApp.room.can_post, "Group creation must return posting membership")
click(otpApp, "ConversationMenu")
click(otpApp, "ConversationInfo")
click(otpApp, "CreateInvite")
assert(otpApp.screen:findChild("Invitation", true):getTextContent():find("Invitation:", 1, true), "Admins must be able to create invitations")
click(otpApp, "RevokeInvite")
assert(otpApp.screen:findChild("Invitation", true):getTextContent() == "Invitation revoked.")
otpApp:show("/new")
click(otpApp, "KindChannel")
otpApp.screen:findChild("RoomName", true).Text = "Our announcements"
click(otpApp, "CreateConversation")
assert(otpApp.room.kind == "channel" and otpApp.room.can_post, "Channel owners can publish")
assert(otpApp.screen:findChild("Composer", true).Visible, "Channel owners need the composer")
local channelId = otpApp.room.id
local channelInvite = assert(otpApp.client:createInvite(channelId)).code
otpApp:show("/profile")
click(otpApp, "SignOut")
click(otpApp, "UsePassword")
otpApp.screen:findChild("Email", true).Text = "alex@example.invalid"
otpApp.screen:findChild("Password", true).Text = "password123"
click(otpApp, "SignIn")
assert(otpApp.client:joinInvite(channelInvite))
local subscriberRoom
for _, room in ipairs(otpApp.client:listRooms()) do if room.id == channelId then subscriberRoom = room end end
assert(subscriberRoom and subscriberRoom.can_post == false, "The mock API must return subscriber publishing restrictions")
otpApp:openRoom(subscriberRoom)
assert(not otpApp.screen:findChild("Composer", true).Visible and otpApp.screen:findChild("ChannelReadOnly", true).Visible,
    "Channel subscribers must see a read-only timeline")
assert(not isShown(otpApp.screen:findChild("AttachPhoto", true)), "Subscribers must not be offered media publishing")
assert(not isShown(otpApp.screen:findChild("SendMessage", true)), "Subscribers must not be offered message publishing")
for _, name in ipairs { "BackToChats", "ConversationMenu" } do
    assert(isShown(otpApp.screen:findChild(name, true)), "Read-only subscribers still need " .. name)
end
assert(not isShown(otpApp.screen:findChild("ConversationInfo", true)), "Secondary commands start hidden for subscribers too")
click(otpApp, "ConversationMenu")
for _, name in ipairs { "PinConversation", "ConversationInfo", "RefreshMessages" } do
    assert(isShown(otpApp.screen:findChild(name, true)), "Subscribers must retain overflow action " .. name)
end
click(otpApp, "DismissConversationMenu")

-- Public channel discovery works for a new account without preassigned membership.
otpApp:show("/new")
click(otpApp, "KindChannel")
click(otpApp, "ConversationVisibility")
otpApp.screen:findChild("RoomName", true).Text = "Open newsroom"
click(otpApp, "CreateConversation")
assert(otpApp.room.visibility == "public", "The public channel option must reach the API")
local publicChannelId = otpApp.room.id
otpApp:show("/profile"); click(otpApp, "SignOut")
click(otpApp, "UsePassword")
otpApp.screen:findChild("Email", true).Text = "new_reader@example.invalid"
click(otpApp, "SendEmailCode")
otpApp.screen:findChild("VerificationCode", true).Text = "123456"
click(otpApp, "VerifyCode")
assert(otpApp.client:getSession().user.email == "new_reader@example.invalid", "OTP must register a fresh reader account")
otpApp:show("/new")
click(otpApp, "SearchChannels")
assert(otpApp.screen:findChild("FindChannels", true).Visible and not otpApp.screen:findChild("FindPeople", true).Visible,
    "Channel discovery must have its own visible search action")
otpApp.screen:findChild("FindPerson", true).Text = "Our announcements"
click(otpApp, "FindChannels")
assert(not otpApp.screen:findChild("JoinChannel" .. channelId, true), "Private channels must not appear in discovery")
otpApp.screen:findChild("FindPerson", true).Text = "Open news"
click(otpApp, "FindChannels")
assert(otpApp.screen:findChild("JoinChannel" .. publicChannelId, true), "Public channel search must return API results")
click(otpApp, "JoinChannel" .. publicChannelId)
assert(otpApp.room.id == publicChannelId and otpApp.room.can_post == false, "Joining a public channel creates a subscriber membership")
assert(not otpApp.screen:findChild("Composer", true).Visible, "Fresh public subscribers cannot publish")
assert(otpApp.screen:findChild("EmptyTimeline", true):getTextContent() == "no posts yet",
    "An empty subscriber timeline must not ask the reader to publish")

-- The picker is injected; upload/storage/message operations remain the real mock API.
local photoFile = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = photoFile:read("*a"); photoFile:close()
local photoMedia = require("Chat/media").new()
local nextPhoto, cancelPick = { jpeg = jpeg, thumbnail = jpeg, width = 32, height = 24,
    thumbnail_width = 32, thumbnail_height = 24, byte_count = #jpeg, mime_type = "image/jpeg" }, false
photoMedia.pick = function()
    if cancelPick then return nil, { code = "cancelled", message = "Photo selection cancelled." } end
    return nextPhoto
end
local photoApp = mockApp(photoMedia)
local photoRoom = photoApp.client:listRooms()[1]
-- Compact bubbles fit the fixture history on screen; add enough to scroll.
for i = 1, 12 do assert(photoApp.client:sendMessage(photoRoom.id, "Earlier message " .. i)) end
photoApp:openRoom(photoRoom)
photoApp.composer.Text = "A caption worth keeping"
cancelPick = true
click(photoApp, "AttachPhoto")
assert(photoApp.composer.Text == "A caption worth keeping" and not photoApp.photoDrafts[photoApp.room.id],
    "Cancelling the picker must preserve the text draft")
cancelPick = false
click(photoApp, "AttachPhoto")
assert(photoApp.screen:findChild("PhotoDraft", true).Visible, "A selected photo must show pending attachment feedback")
photoApp.composer.Text = string.rep("x", 4001)
click(photoApp, "SendMessage")
assert(photoApp.photoDrafts[photoApp.room.id] and photoApp.photoDrafts[photoApp.room.id].attachment,
    "A failed message after upload must retain the attachment for retry: " .. photoApp.status:getTextContent())
assert(#photoApp.composer.Text == 4001, "A failed photo caption must remain editable")
local attachmentId = photoApp.photoDrafts[photoApp.room.id].attachment.id
photoApp.composer.Text = "Our first photo"
click(photoApp, "SendMessage")
assert(not photoApp.photoDrafts[photoApp.room.id] and photoApp.composer.Text == "", "Clear photo drafts only after successful send")
local photoMessages = photoApp.client:listMessages(photoApp.room.id)
assert(photoMessages[#photoMessages].attachments[1].id == attachmentId, "Retry must attach the already uploaded photo")
local photoView = assert(photoApp.screen:findChild("Photo" .. attachmentId, true), "The timeline must render the API thumbnail")
assert(photoView.Source, "The timeline thumbnail must load a real image texture from the private preview file")
assert(photoView.Source.Width == 32 and photoView.Source.Height == 24,
    "The loaded thumbnail texture must be initialized with its actual image dimensions")
photoApp.screen:UpdateLayout(390, 844)
-- Measure the timeline entry: the bubble plus its tail.
local photoBubble = photoView:getParent():getParent()
local timeline, bubble = photoApp.messageList, photoBubble:getParent():getParent()
assert(photoView.ActualX == 0 and photoView.ActualWidth == photoBubble.ActualWidth,
    "Photos must fill their bubble edge to edge")
local offset = timeline.ContentOffset
assert(timeline.ScrollHeight > timeline.ActualHeight, "The photo fixture must overflow the viewport to exercise scrolling")
assert(math.abs(offset.Y - (timeline.ActualHeight - timeline.ScrollHeight)) < 1,
    "Sending a photo must scroll to the actual bottom of the timeline")
assert(bubble.ActualY + offset.Y >= 0 and bubble.ActualY + bubble.ActualHeight + offset.Y <= timeline.ActualHeight,
    "The latest photo and caption must be fully visible above the composer")
local previewPath = photoView.Src
local previewFile = assert(io.open(previewPath, "rb"))
assert(previewFile:read("*a") == jpeg, "The rendered preview must contain the actual mock API thumbnail bytes")
previewFile:close()
photoApp:show("/profile"); click(photoApp, "SignOut")
local leftover = io.open(previewPath, "rb")
if leftover then leftover:close() end
assert(not leftover, "Signing out must delete private photo preview files")

-- Editing and deleting: tap a bubble, pick an action; unsent drafts survive.
local editApp = mockApp()
local function roomById(id) for _, room in ipairs(editApp.client:listRooms()) do if room.id == id then return room end end end
local typo = assert(editApp.client:sendMessage("weekend", "Typo heer"))
editApp:openRoom(roomById("weekend"))
drain(editApp)
local actionsMenu = editApp.screen:findChild("MessageActionsMenu", true)
local function storedBody(id)
    for _, message in ipairs(editApp.client:listMessages("weekend")) do if message.id == id then return message.body, message end end
end
assert(not actionsMenu.Visible, "Message actions stay hidden until a bubble is tapped")
editApp.composer.Text = "unsent draft"
click(editApp, "MessageActions" .. typo.id)
assert(actionsMenu.Visible and editApp.screen:findChild("EditMessage", true) and editApp.screen:findChild("DeleteMessage", true),
    "Tapping your own message offers edit and delete")
editApp.screen:UpdateLayout(390, 844)
local sheet = editApp.screen:findChild("MessageActionSheet", true)
assert(sheet.ActualY + sheet.ActualHeight <= 844 + 1 and sheet.ActualY > 844 / 2, "The action sheet sits at the bottom of the screen")
click(editApp, "EditMessage")
assert(not actionsMenu.Visible and editApp.screen:findChild("EditingMessage", true).Visible, "Editing shows its banner")
assert(editApp.composer.Text == "Typo heer", "Editing loads the message text into the composer")
editApp.composer.Text = "Typo here"
click(editApp, "SendMessage")
local body, stored = storedBody(typo.id)
assert(body == "Typo here" and stored.edited_at, "Sending in edit mode saves the edit")
assert(editApp.composer.Text == "unsent draft" and not editApp.screen:findChild("EditingMessage", true).Visible,
    "Saving an edit restores the unsent draft")
local bubble = editApp.screen:findChild("MessageBubble" .. typo.id, true)
local meta
for child in bubble.children do meta = child end
assert(bubble:getFirstChild():getTextContent() == "Typo here" and meta:getTextContent():match("^edited "),
    "The timeline shows the new text with an edited mark")
click(editApp, "MessageActions" .. typo.id)
click(editApp, "EditMessage")
editApp.composer.Text = "never saved"
click(editApp, "CancelEdit")
assert(storedBody(typo.id) == "Typo here" and editApp.composer.Text == "unsent draft", "Cancelling an edit discards it")
click(editApp, "MessageActions" .. typo.id)
click(editApp, "DeleteMessage")
assert(actionsMenu.Visible and editApp.screen:findChild("ConfirmDeleteMessage", true), "Deleting asks for confirmation")
click(editApp, "CancelDeleteMessage")
assert(not actionsMenu.Visible and storedBody(typo.id), "Cancelling keeps the message")
click(editApp, "MessageActions" .. typo.id)
click(editApp, "DeleteMessage")
click(editApp, "ConfirmDeleteMessage")
assert(not storedBody(typo.id) and not editApp.screen:findChild("MessageBubble" .. typo.id, true),
    "Confirmed deletion removes the message for everyone and from the timeline")
assert(editApp.composer.Text == "unsent draft", "Deleting a message keeps the draft")
click(editApp, "MessageActionsw1")
assert(actionsMenu.Visible and not editApp.screen:findChild("EditMessage", true) and editApp.screen:findChild("DeleteMessage", true),
    "Group owners may delete, but not edit, other people's messages")
click(editApp, "DismissMessageActions")
assert(not actionsMenu.Visible, "Tapping outside dismisses the actions")
editApp:openRoom(roomById("alex"))
drain(editApp)
assert(editApp.screen:findChild("MessageBubblea1", true) and not editApp.screen:findChild("MessageActionsa1", true),
    "Other people's messages in a direct chat have no actions")

-- Interleave a newer tab request inside the offline transport response. Both
-- requests still use the real client and mock endpoint implementation.
local offlineTransport = require("api.mock_transport").new { clock = function() return 1700000000 end }
local raceApp, interruptProfile
local raceClient = assert(Chat.client { url = "https://mock.invalid",
    clock = function() return 1700000000 end,
    transport = function(request)
        if interruptProfile and request.url:find("/rpc/get_chat_profile", 1, true) then
            interruptProfile = false
            raceApp:show("/new")
        end
        return offlineTransport(request)
    end,
})
assert(raceClient:signIn("demo@example.invalid", "password123"))
raceApp = App { client = raceClient }
-- Keep App.show's navigation ownership, suppress only native Paint activation.
raceApp.activate_controller = function(self, controller) self.screen = controller.view; return controller end
raceApp:show()
local raceShell = raceApp.screen
interruptProfile = true
raceApp:show("/profile")
assert(not interruptProfile, "The race fixture must interleave at the real profile endpoint")
assert(raceApp.screen == raceShell and raceApp.shell.active == "New" and raceApp.shell.content.ActivePage == "New",
    "A late profile response must not replace a newer tab selection")
assert(raceApp.shell:has("Profile"), "A completed profile load may populate its retained tab without selecting it")

-- Profile photos replace initials where a tile stands for one person.
local Fixtures = require "api.mock_fixtures"
local photoApp = mockApp(nil, Chat.mock { clock = function() return 1700000000 end, fixtures = function(at)
    local data = Fixtures.new(at)
    data.avatars, data.profilePhotos, data.photoFiles = {}, { alex = "photo-alex" }, { ["photo-alex"] = jpeg }
    return data
end })
local function avatarPhoto(node) return node and node:findChild("AvatarPhoto", true) end
local alexPhoto = avatarPhoto(photoApp.screen:findChild("Room1", true))
assert(alexPhoto and alexPhoto.Src == photoApp.media.keys["avatar-photo-alex"], "Direct chats show the other person's photo")
assert(not avatarPhoto(photoApp.screen:findChild("Room2", true)), "Groups keep their initials")
assert(not avatarPhoto(photoApp.screen:findChild("Room3", true)), "People without a photo keep their initials")
assert(photoApp.avatarPaths.jules == false, "People without a photo are remembered for the session")
photoApp.screen = photoApp:dispatch("/new").view
assert(avatarPhoto(photoApp.screen:findChild("Contactalex", true):getParent()), "Contacts show their photos")
photoApp.room = photoApp.rooms[1]
photoApp.screen = photoApp:dispatch("/details").view
assert(avatarPhoto(photoApp.screen:findChild("Memberalex", true)) and not avatarPhoto(photoApp.screen:findChild("Memberme", true)),
    "Conversation members show their photos")

-- Typing q must reach the focused control rather than close the application.
local system = require "orca.system"
local savedGet, savedDispatch, savedTranslate = system.getMessage, system.dispatchMessage, system.translateMessage
local count, routed = 0, false
local events = {
    { key = "q", is = function(_, kind) return kind == "Node.KeyDown" end },
    { is = function(_, kind) return kind == "Window.Closed" end },
}
system.getMessage = function() count = count + 1; return events[count] end
system.dispatchMessage = function(_, message) routed = message.key == "q"; return false end
system.translateMessage = function() end
app:run()
system.getMessage, system.dispatchMessage, system.translateMessage = savedGet, savedDispatch, savedTranslate
assert(routed, "Typing q must not quit the chat app")
orca.async = savedAsync
print("Chat UI: password/OTP login, profile/linking, private/public discovery, groups/channels/invites, photo retry, navigation races, retained tabs, keyboard and profile photos passed")
