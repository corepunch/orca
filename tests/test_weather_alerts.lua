-- Weather's calm notifications (N2, apps/Weather/alerts.lua): each rule on
-- a synthetic forecast, then the plan (switches, quiet hours, one per kind
-- a day), the sync with a stand-in for orca.notifications, the settings
-- page and the background refresh hook. No network, no device.
local F = dofile("tests/weather_fixture.lua").init()
local check, NOW = F.check, F.NOW
local Forecast = require "Weather/forecast"
local Activities = require "Weather/activities"
local Alerts = require "Weather/alerts"
local Units = require "Weather/units"
Units.use {}

local HOUR = 3600
local function forecastAt(place, extra, marine)
    local forecast = Forecast.parse(F.weather(place.lat, extra), marine and F.marine() or nil, NOW, place)
    forecast.spot = place
    return forecast
end
local home = { id = "home", title = "Home", lat = 47.0, lon = 0.0, activities = { "garden", "hiking", "stargazing", "sailing" } }
local function only(kind, list)
    for _, alert in ipairs(list) do if alert.kind == kind then return alert end end
end
local function iso(epoch) return os.date("!%Y-%m-%dT%H:%M", epoch) end
local ON = { on = true, rain = true, brief = true, frost = true, storm = true, wind = true, dawn = true, clear = true,
    quietFrom = 22, quietTo = 7 }
local function setting(values) return function(key) return values[key] end end

check(#Alerts.kinds() == 7 and Alerts.byId.frost.activity == "garden", "The full app offers every kind")

-- Rain soon: dry now, 80 % and 1.2 mm at 08:00.
local wet = forecastAt(home, {
    precipitation_probability = function(i) return i == 8 and 80 or 10 end,
    precipitation = function(i) return i == 8 and 1.2 or 0 end,
})
local rain = only("rain", Alerts.candidates(wet, home, NOW))
check(rain and rain.title == "Rain soon" and iso(rain.at) == "2026-11-18T07:30" and rain.body:find("about 08:00 at Home %(80%%%)"),
    "Rain soon is said half an hour before the wet hour")
local brief = only("brief", Alerts.candidates(wet, home, NOW))
check(brief and iso(brief.at) == "2026-11-18T07:00" and brief.title == "Today at Home"
    and brief.body == "15° / 7°, rain likely from about 08:00. Bring an umbrella.", "The morning brief at 7 says what to bring")
check(not only("frost", Alerts.candidates(wet, home, NOW)) and not only("storm", Alerts.candidates(wet, home, NOW)),
    "A mild, calm day raises no frost or storm")

-- Frost: 1.5 °C at 04:00 on a clear, calm night.
local frosty = forecastAt(home, {
    temperature_2m = function(i) return i == 28 and 1.5 or 8 end,
    cloud_cover = function() return 10 end, wind_speed_10m = function() return 1 end,
})
local frost = only("frost", Alerts.candidates(frosty, home, NOW))
check(frost and frost.title == "Frost likely tonight" and iso(frost.at) == "2026-11-18T17:00"
    and iso(frost.before) == "2026-11-19T02:00" and frost.body:find("About 2° by 04:00 at Home, clear and calm"),
    "Frost likely tonight at 17:00, from 2 °C on a clear, calm night")
local breezy = forecastAt(home, { temperature_2m = function(i) return i == 28 and 3 or 8 end })
check(not only("frost", Alerts.candidates(breezy, home, NOW)), "3 °C under cloud and wind is no frost")
local hard = forecastAt(home, { temperature_2m = function(i) return i == 28 and -1 or 8 end })
check(only("frost", Alerts.candidates(hard, home, NOW)).title == "Frost likely tonight", "Below freezing is frost whatever the sky")

-- The storm clock: thunder likely at 14:00.
local stormy = forecastAt(home, {
    cape = function(i) return i == 14 and 1500 or 50 end,
    precipitation_probability = function(i) return i == 14 and 50 or 10 end,
})
local storm = only("storm", Alerts.candidates(stormy, home, NOW))
check(storm and storm.title == "Thunder likely from about 14:00" and iso(storm.at) == "2026-11-18T11:00"
    and storm.body:find("off high ground by 13:00") and storm.body:find("an estimate"), "The storm clock warns three hours ahead")

-- A clear night: under 20 % cloud while the sun is 12° down.
local clearSky = forecastAt(home, { cloud_cover = function() return 5 end })
local clear = only("clear", Alerts.candidates(clearSky, home, NOW))
check(clear and clear.title:find("^Clear tonight from ") and clear.at == clear.before - 2 * HOUR
    and clear.body:find("dark and under 20%% cloud"), "A clear night is said two hours before dark")
check(not only("clear", Alerts.candidates(wet, home, NOW)), "Half cloud is no clear night")

-- Good wind and dawn patrol come from their activities' own ratings.
local windy = forecastAt(home, {
    wind_speed_10m = function(i) return (i >= 34 and i <= 37) and 7 or 1 end,
    wind_gusts_10m = function(i) return (i >= 34 and i <= 37) and 9 or 2 end,
})
local wind = only("wind", Alerts.candidates(windy, home, NOW))
check(wind and wind.title:find("^Good wind tomorrow") and iso(wind.at) == "2026-11-18T19:00"
    and iso(wind.before) == "2026-11-19T10:00" and wind.body:find("from 10:00"),
    "A good wind window tomorrow is said the evening before")
local surf = Activities.byId.surf
local dawnPatrol = surf.dawnPatrol
surf.dawnPatrol = function() return { title = "dawn patrol tomorrow", value = "first light 06:15", detail = "good from 07:00",
    tone = "good" } end
local dawn = only("dawn", Alerts.candidates(wet, home, NOW))
check(dawn and iso(dawn.at) == "2026-11-18T20:00" and dawn.body == "first light 06:15. good from 07:00",
    "A good dawn tomorrow is said at 20:00")
surf.dawnPatrol = function() return { title = "dawn patrol tomorrow", value = "x", detail = "y", tone = "poor" } end
check(not only("dawn", Alerts.candidates(wet, home, NOW)), "A poor dawn says nothing")
surf.dawnPatrol = function() error("broken rule") end
check(only("rain", Alerts.candidates(wet, home, NOW)), "A failing rule never stops the others")
surf.dawnPatrol = dawnPatrol

-- The plan: the switch, each kind's switch, quiet hours, one a day.
local candidates = Alerts.candidates(wet, home, NOW)
check(#Alerts.plan(candidates, { now = NOW, setting = setting { on = false, rain = true } }) == 0,
    "Nothing is planned until notifications are on")
local plan = Alerts.plan(candidates, { now = NOW, setting = setting { on = true, rain = true, quietFrom = 22, quietTo = 7 } })
check(#plan == 1 and plan[1].id == "rain:2026-11-18" and plan[1].kind == "rain", "Only switched-on kinds are planned")
local late = { { kind = "rain", title = "a", body = "b", at = NOW + 17 * HOUR } }  -- 23:00
local moved = Alerts.plan(late, { now = NOW, setting = setting(ON) })
check(#moved == 1 and iso(moved[1].at) == "2026-11-19T07:00" and moved[1].id == "rain:2026-11-19",
    "An alert in quiet hours waits for their end")
late[1].before = NOW + 20 * HOUR
check(#Alerts.plan(late, { now = NOW, setting = setting(ON) }) == 0, "and is dropped when that is too late to help")
check(Alerts.quiet(NOW + 2 * HOUR, 7, 7) == false and Alerts.quiet(NOW + 6 * HOUR, 9, 17) == true,
    "Equal quiet hours mean none; a daytime span works too")
local twice = { { kind = "rain", title = "late", body = "", at = NOW + 8 * HOUR },
    { kind = "rain", title = "early", body = "", at = NOW + 3 * HOUR } }
local once = Alerts.plan(twice, { now = NOW, setting = setting(ON) })
check(#once == 1 and once[1].title == "early", "At most one of a kind a day, the earliest")
check(#Alerts.plan(twice, { now = NOW, setting = setting(ON), delivered = { ["rain:2026-11-18"] = "2026-11-18" } }) == 0,
    "A kind already said today stays quiet")
check(#Alerts.plan({ { kind = "rain", title = "", body = "", at = NOW + 40 * HOUR } }, { now = NOW, setting = setting(ON) }) == 0,
    "Nothing is planned beyond the horizon")

-- Sync: schedules through the notifier only when the plan changes, and
-- an alert whose time passed counts as said.
local store = F.storage("alerts")
store:remove(Alerts.FILE)
local sent = { scheduled = {}, cancelled = {}, cancelAll = 0, refresh = 0 }
local notifier = {
    scheduleLocal = function(alert) sent.scheduled[#sent.scheduled + 1] = alert; return true end,
    cancelLocal = function(id)
        if id then sent.cancelled[#sent.cancelled + 1] = id else sent.cancelAll = sent.cancelAll + 1 end
        return true
    end,
    scheduleRefresh = function(seconds) sent.refresh = seconds; return true end,
}
local options = { now = NOW, storage = store, notifier = notifier, setting = setting(ON) }
local first, changed = Alerts.sync(wet, home, options)
check(changed and #sent.scheduled == #first and #first >= 2 and #sent.cancelled == 0 and sent.refresh == Alerts.REFRESH,
    "Sync schedules the plan and asks for a background refresh")
check(sent.scheduled[1].id == "brief:2026-11-18" and sent.scheduled[1].at == first[1].at and sent.scheduled[1].title,
    "Each alert goes out with its id, time, title and body")
local _, again = Alerts.sync(wet, home, options)
check(not again and #sent.scheduled == #first, "The same plan is not scheduled again")
options.now = NOW + 75 * 60   -- 07:15: the brief went out
local later = Alerts.sync(wet, home, options)
check(only("brief", later).id == "brief:2026-11-19" and store:load(Alerts.FILE).delivered["brief:2026-11-18"] == "2026-11-18",
    "A sent alert is remembered for its day; tomorrow's brief comes next")
Alerts.clear { notifier = notifier, storage = store }
check(#store:load(Alerts.FILE).planned == 0 and sent.cancelAll == 1, "Turning off cancels what was planned")
-- A plan that drops an alert cancels just that one.
Alerts.sync(wet, home, { now = NOW, storage = store, notifier = notifier, setting = setting(ON) })
local dry = forecastAt(home)
Alerts.sync(dry, home, { now = NOW, storage = store, notifier = notifier, setting = setting(ON) })
check(sent.cancelled[#sent.cancelled] == "rain:2026-11-18", "Rain no longer coming cancels only its alert")

-- The background hook: the home place fetched afresh, then the same sync.
local net = F.network()
local refreshed = Alerts.refresh { home = home, fetch = net.fetch, now = NOW, storage = store, notifier = notifier,
    setting = setting(ON) }
check(refreshed and #net.requests >= 1 and store:load("forecast-home"), "The background refresh fetches and plans")
net.failing = true
check(Alerts.refresh { home = home, fetch = net.fetch, now = NOW, storage = store, notifier = notifier } == nil,
    "Offline the refresh gives up quietly")

-- In the app: the settings page lists the switches under "notifications";
-- turning them on asks the system once and plans the home place.
local asked = 0
notifier.requestLocalAuthorization = function() asked = asked + 1; return true end
sent.scheduled = {}
local appStore = F.storage("alerts-app", F.PLACES)
appStore:remove(Alerts.FILE)
local ui = F.launch { storage = appStore, notifier = notifier }
ui.press("OpenSettings")
check(ui.find("Settingsettings:alerts:on"):findChild("SettingHeading", true).Text == "notifications"
    and ui.text("Settingsettings:alerts:on", "SettingValue") == "off"
    and ui.text("Settingsettings:alerts:quietFrom", "SettingValue") == "22:00", "Settings list the notifications, off")
ui.find("Settingsettings:alerts:on"):findChild("SettingChange", true):send("Node.LeftButtonUp")
ui.settle()
check(ui.text("Settingsettings:alerts:on", "SettingValue") == "on" and asked == 1, "Turning them on asks the system once")
ui.back("SettingsPivot")
check(appStore:load(Alerts.FILE) ~= nil, "The home place's alerts are planned when its forecast is drawn")
ui.app.hubScope:dispose()

-- background.lua runs the same without screens.
local hook = dofile("apps/Weather/background.lua")
local bgNet = F.network()
check(hook { fetch = bgNet.fetch, notifier = notifier, clock = function() return NOW end } == true and #bgNet.requests >= 1,
    "background.lua refreshes the home place")

-- N3: the widget's snapshot. The app writes it when the home place's fresh
-- forecast is drawn, notifications on or off; background.lua writes it
-- too; a build without an App Group says so and nothing else changes.
local Widget = require "Weather/widget"
local snapshot = Widget.snapshot(wet, home, NOW)
check(snapshot.place == home.title and snapshot.temp:find("°$") and snapshot.verdict ~= "" and snapshot.line ~= ""
    and snapshot.updated == NOW and snapshot.version == 1, "A snapshot has the place, temperature, verdict and a line")
check(Widget.encode { b = 'say "hi"\n', a = 2 } == [[{"a":2,"b":"say \"hi\"\n"}]], "Snapshots are JSON, keys sorted")
local widgets = {}
local widgetNotifier = setmetatable({ writeWidgetSnapshot = function(json) widgets[#widgets + 1] = json; return true end },
    { __index = notifier })
check(Widget.write(wet, home, { notifier = widgetNotifier, now = NOW }) == widgets[1] and widgets[1]:find('"place":"', 1, true),
    "Widget.write hands the JSON to orca.notifications")
check(select(2, Widget.write(wet, home, { notifier = { writeWidgetSnapshot = function() return nil, "no App Group" end } }))
    == "no App Group" and select(2, Widget.write(wet, home, {})) == "no widget", "Without an App Group it says why")
widgets = {}
local widgetStore = F.storage("alerts-widget", F.PLACES)
local wui = F.launch { storage = widgetStore, notifier = widgetNotifier }
check(#widgets >= 1 and widgets[#widgets]:find('"place":"', 1, true), "The app writes the widget for its home place")
wui.app.hubScope:dispose()
widgets = {}
check(hook { fetch = F.network().fetch, notifier = widgetNotifier, clock = function() return NOW end } == true
    and #widgets == 1, "background.lua writes the widget too")

F.done("weather-alerts")
