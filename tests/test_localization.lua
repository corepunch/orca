local core = require "orca.core"
local ui = require "orca.UIKit"
local Localization = require "orca.core.localization"
local catalogs = {
    en = { hello = "Hello, %{name}!", onlyEnglish = "Fallback", Home = "home",
        counter = { one = "%{count} message", other = "%{count} messages" } },
    ru = { hello = "Привет, %{name}!", Home = "главная",
        counter = { one = "%{count} сообщение", few = "%{count} сообщения", many = "%{count} сообщений", other = "%{count} сообщения" } },
}
local ru = Localization.new { locale = "ru_RU.UTF-8", catalogs = catalogs }
assert(ru.language == "ru")
assert(ru:t("hello", { name = "Игорь" }) == "Привет, Игорь!")
assert(ru:t("onlyEnglish") == "Fallback")
assert(ru:t("unknown") == "unknown")
for count, suffix in pairs { [0] = "сообщений", [1] = "сообщение", [2] = "сообщения", [5] = "сообщений", [11] = "сообщений", [21] = "сообщение", [22] = "сообщения", [25] = "сообщений", [111] = "сообщений" } do
    assert(ru:t("counter", {count = count}) == count .. " " .. suffix)
end
assert(ru:t("counter", {count = 1.5}) == "1.5 сообщения")
local en = Localization.new { locale = "fr-FR", catalogs = catalogs }
assert(en.language == "en")
assert(en:t("counter", 1) == "1 message")
assert(en:t("counter", 2) == "2 messages")
assert(type(core.getPreferredLanguage()) == "string")

local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
ru:apply(screen)
local label = screen + ui.TextBlock { TextResourceID = "Home", Text = "Home" }
assert(core.getLocalizedString(label, "Home") == "главная", "late-created template nodes inherit dictionary")
local second = screen + ui.Node2D {}
en:apply(second)
local nested = second + ui.TextBlock { TextResourceID = "Home" }
assert(core.getLocalizedString(nested, "Home") == "home", "nearest resource scope wins")
assert(core.getLocalizedString(label, "onlyEnglish") == "Fallback")
local untrusted = screen + ui.TextBlock { Text = "Home" }
ru:apply(screen)
assert(untrusted.Text == "Home", "user content is never source-matched")
-- Layout must consume the resource, rather than the static English Text.
local reference = screen + ui.TextBlock { Text = "главная" }
screen:UpdateLayout(390, 844)
assert(label.DesiredWidth == reference.DesiredWidth, "native layout uses localized caption")
local dynamic = screen + ui.TextBlock { Text = "Fallback" }
assert(dynamic:attachPropertyProgram("TextResourceID", 'IF(1, "Home", "onlyEnglish")', "OneWay", true))
core.runAllPrograms(screen)
screen:UpdateLayout(390, 844)
assert(dynamic.TextResourceID == "Home", "conditional resource key updates")
assert(dynamic.DesiredWidth == reference.DesiredWidth, "conditional resource binding resolves in layout: " .. dynamic.DesiredWidth .. " vs " .. reference.DesiredWidth)
ru:setLanguage("en-GB")
assert(core.getLocalizedString(label, "Home") == "главная", "selecting language never revisits retained navigation graphs")
ru:apply(screen)
assert(core.getLocalizedString(label, "Home") == "home", "explicitly reapplied dictionaries update language")
assert(en:t("hello", {name = "Alice"}) == "Hello, Alice!", "instances remain independent")
print("PASS: localization fallback, formatting, Russian plurals and scoped native resources")
