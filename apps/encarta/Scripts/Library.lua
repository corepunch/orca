local core = require "orca.core"
local fs = require "orca.filesystem"
local articles = require "encarta/Content/Articles"
local ReadAloud = require "encarta/Scripts/ReadAloud"

local Controller = {}
local FEATURED = "octopus"
local byID, categories = {}, {}
for _, article in ipairs(articles) do
    byID[article.id] = article
    if not categories[article.category] then
        categories[article.category] = true
        categories[#categories + 1] = article.category
    end
end

local function slug(category)
    return (category:lower():gsub("%s+", "-"))
end

local function control(node, name)
    return assert(node:findChild(name, true), name)
end

local function prefab(name)
    return assert(fs.loadObject("encarta/Prefabs/" .. name), name)
end

local function clear(parent)
    for child in parent.children do child:removeFromParent() end
end

local function icon(article)
    return "encarta/Icons/" .. slug(article.category) .. ".svg"
end

-- Library -------------------------------------------------------------------

-- Library tiles show the small copy made by Tools/make_thumbnails.sh; the
-- reader and the featured strip keep the full-size picture.
local function thumbnail(path)
    return (path:gsub("/Images/([^/]+)$", "/Images/Thumbs/%1"))
end

local function setCoverArt(tile, article)
    local art = article.cover or (article.image and article.image.path)
    local image, mark = control(tile, "Art"), control(tile, "Icon")
    image.Visible, mark.Visible = art ~= nil, art == nil
    if art then image.Src = thumbnail(art) else mark.Source = icon(article) end
end

function Controller:card(article)
    local card = prefab("QuestionCard")
    card.Name = "Open_" .. article.id
    card.StyleClass = "card cat-" .. slug(article.category)
    setCoverArt(card, article)
    control(card, "Question").Text = article.question
    control(card, "Title").Text = article.title
    card.Click = function() self:openArticle(article.id) end
    return card
end

function Controller:cover(article)
    local cover = prefab("Cover")
    cover.Name = "Cover_" .. article.id
    cover.StyleClass = "cover cat-" .. slug(article.category)
    control(cover, "Title").Text = article.title
    setCoverArt(cover, article)
    cover.Click = function() self:openArticle(article.id) end
    return cover
end

local function matches(article, query, category)
    if category and article.category ~= category then return false end
    local terms = table.concat({ article.title, article.question, article.category, article.deck, article.keywords }, " "):lower()
    for word in query:gmatch("%S+") do
        if not terms:find(word, 1, true) then return false end
    end
    return true
end

function Controller:refresh()
    local view = self.view
    local wall, shelf = control(view, "Topics"), control(view, "Shelf")
    clear(wall)
    clear(shelf)
    local count = 0
    for _, article in ipairs(articles) do
        if matches(article, self.query, self.category) then
            count = count + 1
            wall:addChild(self:card(article))
            shelf:addChild(self:cover(article))
        end
    end
    wall.Visible, shelf.Visible = not self.shelf, self.shelf
    control(view, "Featured").Visible = self.query == "" and not self.category
    local title = self.shelf and "On the shelf" or "Big questions"
    if self.query ~= "" then title = "Search results"
    elseif self.category then title = self.category end
    control(view, "CatalogTitle").Text = title
    control(view, "ResultCount").Text = (count == 1 and "1 ARTICLE" or count .. " ARTICLES")
    control(view, "EmptyResults").Visible = count == 0
end

function Controller:filter(query)
    query = (query or ""):lower():match("^%s*(.-)%s*$")
    if query == self.query then return end
    self.query = query
    control(self.view, "ClearSearch").Visible = query ~= ""
    self:refresh()
end

function Controller:setCategory(category)
    self.category = category
    for name, chip in pairs(self.chips) do
        chip.StateClasses = (category or "All") == name and "chip-on" or ""
    end
    self:refresh()
end

function Controller:setShelf(shelf)
    self.shelf = shelf
    control(self.view, "ShowWall").StateClasses = shelf and "" or "view-on"
    control(self.view, "ShowShelf").StateClasses = shelf and "view-on" or ""
    self:refresh()
end

function Controller:addChips()
    local row = control(self.view, "Categories")
    self.chips = {}
    for _, name in ipairs({ "All", table.unpack(categories) }) do
        local chip = prefab("Chip")
        chip.Name = "Category_" .. slug(name)
        chip.Text = name
        chip.StyleClass = name == "All" and "chip" or "chip chip-" .. slug(name)
        chip.Click = function() self:setCategory(name ~= "All" and name or nil) end
        self.chips[name] = chip
        row:addChild(chip)
    end
end

function Controller.mount(view, openAudio)
    local self = setmetatable({ view = view, history = {}, query = "", shelf = false }, { __index = Controller })
    self.readAloud = ReadAloud.new(view, openAudio)
    self.timer = core.Timer {
        Tick = function(_, args)
            if args.timerId == self.scrollTimer then self:stepScroll() end
        end,
    }
    self.readAloud.onBlock = function(block) return self:follow(block) end
    local scroll = control(view, "ReaderScroll")
    scroll.onContentOffsetChanged = function(_, offset) self:onScroll(offset.Y) end
    -- Layout places the questions after the article opens, and again on a
    -- resize, so the current question is found once their positions are known.
    scroll.onScrollHeightChanged = function() self:refreshCurrentSection() end
    control(view, "FeaturedQuestion").Text = byID[FEATURED].question
    self:addChips()
    self:setCategory(nil)
    self:setShelf(false)
    local search = control(view, "Search")
    self:filter(search.Text)
    search.onTextChanged = function(_, text) self:filter(text) end
    for name, action in pairs({
        ClearSearch = self.clearQuery, OpenFeatured = self.openFeatured,
        ShowWall = function() self:setShelf(false) end, ShowShelf = function() self:setShelf(true) end,
        Back = self.goBack,
    }) do
        control(view, name).Click = function() action(self) end
    end
    return self
end

function Controller:clearQuery()
    control(self.view, "Search"):send("Input.Clear")
    self:filter("")
end

-- Reader --------------------------------------------------------------------

-- An article is one continuous page. The steps beside it jump to a question
-- and follow the reading position; narration only keeps its word in view.
local SCROLL_EASE, SCROLL_TICK_MS = 0.28, 16

-- A question becomes current once its heading passes this line, measured from
-- the top of the visible page as a share of the page height.
local CURRENT_LINE = 0.3

function Controller:buildSections(article)
    local sections = control(self.view, "Sections")
    clear(sections)
    self.sections, self.headingSection = {}, {}
    local blocks = {}
    for i, entry in ipairs(article.sections) do
        local section = prefab("Section")
        section.Name = "Section" .. i
        local title = control(section, "Title")
        title.Text = entry[1]
        blocks[#blocks + 1], self.headingSection[title] = title, i
        local paragraphs = control(section, "Paragraphs")
        for p = 2, #entry do
            local paragraph = prefab("Paragraph")
            paragraph.Text = entry[p]
            paragraphs:addChild(paragraph)
            blocks[#blocks + 1] = paragraph
        end
        local image = entry.image or (i == 1 and article.image)
        if image then
            local media = control(section, "SectionMedia")
            control(media, "SectionImage").Src = image.path
            control(media, "SectionCaption").Text = image.caption
            media.Visible = true
        end
        sections:addChild(section)
        self.sections[i] = section
    end
    self.readAloud:setTrack(article.id, blocks)
    self:buildSteps(article)
end

function Controller:buildSteps(article)
    local steps = control(self.view, "Steps")
    clear(steps)
    self.steps = {}
    for i, entry in ipairs(article.sections) do
        local step = prefab("Step")
        step.Name = "Step" .. i
        control(step, "Mark").Text = tostring(i)
        control(step, "Label").Text = entry[1]
        step.Click = function() self:scrollToSection(i) end
        steps:addChild(step)
        self.steps[i] = step
    end
    self.currentSection = nil
end

-- Steps before the current question are done, the current one is on.
function Controller:setCurrentSection(n)
    if n == self.currentSection then return end
    self.currentSection = n
    for i, step in ipairs(self.steps) do
        local state = i == n and "on" or (i < n and "done" or nil)
        step.StateClasses = state == "on" and "step-on" or ""
        control(step, "Mark").StateClasses = state and "mark-" .. state or ""
        control(step, "Label").StateClasses = state == "on" and "step-text-on" or ""
    end
end

-- The top of a node in the reader's scrolled content, independent of the
-- current scroll position.
function Controller:contentTop(node)
    local scroll, top = control(self.view, "ReaderScroll"), 0
    while node and node ~= scroll do
        top = top + node.ActualY
        node = node.parent
    end
    return top
end

function Controller:scrollLimit()
    local scroll = control(self.view, "ReaderScroll")
    return math.min(0, scroll.ActualHeight - scroll.ScrollHeight)
end

-- The offset that puts a question's heading at the top of the page, clamped
-- so the last questions stop at the end of the article.
function Controller:sectionOffset(n)
    local top = self:contentTop(self.sections[n]) - self:contentTop(self.sections[1])
    return math.max(self:scrollLimit(), math.min(0, -top))
end

function Controller:sectionAt(offset)
    local scroll = control(self.view, "ReaderScroll")
    if offset <= self:scrollLimit() + 1 and offset < 0 then return #self.sections end
    local line = self:contentTop(self.sections[1]) - offset + scroll.ActualHeight * CURRENT_LINE
    local current = 1
    for i, section in ipairs(self.sections) do
        if self:contentTop(section) <= line then current = i end
    end
    return current
end

function Controller:refreshCurrentSection()
    if not self.sections or #self.sections == 0 then return end
    local offset = control(self.view, "ReaderScroll").ContentOffset.Y
    self:setCurrentSection(self.chosenSection or self:sectionAt(offset))
end

-- Called for every ContentOffset write: wheel, touch, inertia, narration and
-- our own animation. Only a scroll we did not start releases a chosen step.
function Controller:onScroll(offset)
    if not self.sections or #self.sections == 0 then return end
    if not self.scrolling then
        self:cancelScroll()
        self.chosenSection = nil
    end
    self:refreshCurrentSection()
end

function Controller:setScroll(offset)
    local scroll = control(self.view, "ReaderScroll")
    self.scrolling = true
    scroll.ContentOffset = { X = 0, Y = offset }
    self.scrolling = false
end

function Controller:cancelScroll()
    if self.scrollTimer then self.timer:cancelTimer(self.scrollTimer) end
    self.scrollTimer, self.scrollTarget = nil, nil
end

-- Finish an animated jump at once (tests, or a new article).
function Controller:finishScroll()
    local target = self.scrollTarget
    self:cancelScroll()
    if target then self:setScroll(target) end
end

function Controller:stepScroll()
    local scroll, target = control(self.view, "ReaderScroll"), self.scrollTarget
    if not target then return self:cancelScroll() end
    local offset = scroll.ContentOffset.Y
    local next = offset + (target - offset) * SCROLL_EASE
    if math.abs(target - next) < 0.5 then next = target end
    self:setScroll(next)
    if next == target then self:cancelScroll() end
end

-- Narration reaching a question's heading brings that question up, as a
-- step would, without stopping the recording.
function Controller:follow(block)
    local n = self.headingSection and self.headingSection[block]
    if not n then return false end
    self:glideToSection(n)
    return true
end

function Controller:scrollToSection(n, animated)
    self.readAloud:stop()
    self:glideToSection(n, animated)
end

function Controller:glideToSection(n, animated)
    self:cancelScroll()
    self.chosenSection = n
    local target = self:sectionOffset(n)
    self:setCurrentSection(n)
    if animated == false then return self:setScroll(target) end
    self.scrollTarget = target
    self.scrollTimer = self.timer:setTimer(SCROLL_TICK_MS)
end

function Controller:setReaderVisible(visible)
    control(self.view, "LibraryScroll").Visible = not visible
    control(self.view, "Reader").Visible = visible
end

function Controller:showArticle()
    local state, view = self.current, self.view
    local article = byID[state.id]
    control(view, "ArticleCategory").Text = article.category:upper()
    control(view, "ArticleTitle").Text = article.title
    control(view, "ArticleDeck").Text = article.deck
    self:cancelScroll()
    self.chosenSection = nil
    control(view, "StepLabel").Text = string.format("%d QUESTIONS", #article.sections)
    self:buildSections(article)
    self:setReaderVisible(true)
    self:setScroll(state.scroll or 0)
    self:refreshCurrentSection()
    if control(view, "Search"):isFocused() then control(view, "Back"):setFocus() end
end

function Controller:openArticle(id)
    if self.current then
        self:finishScroll()
        self.current.scroll = control(self.view, "ReaderScroll").ContentOffset.Y
        table.insert(self.history, self.current)
    end
    self.current = { id = id }
    self:showArticle()
end

function Controller:openFeatured()
    self:openArticle(FEATURED)
end

function Controller:goBack()
    self.readAloud:stop()
    self:cancelScroll()
    self.current = table.remove(self.history)
    if self.current then
        self:showArticle()
    else
        self:setReaderVisible(false)
    end
end

return Controller
