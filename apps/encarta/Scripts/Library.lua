local fs = require "orca.filesystem"
local ui = require "orca.UIKit"
local articles = require "encarta/Content/Articles"
local ReadAloud = require "encarta/Scripts/ReadAloud"
local Passport = require "encarta/Scripts/Passport"
local Hunt = require "encarta/Scripts/Hunt"

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
    local terms = table.concat({ article.title, article.question, article.category, article.keywords }, " "):lower()
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
    local study = self.mode == "study"
    wall.Visible, shelf.Visible = self.mode == "wall", self.mode == "shelf"
    control(view, "Study").Visible = study
    control(view, "Categories").Visible = not study
    control(view, "CatalogHead").Visible = not study
    control(view, "Featured").Visible = not study and self.query == "" and not self.category
    local title = self.mode == "shelf" and "On the shelf" or "Big questions"
    if self.query ~= "" then title = "Search results"
    elseif self.category then title = self.category end
    control(view, "CatalogTitle").Text = title
    control(view, "ResultCount").Text = (count == 1 and "1 ARTICLE" or count .. " ARTICLES")
    control(view, "EmptyResults").Visible = count == 0 and not study
end

function Controller:filter(query)
    query = (query or ""):lower():match("^%s*(.-)%s*$")
    if query == self.query then return end
    self.query = query
    -- Typing a question leaves the study for the wall, where results show.
    if query ~= "" and self.mode == "study" then return self:setMode("wall") end
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

-- The library has three views: the study, the wall of questions and the
-- bookshelf.
function Controller:setMode(mode)
    self.mode = mode
    for name, button in pairs({ study = "ShowStudy", wall = "ShowWall", shelf = "ShowShelf" }) do
        control(self.view, button).StateClasses = name == mode and "view-on" or ""
    end
    control(self.view, "ClearSearch").Visible = self.query ~= ""
    self:closeBubble()
    self:refresh()
end

-- Study ---------------------------------------------------------------------

-- Each object in the study stands for a category. Tapping one opens a bubble
-- with a few of its questions and the way to all of them.
local BUBBLE_QUESTIONS = 3

function Controller:closeBubble()
    control(self.view, "Bubble").Visible = false
    if self.bubbleObject then self.bubbleObject.StateClasses = "" end
    self.bubbleObject = nil
end

function Controller:openBubble(category, object)
    self:closeBubble()
    local view, list = self.view, control(self.view, "BubbleList")
    clear(list)
    local total = 0
    for _, article in ipairs(articles) do
        if article.category == category then
            total = total + 1
            if total <= BUBBLE_QUESTIONS then
                local item = prefab("BubbleItem")
                item.Name = "Bubble_" .. article.id
                control(item, "Label").Text = article.question
                item.Click = function() self:openArticle(article.id) end
                list:addChild(item)
            end
        end
    end
    control(view, "BubbleTitle").Text = category
    local all = control(view, "BubbleAll")
    all.Text = total == 1 and "See this article" or ("See all %d articles"):format(total)
    all.Click = function()
        self:setCategory(category)
        self:setMode("wall")
    end
    self.bubbleObject = object
    object.StateClasses = "object-on"
    control(view, "Bubble").Visible = true
end

function Controller:addObjects()
    for _, category in ipairs(categories) do
        local object = control(self.view, "Object_" .. slug(category))
        object.Click = function() self:openBubble(category, object) end
    end
    local featured = byID[FEATURED]
    control(self.view, "PostcardQuestion").Text = featured.question
    control(self.view, "PostcardImage").Src = thumbnail(featured.cover or featured.image.path)
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

-- `passportPath` says where stamps are kept; the app's own folder if absent.
function Controller.mount(view, openAudio, passportPath)
    local self = setmetatable({ view = view, history = {}, query = "", mode = "study", home = "LibraryScroll" }, { __index = Controller })
    self.passport = Passport.open(passportPath)
    self.hunt = Hunt.new(view, articles, self.passport, function(id) self:openArticle(id) end)
    self.readAloud = ReadAloud.new(view, openAudio)
    self.readAloud.onBlock = function(block) self:follow(block) end
    control(view, "FeaturedQuestion").Text = byID[FEATURED].question
    self:addChips()
    self:addObjects()
    self:setCategory(nil)
    self:setMode("study")
    local search = control(view, "Search")
    self:filter(search.Text)
    search.onTextChanged = function(_, text) self:filter(text) end
    for name, action in pairs({
        ClearSearch = self.clearQuery, OpenFeatured = self.openFeatured,
        ShowStudy = function() self:setMode("study") end,
        ShowWall = function() self:setMode("wall") end, ShowShelf = function() self:setMode("shelf") end,
        Postcard = self.openFeatured, BubbleClose = self.closeBubble,
        OpenPassport = self.openHunt, OpenHunt = self.openHunt, HuntBack = self.closeHunt,
        Back = self.goBack,
        PrevPage = function() self:turnTo(self.page - 1) end,
        NextPage = function() self:turnTo(self.page + 1) end,
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

-- An article is a book. Each question takes one spread per paragraph: the
-- left page reads, the right page shows. The first spread of a question
-- carries its picture, the last one its extras and the way onward. Tapping
-- a page's outer margin turns it; bookmarks on the book's edge jump between
-- questions.

-- Numbered spots sit on a picture at fractions of its size. One grid holds
-- them all, because a full-size layer per spot would take the taps meant for
-- the spots under it. Each distinct x gets a zero-width column and each
-- distinct y a zero-height row, with flexible tracks between them carrying
-- the fractions; a spot hangs centred on its crossing, so spots may sit as
-- close together as the picture needs.
local SPOT = 44

local function tracks(spots, axis)
    local values, seen = {}, {}
    for _, spot in ipairs(spots) do
        if not seen[spot[axis]] then
            seen[spot[axis]] = true
            values[#values + 1] = spot[axis]
        end
    end
    table.sort(values)
    local parts, rank, last = {}, {}, 0
    for position, value in ipairs(values) do
        parts[#parts + 1] = string.format("%.4ffr 0px", value - last)
        rank[value], last = position, value
    end
    parts[#parts + 1] = string.format("%.4ffr", 1 - last)
    return table.concat(parts, " "), rank, #values * 2 + 1
end

function Controller:addSpots(spread, spots)
    local columns, column, width = tracks(spots, 1)
    local rows, row = tracks(spots, 2)
    local cells, lastCell = {}, 0
    for i, spot in ipairs(spots) do
        local cell = (row[spot[2]] * 2 - 1) * width + column[spot[1]] * 2
        cells[cell], lastCell = i, math.max(lastCell, cell)
    end
    local layer = ui.Grid { Name = "Spots", Columns = columns, Rows = rows }
    local note = control(spread, "SpotNote")
    local buttons = {}
    for cell = 1, lastCell do
        local i = cells[cell]
        if not i then layer:addChild(ui.Node2D {}) else
            local button = prefab("Spot")
            button.MarginLeft, button.MarginTop = -SPOT / 2, -SPOT / 2
            button.Name = "Spot" .. i
            control(button, "Mark").Text = tostring(i)
            button.Click = function()
                for n, other in ipairs(buttons) do control(other, "Mark").StateClasses = n == i and "spot-on" or "" end
                control(note, "SpotTitle").Text = i .. "  " .. spots[i][3]
                control(note, "SpotText").Text = spots[i][4]
                note.Visible = true
            end
            buttons[i] = button
            layer:addChild(button)
        end
    end
    control(spread, "Picture"):addChild(layer)
    control(spread, "SectionCaption").Text = "Tap the numbers to explore the picture."
end

function Controller:buildSections(article)
    local book = control(self.view, "Sections")
    clear(book)
    self.spreads, self.firstSpread, self.blockSpread = {}, {}, {}
    local blocks = {}
    for i, entry in ipairs(article.sections) do
        local pages = #entry - 1
        self.firstSpread[i] = #self.spreads + 1
        for page = 1, pages do
            local spread = prefab("Spread")
            local index = #self.spreads + 1
            spread.Name = "Spread" .. index
            control(spread, "Kicker").Text = string.format("QUESTION %d OF %d  ·  PAGE %d OF %d", i, #article.sections, page, pages)
            local title, running = control(spread, "Title"), control(spread, "Running")
            if page == 1 then
                title.Text = entry[1]
                blocks[#blocks + 1], self.blockSpread[title] = title, index
            else
                title.Visible, running.Visible, running.Text = false, true, entry[1]
            end
            local paragraph = prefab("Paragraph")
            paragraph.Text = entry[page + 1]
            control(spread, "Paragraphs"):addChild(paragraph)
            blocks[#blocks + 1], self.blockSpread[paragraph] = paragraph, index
            if page == 1 then self:addPicture(spread, entry.image or article.image) end
            if page == pages then self:addExtras(spread, article, i) end
            book:addChild(spread)
            self.spreads[index] = { node = spread, section = i, page = page, pages = pages }
        end
    end
    self.readAloud:setTrack(article.id, blocks)
    self:buildSteps(article)
end

function Controller:addPicture(spread, image)
    if not image then return end
    control(spread, "SectionImage").Src = image.path
    control(spread, "SectionCaption").Text = image.caption
    control(spread, "SectionMedia").Visible = true
    if image.spots then self:addSpots(spread, image.spots) end
end

-- The last spread of a question closes it: the surprising fact, something to
-- try, a related article, and the next question or the way home.
function Controller:addExtras(spread, article, n)
    local entry = article.sections[n]
    for name, text in pairs({ Wow = entry.wow, Try = entry.try }) do
        control(spread, name .. "Text").Text = text
        control(spread, name).Visible = true
    end
    local related = n == #article.sections and byID[article.related]
    if related then
        local link = control(spread, "Related")
        control(link, "RelatedText").Text = related.question
        link.Click = function() self:openArticle(related.id) end
        link.Visible = true
    end
    local onward, following = control(spread, "NextUp"), article.sections[n + 1]
    if following then
        control(onward, "NextUpText").Text = following[1]
        onward.Click = function() self:turnTo(self.firstSpread[n + 1]) end
    else
        control(onward, "NextUpTag").Text = "THE END"
        control(onward, "NextUpText").Text = "Back to the library"
        onward.Click = function() self:goBack() end
    end
    onward.Visible = true
end

local TAB, TAB_REACH = 44, 12

function Controller:buildSteps(article)
    local steps = control(self.view, "Steps")
    clear(steps)
    self.steps = {}
    for i in ipairs(article.sections) do
        local step = prefab("Step")
        step.Name = "Step" .. i
        control(step, "Label").Text = tostring(i)
        step.Click = function() self:turnTo(self.firstSpread[i]) end
        steps:addChild(step)
        self.steps[i] = step
    end
    self.page, self.currentSection = nil, nil
end

-- Show one spread: light its bookmark and draw a dot per spread of its
-- question.
function Controller:setPage(n)
    n = math.max(1, math.min(#self.spreads, n))
    if n == self.page then return end
    for i, spread in ipairs(self.spreads) do spread.node.Visible = i == n end
    self.page = n
    local spread = self.spreads[n]
    spread.node.ContentOffset = { X = 0, Y = 0 }
    self.currentSection = spread.section
    -- The current bookmark is stuck onto the page: it reaches over the book's
    -- edge and loses the shade the book casts on the others.
    for i, step in ipairs(self.steps) do
        local on = i == spread.section
        step.StateClasses = on and "tab-on" or (i < spread.section and "tab-done" or "")
        step.Width, step.MarginLeft = on and TAB + TAB_REACH or TAB, on and -TAB_REACH or 0
        control(step, "Label").StateClasses = step.StateClasses ~= "" and "tab-label-" .. step.StateClasses:sub(5) or ""
        control(step, "Shade").Visible = not on
    end
    local dots = control(self.view, "Dots")
    clear(dots)
    for page = 1, spread.pages do
        local on = page == spread.page
        local dot = ui.Node2D { Name = "Dot" .. page, Width = on and 28 or 10, Height = 10, IgnoreHitTest = true }
        dot.StyleClass = on and "dot dot-on" or "dot"
        dots:addChild(dot)
    end
    control(self.view, "PrevPage").Visible = n > 1
    control(self.view, "NextPage").Visible = n < #self.spreads
    -- Reaching the last spread means the whole article has been read.
    if n == #self.spreads then self.passport:add(self.current.id) end
end

-- A page turned by hand stops the narration; the narration turns pages
-- itself through follow.
function Controller:turnTo(n)
    self.readAloud:stop()
    self:setPage(n)
end

-- Keep a narrated block in view: open its spread, and on a spread taller
-- than the book scroll just enough to show it.
function Controller:follow(block)
    local n = self.blockSpread and self.blockSpread[block]
    if not n then return end
    self:setPage(n)
    local scroll, top, node = self.spreads[n].node, 0, block
    while node ~= scroll do top, node = top + node.ActualY, node.parent end
    local offset = scroll.ContentOffset.Y
    local first, last = 20, scroll.ActualHeight - 20
    local target = offset
    if top + offset < first then
        target = first - top
    elseif top + offset + block.ActualHeight > last then
        target = math.max(first - top, last - top - block.ActualHeight)
    end
    target = math.max(math.min(0, scroll.ActualHeight - scroll.ScrollHeight), math.min(0, target))
    if target ~= offset then scroll.ContentOffset = { X = 0, Y = target } end
end

-- The reader covers whichever screen it was opened from: the library or the
-- hunt.
function Controller:setReaderVisible(visible)
    for _, name in ipairs({ "LibraryScroll", "Hunt" }) do
        control(self.view, name).Visible = not visible and name == self.home
    end
    control(self.view, "Reader").Visible = visible
    if not visible and self.home == "Hunt" then self.hunt:show() end
end

function Controller:openHunt()
    self:closeBubble()
    self.home = "Hunt"
    self:setReaderVisible(false)
end

function Controller:closeHunt()
    self.home = "LibraryScroll"
    self:setReaderVisible(false)
end

function Controller:showArticle()
    local state, view = self.current, self.view
    local article = byID[state.id]
    control(view, "ArticleCategory").Text = article.category:upper()
    control(view, "ArticleTitle").Text = article.title
    self:buildSections(article)
    self:setReaderVisible(true)
    self:setPage(state.page or 1)
    if control(view, "Search"):isFocused() then control(view, "Back"):setFocus() end
end

function Controller:openArticle(id)
    if self.current then
        self.current.page = self.page
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
    self.current = table.remove(self.history)
    if self.current then
        self:showArticle()
    else
        self:setReaderVisible(false)
    end
end

return Controller
