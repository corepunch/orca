local fs = require "orca.filesystem"
local articles = require "encarta/Content/Articles"
local ReadAloud = require "encarta/Scripts/ReadAloud"

local Controller = {}
local FEATURED = "octopus"
local byID, categories = {}, {}
for index, article in ipairs(articles) do
    byID[article.id] = article
    article.volume = index
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

function Controller:card(article)
    local card = prefab("QuestionCard")
    card.Name = "Open_" .. article.id
    card.StyleClass = "card cat-" .. slug(article.category)
    control(card, "Icon").Source = icon(article)
    control(card, "Category").Text = article.category:upper()
    control(card, "Question").Text = article.question
    control(card, "Title").Text = article.title .. "  ›"
    card.Click = function() self:openArticle(article.id) end
    return card
end

function Controller:cover(article)
    local cover = prefab("Cover")
    cover.Name = "Cover_" .. article.id
    cover.StyleClass = "cover cat-" .. slug(article.category)
    control(cover, "Volume").Text = string.format("Nº %02d", article.volume)
    control(cover, "Title").Text = article.title
    control(cover, "Question").Text = article.question
    local art = article.cover or (article.image and article.image.path)
    local image, mark = control(cover, "Art"), control(cover, "Icon")
    image.Visible, mark.Visible = art ~= nil, art == nil
    if art then image.Src = art else mark.Source = icon(article) end
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
    self.readAloud.onBlock = function(block) self:follow(block) end
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
        FirstRead = function() self:setReadingLevel("first") end,
        ReadMore = function() self:setReadingLevel("more") end,
        Previous = function() self:goToPage(self.current.page - 1) end,
        Next = function() self:goToPage(self.current.page + 1) end,
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

local MARKS = { fact = "!", quiz = "?" }

function Controller:setReadingLevel(level, page)
    local state = self.current
    if state.level == level and not page then return end
    state.level = level
    self.readAloud:stop()
    local sections = control(self.view, "Sections")
    clear(sections)
    self.sections, self.blockPage, self.pages = {}, {}, {}
    local blocks = {}
    for i, entry in ipairs(byID[state.id][level]) do
        local section = prefab("Section")
        local title = control(section, "Title")
        title.Text = entry[1]
        blocks[#blocks + 1], self.blockPage[title] = title, i
        local paragraphs = control(section, "Paragraphs")
        for p = 2, #entry do
            local paragraph = prefab("Paragraph")
            paragraph.Text = entry[p]
            paragraphs:addChild(paragraph)
            blocks[#blocks + 1], self.blockPage[paragraph] = paragraph, i
        end
        sections:addChild(section)
        self.sections[i] = section
        self.pages[i] = { kind = "section", label = entry[1] }
    end
    self.pages[#self.pages + 1] = { kind = "fact", label = "Did you know?" }
    self.pages[#self.pages + 1] = { kind = "quiz", label = "Check your reading" }
    self.readAloud:setTrack(state.id, level, blocks)
    control(self.view, "FirstRead").StateClasses = level == "first" and "level-on" or ""
    control(self.view, "ReadMore").StateClasses = level == "more" and "level-on" or ""
    self:showPage(page or 1)
end

function Controller:showPage(n, offset)
    local state, view = self.current, self.view
    local pages = self.pages
    n = math.max(1, math.min(n, #pages))
    state.page = n
    local page = pages[n]
    for i, section in ipairs(self.sections) do section.Visible = i == n end
    control(view, "FactPage").Visible = page.kind == "fact"
    control(view, "QuizPage").Visible = page.kind == "quiz"

    local category = byID[state.id].category:upper()
    local count = #self.sections
    control(view, "StepLabel").Text = page.kind == "section"
        and string.format("%s  ·  QUESTION %d OF %d", category, n, count)
        or category .. "  ·  " .. page.label:upper()

    local steps, dots = control(view, "Steps"), control(view, "Dots")
    clear(steps)
    clear(dots)
    for i, entry in ipairs(pages) do
        local stateName = i == n and "on" or (i < n and "done" or nil)
        local step = prefab("Step")
        step.Name = "Step" .. i
        step.StyleClass = stateName == "on" and "step step-on" or "step"
        local mark = control(step, "Mark")
        mark.Text = MARKS[entry.kind] or tostring(i)
        mark.StyleClass = stateName and "mark mark-" .. stateName or "mark"
        local label = control(step, "Label")
        label.Text = entry.label
        label.StyleClass = stateName == "on" and "step-text step-text-on" or "step-text"
        step.Click = function() self:goToPage(i) end
        steps:addChild(step)

        local dot = prefab("Dot")
        dot.StyleClass = i == n and "dot dot-on" or "dot"
        dot.Width = i == n and 28 or 10
        dots:addChild(dot)
    end
    local previous, nextButton = control(view, "Previous"), control(view, "Next")
    previous.Visible = n > 1
    nextButton.Visible = n < #pages
    local upcoming = pages[n + 1]
    if upcoming then
        nextButton.Text = (upcoming.kind == "section" and "Next question" or upcoming.label) .. "  ›"
    end
    control(view, "ReaderScroll").ContentOffset = { X = 0, Y = offset or 0 }
end

function Controller:goToPage(n)
    self.readAloud:stop()
    self:showPage(n)
end

function Controller:follow(block)
    local page = self.blockPage and self.blockPage[block]
    if page and page ~= self.current.page then self:showPage(page) end
end

function Controller:setReaderVisible(visible)
    control(self.view, "LibraryScroll").Visible = not visible
    control(self.view, "Reader").Visible = visible
end

function Controller:showQuiz(article)
    local state, view = self.current, self.view
    local quiz = article.quiz
    local answers, feedback = control(view, "Answers"), control(view, "Feedback")
    control(view, "Question").Text = quiz.question
    local buttons = {}
    local function update()
        feedback.Visible = state.answer ~= nil
        local right = state.answer == quiz.correct
        feedback.Text = right and "You got it! " .. quiz.explanation
            or "Not quite. Look back at the article and try again."
        feedback.StateClasses = right and "feedback-right" or ""
        for i, button in ipairs(buttons) do
            button.StateClasses = i ~= state.answer and "" or (right and "tile-right" or "tile-wrong")
        end
    end
    clear(answers)
    for i, choice in ipairs(quiz.choices) do
        local button = prefab("Choice")
        button.Name = "Answer" .. i
        button.Text = choice
        button.Click = function()
            state.answer = i
            update()
        end
        answers:addChild(button)
        buttons[i] = button
    end
    update()

    local related = control(view, "Related")
    clear(related)
    for _, id in ipairs(article.related) do related:addChild(self:card(byID[id])) end
end

function Controller:showFact(article)
    local state, view = self.current, self.view
    control(view, "Fact").Text = article.fact
    local media, image = control(view, "ArticleMedia"), article.image
    media.Visible = image ~= nil
    if image then
        control(media, "ArticleImage").Src = image.path
        control(media, "ArticleCaption").Text = image.caption
    end
    local words = control(view, "Words")
    clear(words)
    for i, word in ipairs(article.words) do
        local row = prefab("Word")
        local term, meaning = control(row, "Term"), control(row, "Meaning")
        term.Name = "Word" .. i
        term.Text = word[1] .. "  ·  " .. word[2]
        meaning.Text = word[3]
        meaning.Visible = state.revealed[i] == true
        term.Click = function()
            meaning.Visible = not meaning.Visible
            state.revealed[i] = meaning.Visible
        end
        words:addChild(row)
    end
end

function Controller:showArticle()
    local state, view = self.current, self.view
    local article = byID[state.id]
    control(view, "ArticleCategory").Text = article.category:upper()
    control(view, "ArticleTitle").Text = article.title
    control(view, "ArticleDeck").Text = article.deck
    self:showFact(article)
    self:showQuiz(article)
    local page, level = state.page or 1, state.level
    state.level = nil
    self:setReadingLevel(level, page)
    self:showPage(page, state.scroll)
    self:setReaderVisible(true)
    if control(view, "Search"):isFocused() then control(view, "Back"):setFocus() end
end

function Controller:openArticle(id)
    if self.current then
        self.current.scroll = control(self.view, "ReaderScroll").ContentOffset.Y
        table.insert(self.history, self.current)
    end
    self.current = { id = id, level = "first", page = 1, revealed = {} }
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
