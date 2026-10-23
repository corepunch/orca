local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
filesystem.init("apps/chat")
require("orca.core.startup").load_plugins()

local App = require "Chat/App"
local app = App()
assert(not app.remote, "Run the demo UI test with Supabase environment variables unset")
local home = app:dispatch("/").view
assert(home.Width == 390 and home.Height == 844)
assert(home.ResizeMode == "NoResize")
assert(home:findChild("Room1", true), "Conversation list must be rendered")
home:UpdateLayout(390, 844)
assert(home:getFirstChild().ActualHeight >= 843, "App content must fill the portrait viewport")
local firstRoom = home:findChild("Room1", true)
assert(firstRoom.ActualWidth > 300 and firstRoom.ActualHeight > 60, "Conversation rows need usable hit areas")
assert(home.StyleSheet, "The shared interactive stylesheet must load")
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
app.composer.Text = "Keep this draft"
local original = app.client.sendMessage
app.client.sendMessage = function() return nil, { message = "Offline" } end
app:sendMessage()
assert(app.composer.Text == "Keep this draft", "Failed send must retain input")
app.client.sendMessage = original

local newRoom = app:dispatch("/new").view
assert(newRoom:findChild("RoomName", true))
assert(newRoom:findChild("Members", true))
local profile = app:dispatch("/profile").view
profile:UpdateLayout(390, 844)
assert(profile:findChild("BackToChats", true), "Profile must offer a return to chats")
app.client:signOut()
local auth = app:dispatch("/").view
assert(auth:findChild("Email", true) and auth:findChild("Password", true))
assert(auth:findChild("Password", true).Type == "Password")

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
print("Chat UI: search, filters, unread state, conversations, send, failure, new chat, profile, auth and keyboard passed")
