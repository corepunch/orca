-- Surf: is it worth paddling out? Surfers read the swell (each train apart:
-- size, period, direction), the wind against the beach, the tide and the
-- light. Every break has a profile (editions/surf/breaks.lua): the swell
-- window it opens to, its offshore wind, the sizes and tide it likes and
-- who it suits. The same swell is clean at one beach and blown out or
-- shut down at the next. The waves on the beach are an estimate from the
-- open-ocean swell, and the screens say so.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Settings = require "Weather/settings"
local Breaks = require "Weather/editions/surf/breaks"
local Waves = require "Weather/editions/surf/waves"
local Tides = require "Weather/editions/surf/tides"
local Outlook = require "Weather/editions/surf/outlook"
local Logbook = require "Weather/editions/surf/logbook"
local Hazards = require "Weather/editions/surf/hazards"
local Buoys = require "Weather/editions/surf/buoys"
local Comfort = require "Weather/editions/surf/comfort"
local WaterQuality = require "Weather/editions/surf/waterquality"
local Compare = require "Weather/compare"

local Surf = {
    id = "surf", title = "surf", needsMarine = true,
    hubBackground = "Weather/editions/surf/ocean-beach.jpg",
}

local marine = { Tides.COLUMN }
for _, name in ipairs(Waves.COLUMNS) do marine[#marine + 1] = name end
-- Three past days of rain for the runoff warning; 16 days for the outlook.
Surf.fields = { marine = marine, pastDays = 3, forecastDays = Outlook.DAYS }
-- Services that already exist (Research/services-surf.md), each optional:
-- beach warnings, the buoy, measured water temperature, the current and
-- official bathing-water status. A break without a station asks nothing.
Surf.sources = { Outlook.source, Hazards.source, Buoys.source, Buoys.waterSource, Comfort.source }
for _, source in ipairs(WaterQuality.sources) do Surf.sources[#Surf.sources + 1] = source end

-- Credit on the settings' "about": a country's bathing-water line only
-- once a break reads it.
function Surf.attribution()
    local list = { Hazards.attribution, Buoys.attribution[1], Buoys.attribution[2] }
    for _, profile in pairs(Breaks.list) do
        if profile.bathingWater and profile.bathingWater.ie then
            list[#list + 1] = WaterQuality.attribution[1]
            break
        end
    end
    return list
end

-- The surfer's level (SU9): what is too big for them, and the size they
-- enjoy most. Until they choose, the app rates for an intermediate.
local LEVELS = {
    beginner = { ideal = { 0.3, 1.0 }, cap = 1.4 },
    intermediate = { ideal = { 0.6, 2.0 }, cap = 3.0 },
    advanced = { ideal = { 0.9, 4.0 }, cap = 6.0 },
}
Surf.settings = {
    { key = "level", title = "your level", kind = "choice", default = "unset",
        options = { { "beginner", "beginner" }, { "intermediate", "intermediate" }, { "advanced", "advanced" } },
        detail = "big days are rated for you",
        format = function(value) return LEVELS[value] and value or "not set" end },
}

-- What the place editor asks of a break (P3); Breaks.of reads them first.
Surf.placeFields = {
    { key = "facing", title = "the shore faces", kind = "compass",
        detail = "from the beach out to sea: the wind from behind you is offshore",
        needed = "not set: tap to choose where the sea is, so wind and swell can be read" },
    { key = "breakLevel", title = "the break suits", kind = "choice",
        options = { { "beginner", "beginners" }, { "intermediate", "intermediate surfers" }, { "advanced", "advanced surfers" } },
        default = function(place) return Breaks.of(place).level end },
    { key = "breakTide", title = "best tide", kind = "choice",
        options = { { "all", "any tide" }, { "low", "low" }, { "low-mid", "low to mid" }, { "mid", "mid" },
            { "mid-high", "mid to high" }, { "high", "high" } },
        default = function(place) return Breaks.of(place).tide end },
}

function Surf.level()
    local value = Settings.get("surf", "level")
    return LEVELS[value] and value or "intermediate", LEVELS[value] ~= nil
end

-- Surfers paddle out at first light, before sunrise: civil twilight counts.
local function light(hour)
    local darkness = Score.darkness(hour)
    if darkness then return darkness == "day" or darkness == "civil" end
    return hour.isDay
end
Surf.light = light

-- offshore, onshore or cross-shore at the break.
local function windTo(hour, profile)
    if not (hour.windDir and profile.wind) then return "cross-shore" end
    if Sky.angle(hour.windDir, profile.wind) <= 45 then return "offshore" end
    if Sky.angle(hour.windDir, profile.wind + 180) <= 45 then return "onshore" end
    return "cross-shore"
end

local function capital(text) return (text:gsub("^%l", string.upper)) end

-- Rain in the 72 hours before an hour, mm, from the forecast's past hours.
local positions = setmetatable({}, { __mode = "k" })
local function rainBefore(forecast, hour)
    local hours = forecast.hours
    if not (hours and hour.epoch) then return nil end
    local index = positions[hours]
    if not index then
        index = {}
        for i, h in ipairs(hours) do index[h.time] = i end
        positions[hours] = index
    end
    local at, total = index[hour.time], 0
    if not at then return nil end
    for i = math.max(1, at - 72), at - 1 do total = total + (hours[i].rain or 0) end
    return total
end
Surf.rainBefore = rainBefore
local function rainText(mm) return Units.rain(mm, Units.current("rain") == "in" and 1 or 0) end

-- The size rule: how the estimated surf suits the break and the surfer.
local function size(card, surf, profile, level)
    local face, limits = surf.face, LEVELS[level]
    local words = capital(Waves.words(surf.low, surf.high)) .. " (estimate)"
    if face < 0.3 then return card:add(-30, "Flat: ankle high at best (estimate).") end
    if face > 5 then return card:cap(30, words .. ": too heavy for most breaks.") end
    if face > limits.cap then return card:cap(40, words .. ": too big for a " .. level .. ".") end
    local fitsBreak = face >= profile.size[1] and face <= profile.size[2]
    local fitsYou = face >= limits.ideal[1] and face <= limits.ideal[2]
    if fitsBreak and fitsYou then return card:add(25, words .. ": right for this break and your level.") end
    if face > limits.ideal[2] then return card:add(-5, words .. ": big for your level.") end
    if face > profile.size[2] then return card:add(-5, words .. ": more than this break handles; expect close-outs.") end
    if face < 0.5 then return card:add(-10, words .. ": small; a longboard helps.") end
    return card:add(5, words .. ": on the small side; a longboard helps.")
end

local function period(card, seconds)
    if not seconds then return end
    if seconds < 7 then card:add(-12, "Short period: weak, choppy windswell.")
    elseif seconds < 10 then card:add(2, "Medium-period swell.")
    else card:add(math.floor(Score.curve(seconds, { { 10, 10 }, { 15, 15 } })),
        "Long-period groundswell: powerful, lined-up waves.") end
end

function Surf.rate(hour, forecast)
    local card, spot = Score.card(40), forecast.spot or {}
    local profile, level = Breaks.of(spot), Surf.level()
    if Sky.isStorm(hour.code) then card:cap(5, "Lightning: get out of the water.") end
    if not light(hour) then card:cap(30, "Dark: wait for first light.")
    elseif Score.darkness(hour) == "civil" then card:note("First light: dawn patrol.") end
    Hazards.rate(card, hour, forecast, level)

    local surf = Waves.surf(hour, profile)
    if not surf then card:cap(20, "No swell data for this spot.")
    else
        size(card, surf, profile, level)
        period(card, surf.train.period)
        local train = surf.train
        if Breaks.outside(profile.window, train.direction) > 30 then
            card:add(-10, ("The %s from %s misses this break: it needs %s."):format(train.name,
                Sky.compass(train.direction), Breaks.windowText(profile.window)))
        end
        if hour.oneModel then card:note("Far ahead: one wave model only, low confidence.") end
        local logged = spot.id and Logbook.summary(Logbook.similar(spot.id, Surf.conditions(hour, forecast, surf)))
        if logged then card:note(capital(logged) .. ".") end
    end
    if profile.level == "advanced" and level == "beginner" then
        card:cap(55, "This break is for experienced surfers: try a gentler beach.")
    end

    local wind, relation = hour.wind or 0, windTo(hour, profile)
    if wind < 2 then card:add(12, "Glassy: almost no wind.")
    elseif relation == "offshore" then
        if wind <= 8 then card:add(10, "Offshore wind grooms the wave faces.")
        else card:add(-8, "Strong offshore: hard to paddle into waves.") end
    elseif relation == "onshore" then card:add(-math.floor(wind * 2.5 + 0.5), "Onshore wind blows the waves out.")
    else card:add(-math.floor(wind + 0.5), "Cross-shore wind adds chop.") end

    -- The tide: only where the break cares and the tide is big enough to matter.
    local tide = Tides.at(forecast, hour)
    if tide and profile.tide ~= "all" and tide.range >= 0.3 then
        local miss = Breaks.tideMiss(profile.tide, tide.level)
        if miss == 0 then
            card:add(8, ("%s tide, %s: when this break works best."):format(capital(tide.word),
                tide.rising and "rising" or "falling"))
        else
            card:add(-math.floor(6 + miss * 20 + 0.5), ("%s tide: this break works best at %s tide."):format(
                capital(tide.word), Breaks.TIDE[profile.tide]))
        end
    end

    -- An official restriction today (SV-SU4) replaces the runoff rule after
    -- heavy rain (SU8: germs from rivers and drains); "none listed" does not.
    local official = WaterQuality.status(forecast)
    local today = forecast.hours and forecast.now and forecast.hours[forecast.now]
    local officialToday = official and official.status ~= "none listed" and today and hour.date == today.date
    if officialToday and official.status == "advice against bathing" then
        card:add(-25, ("Official advice against bathing (%s)."):format(official.source))
    elseif officialToday then
        card:add(-10, ("Official water-quality risk (%s)."):format(official.source))
    end
    local rain = not officialToday and profile.runoff and rainBefore(forecast, hour)
    if rain and rain >= profile.runoff then
        card:add(-15, ("About %s of rain in 3 days: runoff can make the water unsafe."):format(rainText(rain)))
    end
    return card
end

function Surf.window(hour) return light(hour) end

-- What the rules read in an hour, as the log keeps it (SU10).
function Surf.conditions(hour, forecast, surf)
    local profile = Breaks.of(forecast.spot)
    surf = surf or Waves.surf(hour, profile)
    local tide = Tides.at(forecast, hour)
    return { face = surf and math.floor(surf.face * 10 + 0.5) / 10, period = surf and surf.train.period,
        wind = (hour.wind or 0) < 2 and "glassy" or windTo(hour, profile), tide = tide and tide.word }
end

-- Logs a session in the forecast's conditions with a one-tap rating.
function Surf.log(place, forecast, hour, rating)
    return Logbook.add(place, hour, rating, Surf.conditions(hour, forecast))
end

-- The verdict card shows the current hour, or first light when it is dark.
function Surf.focus(forecast)
    for index = forecast.now, math.min(#forecast.hours, forecast.now + 24) do
        if light(forecast.hours[index]) then return index end
    end
    return forecast.now
end

function Surf.note(hour, forecast)
    local surf = Waves.surf(hour, Breaks.of(forecast.spot))
    local tide = Tides.at(forecast, hour)
    return ("%s · %s · %s %s%s"):format(Waves.word(surf and surf.face), Score.fmt(surf and surf.train.period, " s"),
        windTo(hour, Breaks.of(forecast.spot)), Units.wind(hour.wind), tide and " · " .. tide.word .. " tide" or "")
end

local function wetsuit(temp)
    if temp == nil then return nil end
    if temp < 12 then return "5/4 mm, boots and hood" end
    if temp < 15 then return "4/3 mm wetsuit" end
    if temp < 18 then return "3/2 mm wetsuit" end
    if temp < 22 then return "spring suit" end
    return "boardshorts"
end

-- "1.5 m · 13 s", a train's size and period.
local function train(t) return Units.height(t.height, 1) .. " · " .. Score.fmt(t.period, " s") end

function Surf.metrics(hour, forecast)
    local profile = Breaks.of(forecast.spot)
    local surf = Waves.surf(hour, profile)
    local rows = {
        { label = "surf (estimate)", value = Waves.range(surf),
            note = surf and Waves.words(surf.low, surf.high) .. " at this break" or "no sea data", tone = "info" },
    }
    for _, t in ipairs(Waves.trains(hour)) do
        local note = "from " .. Sky.compass(t.direction)
        if Breaks.outside(profile.window, t.direction) > 30 then note = note .. ", outside this break's window" end
        if t.name == "swell" and hour.swell_wave_peak_period then
            note = note .. " · peak " .. Score.fmt(hour.swell_wave_peak_period, " s")
        end
        if t.name == "wind sea" then note = "local chop " .. note end
        rows[#rows + 1] = { label = t.name, value = train(t), note = note }
    end
    local wind = Report.wind(hour)
    wind.note = windTo(hour, profile) .. " · " .. wind.note
    rows[#rows + 1] = wind
    local tide = Tides.at(forecast, hour)
    if tide then
        rows[#rows + 1] = { label = "tide (approximate)", value = Tides.text(tide),
            note = ("next %s %s"):format(tide.next.kind, Units.clock(tide.next.time)) }
    end
    -- Measured at a station when fresh (SV-SU3), else the model's sea surface.
    local water, from, station = Buoys.water(hour, forecast)
    local note = wetsuit(water)
    if note and from then
        note = note .. " · " .. (from == "measured" and "measured" .. (station and " at " .. station or "") or "model")
    end
    rows[#rows + 1] = { label = "water", value = Units.temp(water), note = note }
    local day = hour.day or {}
    local civil = day.twilight and day.twilight.civil
    rows[#rows + 1] = { label = "first light · sunrise", value = Units.clock(civil and civil.dawn) .. " · "
        .. Units.clock(day.sunrise), note = "sunset " .. Units.clock(day.sunset) }
    rows[#rows + 1] = Surf.breakRow(profile)
    rows[#rows + 1] = Report.moon(forecast)
    return rows
end

-- What the break needs, in one row (SU3).
function Surf.breakRow(profile)
    local level, chosen = Surf.level()
    return { label = profile.guessed and "this beach (guessed from its facing)" or "this break",
        value = "needs " .. Breaks.windowText(profile.window) .. " swell",
        note = ("%s wind · %s tide · %s–%s · for %s surfers · you: %s"):format(Sky.compass(profile.wind),
            Breaks.TIDE[profile.tide], Units.height(profile.size[1], 1):match("^[%d%.]+"), Units.height(profile.size[2], 1),
            profile.level, chosen and level or "level not set") }
end

-- Cards ---------------------------------------------------------------------

-- "head high (estimate), offshore, mid tide rising" for an hour.
local function describe(hour, forecast)
    local profile = Breaks.of(forecast.spot)
    local surf = Waves.surf(hour, profile)
    local wind = (hour.wind or 0) < 2 and "glassy" or windTo(hour, profile)
    local tide = Tides.at(forecast, hour)
    return ("%s (estimate), %s%s"):format(Waves.word(surf and surf.face), wind,
        tide and ", " .. tide.word .. " tide " .. (tide.rising and "rising" or "falling") or "")
end

-- The wind drawn against the beach (SU11): the sea at the top, the shore
-- at the bottom, the arrow where the wind blows to. Pointing up, out to sea,
-- is offshore. nil without a wind direction.
function Surf.windIcon(hour, profile)
    if not (hour.windDir and profile.wind) then return nil end
    local turn = math.floor((hour.windDir - profile.wind) % 360 / 45 + 0.5) % 8 * 45
    return "Weather/editions/surf/wind/" .. turn .. ".svg"
end

-- Dawn patrol (SU4): first light and the first good hour. Before today's
-- first light it is about this morning, after it about tomorrow.
function Surf.dawnPatrol(forecast)
    local now = forecast.hours[forecast.now]
    if not now then return nil end
    local index = forecast.today or 1
    local day = forecast.days[index]
    local function dawn(d)
        local civil = d and d.twilight and d.twilight.civil
        return civil and civil.dawn or d and d.sunrise
    end
    local today = day and dawn(day) and now.time < dawn(day)
    if not today then day = forecast.days[index + 1] end
    if not day then return nil end
    local first, fair
    for i = forecast.now, #forecast.hours do
        local hour = forecast.hours[i]
        if hour.date == day.date and light(hour) then
            local result = Report.rate(Surf, hour, forecast)
            if result.verdict == "good" then first = { hour = hour, result = result }; break end
            fair = fair or result.verdict == "fair" and { hour = hour, result = result } or nil
        end
    end
    local pick = first or fair
    return {
        id = "dawn", title = today and "dawn patrol this morning" or "dawn patrol tomorrow",
        value = "first light " .. Units.clock(dawn(day)),
        detail = pick and ("%s from %s: %s"):format(pick.result.word, Units.clock(pick.hour.time),
            describe(pick.hour, forecast)) or "no good hour: the whole day rates poor",
        tone = pick and pick.result.verdict or "poor",
        icon = pick and Surf.windIcon(pick.hour, Breaks.of(forecast.spot)),
    }
end

-- Tides (SU5): the next high and low, approximate.
function Surf.tideCard(forecast)
    local now = forecast.hours[forecast.now]
    local state = now and Tides.at(forecast, now)
    if not state then return nil end
    local parts = {}
    for _, extreme in ipairs(Tides.extremes(forecast)) do
        if extreme.time >= now.time and #parts < 2 then
            parts[#parts + 1] = extreme.kind .. " " .. Units.clock(extreme.time)
        end
    end
    local profile = Breaks.of(forecast.spot)
    local best = profile.tide == "all" and "works at any tide" or "best here at " .. Breaks.TIDE[profile.tide] .. " tide"
    return { id = "tide", title = "tides (approximate)", value = table.concat(parts, " · "),
        detail = ("now %s · %s · a model's sea level, not for navigation"):format(Tides.text(state), best),
        tone = "info" }
end

-- Water quality: the official status where a country publishes one
-- (SV-SU4), else only when the rain says so (SU8).
function Surf.waterCard(forecast)
    local official = WaterQuality.card(forecast)
    if official and official.tone == "poor" then return official end
    local profile, now = Breaks.of(forecast.spot), forecast.hours[forecast.now]
    local rain = profile.runoff and now and rainBefore(forecast, now)
    if not (rain and rain >= profile.runoff) then return official end
    return { id = "water", title = "water quality", value = "runoff risk", tone = "poor",
        detail = ("about %s of rain in 3 days: drains and rivers wash germs into the sea; "
            .. "stay out for 72 h after heavy rain · %s"):format(rainText(rain),
            official and "no official restriction listed yet" or "no official reading here") }
end

-- Pages about the break (SV-SU6): cams, water quality, sharks, the surf
-- zone forecast. Named with their address, never copied; the app has no
-- way yet to open an outside page from a card, so they are text.
function Surf.linksCard(forecast)
    local links = Breaks.of(forecast.spot).links
    if not (links and #links > 0) then return nil end
    local parts = {}
    for index = 1, math.min(3, #links) do parts[index] = links[index][1] .. ": " .. links[index][2] end
    return { id = "links", title = "more about this beach", value = links[1][1]:lower(), tone = "info", hub = false,
        detail = table.concat(parts, " · ") }
end

-- A break's best window today, or tomorrow once today's has passed.
local function bestSoon(forecast)
    local ahead = Report.daysAhead(forecast)
    for index = 1, math.min(2, #ahead) do
        local result, span, hour = Report.bestOfDay(Surf, forecast, ahead[index].date)
        if result then return { result = result, span = span, hour = hour, tomorrow = index == 2, later = index == 2 } end
    end
end

-- Best break among several (SU6): `entries` are { place, forecast }. Returns
-- them rated, best first: today's windows before tomorrow's (the shared
-- comparison, Weather/compare.lua).
function Surf.compare(entries)
    return Compare.rank(entries, function(entry) return bestSoon(entry.forecast) end,
        { tie = function(a, b) return a.place.id < b.place.id end })
end

-- The forecasts this device keeps for my other breaks, as of `epoch`. The
-- provider fetches only the place on screen, so a break shows up here once
-- it has been opened: comparing costs no request.
function Surf.others(place, epoch)
    local list = {}
    for _, other in ipairs(Compare.places("surf")) do
        if other.id ~= place.id then
            local forecast = Compare.saved(other, epoch)
            if forecast then list[#list + 1] = { place = other, forecast = forecast } end
        end
    end
    return list
end

function Surf.bestBreakCard(forecast, place)
    local entries = { { place = place, forecast = forecast } }
    for _, entry in ipairs(Surf.others(place, forecast.epoch)) do entries[#entries + 1] = entry end
    if #entries < 2 then return nil end
    local top = Surf.compare(entries)[1]
    if not top then return nil end
    return { id = "best-break", title = top.tomorrow and "best break tomorrow" or "best break today",
        value = top.place.id == place.id and "this one" or top.place.title:lower(), tone = top.result.verdict,
        detail = ("%s %s: %s · of %d breaks with a forecast on this device"):format(top.result.word, top.span,
            describe(top.hour, top.forecast), #entries) }
end

-- The outlook (SU7): the best day after tomorrow, and how sure it is.
function Surf.outlookCard(forecast)
    local ahead = Report.daysAhead(forecast)
    if #ahead < 3 then return nil end
    local top
    for index = 3, #ahead do
        local result, span, hour = Report.bestOfDay(Surf, forecast, ahead[index].date)
        if result and (not top or result.score > top.result.score) then
            top = { result = result, span = span, hour = hour, date = ahead[index].date }
        end
    end
    if not top or top.result.verdict == "poor" then
        return { id = "outlook", title = "the next " .. #ahead .. " days", value = "no good swell", tone = "poor",
            detail = "nothing better than poor after tomorrow, as far as the wave models see" }
    end
    local confidence, why = Outlook.confidence(forecast, top.date)
    return { id = "outlook", title = "best day ahead", value = Sky.weekday(top.date), tone = top.result.verdict,
        detail = ("%s %s: %s · %s confidence (%s)"):format(top.result.word, top.span, describe(top.hour, forecast),
            confidence, why) }
end

-- First run (SU9): ask for the level until it is chosen, one tap on the
-- card (or in settings). It leads the hub until then.
function Surf.setupCard()
    local _, chosen = Surf.level()
    if chosen then return nil end
    return { id = "setup", title = "your level", value = "not set yet", tone = "info", rank = 0,
        detail = "so big days are rated for you; until then, for an intermediate",
        actions = { { "level:beginner", "beginner" }, { "level:intermediate", "intermediate" },
            { "level:advanced", "advanced" } } }
end

-- The session log (SU10) in one tap: how was it, for the hour just
-- surfed? On the activity page only (`hub = false`); the card says what
-- was saved.
local logged = {}
function Surf.logCard(place)
    local last = place and logged[place.id]
    local count = 0
    for _, entry in ipairs(Logbook.sessions()) do if place and entry.place == place.id then count = count + 1 end end
    local detail = last and ("saved a %s session at %s · %d here in your log"):format(last.rating,
        Units.clock(last.time), count) or "one tap saves the hour's waves, wind and tide; your log stays on this phone"
    return { id = "log", title = "log a session", value = last and "saved" or "how was it?", detail = detail,
        tone = last and last.rating or "info", hub = false,
        actions = { { "log:good", "good" }, { "log:fair", "fair" }, { "log:poor", "poor" } } }
end

-- A card's link: "level:<level>" chooses the level, "log:<rating>" logs
-- the current hour at the place.
function Surf.act(id, context)
    local level = id:match("^level:(%a+)$")
    if level and LEVELS[level] then Settings.set("surf", "level", level); return end
    local rating = id:match("^log:(%a+)$")
    local forecast = context.forecast
    if not (rating and forecast and context.place) then return nil end
    local entry = Surf.log(context.place, forecast, forecast.hours[forecast.now], rating)
    if entry then logged[context.place.id] = entry end
end

function Surf.cards(forecast, place)
    local cards = {}
    for _, card in ipairs {
        Hazards.card(forecast) or false, Surf.setupCard() or false, Surf.dawnPatrol(forecast) or false,
        Surf.waterCard(forecast) or false, Buoys.card(forecast) or false, Surf.tideCard(forecast) or false,
        Comfort.card(forecast, forecast.hours[Surf.focus(forecast)]) or false,
        Surf.bestBreakCard(forecast, place) or false, Surf.outlookCard(forecast) or false, Surf.logCard(place),
        Surf.linksCard(forecast) or false,
    } do
        if card then cards[#cards + 1] = card end
    end
    return cards
end

-- How sure each day's swell is, on the week's rows (SU7).
function Surf.dayNote(day, forecast)
    local word = Outlook.confidence(forecast, day.date)
    return word and word .. " confidence"
end

-- The charts on the activity's "today" view, over the next 24 hours: the
-- surf, the wind and, where there is one, the tide (approximate).
-- `value` is metric; `unit` (a Units kind) converts it and ends the title.
Surf.charts = {
    { title = "estimated surf", unit = "height", kind = "column",
        value = function(hour, forecast)
            local surf = Waves.surf(hour, Breaks.of(forecast and forecast.spot))
            return surf and surf.face or 0
        end },
    { title = "wind", unit = "wind", kind = "line", value = function(hour) return hour.wind or 0 end },
    { title = "tide (approximate)", unit = "height", kind = "line", value = function(hour) return hour[Tides.COLUMN] or 0 end },
}

return Surf
