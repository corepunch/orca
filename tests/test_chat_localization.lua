-- Real XML/Lua views and mock data: locale captions must not modify service content.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local I18n = require "Chat/i18n"
local App = require "Chat/App"
local Chat = require "api.chat"
local Provider = require "Chat/providers/chat"
local en, ru = require "Chat/Locales/en", require "Chat/Locales/ru"
local function placeholders(value)
    local names = {}
    local function collect(text)
        for name in text:gmatch("%%{([%w_]+)}") do names[name] = true end
    end
    if type(value) == "table" then for _, text in pairs(value) do collect(text) end else collect(value) end
    return names
end
for key, value in pairs(en) do
    assert(ru[key] ~= nil, "Missing Russian Chat key: " .. key)
    local a, b = placeholders(value), placeholders(ru[key])
    for name in pairs(a) do assert(b[name], key .. " lost placeholder " .. name) end
    for name in pairs(b) do assert(a[name], key .. " added placeholder " .. name) end
end
for key in pairs(ru) do assert(en[key] ~= nil, "Missing English Chat key: " .. key) end
local queue, app = {}, nil
local oldAsync = orca.async
orca.async = function(fn, ...)
    local args = table.pack(...)
    queue[#queue + 1] = coroutine.create(function() fn(table.unpack(args, 1, args.n)) end)
end
local function settle()
    repeat
        core.flushQueue()
        local count = #queue
        while #queue > 0 do
            local co = table.remove(queue, 1)
            while coroutine.status(co) ~= "dead" do local ok, err = coroutine.resume(co); assert(ok, err) end
        end
    until count == 0
    core.advanceFrame()
    if app and app.screen then core.runAllPrograms(app.screen) end
    core.flushQueue()
end
local function construct(language, signedIn)
    local client = Chat.mock()
    if signedIn then assert(client:signIn("demo@example.invalid", "password123")) end
    local app = App { client = client, language = language }
    app.activate_controller = function(self, controller) self.screen = controller.view; return controller end
    app.show = function(self, route) self:activate_controller(self:dispatch(route or "/")) end
    return app
end
app = construct("ru-RU", false)
app:show("/auth/email")
settle()
assert(app.screen:findChild("AuthTitle", true):getTextContent() == "вход", "language=" .. I18n.language .. " title=" .. tostring(app.screen:findChild("AuthTitle", true):getTextContent()) .. " t=" .. I18n:t("sign in"))
assert(app.screen:findChild("SignIn", true):getTextContent() == "Войти")
assert(app.screen:findChild("Email", true) and app.screen:findChild("Password", true), "Control identifiers stay stable")
assert(app.screen:findChild("Password", true).PlaceholderText == "Ваш пароль")
app.screen:findChild("SignIn", true):send("Node.LeftButtonUp")
settle()
assert(app.screen:findChild("AuthStatus", true):getTextContent() == "Введите почту и пароль.", "Client validation errors are localized")
assert(I18n:initials("игорь чернаков") == "ИЧ")
assert(I18n:date("2026-11-03T12:00:00Z") == "вт, 03 ноя")
assert(I18n:t("photos.ready", { count = 1, kb = 2 }) == "1/9 фото готово · 2 КБ")
assert(I18n:t("photos.ready", { count = 2, kb = 2 }) == "2/9 фото готовы · 2 КБ")
assert(I18n:t("photos.ready", { count = 5, kb = 2 }) == "5/9 фото готовы · 2 КБ")
assert(Provider.historyText({ gap = true }) == "Новые сообщения · перейти")

app = construct("ru", true)
app:show("/")
settle()
assert(app.pivot.Title == "Мессенджер")
assert(app.pivot:findChild("FilterUnread", true).Header == "Непрочитанные")
assert(app.screen:findChild("Search", true).PlaceholderText == "поиск")
local empty = app.screen:findChild("InboxEmptyAll", true):getFirstChild()
assert(core.getLocalizedString(empty, empty.TextResourceID) == "новых сообщений нет")
app.scope:member("Matching"):set("Search", "nobody")
settle()
assert(core.getLocalizedString(empty, empty.TextResourceID) == "чаты не найдены", "Conditional captions remain localized")
app.room = { id = "alex", name = "settings", kind = "direct", can_post = true, role = "owner" }
app:show("/thread")
settle()
assert(app.screen:findChild("ThreadPage", true):getFirstChild().Header == "settings", "User room names are not catalog keys")
assert(app.screen:findChild("Message", true).PlaceholderText == "напишите сообщение")
local message = app.threadScope.store:get("ChatMessage", "a1")
assert(message.values.Body == "Hey! How's your morning going?", "Service messages are never translated")
local composer = app.screen:findChild("EditingMessage", true):getFirstChild()
assert(core.getLocalizedString(composer, composer.TextResourceID) == "Редактирование сообщения")
app:show("/details")
settle()
local member = app.routeScope.store:get("ChatMember", "alex:alex")
assert(member.values.Role == "member" and member.values.RoleText == "участник", "Membership protocol roles remain stable while labels translate")
local remove = app.screen:findChild("Remove", true)
assert(core.getLocalizedString(remove, remove.TextResourceID) == "Удалить", "Late-created XML template captions inherit the locale")
app:closePages()
if app.scope then app.scope:dispose() end

app = construct("de-DE", false)
app:show("/auth/email")
settle()
assert(I18n.language == "en" and app.screen:findChild("SignIn", true):getTextContent() == "Sign in", "Unsupported languages fall back to English")
orca.async = oldAsync
print("Chat English/Russian localization passed")
