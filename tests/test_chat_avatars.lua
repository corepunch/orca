local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
local core = require "orca.core"
local system = require "orca.system"
require("orca.renderer").init(390, 844, true)
require("orca.filesystem").init("apps/chat")
local Avatar = require "Chat/views/profile_avatar"
local Avatars = require "orca.avatars"
local C = require "Chat/views/components"
local media = require("Chat/media").new()
local savedLoad, savedAsync = Avatars.load, orca.async
orca.async = function(fn) return fn() end
local photo = assert(Avatars.load("orca"))
local loaded
Avatars.load = function(id) loaded = id; photo = assert(savedLoad(id)); return photo end
local session = { user = { id = "me" } }
local calls, invalidated, selected, pending, failed = 0, 0
local app = { sessionEpoch = 1, navigationEpoch = 1, avatarPaths = {}, avatars = {}, client = {}, media = {} }
app.screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize",
    StyleSheet = "Chat/Styles/app.css", StateClasses = "chat-light" }
local tile, status = C.avatar("Me", 96, "me"), ui.TextBlock { Name = "ProfileStatus" }
function app.client:getSession() return session end
function app.client:setAvatar(value)
    calls, selected = calls + 1, value
    coroutine.yield()
    if failed then return nil, { message = "Try saving again" } end
    return { avatar_id = "picture" .. calls, user_id = "me" }
end
function app.avatars:invalidate(id) assert(id == "me"); invalidated = invalidated + 1 end
function app.media:storePreview(bytes, key) assert(bytes == photo.jpeg); return media:storePreview(bytes, key) end
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
local function origin(node)
    local x, y = 0, 0
    while node and node ~= app.screen do
        x, y = x + (node.ActualX or 0), y + (node.ActualY or 0)
        node = node:getParent()
    end
    return x, y
end
click("ChooseAvatar")
local picker = assert(app.avatarPicker)
assert(app.screen:findChild("AvatarChoiceorca", true) and app.screen:findChild("AvatarChoicesquid", true),
    "The XML picker presents the shared character catalogue")
local choices = app.screen:findChild("AvatarChoices", true)
local count = 0
for _ in choices.children do count = count + 1 end
assert(count == 16, "Every character has one native image button")
for _, width in ipairs { 390, 320 } do
    app.screen.Width = width
    flush()
    assert(app.avatarPicker.root.ActualWidth == width, "The picker fills the requested viewport")
    local portraits = {}
    for row in choices.children do
        local portrait = row:findChild("AvatarPortrait", true)
        assert(portrait.ActualWidth > 40 and math.abs(portrait.ActualWidth - portrait.ActualHeight) < 1,
            "Character portraits stay square at " .. width .. " points")
        assert(math.abs(row.ActualHeight - portrait.ActualHeight) < 1,
            "Character buttons reserve their full visible image for layout and touch")
        assert(not row:findChild("AvatarLabel", true), "The picker shows portraits without name captions")
        portraits[#portraits + 1] = portrait
    end
    local left, top = origin(portraits[1])
    assert(left == 12, "The photo grid aligns with the left edge of the page title; got " .. left)
    for i, portrait in ipairs(portraits) do
        local x, y = origin(portrait)
        if (i - 1) % 4 == 0 then
            assert(x == left, "Every row starts at the left edge")
        else
            local previous = portraits[i - 1]
            local previousX, previousY = origin(previous)
            assert(math.abs(x - previousX - previous.ActualWidth - 6) <= 1 and y == previousY,
                "Four portraits per row have 6-point horizontal gaps, within pixel rounding")
        end
        if i > 4 then
            local above = portraits[i - 4]
            local aboveX, aboveY = origin(above)
            assert(x == aboveX and math.abs(y - aboveY - above.ActualHeight - 6) <= 1,
                "Photo rows have matching 6-point vertical gaps without overlap")
        else assert(y == top, "The first four portraits share one row") end
    end
    assert(not picker.root:findChild("CancelAvatarPicker", true), "The picker uses its header back button")
    local function noSharingCaption(node)
        assert(node.Text ~= "Your picture is shared with Octopus.", "The picker omits the sharing caption")
        for child in node.children do noSharingCaption(child) end
    end
    noSharingCaption(picker.root)
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
local selectedButton = app.screen:findChild("AvatarChoicesquid", true)
local selectedPortrait = selectedButton:findChild("AvatarPortrait", true)
local x, y = origin(selectedPortrait)
x, y = x + selectedPortrait.ActualWidth / 2, y + selectedPortrait.ActualHeight / 2
system.dispatchMessage { target = app.screen, message = "LeftButtonDown", x = x, y = y }
system.dispatchMessage { target = app.screen, message = "LeftButtonUp", x = x, y = y }
flush()
assert(coroutine.status(pending) == "suspended", "Tapping the visible portrait must reach its character button")
step(); step()
assert(calls == 2 and loaded == "squid" and selected == photo and invalidated == 1 and not app.avatarPicker,
    "A retry saves through the shared profile API and closes only after success")
flush()
local savedPhoto = assert(tile:findChild("AvatarPhoto", true))
assert(savedPhoto.Src == app.avatarPaths.me and status:getTextContent() == "Picture updated.")
assert(savedPhoto.Source and savedPhoto.ActualWidth == 96 and savedPhoto.ActualHeight == 96,
    "A selected character replaces settings initials with a decoded photo filling the avatar tile")

click("ChooseAvatar")
Avatars.load = function() return nil, { message = "Character file unavailable" } end
click("AvatarChoiceorca")
assert(calls == 2 and app.screen:findChild("AvatarChoiceStatus", true):getTextContent() == "Character file unavailable",
    "Missing bundled artwork stays visible as an error without uploading")
Avatars.load = function() return photo end
local title = app.avatarPicker.root:getFirstChild()
while title and title.className ~= "TextBlock" do title = title:getNext() end
local backX, backY = origin(assert(title, "The picker has a header back tap area"))
system.dispatchMessage { target = app.screen, message = "LeftButtonDown", x = backX + 4, y = backY + 4 }
system.dispatchMessage { target = app.screen, message = "LeftButtonUp", x = backX + 4, y = backY + 4 }
flush()
assert(not app.avatarPicker and calls == 2 and app.avatarPaths.me == savedPhoto.Src,
    "Header Back closes the picker without changing the picture")

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
media:clear()
print("Chat default avatar picker tests passed")
