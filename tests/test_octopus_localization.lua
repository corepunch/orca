-- Real EN/RU app routes, native XML resources, provider labels and untrusted content.
local orca = require "orca"
orca.init()
local fs, core, ui = require "orca.filesystem", require "orca.core", require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
orca.async = function(fn, ...) return fn(...) end
local Locales = require "Octopus/Locales"
local Provider = require "Octopus/providers/octopus"
local Format = require "Octopus/format"
local ru, en = Locales.new("ru-RU"), Locales.new("en-GB")
local english, russian = require "Octopus/Locales/en", require "Octopus/Locales/ru"
local function variables(value)
    local names = {}
    for name in value:gmatch("%%{([%w_]+)}") do names[name] = true end
    local ordered = {}
    for name in pairs(names) do ordered[#ordered + 1] = name end
    table.sort(ordered)
    return table.concat(ordered, ",")
end
for key, source in pairs(english) do
    local translated = assert(russian[key], "Missing Russian resource: " .. key)
    assert(type(source) == type(translated), "Resource shape differs: " .. key)
    local expected = variables(type(source) == "table" and source.other or source)
    for _, forms in ipairs { source, translated } do
        if type(forms) == "string" then forms = { forms } end
        for _, value in pairs(forms) do
            assert(variables(value) == expected, "Interpolation variables differ: " .. key)
        end
    end
end
for key in pairs(russian) do assert(english[key] ~= nil, "Missing English resource: " .. key) end
local computed = Provider.new { i18n = ru, client = {}, now = function() return 1761400000 end }.computed
assert(computed.kindLabel { PostType = "text" } == "запись")
assert(computed.composeHeading { Kind = "quote" } == "новая цитата")
for count, suffix in pairs { [1] = "комментарий", [2] = "комментария", [5] = "комментариев", [11] = "комментариев", [21] = "комментарий", [22] = "комментария" } do
    assert(computed.commentsLabel { Comments = count } == suffix)
end
assert(computed.repostedText { RepostedByName = "home" } == "home: репост")
assert(computed.plain { Content = "Sign in" } == "Sign in")
assert(computed.personName { DisplayName = "home" } == "home")
assert(computed.followersText { Followers = 1500 } == "1,5 тыс.")
assert(Format.ago("2026-10-01T00:00:00Z", Format.epoch("2026-10-01T00:05:00Z"), ru) == "5 мин")
assert(Format.ago("2026-10-01T00:00:00Z", Format.epoch("2026-10-10T00:00:00Z"), ru) == "1 окт")
assert(Format.stamp("2026-10-01T00:00:00Z", ru):find("окт", 1, true))
local raw = { post = { title = "Sign in", content = "home" }, read = function(self) return self.post end,
    fail = function() return nil, { code = "validation", message = "Enter your email and password." } end }
local wrapped = Locales.client(raw, ru)
assert(wrapped:read() == raw.post and wrapped:read().title == "Sign in")
local result, err = wrapped:fail()
assert(result == nil and err.code == "validation" and err.message == "Введите электронную почту и пароль.")

local function settle(screen)
    core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
end
local App, Octopus = require "Octopus/App", require "Octopus/api/octopus"
local app = App { client = Octopus.mock { clock = function() return 1761400000 end }, language = "ru" }
app.activate_controller = function(self, controller) self.screen = controller.view; return controller end
app.repaint = function() end
app:show("/"); settle(app.screen)
assert(app.pivot:findChild("Feedhome", true).Header == "лента")
assert(app.shell.bar:findChild("AppBarSearch", true).Text == "поиск")
local signIn = assert(app.screen:findChild("SignInhome", true))
assert(core.getLocalizedString(signIn, signIn.TextResourceID) == "Войти")
app:show("/auth/signup"); settle(app.screen)
local password = assert(app.screen:findChild("Password", true))
assert(password.Type == "Password", "Input enum and field identity must stay untranslated")
assert(password.PlaceholderText == "Не менее 8 символов")
assert(app.screen:findChild("AuthTitle", true):getTextContent() == "присоединяйтесь к octopus")
app:show("/write"); settle(app.screen)
local title = assert(app.screen:findChild("PostTitle", true))
assert(title.PlaceholderText == "Добавьте заголовок")
local publish = assert(app.screen:findChild("Publish", true))
assert(publish.TextResourceID == "Publish")
assert(core.getLocalizedString(publish, publish.TextResourceID) == "Опубликовать")
assert(app.client:signIn("demo@example.invalid", "password123"))
app:resetSessionViews(); app:show("/me"); settle(app.screen)
local choose = assert(app.screen:findChild("ChooseAvatar", true))
assert(core.getLocalizedString(choose, choose.TextResourceID) == "Выбрать персонажа")
choose:send("Node.LeftButtonUp"); settle(app.screen)
assert(choose.TextResourceID == "Hide characters")
assert(core.getLocalizedString(choose, choose.TextResourceID) == "Скрыть персонажей")
choose:send("Node.LeftButtonUp"); settle(app.screen)
assert(choose.TextResourceID == "Choose a character")
-- An arbitrary user's profile title is kept even when it equals a UI key.
local userTitle = ui.PivotItem { Header = "home" }
local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
screen:addChild(userTitle); ru:apply(screen)
assert(userTitle.Header == "home")
print("PASS: Octopus Russian routes, labels, service errors, plurals, dates and preserved user content")
