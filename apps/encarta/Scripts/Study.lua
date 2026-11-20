local fs = require "orca.filesystem"

-- Rectangles on the camera's 1024 x 768 reference frame, including each
-- specimen's caption. The camera fits this frame inside the current window.
local originalObjects = {
    Object_space       = { 138, 118, 174, 153 },
    Object_animals     = { 412, 131, 139, 111 },
    ["Object_your-body"] = { 404, 250, 154, 103 },
    Object_earth       = { 203, 288, 146, 139 },
    Object_plants      = { 515, 337, 96, 101 },
    Object_science     = { 701, 282, 112, 141 },
    ["Object_long-ago"] = { 837, 365, 132, 78 },
    OpenPassport       = { 109, 491, 242, 71 },
    OpenHunt           = { 673, 491, 242, 71 },
    DeskQuestions      = { 115, 572, 236, 73 },
    DeskBookshelf      = { 673, 572, 236, 73 },
}

-- A pulled drawer projects farther down and toward the outside of the desk.
local drawers = {
    OpenPassport = {
        group = "DrawerLeftUpper", face = { 58, 523, 274, 78 }, destination = "passport",
        title = "My Passport", action = "See my stamps",
        hint = "Finish a book or solve a riddle to earn a stamp. Which ones have you collected?",
    },
    OpenHunt = {
        group = "DrawerRightUpper", face = { 694, 523, 267, 78 }, destination = "hunt",
        title = "Question Hunt", action = "Play a round",
        hint = "Can you solve a riddle by picking the right picture?",
    },
    DeskQuestions = {
        group = "DrawerLeftLower", face = { 63, 610, 271, 81 }, destination = "wall",
        title = "Questions", action = "Explore the questions",
        hint = "Pick a question and see where it takes you.",
    },
    DeskBookshelf = {
        group = "DrawerRightLower", face = { 692, 610, 264, 81 }, destination = "shelf",
        title = "Bookshelf", action = "Browse the books",
        hint = "Choose an illustrated book to open.",
    },
}

local rooms = {
    Nature = "The Specimen Cabinet",
    Science = "The Inventor’s Planetarium",
    Clubhouse = "The Clubhouse",
    Abyss = "The Deep-Sea Station",
    Original = "The Explorer’s Study",
}

-- The front plane covers 864 by 648 units of the camera's reference frame.
local function roomRectangle(left, top, right, bottom)
    return { 80 + left * 864, 64 + top * 648, (right - left) * 864, (bottom - top) * 648 }
end

local function pulledFace(rect)
    local magnification = 4 / (4 - .46)
    return {
        512 + (rect[1] - 512) * magnification, 384 + (rect[2] - 384) * magnification,
        rect[3] * magnification, rect[4] * magnification,
    }
end

local Study = {}
Study.__index = Study

local function bind(node, property, expression)
    assert(node:attachPropertyProgram("Node." .. property, expression, "Expression", true), property)
end

local function position(button, rect, scale)
    local x, y, width, height = table.unpack(rect)
    bind(button, "Width", ("MAX(44, %.3f * %s)"):format(width, scale))
    bind(button, "Height", ("MAX(44, %.3f * %s)"):format(height, scale))
    bind(button, "MarginLeft", ("{Screen.Width} / 2 + (%.3f * %s)"):format(x - 512, scale))
    bind(button, "MarginTop", ("{Screen.Height} / 2 + (%.3f * %s)"):format(y - 384, scale))
end

function Study.mount(view, enter)
    local self = setmetatable({ view = view, enter = enter }, Study)
    self.container = assert(view:findChild("Room", true))
    self.picker = assert(view:findChild("RoomPicker", true))
    self.contents = assert(view:findChild("RoomContents", true))
    self.face = assert(view:findChild("PulledDrawer", true))
    self.face.Click = function() self:closeDrawer(false) end
    self.note = assert(view:findChild("DrawerNote", true))
    for _, panel in ipairs({ self.note, assert(view:findChild("Bubble", true)) }) do
        bind(panel, "Width", "MIN(354, {Screen.Width} - 24)")
    end
    return self
end

function Study:showPicker(visible)
    self.contents.Visible = not visible
    self.picker.Visible = visible
end

function Study:selectRoom(id)
    local title = assert(rooms[id], id)
    self:showPicker(false)
    if self.roomID == id then return end

    local original = id == "Original"
    local folder = original and "encarta/Study" or ("encarta/Rooms/" .. id)
    local scene = assert(fs.loadObject(folder .. "/Scene"), folder)
    self:closeDrawer(true)
    if self.scene then self.scene:removeFromParent() end
    self.container:addChild(scene)
    self.scene, self.roomID = scene, id
    self.view:findChild("Scene", true).Camera = original and "StudyCamera" or "ExhibitCamera"
    self.view:findChild("StudyTitle", true).Text = title
    self.view.StateClasses = id:lower() .. "-room"
    self.scale = original and "MIN({Screen.Width} / 1024, {Screen.Height} / 768)"
                           or "MIN({Screen.Width} / 864, {Screen.Height} / 768)"
    self.faces = {}
    if original then
        for name, rect in pairs(originalObjects) do
            position(self.view:findChild(name, true), rect, self.scale)
        end
        for name, drawer in pairs(drawers) do self.faces[name] = drawer.face end
    else
        local layout = require(folder .. "/layout")
        for subject, bounds in pairs(layout.subjects) do
            local x, y, w, h = table.unpack(bounds)
            local rect = roomRectangle(x - w / 2, y - h / 2, x + w / 2, y + h / 2)
            position(self.view:findChild("Object_" .. subject, true), rect, self.scale)
        end
        for name, bounds in pairs(layout.drawers) do
            local rect = roomRectangle(table.unpack(bounds))
            position(self.view:findChild(name, true), rect, self.scale)
            self.faces[name] = pulledFace(rect)
        end
    end
end

function Study:closeDrawer(immediate)
    self.note.Visible, self.face.Visible = false, false
    if self.drawer then
        local group = self.view:findChild(self.drawer.group, true)
        local player = group and group:findChild("Slide")
        if player then
            if immediate then
                player:send("AnimationPlayer.Pause")
                player.CurrentTime = 0
            else
                -- Resume can receive a zero-time frame at the end of the clip.
                player.CurrentTime = math.min(player.CurrentTime, player.Clip.StopTime - 0.001)
                player.Speed = -1
                player:send("AnimationPlayer.Resume")
            end
        end
        self.drawer = nil
    end
end

function Study:openDrawer(name)
    local drawer = assert(drawers[name])
    local same = self.drawer == drawer
    self:closeDrawer(false)
    if same then return end
    self.drawer = drawer
    local group = self.view:findChild(drawer.group, true)
    local player = group and group:findChild("Slide")
    if player then
        player.Speed = 1
        player:send("AnimationPlayer.Resume")
    end
    self.note:findChild("DrawerTitle", true).Text = drawer.title
    self.note:findChild("DrawerHint", true).Text = drawer.hint
    self.note:findChild("DrawerEnter", true).Text = drawer.action
    position(self.face, self.faces[name], self.scale)
    self.note.Visible, self.face.Visible = true, true
end

function Study:wireControls(closeBubble)
    self.view:findChild("OpenRooms", true).Click = function()
        closeBubble()
        self:closeDrawer(true)
        self:showPicker(true)
    end
    self.picker:findChild("PickerClose", true).Click = function() self:showPicker(false) end
    for id in pairs(rooms) do
        self.picker:findChild("Choose" .. id, true).Click = function() self:selectRoom(id) end
    end
    for name in pairs(drawers) do
        self.view:findChild(name, true).Click = function()
            closeBubble()
            self:openDrawer(name)
        end
    end
    self.note:findChild("DrawerClose", true).Click = function() self:closeDrawer(false) end
    self.note:findChild("DrawerEnter", true).Click = function()
        if self.drawer then self.enter(self.drawer.destination) end
    end
end

return Study
