local core = require "orca.core"
local fs = require "orca.filesystem"
local media = require "orca.media"
local renderer = require "orca.renderer"
local recordings = require "encarta/Narration/Timings"

local ReadAloud = {}
ReadAloud.__index = ReadAloud

function ReadAloud.new(view, openAudio)
    local self = setmetatable({
        view = view,
        openAudio = openAudio or media.openAudio,
        scroll = assert(view:findChild("ReaderScroll", true)),
        play = assert(view:findChild("ReadAloud", true)),
        playIcon = assert(view:findChild("ReadAloudIcon", true)),
        playLabel = assert(view:findChild("ReadAloudLabel", true)),
        stopButton = assert(view:findChild("StopReading", true)),
        error = assert(view:findChild("ReadAloudError", true)),
    }, ReadAloud)
    self.timer = core.Timer {
        Tick = function(_, args)
            if self.timerID == args.timerId then self:update() end
        end,
    }
    self.play.Click = function() self:toggle() end
    self.stopButton.Click = function() self:stop() end
    return self
end

function ReadAloud:setPlayLabel(text, icon)
    self.playLabel.Text = text
    self.playIcon.Source = "encarta/Icons/" .. icon .. ".svg"
end

function ReadAloud:cancelTimer()
    if self.timerID then
        self.timer:cancelTimer(self.timerID)
        self.timerID = nil
    end
end

function ReadAloud:clearHighlight()
    if self.highlighted then
        self.highlighted.HighlightedRange = { X = 0, Y = 0 }
        self.highlighted = nil
        self.activeWord = nil
        self.view:post("Window.Paint", renderer.getSize())
    end
end

function ReadAloud:stop()
    self:cancelTimer()
    if self.player then
        self.player:close()
        self.player = nil
    end
    self:clearHighlight()
    self.followedBlock = nil
    self:setPlayLabel("Read aloud", "volume-2")
    self.stopButton.Visible = false
    self.error.Visible = false
end

function ReadAloud:setTrack(name, blocks)
    self:stop()
    self.track = nil
    self.words = {}
    local recording = recordings[name]
    self.play.Visible = false
    if not recording or #recording ~= #blocks then return end
    for index, passage in ipairs(recording) do
        if passage.text ~= blocks[index].Text then return end
    end
    for index, passage in ipairs(recording) do
        for _, timing in ipairs(passage.words) do
            self.words[#self.words + 1] = { timing = timing, block = blocks[index] }
        end
    end
    self.track = "encarta/Narration/" .. name .. ".wav"
    self.play.Visible = true
end

function ReadAloud:reveal(block)
    local scroll = self.scroll
    local top, node = 0, block
    while node ~= scroll do
        top = top + node.ActualY
        node = node.parent
    end
    -- The page runs under the title bar: its visible part starts below the
    -- inset that layout publishes.
    local offset = scroll.ContentOffset.Y
    local visibleTop, visibleBottom = top + offset, top + offset + block.ActualHeight
    local first = scroll.AdjustedContentInsetTop + 20
    local last = scroll.ActualHeight - scroll.AdjustedContentInsetBottom - 20
    local target = offset
    if visibleTop < first then
        target = first - top
    elseif visibleBottom > last then
        target = math.max(first - top, last - top - block.ActualHeight)
    end
    target = math.max(math.min(0, scroll.ActualHeight - scroll.ScrollHeight), math.min(0, target))
    if target ~= offset then scroll.ContentOffset = { X = 0, Y = target } end
end

function ReadAloud:update()
    local elapsed, playing = self.player:status()
    if not playing then
        self:stop()
        self:setPlayLabel("Read again", "rotate-ccw")
        return
    end

    local time = elapsed * 1000
    local low, high = 1, #self.words
    while low <= high do
        local middle = (low + high) // 2
        if self.words[middle].timing[1] <= time then low = middle + 1
        else high = middle - 1 end
    end
    local word = self.words[high]
    if word and time >= word.timing[2] then word = nil end
    if word == self.activeWord then return end
    self:clearHighlight()
    if word then
        self.activeWord = word
        self.highlighted = word.block
        word.block.HighlightedRange = { X = word.timing[3], Y = word.timing[4] }
        if word.block ~= self.followedBlock then
            -- The page may take over, for example to bring a new question up.
            if not (self.onBlock and self.onBlock(word.block)) then self:reveal(word.block) end
            self.followedBlock = word.block
        end
        self.view:post("Window.Paint", renderer.getSize())
    end
end

function ReadAloud:toggle()
    if not self.track then return end
    if self.timerID then
        self:cancelTimer()
        self.player:pause()
        self:setPlayLabel("Resume", "play")
        return
    end

    if not self.player then
        local data = fs.readTextFile(self.track)
        if data then self.player = self.openAudio(data) end
    end
    if not self.player or not self.player:play() then
        self:stop()
        self.error.Visible = true
        return
    end
    self.error.Visible = false
    self:setPlayLabel("Pause", "pause")
    self.stopButton.Visible = true
    self.timerID = self.timer:setTimer(33)
end

return ReadAloud
