local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
local core = require "orca.core"
require("orca.renderer").init(390, 844, true)
require("orca.filesystem").init("apps/chat")
local Avatar = require "Chat/views/profile_avatar"
local Avatars = require "orca.avatars"
local savedLoad, savedAsync = Avatars.load, orca.async
orca.async = function(fn) return fn() end
local photo = { jpeg = "\255\216\255test\255\217" }
local loaded
Avatars.load = function(id) loaded = id; return photo end
local session = { user = { id = "me" } }
local calls, invalidated, selected, pending, failed = 0, 0
local app = { sessionEpoch = 1, navigationEpoch = 1, avatarPaths = {}, avatars = {}, client = {}, media = {} }
app.screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize",
    StyleSheet = "Chat/Styles/app.css", StateClasses = "chat-light" }
local tile, status = ui.Grid { Width = 72, Height = 72 }, ui.TextBlock { Name = "ProfileStatus" }
function app.client:getSession() return session end
function app.client:setAvatar(value)
    calls, selected = calls + 1, value
    coroutine.yield()
    if failed then return nil, { message = "Try saving again" } end
    return { avatar_id = "picture" .. calls, user_id = "me" }
end
function app.avatars:invalidate(id) assert(id == "me"); invalidated = invalidated + 1 end
function app.media:storePreview(bytes, key) assert(bytes == photo.jpeg); return "tests/fixtures/chat/photo.jpg" end
function app.media:pick() return photo end
function app:load(fn) pending = coroutine.create(fn) end
function app:avatarPhoto() error("Successful local previews should not fetch again") end
app.screen:addChild(tile)
app.screen:addChild(status)
app.screen:addChild(Avatar.controls(app, tile, status))
local function flush()
    core.flushQueue()
    core.advanceFrame()
    core.runAllPrograms(app.screen)
    core.flushQueue()
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
end
local function click(name)
    assert(app.screen:findChild(name, true), "Missing " .. name):send("Node.LeftButtonUp")
    flush()
end
local function step()
    local ok, err = coroutine.resume(pending)
    assert(ok, err)
end
click("ChooseAvatar")
local picker = assert(app.avatarPicker)
assert(app.screen:findChild("AvatarChoiceorca", true) and app.screen:findChild("AvatarChoicesquid", true),
    "The XML picker presents the shared character catalogue")
local choices = app.screen:findChild("AvatarChoices", true)
local count = 0
for _ in choices.children do count = count + 1 end
assert(count == 16, "Every character has one labelled native button")
for _, width in ipairs { 390, 320 } do
    app.screen.Width = width
    flush()
    assert(app.avatarPicker.root.ActualWidth == width, "The picker fills the requested viewport")
    for row in choices.children do
        local portrait = row:findChild("AvatarPortrait", true)
        local label = row:findChild("AvatarLabel", true)
        assert(portrait.ActualWidth > 40 and math.abs(portrait.ActualWidth - portrait.ActualHeight) < 1,
            "Character portraits stay square at " .. width .. " points")
        assert(portrait.ActualY + portrait.ActualHeight <= label.ActualY + 1,
            "Character labels sit below their portraits")
    end
end
app.screen.Width = 390
flush()
failed = true
click("AvatarChoiceorca")
assert(pending, "Picker click must schedule a save; status=" .. tostring(app.screen:findChild("AvatarChoiceStatus", true):getTextContent()))
step()
click("AvatarChoiceoctopus")
assert(calls == 1, "Pending saves reject duplicate selections")
step()
assert(app.avatarPicker == picker and app.screen:findChild("AvatarChoiceStatus", true):getTextContent() == "Try saving again",
    "A failed save keeps the picker and a visible retryable error")
failed = false
click("AvatarChoicesquid")
step(); step()
assert(calls == 2 and loaded == "squid" and selected == photo and invalidated == 1 and not app.avatarPicker,
    "A retry saves through the shared profile API and closes only after success")
assert(tile:findChild("AvatarPhoto", true).Src == "tests/fixtures/chat/photo.jpg" and status:getTextContent() == "Picture updated.")

click("ChooseAvatar")
Avatars.load = function() return nil, { message = "Character file unavailable" } end
click("AvatarChoiceorca")
assert(calls == 2 and app.screen:findChild("AvatarChoiceStatus", true):getTextContent() == "Character file unavailable",
    "Missing bundled artwork stays visible as an error without uploading")
Avatars.load = function() return photo end
click("CancelAvatarPicker")
assert(not app.avatarPicker, "Back closes the picker without changing the picture")

click("ChooseAvatar")
click("AvatarChoiceorca")
step()
Avatar.close(app)
step()
assert(not app.avatarPicker and invalidated == 2, "Closing a pending save never reopens the picker")

click("UploadAvatar")
step(); step()
assert(calls == 4 and invalidated == 3, "Photo uploads use the same public profile API")
function app.media:pick() return nil, { code = "cancelled" } end
local before = status:getTextContent()
click("UploadAvatar")
assert(calls == 4 and status:getTextContent() == before, "Cancelling a photo selection is silent")
function app.media:pick() return nil, { message = "Cannot read that photo" } end
click("UploadAvatar")
assert(status:getTextContent() == "Cannot read that photo", "Photo picker failures are visible")
function app.media:pick() return photo end
click("UploadAvatar")
app.navigationEpoch = app.navigationEpoch + 1
step()
assert(calls == 4 and status:getTextContent() == "", "Leaving before upload starts clears the retained profile's loading message")
click("UploadAvatar")
step()
app.navigationEpoch = app.navigationEpoch + 1
step()
assert(status:getTextContent() == "Picture updated." and invalidated == 4,
    "A completed upload updates its retained profile after navigating away")

click("ChooseAvatar")
click("AvatarChoiceorca")
step()
session, app.sessionEpoch = { user = { id = "other" } }, 2
Avatar.close(app)
step()
assert(invalidated == 4, "A late save from another account never updates this account's portraits")
Avatars.load, orca.async = savedLoad, savedAsync
print("Chat default avatar picker tests passed")
