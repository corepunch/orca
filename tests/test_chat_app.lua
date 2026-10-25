local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
filesystem.init("apps/chat")
require("orca.core.startup").load_plugins()

local App = require "Chat/App"
local Chat = require "api.chat"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
local function drain(app)
    core.flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(app.screen, message) end
    end
    core.flushQueue()
end
local function mockApp(media)
    local app = App { client = Chat.mock { clock = function() return 1700000000 end }, media = media }
    -- Keep navigation real while avoiding Paint on an uninitialized renderer.
    app.show = function(self, route) self.screen = self:dispatch(route or "/").view end
    app:show("/")
    local login = app.screen
    assert(not app.client:getSession() and login:findChild("SignIn", true), "Mock mode must start at real login")
    login:findChild("UsePassword", true):send("Node.LeftButtonUp")
    drain(app)
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
assert(firstRoom.ActualWidth > 300 and firstRoom.ActualHeight > 60, "Conversation rows need usable hit areas")
assert(home.StyleSheet, "The shared interactive stylesheet must load")
local savedPlatform = PLATFORM
PLATFORM = "ios"
local mobile = app:frame(require("orca.UIKit").Grid { Rows = "1fr" })
assert(mobile.ResizeMode == "CanResize", "iOS screens must track the drawable size")
mobile:UpdateLayout(834, 1100)
assert(mobile.Width == 834 and mobile.Height == 1100, "iPad layout must use available points")
mobile:UpdateLayout(390, 460)
assert(mobile.Width == 390 and mobile.Height == 460, "Layout must shrink above the iPhone keyboard")
PLATFORM = savedPlatform
assert(home:findChild("Search", true) and home:findChild("FilterUnread", true))
app.searchInput.Text = "Jules"
app:updateSearch()
assert(not home:findChild("Room1", true) and home:findChild("Room3", true), "Search must filter conversations")
app.searchInput.Text = ""
app:updateSearch()
app.filter = "Unread"
app:renderRoomList()
assert(home:findChild("Room1", true) and not home:findChild("Room2", true), "Unread filter must use unread counts")
app.readRooms.alex = true
app:renderRoomList()
assert(not home:findChild("Room1", true), "Opening a chat must clear its local unread state")
app.filter = "Pinned"
app:renderRoomList()
assert(home:findChild("Room1", true) and home:findChild("Room2", true) and not home:findChild("Room3", true))
app.filter = "All"
app:renderRoomList()

app.room = app.client:listRooms()[1]
local screen = app:dispatch("/thread").view
app.screen = screen
assert(screen:findChild("MessageBubblea1", true), "Render history before the first thread paint")
core.flushQueue()
screen:UpdateLayout(390, 844)
assert(screen:findChild("Message", true), "Thread must have a composer")
assert(app.composer.ActualHeight >= 44 and app.composer.ActualWidth > 200,
    string.format("Composer dimensions: %.1f x %.1f", app.composer.ActualWidth, app.composer.ActualHeight))
local before = #app.client:listMessages(app.room.id)
app.composer.Text = "A quick hello from the UI test"
app:sendMessage()
core.flushQueue()
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
assert(app.newSelection.alex, "Selecting a contact must use its API ID")
newRoom:findChild("Contactalex", true):send("Node.LeftButtonUp")
drain(app)
assert(not app.newSelection.alex, "Selecting a contact again must remove it")
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
    core.flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(shell, message) end
    end
    core.flushQueue()
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
assert(auth:findChild("Email", true) and auth:findChild("Password", true))
assert(auth:findChild("Password", true).Type == "Password")

-- Phone/email verification uses the same client and in-process mock protocol.
local otpApp = App { client = Chat.mock { clock = function() return 1700000000 end } }
otpApp.show = function(self, route) self.screen = self:dispatch(route or "/").view end
otpApp:show()
local function click(app, name)
    assert(app.screen:findChild(name, true), "Missing action " .. name):send("Node.LeftButtonUp")
    drain(app)
end
otpApp.screen:findChild("Identifier", true).Text = "demo@example.invalid"
click(otpApp, "RequestCode")
assert(otpApp.screen:findChild("CodeStep", true).Visible, "Requesting a code must show the verification step")
otpApp.screen:findChild("VerificationCode", true).Text = "000000"
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
otpApp.screen:findChild("LinkCode", true).Text = "123456"
click(otpApp, "VerifyLink")
assert(otpApp.client:getProfile().phone == "+447700900987", "Verified phone must attach to the existing account")
otpApp:show("/new")
otpApp.screen:findChild("FindPerson", true).Text = "@alex"
click(otpApp, "FindPeople")
assert(otpApp.screen:findChild("FoundPersonalex", true), "Exact username discovery must render API results")
click(otpApp, "FoundPersonalex")
click(otpApp, "CreateConversation")
assert(otpApp.room.kind == "direct", "The direct action must open a canonical direct chat")
otpApp:show("/new")
click(otpApp, "KindGroup")
otpApp.screen:findChild("RoomName", true).Text = "Our group"
click(otpApp, "Contactalex")
click(otpApp, "CreateConversation")
assert(otpApp.room.kind == "group" and otpApp.room.can_post, "Group creation must return posting membership")
click(otpApp, "ConversationInfo")
click(otpApp, "CreateInvite")
assert(otpApp.screen:findChild("Invitation", true):getTextContent():find("Invitation:", 1, true), "Admins must be able to create invitations")
click(otpApp, "RevokeInvite")
assert(otpApp.screen:findChild("Invitation", true):getTextContent() == "Invitation revoked.")
otpApp:show("/new")
click(otpApp, "KindChannel")
otpApp.screen:findChild("RoomName", true).Text = "Our announcements"
click(otpApp, "Contactalex")
click(otpApp, "CreateConversation")
assert(otpApp.room.kind == "channel" and otpApp.room.can_post, "Channel owners can publish")
assert(otpApp.screen:findChild("Composer", true).Visible, "Channel owners need the composer")
local channelId = otpApp.room.id
otpApp:show("/profile")
click(otpApp, "SignOut")
click(otpApp, "UsePassword")
otpApp.screen:findChild("Email", true).Text = "alex@example.invalid"
otpApp.screen:findChild("Password", true).Text = "password123"
click(otpApp, "SignIn")
local subscriberRoom
for _, room in ipairs(otpApp.client:listRooms()) do if room.id == channelId then subscriberRoom = room end end
assert(subscriberRoom and subscriberRoom.can_post == false, "The mock API must return subscriber publishing restrictions")
otpApp:openRoom(subscriberRoom)
assert(not otpApp.screen:findChild("Composer", true).Visible and otpApp.screen:findChild("ChannelReadOnly", true).Visible,
    "Channel subscribers must see a read-only timeline")
assert(not isShown(otpApp.screen:findChild("AttachPhoto", true)), "Subscribers must not be offered media publishing")

-- Public channel discovery works for a new account without preassigned membership.
otpApp:show("/new")
click(otpApp, "KindChannel")
click(otpApp, "ConversationVisibility")
otpApp.screen:findChild("RoomName", true).Text = "Open newsroom"
click(otpApp, "CreateConversation")
assert(otpApp.room.visibility == "public", "The public channel option must reach the API")
local publicChannelId = otpApp.room.id
otpApp:show("/profile"); click(otpApp, "SignOut")
otpApp.screen:findChild("Identifier", true).Text = "new_reader@example.invalid"
click(otpApp, "RequestCode")
otpApp.screen:findChild("VerificationCode", true).Text = "123456"
click(otpApp, "VerifyCode")
assert(otpApp.client:getSession().user.email == "new_reader@example.invalid", "OTP must register a fresh reader account")
otpApp:show("/new")
click(otpApp, "SearchChannels")
assert(otpApp.screen:findChild("FindChannels", true).Visible and not otpApp.screen:findChild("FindPeople", true).Visible,
    "Channel discovery must have its own visible search action")
otpApp.screen:findChild("FindPerson", true).Text = "Our announcements"
click(otpApp, "FindChannels")
assert(not otpApp.screen:findChild("FoundChannel" .. channelId, true), "Private channels must not appear in discovery")
otpApp.screen:findChild("FindPerson", true).Text = "Open news"
click(otpApp, "FindChannels")
assert(otpApp.screen:findChild("FoundChannel" .. publicChannelId, true), "Public channel search must return API results")
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
photoApp:openRoom(photoApp.client:listRooms()[1])
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
local timeline, bubble = photoApp.messageList, photoView:getParent():getParent()
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
assert(not raceApp.shell:has("Profile"), "A cancelled tab constructor must not mount stale content")

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
print("Chat UI: password/OTP login, profile/linking, private/public discovery, groups/channels/invites, photo retry, navigation races, retained tabs and keyboard passed")
