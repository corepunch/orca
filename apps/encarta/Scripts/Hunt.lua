local fs = require "orca.filesystem"

-- Question Hunt: a riddle and three pictures. The right picture earns its
-- article's stamp in the passport shown beside it. Nothing is timed or lost;
-- a wrong pick only fades.
local Hunt = {}
Hunt.__index = Hunt

local function control(node, name)
    return assert(node:findChild(name, true), name)
end

local function clear(parent)
    for child in parent.children do child:removeFromParent() end
end

local function slug(category)
    return (category:lower():gsub("%s+", "-"))
end

local function thumbnail(article)
    local path = article.cover or article.image.path
    return (path:gsub("/Images/([^/]+)$", "/Images/Thumbs/%1"))
end

-- `read(id)` opens an article in the reader.
function Hunt.new(view, articles, passport, read)
    local self = setmetatable({ view = view, articles = articles, passport = passport, read = read, cursor = 0 }, Hunt)
    control(view, "NextHunt").Click = function() self:next() end
    control(view, "ReadHunted").Click = function() self.read(self.article.id) end
    return self
end

-- The passport page: a stamp for every article, earned ones in colour.
function Hunt:refreshPassport()
    local grid = control(self.view, "Stamps")
    clear(grid)
    for _, article in ipairs(self.articles) do
        local stamp = assert(fs.loadObject("encarta/Prefabs/Stamp"))
        stamp.Name = "Stamp_" .. article.id
        local earned = self.passport:has(article.id)
        if earned then
            stamp.StyleClass = "stamp stamp-on cat-" .. slug(article.category)
            control(stamp, "Art").Src = thumbnail(article)
        end
        control(stamp, "Art").Visible, control(stamp, "Blank").Visible = earned, not earned
        grid:addChild(stamp)
    end
    local total = #self.articles
    control(self.view, "StampCount").Text = ("%d of %d stamps"):format(self.passport.count, total)
    control(self.view, "PassportHint").Text = self.passport.count == total
        and "Your passport is full. You can still hunt for fun!"
        or "Read a whole article or win a hunt to earn its stamp."
end

-- Hunts go through the articles in order, skipping the ones already stamped
-- until every stamp is earned.
function Hunt:pick()
    local total = #self.articles
    for step = 1, total do
        local index = (self.cursor + step - 1) % total + 1
        if not self.passport:has(self.articles[index].id) then return index end
    end
    return self.cursor % total + 1
end

function Hunt:next()
    local articles, view = self.articles, self.view
    local total = #articles
    local index = self:pick()
    self.cursor, self.article, self.solved = index, articles[index], false
    -- Two other articles, a third and two thirds of the way round the list,
    -- with the answer's place among the three changing from hunt to hunt.
    local third = total // 3
    local picks = { index, (index - 1 + third) % total + 1, (index - 1 + 2 * third) % total + 1 }
    for _ = 1, index % 3 do table.insert(picks, table.remove(picks, 1)) end
    control(view, "HuntKicker").Text = "HUNT  ·  " .. self.article.category:upper()
    control(view, "HuntQuestion").Text = self.article.hunt[1]
    control(view, "HuntHint").Text = "Pick a picture."
    control(view, "HuntFeedback").Visible, control(view, "HuntActions").Visible = false, false
    local row = control(view, "Choices")
    clear(row)
    for _, pick in ipairs(picks) do
        local article = articles[pick]
        local choice = assert(fs.loadObject("encarta/Prefabs/Choice"))
        choice.Name = "Choice_" .. article.id
        control(choice, "Art").Src = thumbnail(article)
        control(choice, "Title").Text = article.title
        choice.Click = function() self:choose(article, choice) end
        row:addChild(choice)
    end
end

function Hunt:choose(article, choice)
    if self.solved then return end
    local view = self.view
    if article ~= self.article then
        choice.StateClasses = "choice-wrong"
        control(view, "HuntHint").Text = "Not this one. Try another picture."
        return
    end
    self.solved = true
    choice.StateClasses = "choice-right"
    local new = self.passport:add(article.id)
    control(view, "HuntHint").Text = "You found it!"
    control(view, "HuntFeedbackTitle").Text = new and "Yes! New stamp" or "Yes! You know this one"
    control(view, "HuntReveal").Text = article.hunt[2]
    control(view, "ReadHuntedText").Text = "Read “" .. article.question .. "”"
    control(view, "HuntFeedback").Visible, control(view, "HuntActions").Visible = true, true
    self:refreshPassport()
end

-- Called when the screen is shown: stamps may have been earned by reading.
function Hunt:show()
    self:refreshPassport()
    if not self.article or self.solved then self:next() end
end

return Hunt
