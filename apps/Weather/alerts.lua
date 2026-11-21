-- Calm notifications (N2): a few rules read the home place's forecast and
-- say one thing worth knowing ahead, at a sensible time. Everything here
-- is pure Lua; orca.notifications only delivers what `Alerts.plan` keeps.
--
--   Alerts.candidates(forecast, place, now)  every rule's suggestion
--   Alerts.plan(candidates, options)         toggles, quiet hours, one per kind a day
--   Alerts.sync(forecast, place, options)    plan, then (re)schedule through the notifier
--   Alerts.refresh(options)                  background: fetch home, then sync
--
-- A rule returns `{ title, body, at, before }` or nil: `at` is when to say
-- it (Unix seconds), `before` the latest time it still helps (the frost, the
-- first thunder, the wind). An activity may add its own with
-- `activity.alerts(forecast, place, now)` returning `{ kind = …, … }`
-- items; its kinds need an entry in `Alerts.KINDS`.
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Settings = require "Weather/settings"
local Activities = require "Weather/activities"
local Alerts = { GROUP = "alerts", FILE = "alerts", HORIZON = 36 * 3600, REFRESH = 3 * 3600 }

-- The kinds in the settings order. `activity` is whose forecast it reads;
-- a kind shows only where that activity is in the app.
Alerts.KINDS = {
    { id = "rain", activity = "outdoors", title = "rain soon", detail = "about half an hour before likely rain" },
    { id = "brief", activity = "outdoors", title = "morning brief", detail = "the day in one line, and what to bring" },
    { id = "frost", activity = "garden", title = "frost tonight", detail = "in the late afternoon, before a frosty night" },
    { id = "storm", activity = "hiking", title = "storm clock", detail = "hours before likely thunder (an estimate)" },
    { id = "wind", activity = "sailing", title = "good wind", detail = "the evening before, or hours before, a good window" },
    { id = "dawn", activity = "surf", title = "dawn patrol", detail = "the evening before a good first hour" },
    { id = "clear", activity = "stargazing", title = "clear night", detail = "before dark, when the night is clear" },
}
Alerts.byId = {}
for _, kind in ipairs(Alerts.KINDS) do Alerts.byId[kind.id] = kind end

-- The kinds this build offers: their activity is in it.
function Alerts.kinds()
    local list = {}
    for _, kind in ipairs(Alerts.KINDS) do
        if Activities.byId[kind.activity] then list[#list + 1] = kind end
    end
    return list
end

local function clockOf(v) return Units.clock(("%02d:00"):format(math.floor(v) % 24)) end

-- The settings group: one switch for all (off until the user asks), one per
-- kind (the edition's own and rain soon start on) and quiet hours.
function Alerts.group()
    local settings = {
        { key = "on", title = "notifications", kind = "toggle", default = false,
          detail = "for your home place: at most one of each kind a day, never in quiet hours" },
    }
    for _, kind in ipairs(Alerts.kinds()) do
        settings[#settings + 1] = { key = kind.id, title = kind.title, kind = "toggle", detail = kind.detail,
            default = kind.id == "rain" or kind.activity == Activities.edition }
    end
    settings[#settings + 1] = { key = "quietFrom", title = "quiet from", kind = "number", default = 22, min = 0, max = 23,
        step = 1, format = clockOf }
    settings[#settings + 1] = { key = "quietTo", title = "quiet until", kind = "number", default = 7, min = 0, max = 23,
        step = 1, format = clockOf }
    return { id = Alerts.GROUP, title = "notifications", settings = settings }
end
Alerts.settingsGroup = Alerts.group()
Settings.register(Alerts.settingsGroup)

-- Local time helpers: a forecast keeps its place's clock in `offset`.
local function localDate(epoch, offset) return os.date("!%Y-%m-%d", epoch + (offset or 0)) end
local function at(date, hour, offset) return Sky.epochOf(date) + hour * 3600 - (offset or 0) end
Alerts.localDate = localDate

local function ahead(forecast, now, seconds)
    local list = {}
    for i = forecast.now or 1, #(forecast.hours or {}) do
        local hour = forecast.hours[i]
        if hour.epoch and hour.epoch > now and hour.epoch <= now + seconds then list[#list + 1] = hour end
    end
    return list
end

local function placeName(place) return place and place.title or "your place" end

Alerts.RULES = {}

-- Rain soon: the first wet hour of the next three, when it is dry now.
function Alerts.RULES.rain(forecast, place, now)
    local current = forecast.hours[forecast.now]
    if current and (current.rain or 0) >= 0.2 then return nil end
    for _, hour in ipairs(ahead(forecast, now, 3 * 3600)) do
        if (hour.rainChance or 0) >= 60 and (hour.rain or 0) >= 0.3 then
            local what = (hour.snowfall or 0) > 0 and "Snow" or "Rain"
            return { title = what .. " soon",
                body = ("%s likely from about %s at %s (%d%%). Take an umbrella."):format(what, Units.clock(hour.time),
                    placeName(place), math.floor(hour.rainChance + 0.5)),
                at = math.max(now + 60, hour.epoch - 30 * 60), before = hour.epoch }
        end
    end
end

-- The morning brief: at 7 the day in one line and what to bring.
function Alerts.RULES.brief(forecast, place, now)
    local offset = forecast.offset or 0
    local date = localDate(now, offset)
    if now >= at(date, 7, offset) then date = localDate(now + 86400, offset) end
    local day
    for _, d in ipairs(forecast.days or {}) do if d.date == date then day = d end end
    if not (day and day.max and day.min) then return nil end
    local wet, bring = nil, {}
    for _, hour in ipairs(forecast.hours) do
        if hour.date == date and not wet and (Sky.hour(hour.time) or 0) >= 7 and (hour.rainChance or 0) >= 50 then wet = hour end
    end
    if wet then bring[#bring + 1] = "an umbrella" end
    if day.min <= 5 then bring[#bring + 1] = "a warm coat" end
    if (day.uv_index_max or 0) >= 6 then bring[#bring + 1] = "sunscreen" end
    local body = ("%s / %s, %s."):format(Units.temp(day.max), Units.temp(day.min),
        wet and ("rain likely from about %s"):format(Units.clock(wet.time)) or "mostly dry")
    if #bring > 0 then body = body .. " Bring " .. table.concat(bring, " and ") .. "." end
    return { title = "Today at " .. placeName(place), body = body, at = at(date, 7, offset), before = at(date, 10, offset) }
end

-- Frost tonight (GA1's thresholds): possible from 4 °C and likely from 2 °C
-- on a clear, calm night; likely at or below 0.5 °C whatever the sky.
function Alerts.RULES.frost(forecast, place, now)
    local offset = forecast.offset or 0
    -- The coming night: its hours from 18:00 to 09:00, named by the date
    -- of its evening; said at 17:00 that evening.
    -- Before 06:00 that is the night under way, from then on tonight.
    local nowHour = math.floor(((now + offset) % 86400) / 3600)
    local coldest, evening = nil, localDate(now - (nowHour < 6 and 86400 or 0), offset)
    for _, hour in ipairs(ahead(forecast, now, 30 * 3600)) do
        local h = Sky.hour(hour.time) or 12
        if h >= 18 or h <= 9 then
            local night = localDate(hour.epoch - (h <= 9 and 86400 or 0), offset)
            if night == evening and hour.temp and (not coldest or hour.temp < coldest.temp) then coldest = hour end
        end
    end
    if not coldest then return nil end
    local clearCalm = (coldest.cloud or 100) < 30 and (coldest.wind or 99) < 2.5
    local likely = coldest.temp <= 0.5 or (coldest.temp <= 2 and clearCalm)
    if not (likely or (coldest.temp <= 4 and clearCalm)) then return nil end
    return { title = likely and "Frost likely tonight" or "Frost possible tonight",
        body = ("About %s by %s at %s%s. Cover tender plants."):format(Units.temp(coldest.temp), Units.clock(coldest.time),
            placeName(place), clearCalm and ", clear and calm" or ""),
        at = math.max(now + 60, at(evening, 17, offset)), before = coldest.epoch - 2 * 3600 }
end

-- The storm clock: the first daylight hour of likely thunder, an estimate.
function Alerts.RULES.storm(forecast, place, now)
    for _, hour in ipairs(ahead(forecast, now, 30 * 3600)) do
        local code = hour.code or 0
        if hour.isDay and (code >= 95 or ((hour.cape or 0) >= 1000 and (hour.rainChance or 0) >= 40)) then
            local off = hour.epoch - 3600
            return { title = "Thunder likely from about " .. Units.clock(hour.time),
                body = ("Storms may build from about %s at %s (an estimate). Plan to be off high ground by %s."):format(
                    Units.clock(hour.time), placeName(place), Units.clock(os.date("!%H:%M", off + (forecast.offset or 0)))),
                at = math.max(now + 60, hour.epoch - 3 * 3600), before = off }
        end
    end
end

-- Good wind: the first window of two or more good daylight hours, said the
-- evening before (19:00) or three hours ahead on the day.
local function window(activity, forecast, now, light)
    local first, last
    for _, hour in ipairs(ahead(forecast, now + 3600, Alerts.HORIZON)) do
        local ok, result = pcall(Report.rate, activity, hour, forecast)
        local good = ok and result.verdict == "good" and light(hour)
        if good and (not last or hour.epoch - last.epoch == 3600) then
            first, last = first or hour, hour
        elseif first and last ~= first then break
        else first, last = nil, nil end
    end
    if first and last ~= first then return first, last end
end
local function eveningBefore(first, forecast, now)
    local offset = forecast.offset or 0
    local today, date = localDate(now, offset), localDate(first.epoch, offset)
    if date ~= today then
        local evening = at(localDate(first.epoch - 86400, offset), 19, offset)
        if evening > now then return evening end
    end
    return math.max(now + 60, first.epoch - 3 * 3600)
end
local function dayWord(epoch, forecast, now)
    local offset = forecast.offset or 0
    if localDate(epoch, offset) == localDate(now, offset) then return "today" end
    if localDate(epoch, offset) == localDate(now + 86400, offset) then return "tomorrow" end
    return Sky.weekday(localDate(epoch, offset))
end

function Alerts.RULES.wind(forecast, place, now)
    local sailing = Activities.byId.sailing
    if not sailing then return nil end
    local first, last = window(sailing, forecast, now, function(hour) return hour.isDay end)
    if not first then return nil end
    local span = Units.span(first.time, os.date("!%Y-%m-%dT%H:00", last.epoch + 3600 + (forecast.offset or 0)))
    return { title = ("Good wind %s, %s"):format(dayWord(first.epoch, forecast, now), span),
        body = ("Good wind at %s from %s: %s."):format(placeName(place), Units.clock(first.time), Units.gust(first.wind, first.gust)),
        at = eveningBefore(first, forecast, now), before = first.epoch }
end

-- Dawn patrol: Surfers' own card, when tomorrow's first light is good.
function Alerts.RULES.dawn(forecast, place, now)
    local surf = Activities.byId.surf
    if not (surf and surf.dawnPatrol) then return nil end
    forecast.spot = forecast.spot or place
    local card = surf.dawnPatrol(forecast)
    if not (card and card.tone == "good" and card.title:find("tomorrow")) then return nil end
    local offset = forecast.offset or 0
    local evening = at(localDate(now, offset), 20, offset)
    return { title = "Dawn patrol tomorrow at " .. placeName(place), body = card.value .. ". " .. card.detail,
        at = math.max(now + 60, evening), before = at(localDate(now + 86400, offset), 5, offset) }
end

-- A clear night: two or more dark hours (sun 12° down) under 20 % cloud.
function Alerts.RULES.clear(forecast, place, now)
    local first, last
    for _, hour in ipairs(ahead(forecast, now, 24 * 3600)) do
        local dark = hour.sunAltitude and hour.sunAltitude < -12
        local clear = dark and (hour.cloud or 100) <= 20
        if clear and (not last or hour.epoch - last.epoch == 3600) then
            first, last = first or hour, hour
        elseif first and last ~= first then break
        else first, last = nil, nil end
    end
    if not (first and last ~= first) then return nil end
    local until_ = os.date("!%H:%M", last.epoch + 3600 + (forecast.offset or 0))
    return { title = ("Clear %s from %s"):format(dayWord(first.epoch, forecast, now) == "today" and "tonight" or
            dayWord(first.epoch, forecast, now), Units.clock(first.time)),
        body = ("Clear from %s to %s at %s: dark and under 20%% cloud."):format(Units.clock(first.time),
            Units.clock(until_), placeName(place)),
        at = math.max(now + 60, first.epoch - 2 * 3600), before = first.epoch }
end

-- Every rule's suggestion for a place, each tagged with its kind. A rule
-- that fails is left out; it never stops the others.
function Alerts.candidates(forecast, place, now)
    local list = {}
    if not (forecast and forecast.hours and forecast.hours[forecast.now or 1]) then return list end
    now = now or forecast.epoch or os.time()
    for _, kind in ipairs(Alerts.kinds()) do
        local rule = Alerts.RULES[kind.id]
        local ok, found = pcall(rule, forecast, place, now)
        if ok and type(found) == "table" and found.at then
            found.kind = kind.id
            list[#list + 1] = found
        end
    end
    for _, activity in ipairs(Activities.list) do
        if activity.alerts then
            local ok, found = pcall(activity.alerts, forecast, place, now)
            for _, item in ipairs(ok and type(found) == "table" and found or {}) do
                if Alerts.byId[item.kind] and item.at then list[#list + 1] = item end
            end
        end
    end
    return list
end

-- Whether a time falls in quiet hours (`from` to `to`, local hours; equal
-- means none), and when they end after it.
function Alerts.quiet(epoch, from, to, offset)
    if from == to then return false end
    local seconds = (epoch + (offset or 0)) % 86400
    local inside
    if from > to then inside = seconds >= from * 3600 or seconds < to * 3600
    else inside = seconds >= from * 3600 and seconds < to * 3600 end
    if not inside then return false end
    local endAt = epoch - seconds + to * 3600
    if endAt <= epoch then endAt = endAt + 86400 end
    return true, endAt
end

-- What to deliver. `options`: now, offset (the place's clock), setting(key)
-- (defaults to Settings.get("alerts", key)) and delivered (`{ ["frost:2026-11-18"] = true }`,
-- what went out already). An alert in quiet hours moves to their end, or is
-- dropped when that is too late to help; each kind says one thing a day at
-- most, the earliest. Ids are `<kind>:<local date>`.
function Alerts.plan(candidates, options)
    options = options or {}
    local setting = options.setting or function(key) return Settings.get(Alerts.GROUP, key) end
    local now, offset, delivered = options.now or os.time(), options.offset or 0, options.delivered or {}
    if not setting("on") then return {} end
    local from, to = setting("quietFrom") or 22, setting("quietTo") or 7
    local chosen = {}
    for _, alert in ipairs(candidates) do
        local when = alert.at
        local quiet, after = Alerts.quiet(when, from, to, offset)
        if quiet then when = after end
        local late = alert.before and when > alert.before
        local id = alert.kind .. ":" .. localDate(when, offset)
        if setting(alert.kind) and not late and when >= now and when <= now + Alerts.HORIZON and not delivered[id]
            and not (chosen[id] and chosen[id].at <= when) then
            chosen[id] = { id = id, kind = alert.kind, title = alert.title, body = alert.body, at = when }
        end
    end
    local list = {}
    for _, alert in pairs(chosen) do list[#list + 1] = alert end
    table.sort(list, function(a, b) return a.at < b.at or (a.at == b.at and a.id < b.id) end)
    return list
end

-- A plan as text, to tell whether it changed.
local function signature(list)
    local parts = {}
    for _, alert in ipairs(list) do parts[#parts + 1] = ("%s@%d %s %s"):format(alert.id, alert.at, alert.title, alert.body) end
    return table.concat(parts, "\n")
end

-- Plans the home place's alerts and hands them to `options.notifier` (the
-- orca.notifications module or a stand-in: scheduleLocal, cancelLocal,
-- scheduleRefresh). `options.storage` keeps what was planned, so an alert
-- whose time has passed counts as said for its day. Scheduling happens only
-- when the plan changed. Returns the plan.
function Alerts.sync(forecast, place, options)
    options = options or {}
    local now = options.now or os.time()
    local storage, notifier = options.storage, options.notifier
    local saved = storage and storage:load(Alerts.FILE) or {}
    local delivered, planned = {}, {}
    for id, date in pairs(type(saved.delivered) == "table" and saved.delivered or {}) do
        -- Forget days long gone.
        if type(date) == "string" and date >= localDate(now - 2 * 86400, 0) then delivered[id] = date end
    end
    for _, alert in ipairs(type(saved.planned) == "table" and saved.planned or {}) do
        if type(alert) == "table" and type(alert.at) == "number" then
            if alert.at <= now then delivered[alert.id] = alert.id:match(":(.+)$") else planned[#planned + 1] = alert end
        end
    end
    local plan = Alerts.plan(Alerts.candidates(forecast, place, now), { now = now, offset = forecast and forecast.offset,
        setting = options.setting, delivered = delivered })
    local changed = signature(plan) ~= signature(planned)
    if notifier and changed then
        -- The same id replaces itself; only alerts no longer planned go.
        local keep = {}
        for _, alert in ipairs(plan) do keep[alert.id] = true end
        for _, alert in ipairs(planned) do
            if not keep[alert.id] and notifier.cancelLocal then pcall(notifier.cancelLocal, alert.id) end
        end
        for _, alert in ipairs(plan) do
            if notifier.scheduleLocal then
                pcall(notifier.scheduleLocal, { id = alert.id, title = alert.title, body = alert.body, at = alert.at })
            end
        end
    end
    local on = (options.setting or function(key) return Settings.get(Alerts.GROUP, key) end)("on")
    if notifier and on and notifier.scheduleRefresh then pcall(notifier.scheduleRefresh, Alerts.REFRESH) end
    if storage then storage:save(Alerts.FILE, { planned = plan, delivered = delivered, place = place and place.id }) end
    return plan, changed
end

-- Turned off: nothing stays scheduled.
function Alerts.clear(options)
    options = options or {}
    if options.notifier and options.notifier.cancelLocal then pcall(options.notifier.cancelLocal) end
    if options.storage then
        local saved = options.storage:load(Alerts.FILE) or {}
        options.storage:save(Alerts.FILE, { planned = {}, delivered = saved.delivered or {} })
    end
end

-- The background refresh (background.lua): the home place's forecast,
-- fetched fresh, then sync. `options`: home (a place), fetch, now, storage,
-- notifier, setting. Returns the plan and the forecast, or nil and why.
function Alerts.refresh(options)
    local Forecast = require "Weather/forecast"
    local home = options.home
    if not home then return nil, "no place" end
    local now = options.now or os.time()
    local request = Activities.request(home)
    local forecast, err, raw = Forecast.load(home, Activities.needsMarine(home) and "optional", options.fetch, now, request)
    if not forecast then return nil, err end
    forecast.spot = home
    if options.storage and raw then options.storage:save("forecast-" .. home.id, raw) end
    return (Alerts.sync(forecast, home, options)), forecast
end

return Alerts
