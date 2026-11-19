require("orca").init()
local core = require "orca.core"
local fs = require "orca.filesystem"
require "orca.UIKit"
require "orca.renderer"

local project = fs.init("apps/encarta")
local orca = require "orca"
local position, playing, opened, closed = 0, false, 0, 0
-- Stamps go to a scratch file, never to the real passport.
local passportPath = os.tmpname()
os.remove(passportPath)
local app = require(project.StartupViewController)(function(bytes)
    assert(#bytes > 100 and bytes:sub(1, 4) == "RIFF" and bytes:sub(9, 12) == "WAVE", "read packaged WAV audio")
    position, opened = 0, opened + 1
    return {
        play = function() playing = true; return true end,
        pause = function() playing = false end,
        status = function() return position, playing end,
        close = function() playing = false; closed = closed + 1 end,
    }
end, passportPath)
app:activate_route("/")
local view = app.screen
view.ResizeMode = "NoResize"

local function control(name, root)
    return assert((root or view):findChild(name, true), name)
end

local function click(name, root)
    control(name, root):send("Node.LeftButtonUp")
    core.flushQueue()
end

local function layout(width, height)
    view.Width, view.Height = width, height
    core.advanceFrame()
    core.runAllPrograms(view)
    view:UpdateLayout(width, height)
    core.flushQueue()
end

local library = control("LibraryScroll")
local reader = control("Reader")
local wall = control("Topics")
local shelf = control("Shelf")
local sections = control("Sections")
local search = control("Search")
local function count(node) local n = 0 for _ in node.children do n = n + 1 end return n end
core.flushQueue()
layout(1024, 768)
assert(library.Visible and not reader.Visible)

-- The library opens in the study: one picture with a clear button over each
-- object, one object per category.
local study, bubble = control("Study"), control("Bubble")
local function left(node) local x = 0 while node and node ~= view do x, node = x + node.ActualX, node.parent end return x end
local function top(node) local y = 0 while node and node ~= view do y, node = y + node.ActualY, node.parent end return y end
assert(study.Visible and not shelf.Visible, "the study opens first")
assert(not control("Categories").Visible and not control("Featured").Visible)
assert(wall.Visible and count(wall) == 15 and control("CatalogTitle").Text == "Big questions" and top(wall) > top(study) + study.ActualHeight,
    "the wall of questions runs under the study")
-- On an upright iPad the questions fill the space below the desk.
layout(1024, 1366)
assert(top(study) + study.ActualHeight < 800 and top(control("Open_mars", wall)) < 1366, "question cards show under the study when the screen is tall")
layout(1024, 768)
assert(control("ShowStudy").BackgroundColor.R > 0.9 and control("ShowWall").BackgroundColor.R < 0.5)
assert(math.abs(study.ActualWidth / study.ActualHeight - 1.6) < 0.01 and study.ActualWidth > 900, "the study keeps its picture's proportions")
assert(control("StudyArt").ActualWidth == study.ActualWidth and control("StudyArt").ActualHeight == study.ActualHeight)
assert(top(study) + study.ActualHeight <= 768, "the study fits the screen without scrolling")
for _, name in ipairs({ "animals", "space", "earth", "science", "plants", "your-body", "long-ago" }) do
    local object = control("Object_" .. name)
    assert(object.ActualWidth >= 88 and object.ActualHeight >= 88, name .. " is easy to tap")
end
-- The grid follows the picture: the window on the left third, the globe in
-- the middle, the aquarium at the right edge, all standing on the desk.
local earth, space, animals = control("Object_earth"), control("Object_space"), control("Object_animals")
local desk = top(study) + study.ActualHeight * 412 / 625
assert(left(space) == left(study) and math.abs(space.ActualWidth - study.ActualWidth * 0.3) < 1)
assert(math.abs(left(earth) + earth.ActualWidth / 2 - left(study) - study.ActualWidth * 0.505) < 1, "the globe's button sits on the globe")
assert(math.abs(left(animals) + animals.ActualWidth - left(study) - study.ActualWidth) < 1)
assert(math.abs(top(earth) + earth.ActualHeight - desk) < 1 and math.abs(top(space) + space.ActualHeight - desk) < 1, "objects stand on the desk")
local postcard = control("Postcard")
assert(top(postcard) > desk and control("PostcardQuestion").Text == "How does an octopus hide?"
    and control("PostcardImage").Src == "encarta/Images/Thumbs/octopus.jpg", "today's question lies in the desk")

-- Tapping an object opens a bubble with a few of its questions.
assert(not bubble.Visible)
click("Object_earth")
layout(1024, 768)
assert(bubble.Visible and control("BubbleTitle").Text == "Earth" and count(control("BubbleList")) == 3)
assert(control("BubbleAll").Text == "See all 3 articles" and control("Label", control("Bubble_volcanoes")).Text == "What is inside a volcano?")
assert(top(bubble) + bubble.ActualHeight < top(study) + study.ActualHeight and control("Bubble_water").ActualHeight >= 44)
click("Object_space")
assert(bubble.Visible and control("BubbleTitle").Text == "Space" and not view:findChild("Bubble_volcanoes", true), "another object replaces the bubble")
click("BubbleClose")
assert(not bubble.Visible)
click("Object_earth")
click("Bubble_volcanoes")
assert(reader.Visible and control("ArticleTitle").Text == "Volcanoes", "a question in the bubble opens its article")
click("Back")
assert(library.Visible and study.Visible, "Back returns to the study")
click("Object_space")
click("BubbleAll")
assert(wall.Visible and not study.Visible and not bubble.Visible and control("CatalogTitle").Text == "Space" and count(wall) == 3,
    "See all opens the category on the wall")
click("Category_all")
click("ShowStudy")
click("Postcard")
assert(reader.Visible and control("ArticleTitle").Text == "Octopuses", "the postcard opens today's question")
click("Back")
-- The top drawers open the passport and the hunt.
local huntDrawer, hunt = control("OpenHunt"), control("Hunt")
assert(math.abs(left(huntDrawer) - left(study) - study.ActualWidth * 0.674) < 1 and math.abs(huntDrawer.ActualWidth - study.ActualWidth * 0.29) < 1
    and math.abs(top(huntDrawer) - top(study) - study.ActualHeight * 456 / 625) < 1 and huntDrawer.ActualHeight >= 44, "the hunt's button covers the top right drawer")
assert(math.abs(left(control("OpenPassport")) - left(study) - study.ActualWidth * 0.036) < 1, "the passport's covers the top left one")
click("OpenHunt")
layout(1024, 768)
assert(hunt.Visible and not library.Visible and not reader.Visible)
local passport, card, stamps = control("Passport"), control("HuntCard"), control("Stamps")
assert(left(card) > left(passport) + passport.ActualWidth and card.ActualWidth > passport.ActualWidth, "the passport sits beside the hunt")
assert(control("StampCount").Text == "0 of 15 stamps" and count(stamps) == 15)
local blank = control("Stamp_octopus")
assert(control("Blank", blank).Visible and not control("Art", blank).Visible and math.abs(blank.ActualWidth - blank.ActualHeight) < 1, "an unearned stamp is a round question mark")
-- A hunt is a riddle and three pictures; the first is for the first article.
local choices = control("Choices")
assert(control("HuntQuestion").Text == "Which animal can squeeze through any hole its beak fits through?")
assert(control("HuntKicker").Text == "HUNT  ·  ANIMALS" and count(choices) == 3 and control("HuntHint").Text == "Pick a picture.")
assert(control("Art", control("Choice_octopus")).Src == "encarta/Images/Thumbs/octopus.jpg" and control("Title", control("Choice_dinosaurs")).Text == "Dinosaurs")
assert(control("Choice_magnets").ActualHeight >= 88 and not control("HuntFeedback").Visible and not control("HuntActions").Visible)
click("Choice_dinosaurs")
assert(control("HuntHint").Text == "Not this one. Try another picture." and not control("HuntFeedback").Visible
    and control("StampCount").Text == "0 of 15 stamps", "a wrong pick costs nothing")
click("Choice_octopus")
layout(1024, 768)
assert(control("HuntFeedback").Visible and control("HuntFeedbackTitle").Text == "Yes! New stamp" and control("HuntReveal").Text:match("no bones"))
assert(control("StampCount").Text == "1 of 15 stamps" and control("HuntActions").Visible)
local earned = control("Stamp_octopus")
assert(control("Art", earned).Visible and control("Art", earned).Src == "encarta/Images/Thumbs/octopus.jpg" and not control("Blank", earned).Visible)
assert(earned.BackgroundColor.R > 0.8 and earned.BackgroundColor.B < 0.3, "an earned stamp wears its category colour")
assert(card.ScrollHeight <= card.ActualHeight + 1 and passport.ScrollHeight <= passport.ActualHeight + 1, "the hunt and the passport fit the screen")
click("Choice_dinosaurs")
assert(control("HuntHint").Text == "You found it!", "a solved hunt ignores further picks")
-- On an upright iPad the passport lies across the top, above the hunt.
layout(1024, 1366)
assert(top(card) >= top(passport) + passport.ActualHeight and math.abs(card.ActualWidth - passport.ActualWidth) < 1, "the passport lies above the hunt when upright")
assert(top(control("Stamp_earth-layers")) < top(control("Stamp_octopus")) + control("Stamp_octopus").ActualHeight * 2, "its stamps run in two rows")
assert(card.ScrollHeight <= card.ActualHeight + 1 and top(card) + card.ActualHeight <= 1366, "and the hunt fits below it")
layout(1024, 768)
click("NextHunt")
assert(control("HuntQuestion").Text == "Where have only twelve people ever walked?" and not control("HuntFeedback").Visible, "the next hunt is the next unstamped article")
click("Choice_moon")
assert(control("StampCount").Text == "2 of 15 stamps" and control("ReadHuntedText").Text == "Read “Why does the Moon change shape?”")
click("ReadHunted")
assert(reader.Visible and not hunt.Visible and control("ArticleTitle").Text == "The Moon", "the hunt leads into its article")
click("Back")
assert(hunt.Visible and not library.Visible, "Back returns to the hunt")
assert(control("HuntQuestion").Text == "What do about a million tiny cloud drops make when they join?", "a solved hunt makes way for a new one")
-- A real tap, through hit testing.
local leave = control("HuntBack")
orca.system.dispatchMessage { target = view, message = "LeftButtonDown", x = left(leave) + 20, y = top(leave) + 20 }
orca.system.dispatchMessage { target = view, message = "LeftButtonUp", x = left(leave) + 20, y = top(leave) + 20 }
core.flushQueue()
assert(library.Visible and study.Visible and not hunt.Visible, "the hunt's way back answers a tap")
-- Reading an article to its last spread earns its stamp too, and stamps are
-- kept on disk.
app.libraryController:openArticle("heart")
assert(not app.libraryController.passport:has("heart"))
app.libraryController:setPage(#app.libraryController.spreads)
click("Back")
click("OpenPassport")
assert(hunt.Visible and control("StampCount").Text == "3 of 15 stamps" and control("Art", control("Stamp_heart")).Visible)
assert(control("HuntQuestion").Text:match("cloud drops"), "an unsolved hunt waits for you")
local kept = require("encarta/Scripts/Passport").open(passportPath)
assert(kept.count == 3 and kept:has("octopus") and kept:has("moon") and kept:has("heart") and not kept:has("water"), "stamps survive a restart")
assert(not kept:add("moon") and kept.count == 3, "a stamp is earned once")
click("HuntBack")
layout(1024, 768)

-- Typing a question leaves the study for the wall, where results show.
search.Text = "moon"
core.flushQueue()
assert(wall.Visible and not study.Visible and control("Categories").Visible and control("ResultCount").Text == "1 ARTICLE")
click("ClearSearch")
assert(wall.Visible, "clearing the search stays on the wall")
layout(1024, 768)

assert(control("ResultCount").Text == "15 ARTICLES" and count(wall) == 15 and wall.Visible and not shelf.Visible)
assert(control("CatalogTitle").Text == "Big questions")
local card = control("Open_mars", wall)
assert(card.ActualWidth > 200 and math.abs(card.ActualWidth / card.ActualHeight - 1.4) < 0.02, "four landscape question cards per row")
assert(control("Question", card).Text == "Why is Mars red?")
local pyramidCover = control("Art", control("Open_pyramids", wall))
local earthCover = control("Art", control("Open_earth-layers", wall))
assert(pyramidCover.Visible and pyramidCover.Src == "encarta/Images/Thumbs/pyramids.jpg")
assert(earthCover.Visible and earthCover.Src == "encarta/Images/Thumbs/earth-cover.jpg"
    and earthCover.ActualHeight == control("Open_earth-layers", wall).ActualHeight, "art fills the question card")
assert(search.ActualHeight >= 44 and control("OpenFeatured").ActualHeight >= 44)
assert(control("FeaturedImage").ActualWidth > 900 and control("FeaturedImage").ActualHeight == 176)
local background = card.BackgroundColor
assert(background.B > 0.5 and background.R < 0.3, "space cards resolve the packaged category color")

assert(control("ShowWall").BackgroundColor.R > 0.9, "the question wall starts selected")
click("ShowShelf")
assert(control("ShowWall").BackgroundColor.R < 0.5 and control("ShowShelf").BackgroundColor.R > 0.9, "only the chosen view is highlighted")
assert(shelf.Visible and not wall.Visible and count(shelf) == 15)
assert(control("CatalogTitle").Text == "On the shelf")
local octopusCover, moonCover = control("Cover_octopus", shelf), control("Cover_moon", shelf)
assert(control("Art", octopusCover).Visible and not control("Icon", octopusCover).Visible, "art replaces the icon")
layout(1024, 768)
local moonArt = control("Art", moonCover)
assert(moonArt.Src == "encarta/Images/Thumbs/moon.jpg" and control("Title", moonCover).Text == "The Moon")
assert(moonArt.ActualHeight > moonArt.ActualWidth and moonArt.ActualHeight == moonCover.ActualHeight, "art fills the portrait cover")
assert(control("Art", control("Cover_volcanoes", shelf)).Src == "encarta/Images/Thumbs/volcanoes.jpg", "a dedicated cover takes priority")
click("Category_space")
assert(count(shelf) == 3 and control("CatalogTitle").Text == "Space" and not control("Featured").Visible)
click("Category_all")
click("ShowWall")
assert(count(wall) == 15 and control("Featured").Visible)

search.Text = "  SPACE moon  "
core.flushQueue()
assert(control("ResultCount").Text == "1 ARTICLE" and not control("Featured").Visible)
assert(count(wall) == 1 and control("Open_moon", wall))
click("Open_moon", wall)
assert(not library.Visible and reader.Visible)
assert(control("ArticleTitle").Text == "The Moon")
layout(1024, 768)
local reading = app.libraryController
local function spread(n) return control("Spread" .. n, sections) end
local function part(name, n) return control(name, spread(n)) end
local function shown()
    local found
    for node in sections.children do
        if node.Visible then
            assert(not found, "one spread at a time")
            found = node
        end
    end
    return found
end
local function screenTop(node)
    local top = 0
    while node and node ~= view do top, node = top + node.ActualY, node.parent end
    return top
end
local function screenLeft(node)
    local left = 0
    while node and node ~= view do left, node = left + node.ActualX, node.parent end
    return left
end
-- The current question is the one whose bookmark is painted sun yellow on
-- the next frame.
local function current()
    layout(view.Width, view.Height)
    local found
    for i = 1, count(control("Steps")) do
        local tab = control("Step" .. i).BackgroundColor
        if tab.R > 0.9 and tab.G > 0.7 and tab.B < 0.4 then
            assert(not found, "only one bookmark is current")
            found = i
        end
    end
    assert(found == reading.currentSection)
    return found
end
local function fits(n)
    local node = spread(n)
    return node.ScrollHeight <= node.ActualHeight + 1
end

-- A question takes one spread per paragraph; only one spread is open.
assert(count(sections) == 10 and shown() == spread(1), "five questions of two paragraphs make ten spreads")
assert(part("Title", 1).Text == "Why does the Moon seem to change shape?")
assert(part("Kicker", 1).Text == "QUESTION 1 OF 5  ·  PAGE 1 OF 2")
assert(control("ArticleCategory").Text == "SPACE" and control("ArticleTitle").Text == "The Moon" and current() == 1)
assert(count(control("Steps")) == 5 and control("Label", control("Step3")).Text == "3", "one bookmark per question")

-- The bar and the book fill the height; the bar keeps its content below
-- the status bar and the book stays above the home indicator.
local bar, book = control("ReaderBar"), control("Book")
view.SafeAreaTop, view.SafeAreaBottom = 24, 20
layout(1024, 768)
assert(screenTop(bar) == 0 and screenTop(control("Back")) >= 24, "Back sits below the status bar")
assert(screenTop(book) >= screenTop(bar) + bar.ActualHeight and screenTop(book) + book.ActualHeight <= 748, "the book stays above the home indicator")
assert(control("Back").ActualHeight >= 44 and control("Step1").ActualHeight >= 44)
view.SafeAreaTop, view.SafeAreaBottom = 0, 0
layout(1024, 768)
assert(not view:findChild("Pager", true) and screenTop(book) + book.ActualHeight > 700, "the book runs nearly to the foot of the screen")

-- A page's outer margin turns it; the dots sit under the book.
local back, onward, dots = control("PrevPage"), control("NextPage"), control("Dots")
assert(onward.ActualWidth >= 40 and count(onward) == 0 and onward.BackgroundColor.A == 0 and onward.ActualHeight == book.ActualHeight
    and screenLeft(onward) + onward.ActualWidth == screenLeft(book) + book.ActualWidth, "the right margin turns the page onward, and shows nothing")
assert(screenLeft(back) == screenLeft(book) and back.ActualWidth >= 40, "the left margin turns it back")
local foot = screenTop(book) + book.ActualHeight
assert(screenTop(dots) >= foot and math.abs(screenTop(dots) + dots.ActualHeight / 2 - (foot + 768) / 2) < 1
    and math.abs(screenLeft(dots) + dots.ActualWidth / 2 - screenLeft(book) - book.ActualWidth / 2) < 1,
    "the dots sit midway between the book and the foot of the screen")
local edge = screenLeft(sections) + sections.ActualWidth
assert(screenLeft(control("Step2")) == edge and control("Shade", control("Step2")).Visible, "a resting bookmark is tucked behind the book, in its shade")
assert(screenLeft(control("Step1")) < edge and not control("Shade", control("Step1")).Visible
    and screenLeft(control("Step1")) + control("Step1").ActualWidth == screenLeft(control("Step2")) + control("Step2").ActualWidth,
    "the current bookmark reaches onto the page")

-- The left page reads, the right page shows.
local text, side = part("TextPage", 1), part("SidePage", 1)
assert(screenLeft(side) > screenLeft(text) + text.ActualWidth and math.abs(side.ActualWidth - text.ActualWidth) < 2, "two equal pages")
local image = part("SectionImage", 1)
assert(part("SectionMedia", 1).Visible and image.Src == "encarta/Images/moon-phases.jpg", "a question shows its own diagram")
assert(part("SectionCaption", 1).Text:match("Northern Hemisphere"))
assert(image.ActualWidth > 300 and math.abs(image.ActualWidth / image.ActualHeight - 1.5) < 0.02, "the artwork keeps its proportions")
assert(part("SectionImage", 3).Src == "encarta/Images/moon.jpg", "a question without its own picture shows the article's")
assert(not part("SectionMedia", 2).Visible, "the picture opens a question; its last spread holds the extras")
assert(fits(1), "the first spread fits the book without scrolling")
local middle = screenLeft(book) + book.ActualWidth / 2
assert(math.abs((screenLeft(text) - screenLeft(book)) - (middle - screenLeft(text) - text.ActualWidth)) < 1
    and math.abs((screenLeft(side) - middle) - (screenLeft(book) + book.ActualWidth - screenLeft(side) - side.ActualWidth)) < 1,
    "each page has the same margin at the fold as at its outer edge")
local fold = control("Fold")
local words = control("Paragraphs", spread(1))
assert(screenLeft(words) >= screenLeft(back) + back.ActualWidth
    and screenLeft(image) + image.ActualWidth <= screenLeft(onward), "the page margins stay clear of the text and the picture")
assert(fold.Opacity == 1 and math.abs(fold.ActualHeight - book.ActualHeight) < 1
    and math.abs(screenLeft(fold) + fold.ActualWidth / 2 - screenLeft(book) - book.ActualWidth / 2) < 1, "the fold shades the middle of the book")
assert(not control("PrevPage").Visible and control("NextPage").Visible)
assert(count(control("Dots")) == 2, "a dot per spread of the question")

-- Turning pages.
click("NextPage")
assert(shown() == spread(2) and current() == 1 and part("Kicker", 2).Text == "QUESTION 1 OF 5  ·  PAGE 2 OF 2")
assert(not part("Title", 2).Visible and part("Running", 2).Visible and part("Running", 2).Text == part("Title", 1).Text,
    "later spreads repeat the question as a running head")
assert(control("PrevPage").Visible and fits(2))
assert(control("Dot2").ActualWidth > control("Dot1").ActualWidth, "the current spread's dot is the long one")
assert(part("NextUp", 2).Visible and part("NextUpText", 2).Text == "How far away is the Moon?")
assert(part("Wow", 2).Visible and not part("Try", 2).Visible and not part("Related", 2).Visible, "extras appear only when written")
click("NextUp", spread(2))
assert(shown() == spread(3) and current() == 2, "the last spread leads to the next question")
click("Step4")
assert(shown() == spread(7) and current() == 4, "a bookmark opens its question")
click("PrevPage")
assert(shown() == spread(6) and current() == 3)
click("Step5")
click("NextPage")
assert(shown() == spread(10) and not control("NextPage").Visible, "the book ends at its last spread")
assert(part("NextUpTag", 10).Text == "THE END" and part("NextUpText", 10).Text == "Back to the library")
click("Step1")
assert(shown() == spread(1) and not control("PrevPage").Visible)

-- Real touches, sent through hit testing: a sideways swipe turns the page,
-- and so does a tap on a page's outer margin.
local function touch(message, x, y)
    orca.system.dispatchMessage { target = view, message = message, x = x, y = y }
    core.flushQueue()
end
local function swipe(x1, y1, x2, y2)
    touch("LeftButtonDown", x1, y1)
    touch("LeftButtonDragged", (x1 + x2) / 2, (y1 + y2) / 2)
    touch("LeftButtonDragged", x2, y2)
    touch("LeftButtonUp", x2, y2)
end
local function tap(x, y)
    touch("LeftButtonDown", x, y)
    touch("LeftButtonUp", x, y)
end
local midX, midY = screenLeft(book) + book.ActualWidth * 0.3, screenTop(book) + book.ActualHeight * 0.6
swipe(midX, midY, midX + 200, midY)
assert(shown() == spread(1), "the first spread has nothing before it")
swipe(midX + 200, midY, midX, midY)
assert(shown() == spread(2) and current() == 1, "swiping left turns the page onward")
swipe(midX + 20, midY, midX, midY)
swipe(midX, midY - 150, midX + 30, midY + 150)
assert(shown() == spread(2), "a short or an up-and-down drag does not turn the page")
-- This swipe ends over the card that leads to the next question.
local card = part("NextUp", 2)
local cardX, cardY = screenLeft(card) + card.ActualWidth / 2, screenTop(card) + card.ActualHeight / 2
swipe(cardX - 200, cardY, cardX, cardY)
assert(shown() == spread(1), "swiping right turns back, and letting go is not a tap")
tap(screenLeft(book) + book.ActualWidth - 20, midY)
assert(shown() == spread(2), "a tap on the right margin turns the page")
tap(cardX, cardY)
assert(shown() == spread(3) and current() == 2, "after a swipe, taps reach the page again")
tap(screenLeft(book) + 20, midY)
assert(shown() == spread(2), "a tap on the left margin turns back")
-- The bar and the bookmarks answer real taps too: nothing lies over them.
local function tapOn(node) tap(screenLeft(node) + node.ActualWidth / 2, screenTop(node) + node.ActualHeight / 2) end
tapOn(control("Step3"))
assert(shown() == spread(5) and current() == 3, "a bookmark answers a tap")
tapOn(control("Back"))
assert(library.Visible and not reader.Visible, "Back answers a tap")
click("Open_moon", wall)
layout(1024, 768)

-- On a narrow screen the pages stack and the spread scrolls.
layout(600, 900)
text, side = part("TextPage", 1), part("SidePage", 1)
assert(control("Fold").Opacity == 0, "stacked pages have no fold")
assert(screenTop(side) > screenTop(text) and side.ActualWidth > 420 and math.abs(side.ActualWidth - text.ActualWidth) < 2, "pages stack when narrow")
-- On an upright iPad they stack too, and the picture fills the width.
layout(1024, 1366)
text, side = part("TextPage", 1), part("SidePage", 1)
assert(screenTop(side) > screenTop(text) and side.ActualWidth > 800 and control("Fold").Opacity == 0, "pages stack when the screen is upright")
assert(part("SectionImage", 1).ActualWidth > 800 and fits(1), "the stacked spread fills the upright screen without scrolling")
layout(1024, 768)

local listen, listenIcon, listenLabel = control("ReadAloud"), control("ReadAloudIcon"), control("ReadAloudLabel")
local narration = reading.readAloud
local timings = require "encarta/Narration/Timings"
local function icon(name) return listenIcon.Source == "encarta/Icons/" .. name .. ".svg" end
assert(listen.Visible and listen.ActualHeight >= 44 and listen.ActualWidth < 200 and icon("volume-2"))
click("ReadAloud")
assert(playing and listenLabel.Text == "Pause" and icon("pause") and control("StopReading").Visible)
local function hear(mark)
    position = (mark[1] + mark[2]) / 2000
    narration:update()
    core.flushQueue()
end
local first = timings.moon[1].words
local title = part("Title", 1)
hear(first[1])
assert(title.HighlightedRange.X == first[1][3] and title.HighlightedRange.Y == first[1][4])
hear(first[2])
assert(title.HighlightedRange.X == first[2][3] and title.HighlightedRange.Y == first[2][4])

click("ReadAloud")
assert(not playing and listenLabel.Text == "Resume" and icon("play") and title.HighlightedRange.Y == first[2][4])
click("ReadAloud")
assert(playing and opened == 1, "Resume reuses the same recording and position")
hear(timings.moon[2].words[1])
assert(title.HighlightedRange.Y == 0 and shown() == spread(1))
hear(timings.moon[3].words[1])
assert(playing and shown() == spread(2), "the narration turns the page itself")
hear(timings.moon[7].words[1])
assert(part("Title", 5).HighlightedRange.Y > 0 and shown() == spread(5) and current() == 3, "the bookmarks follow the narration")
hear(timings.moon[8].words[1])
local paragraph = part("Paragraphs", 5)
local top = screenTop(paragraph) - screenTop(spread(5))
assert(playing and top >= 0 and top < spread(5).ActualHeight and shown() == spread(5), "narration keeps its word in view")

click("StopReading")
assert(not playing and closed == 1 and listenLabel.Text == "Read aloud" and icon("volume-2"))
click("ReadAloud")
assert(playing and opened == 2 and position == 0)
click("Step2")
assert(not playing and closed == 2 and listenLabel.Text == "Read aloud", "turning to a question by hand stops the narration")
click("Back")
assert(library.Visible and not reader.Visible and control("ResultCount").Text == "1 ARTICLE")

search.Text = "no-such-article"
core.flushQueue()
assert(control("EmptyResults").Visible and control("ResultCount").Text == "0 ARTICLES")
assert(control("ClearSearch").Visible, "Clear appears once there is something to clear")
click("ClearSearch")
assert(control("Featured").Visible and not control("EmptyResults").Visible and not control("ClearSearch").Visible)
click("OpenFeatured")
assert(control("ArticleTitle").Text == "Octopuses")
assert(part("SectionImage", 1).Src == "encarta/Images/octopus.jpg")
layout(1024, 768)
assert(shown() == spread(1) and current() == 1, "a new article starts at its first spread")

-- Numbered spots sit on the picture where the article puts them; tapping one
-- opens its note.
local picture = part("Picture", 1)
local function center(node)
    return (screenLeft(node) + node.ActualWidth / 2 - screenLeft(picture)) / picture.ActualWidth,
        (screenTop(node) + node.ActualHeight / 2 - screenTop(picture)) / picture.ActualHeight
end
for i, at in ipairs({ { 0.15, 0.48 }, { 0.55, 0.77 }, { 0.80, 0.29 } }) do
    local spot = part("Spot" .. i, 1)
    local x, y = center(spot)
    assert(control("Mark", spot).Text == tostring(i) and spot.ActualWidth >= 44 and spot.ActualHeight >= 44, "a spot is big enough to tap")
    assert(math.abs(x - at[1]) < 0.01 and math.abs(y - at[2]) < 0.01, ("spot %d sits at %.2f, %.2f"):format(i, x, y))
end
assert(not part("SpotNote", 1).Visible and part("SectionCaption", 1).Text:match("Tap the numbers"))
-- A real tap, to be sure the spot is hit where it is drawn.
local second = part("Spot2", 1)
tap(screenLeft(second) + second.ActualWidth / 2, screenTop(second) + second.ActualHeight / 2)
layout(1024, 768)
assert(part("SpotNote", 1).Visible and part("SpotTitle", 1).Text == "2  Suckers" and part("SpotText", 1).Text:match("Round cups"))
local chosen, other = control("Mark", part("Spot2", 1)).BackgroundColor, control("Mark", part("Spot1", 1)).BackgroundColor
assert(chosen.R > 0.8 and chosen.B < 0.3 and other.B > 0.9, "the chosen spot is filled")
assert(fits(1), "the spread still fits with a note open")

-- A question's last spread carries its written extras.
click("NextPage")
layout(1024, 768)
assert(part("Wow", 2).Visible and part("WowText", 2).Text:match("smell what it touches") and not part("Try", 2).Visible)
assert(part("Try", 4).Visible and part("TryText", 4).Text:match("balloon") and part("Wow", 4).Visible)
assert(not part("Related", 4).Visible and part("Related", 8).Visible, "See also closes the article")
for n = 1, 8 do
    reading:setPage(n)
    layout(1024, 768)
    assert(fits(n), "octopus spread " .. n .. " fits the book")
end
click("Related", spread(8))
assert(control("ArticleTitle").Text == "Penguins" and shown() == spread(1), "See also opens the related article")
click("Back")
assert(control("ArticleTitle").Text == "Octopuses" and shown() == spread(8), "Back returns to the spread you left")
reading:setPage(1)
click("ReadAloud")
hear(timings.octopus[1].words[1])
assert(playing and part("Title", 1).HighlightedRange.Y > 0)
local octopusTitle = part("Title", 1)
click("Back")
assert(not playing and octopusTitle.HighlightedRange.Y == 0 and not control("StopReading").Visible)

click("Open_pyramids", wall)
assert(control("ArticleTitle").Text == "The Pyramids" and part("SectionImage", 1).Src == "encarta/Images/pyramids.jpg")
assert(not control("ReadAloud").Visible, "articles without a recording hide Read aloud")
click("Back")
click("Open_earth-layers", wall)
assert(control("ArticleTitle").Text == "Inside Earth" and part("SectionImage", 1).Src == "encarta/Images/earth-layers.jpg")
click("Back")

-- Every spread of every article fits the book at the iPad size, with a
-- picture note open where there is one.
for _, article in ipairs(require "encarta/Content/Articles") do
    reading:openArticle(article.id)
    for n in ipairs(reading.spreads) do
        reading:setPage(n)
        local spot = spread(n):findChild("Spot1", true)
        if spot then spot:send("Node.LeftButtonUp") end
        layout(1024, 768)
        local art, image = spread(n):findChild("Picture", true), article.sections[reading.spreads[n].section].image or article.image
        for i, at in ipairs(reading.spreads[n].page == 1 and image.spots or {}) do
            local mark = control("Spot" .. i, spread(n))
            local x = (screenLeft(mark) + mark.ActualWidth / 2 - screenLeft(art)) / art.ActualWidth
            local y = (screenTop(mark) + mark.ActualHeight / 2 - screenTop(art)) / art.ActualHeight
            assert(math.abs(x - at[1]) < 0.01 and math.abs(y - at[2]) < 0.01,
                ("%s spot %d sits at %.2f, %.2f instead of %.2f, %.2f"):format(article.id, i, x, y, at[1], at[2]))
        end
        assert(fits(n), ("%s spread %d fits the book (%d of %d)"):format(article.id, n, spread(n).ScrollHeight, spread(n).ActualHeight))
    end
    reading:goBack()
end

local function checkArtwork(path)
    local bytes = assert(fs.readTextFile(path), path)
    assert(bytes:sub(1, 2) == string.char(0xff, 0xd8), path .. " must be a bundled JPEG")
end
for _, article in ipairs(require "encarta/Content/Articles") do
    assert(article.question and article.image.caption ~= "", article.id)
    assert(#article.hunt == 2 and article.hunt[1]:sub(-1) == "?" and article.hunt[2] ~= "", article.id .. " has a riddle and its fact")
    checkArtwork(article.image.path)
    if article.cover then checkArtwork(article.cover) end
    assert(not article.related or article.related ~= article.id and (function()
        for _, other in ipairs(require "encarta/Content/Articles") do
            if other.id == article.related then return true end
        end
    end)(), article.id .. " names a related article that exists")
    for _, spot in ipairs(article.image.spots or {}) do
        assert(spot[1] > 0 and spot[1] < 1 and spot[2] > 0 and spot[2] < 1 and spot[3] ~= "" and spot[4] ~= "", article.id)
    end
    for _, section in ipairs(article.sections) do
        if section.image then checkArtwork(section.image.path) end
    end
end

local media = require "orca.media"
local invalid, err = media.openAudio("not an audio file")
assert(not invalid and err.code)
for name in pairs(timings) do
    local bytes = assert(fs.readTextFile("encarta/Narration/" .. name .. ".wav"))
    local audio, failure = media.openAudio(bytes)
    if not audio then assert(failure.code == "unsupported") else
        local time, active = audio:status()
        assert(time == 0 and not active)
        audio:close()
    end
end

os.remove(passportPath)
print("PASS: Encarta study, question hunt and passport, question wall, illustrated bookshelf, paged book reader, offline recordings and synchronized highlights")
