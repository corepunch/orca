-- Sailing, windsurf, wing and kitesurf: is there a steady, usable wind for
-- this rider, at this spot? The wind band comes from the rider's profile
-- (editions/sailing/profile.lua): a learner kiter's "too strong" is an
-- expert's good day. Kiters and boards never get an offshore wind or a
-- wind outside the spot's safe sector (editions/sailing/spots.lua); boats
-- get a caution instead. Lightning, dark, gale, fog and squalls cap every
-- profile. Every spot says which way its shore faces (`facing`, degrees
-- toward the water); lakes have no waves or tides to read.
--
-- Extra requests: the same wind from several models (confidence), the
-- approximate tide, and the other spots within range ("this weekend").
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Profile = require "Weather/editions/sailing/profile"
local Spots = require "Weather/editions/sailing/spots"
local Models = require "Weather/editions/sailing/models"
local Tides = require "Weather/editions/sailing/tides"
local Rank = require "Weather/editions/sailing/rank"

local Sailing = {
    id = "sailing", title = "sailing",
    hubBackground = "Weather/editions/sailing/bay-under-sail.jpg",
    -- Waves and the sea level on a coast; a lake has neither, so it does not ask.
    needsMarine = function(place) return not (place and place.lake) and "optional" end,
    settings = Profile.settings,
    sources = { Models.source, Tides.source, Rank.source(function() return Profile.current().range end) },
}
-- What the place editor asks of a spot (P3); Spots.sector reads the safe
-- sector the user set before its table.
Sailing.placeFields = {
    { key = "facing", title = "the shore faces", kind = "compass",
        detail = "from the beach out over the water: wind from behind you is offshore",
        needed = "not set: tap to choose where the water is, so offshore wind can be told" },
    { key = "safeFrom", title = "safe wind from", kind = "compass",
        default = function(place) local sector = Spots.sector(place); return sector and sector.from end,
        detail = "the safe sector runs clockwise from here: wind that brings you back to the beach" },
    { key = "safeTo", title = "safe wind to", kind = "compass",
        default = function(place) local sector = Spots.sector(place); return sector and sector.to end,
        detail = "ask the local school or club: these are estimates" },
}

Sailing.Profile, Sailing.Spots, Sailing.Models, Sailing.Tides, Sailing.Rank = Profile, Spots, Models, Tides, Rank

local GALE = 17.2 -- m/s, Beaufort 8, for every profile

local W = Units.wind

-- offshore, onshore or cross-shore for the spot's shore.
local function windTo(hour, spot)
    if not (hour.windDir and spot.facing) then return "cross-shore" end
    if Sky.angle(hour.windDir, spot.facing + 180) <= 45 then return "offshore" end
    if Sky.angle(hour.windDir, spot.facing) <= 45 then return "onshore" end
    return "cross-shore"
end
Sailing.windTo = windTo

-- How much stronger the gusts are than the wind; 1.0 is perfectly steady.
local function gustiness(hour)
    if not (hour.wind and hour.gust) or hour.wind < 1 then return 1 end
    return hour.gust / hour.wind
end

-- "12–28 kn": a wind range in the user's units.
local function range(lo, hi)
    return ("%d–%s"):format(math.floor(Units.value("wind", lo) + 0.5), W(hi))
end

-- The hour `n` hours after `hour` in its forecast, through a per-forecast
-- index of times.
local indexes = setmetatable({}, { __mode = "k" })
local function after(forecast, hour, n)
    local hours = forecast.hours
    if not (hours and hour.time) then return nil end
    local index = indexes[hours]
    if not index then
        index = {}
        for i, h in ipairs(hours) do index[h.time] = i end
        indexes[hours] = index
    end
    local at = index[hour.time]
    return at and hours[at + n]
end

-- The biggest turn of the wind over the next three hours, with the hour it
-- turns to.
local function shift(forecast, hour)
    if not hour.windDir then return 0 end
    local most, to = 0, nil
    for n = 1, 3 do
        local later = after(forecast, hour, n)
        if later and later.windDir and (later.wind or 0) >= 3 then
            local turn = Sky.angle(hour.windDir, later.windDir)
            if turn > most then most, to = turn, later end
        end
    end
    return most, to
end

-- Sea breeze (SA5) on a date at a sea spot, an estimate: a sunny middle of
-- the day, land at least 4° warmer than the sea, and a light morning wind
-- that does not blow the breeze away. "likely" when the forecast itself
-- shows an onshore build after noon, else "possible"; nil otherwise.
local breezes = setmetatable({}, { __mode = "k" })
function Sailing.seaBreeze(forecast, date)
    local spot = forecast.spot or {}
    if spot.lake or not spot.facing or not forecast.hours then return nil end
    local memo = breezes[forecast.hours] or {}
    breezes[forecast.hours] = memo
    if memo[date] ~= nil then return memo[date] or nil end
    local morning, midday, afternoon, sea, warm = {}, {}, {}, nil, -math.huge
    for _, hour in ipairs(forecast.hours) do
        local h = hour.date == date and Sky.hour(hour.time)
        if h then
            if h >= 9 and h <= 11 then morning[#morning + 1] = hour end
            if h >= 11 and h <= 15 then midday[#midday + 1] = hour end
            if h >= 13 and h <= 17 then afternoon[#afternoon + 1] = hour end
            if h >= 11 and h <= 16 then
                warm = math.max(warm, hour.temp or -math.huge)
                sea = sea or hour.seaTemp
            end
        end
    end
    local function mean(list, key)
        local sum, n = 0, 0
        for _, hour in ipairs(list) do if hour[key] then sum, n = sum + hour[key], n + 1 end end
        return n > 0 and sum / n or nil
    end
    local gradient, cloud = mean(morning, "wind"), mean(midday, "cloud")
    local result = false
    if sea and gradient and cloud and gradient < 5 and cloud < 40 and warm - sea >= 4 then
        local build
        for _, hour in ipairs(afternoon) do
            if (hour.wind or 0) >= gradient + 2 and hour.windDir and Sky.angle(hour.windDir, spot.facing) <= 70 then
                build = build or hour
            end
        end
        result = { word = build and "likely" or "possible", from = build and build.time, land = warm, sea = sea }
    end
    memo[date] = result
    return result or nil
end

function Sailing.rate(hour, forecast)
    local card, spot, rider = Score.card(40), forecast.spot or {}, Profile.current()
    local boat = not rider.board
    if Sky.isStorm(hour.code) then card:cap(5, "Lightning: get off the water.") end
    if not hour.isDay then card:cap(25, "Dark: nobody can see you if you need help.") end

    -- The rider's own wind band (SA2); a gale caps every profile.
    local wind, lo, hi = hour.wind or 0, rider.min, rider.max
    if wind >= GALE then card:cap(15, "Gale: stay ashore.")
    elseif wind < lo * 0.75 then card:add(-25, ("Too little wind for you: %s, you need %s."):format(W(wind), W(lo)))
    elseif wind < lo then card:add(-6, ("Marginal: %s, just under your %s."):format(W(wind), W(lo)))
    elseif wind <= hi then card:add(25, ("%s: in your %s range."):format(W(wind), range(lo, hi)))
    elseif rider.level == "beginner" then card:cap(25, ("Too strong while you learn: %s, over your %s."):format(W(wind), W(hi)))
    elseif wind <= hi * 1.2 then card:add(-15, ("Overpowered: %s, over your %s."):format(W(wind), W(hi)))
    else card:cap(20, ("Far too strong for you: %s."):format(W(wind))) end

    -- Gusts, lulls and squalls (SA3): forecast gusts are smoothed, so
    -- unstable air and a falling glass count against the hour.
    local usable = wind >= lo * 0.75
    local ratio = gustiness(hour)
    if usable and ratio >= 1.6 then card:add(-15, "Very gusty: lulls and sudden pulls.")
    elseif usable and ratio >= 1.35 then card:add(-6, "Gusty wind.")
    elseif usable then card:add(6, "Steady wind.") end
    if usable and wind <= hi and hour.gust and hour.gust > hi then
        card:add(rider.level == "beginner" and -12 or -6, ("Gusts to %s, over your %s."):format(W(hour.gust), W(hi)))
    end
    local lull = hour.gust and wind - (hour.gust - wind)
    if lull and wind >= lo and lull < lo then
        card:add(-6, ("Lulls may drop to about %s, under your %s (an estimate)."):format(W(math.max(0, lull)), W(lo)))
    end
    local cape, showery = hour.cape or 0, (hour.rainChance or 0) >= 30 or Sky.isWet(hour.code)
    if not Sky.isStorm(hour.code) then
        if cape >= 1500 and showery then card:cap(35, "Unstable air and showers: squalls could bring gusts far above the forecast.")
        elseif cape >= 500 then card:add(-10, "Unstable air: gusts may run higher than forecast.") end
    end
    local trend = hour.pressureTrend or 0
    if trend <= -6 then card:cap(25, "Pressure falling very fast: a front or a storm is close.")
    elseif trend <= -3.5 then card:add(-10, "Pressure falling quickly: the wind may rise and turn.") end
    local turn, to = shift(forecast, hour)
    if usable and turn >= 45 then
        card:add(-8, ("The wind turns from %s to %s by %s."):format(Sky.compass(hour.windDir), Sky.compass(to.windDir),
            Units.clock(to.time)))
    end
    if hour.visibility and hour.visibility < 1000 then card:cap(30, "Fog: other boats cannot see you.")
    elseif hour.visibility and hour.visibility < 4000 then card:add(-8, "Poor visibility: hard to see and be seen.") end
    if (hour.rain or 0) >= 4 then card:add(-6, "Heavy rain.") end

    -- The shore and the spot's safe sector (SA8). Offshore wind is a ban
    -- for kites and boards and a caution for boats.
    local relation = windTo(hour, spot)
    local sector = Spots.sector(spot)
    if wind >= 3 then
        if relation == "offshore" then
            if boat then
                card:add(rider.level == "beginner" and -25 or -12, "Offshore wind: easy to sail out, hard to come back.")
            else card:cap(30, "Offshore wind: it blows you away from the shore.") end
        elseif not Spots.inside(sector, hour.windDir) then
            local text = ("Wind from the %s, outside this spot's safe %s."):format(Sky.compass(hour.windDir), Spots.text(sector))
            if boat then card:add(-8, text) else card:cap(35, text) end
        elseif relation == "cross-shore" then
            card:add(6, boat and "Cross-shore wind: an easy launch and return." or "Cross-shore wind: the safest for kiting.")
        end
    end

    if not spot.lake and hour.waveHeight and hour.waveHeight > 2.5 then card:add(-10, "Big waves at the shore break.") end
    if not spot.lake and Tides.windAgainst(hour) then card:add(-8, "Wind against tide: short, steep chop (approximate).") end
    if (hour.temp or 15) < 8 then card:add(-6, "Cold: wear a thick wetsuit or a drysuit.") end

    -- How sure the forecast is (SA4).
    local agreement = hour.agreement
    if agreement and agreement.word == "uncertain" then
        card:add(-6, ("The models disagree: anywhere from %s."):format(range(agreement.low, agreement.high)))
    elseif agreement and agreement.word == "sure" then card:add(3, "The weather models agree.")
    elseif agreement then card:note("The models roughly agree.") end

    local h = Sky.hour(hour.time)
    local breeze = h and h >= 12 and h <= 18 and Sailing.seaBreeze(forecast, hour.date)
    if breeze then card:note("A sea breeze may build onshore this afternoon (an estimate).") end
    return card
end

function Sailing.window(hour) return hour.isDay end

-- What to rig in this hour's wind, when the rider can use it.
local function rigFor(hour, rider)
    local wind = hour.wind or 0
    if wind < rider.min * 0.75 or wind > rider.max * 1.2 or wind >= GALE then return nil end
    return Profile.rig(rider, wind)
end

function Sailing.note(hour, forecast)
    local text = ("%s · %s %s"):format(Units.gust(hour.wind, hour.gust), windTo(hour, forecast.spot or {}),
        Sky.compass(hour.windDir))
    local rider = Profile.current()
    local rig = hour.isDay ~= false and rigFor(hour, rider)
    if rig and rider.board then text = text .. " · " .. rig:gsub("^the ", "")
    elseif rig and rig ~= "full main" and rig ~= "full power" then text = text .. " · " .. rig end
    if hour.agreement then text = text .. " · " .. hour.agreement.word end
    return text
end

-- "ECMWF 16 · GFS 19 kn": each model's wind at an hour.
local function strip(agreement)
    local parts = {}
    for _, m in ipairs(agreement.models) do
        parts[#parts + 1] = m.name .. " " .. math.floor(Units.value("wind", m.wind) + 0.5)
    end
    return table.concat(parts, " · ") .. " " .. Units.label("wind")
end
Sailing.strip = strip

function Sailing.metrics(hour, forecast)
    local spot, ratio, rider = forecast.spot or {}, gustiness(hour), Profile.current()
    local sector = Spots.sector(spot)
    local rig, why = Profile.rig(rider, hour.wind)
    local squally = (hour.cape or 0) >= 500 and hour.gust
    local list = {
        { label = "wind", value = Units.gust(hour.wind, hour.gust) .. " " .. Sky.compass(hour.windDir),
            note = windTo(hour, spot) .. (sector and " · safe " .. Spots.text(sector) or "") .. " · wind, then gusts" },
        { label = "gusts", value = Units.wind(hour.gust),
            note = squally and ("could reach %s in a squall (an estimate)"):format(W(hour.gust * 1.3))
                or ratio >= 1.35 and "gusty" or "steady" },
        { label = "your range", value = range(rider.min, rider.max), note = Profile.title(rider) },
    }
    if rig then list[#list + 1] = { label = "rig", value = rig, note = why } end
    if hour.agreement then
        list[#list + 1] = { label = "models", value = hour.agreement.word, note = strip(hour.agreement) }
    end
    list[#list + 1] = { label = "waves", value = spot.lake and "lake" or Units.height(hour.waveHeight),
        note = spot.lake and "no sea swell" or "period " .. Score.fmt(hour.wavePeriod, " s") }
    if not spot.lake and hour.seaLevel then
        list[#list + 1] = { label = "tide", value = Units.height(hour.seaLevel),
            note = (hour.current and ("stream %s to the %s · "):format(W(hour.current), Sky.compass(hour.currentDir)) or "")
                .. "approximate, not for navigation" }
    end
    list[#list + 1] = { label = "water", value = Units.temp(hour.seaTemp), note = "air " .. Units.temp(hour.temp) }
    list[#list + 1] = Report.sun(hour)
    return list
end

-- The hour to plan around today: now when it is usable daylight, else
-- today's best window, else tomorrow's. Returns the hour, its result, the
-- window text and when ("now", "today", "tomorrow").
local function planned(forecast)
    local now = forecast.hours[forecast.now]
    if not now then return nil end
    local current = Report.rate(Sailing, now, forecast)
    if now.isDay and current.verdict ~= "poor" then return now, current, nil, "now" end
    -- Today's best window if it is any good, else tomorrow's, else the
    -- better of two poor ones.
    local days, fallback = Report.daysAhead(forecast), nil
    for index = 1, math.min(2, #days) do
        local best, span, hour = Report.bestOfDay(Sailing, forecast, days[index].date)
        local entry = best and { hour, best, span, index == 1 and "today" or "tomorrow" }
        if best and best.verdict ~= "poor" then return table.unpack(entry) end
        if best and (not fallback or best.score > fallback[2].score) then fallback = entry end
    end
    if fallback then return table.unpack(fallback) end
    return now, current, nil, "now"
end

-- Safety in the next 12 hours: the first thunder, gale, squall, fog or
-- falling glass, as a card. Never behind a setting.
local function hazard(forecast)
    for index = forecast.now, math.min(#forecast.hours, forecast.now + 11) do
        local hour = forecast.hours[index]
        local at = index == forecast.now and "now" or "from " .. Units.clock(hour.time)
        if Sky.isStorm(hour.code) then return "thunder " .. at, "Lightning: get off the water and stay off." end
        if (hour.wind or 0) >= GALE or (hour.gust or 0) >= GALE * 1.4 then
            return "gale " .. at, ("Wind %s: stay ashore."):format(Units.gust(hour.wind, hour.gust))
        end
        if (hour.cape or 0) >= 1500 and ((hour.rainChance or 0) >= 30 or Sky.isWet(hour.code)) then
            return "squalls " .. at, "Unstable air and showers: gusts could be far above the forecast."
        end
        if (hour.pressureTrend or 0) <= -6 then return "storm " .. at, "Pressure falling very fast: a front is close." end
        if hour.visibility and hour.visibility < 1000 and hour.isDay then return "fog " .. at, "Other boats cannot see you." end
    end
end

-- The big answers (F7): what to rig, any danger, the best spot this
-- weekend, how sure the forecast is, a sea breeze, the tide.
function Sailing.cards(forecast, place)
    local cards, rider = {}, Profile.current()
    local hour, result, span, when = planned(forecast)
    if not hour then return cards end
    local label = when == "now" and "now" or when .. " " .. (span or Units.clock(hour.time))
    local rig, why = rigFor(hour, rider)
    if rig and result.verdict ~= "poor" then
        cards[#cards + 1] = { id = "rig", title = "rig · " .. label, value = rig, tone = result.verdict,
            detail = Units.gust(hour.wind, hour.gust) .. " " .. Sky.compass(hour.windDir) .. " · " .. why }
    else
        local reason = result.reasons[1]
        cards[#cards + 1] = { id = "rig", title = when == "tomorrow" and "today and tomorrow" or "today",
            value = "stay ashore", tone = "poor",
            detail = reason and reason.text or "No usable wind for you today or tomorrow." }
    end

    local danger, because = hazard(forecast)
    if danger then cards[#cards + 1] = { id = "hazard", title = "watch out", value = danger, detail = because, tone = "poor" } end

    local dates = Rank.dates(forecast)
    if forecast.spots and #forecast.spots > 0 and #dates > 0 then
        local ranked = Rank.rank(place or forecast.spot or {}, forecast, dates, function(f, date)
            return Report.bestOfDay(Sailing, f, date)
        end)
        local top, next = ranked[1], ranked[2]
        if top then
            local day = Sky.weekday(top.date):sub(1, 3)
            local detail = ("%s %s %s"):format(top.result.verdict, day, top.span)
            if next then
                detail = detail .. (" · then %s, %s %s"):format(next.place.title, Sky.weekday(next.date):sub(1, 3),
                    next.result.verdict)
            end
            cards[#cards + 1] = { id = "weekend", title = "this weekend", tone = top.result.verdict,
                value = top.result.verdict == "poor" and "no good wind" or top.place.title,
                detail = top.result.verdict == "poor" and ("best: %s, %s"):format(top.place.title, day) or detail }
        end
    end

    if hour.agreement then
        local tone = ({ sure = "good", likely = "fair", uncertain = "poor" })[hour.agreement.word]
        cards[#cards + 1] = { id = "models", title = "forecast confidence · " .. label, value = hour.agreement.word,
            detail = strip(hour.agreement), tone = tone }
    end

    local breeze = Sailing.seaBreeze(forecast, hour.date)
    if breeze then
        cards[#cards + 1] = { id = "breeze", title = "sea breeze", tone = "info",
            value = breeze.word .. (breeze.from and " from " .. Units.clock(breeze.from) or " after noon"),
            detail = ("sunny, land %s warmer than the sea, light morning wind. An estimate: sea breezes are poorly "
                .. "forecast, so check a station."):format(Units.tempDelta(breeze.land - breeze.sea)) }
    end

    local spot = forecast.spot or place or {}
    if forecast.tides and not spot.lake then
        local now = forecast.hours[forecast.now]
        local turns, ahead = {}, {}
        for _, day in ipairs(Report.daysAhead(forecast)) do
            for _, t in ipairs(Tides.turns(forecast, day.date)) do turns[#turns + 1] = t end
            if #turns >= 4 then break end
        end
        for _, t in ipairs(turns) do if t.time >= now.time then ahead[#ahead + 1] = t end end
        if ahead[1] then
            local against
            for index = forecast.now, math.min(#forecast.hours, forecast.now + 11) do
                local h = forecast.hours[index]
                if h.isDay and Tides.windAgainst(h) then against = against or h.time end
            end
            local detail = ahead[2] and ("then %s %s · "):format(ahead[2].kind, Units.clock(ahead[2].time)) or ""
            if against then detail = detail .. "wind against tide from " .. Units.clock(against) .. " · " end
            cards[#cards + 1] = { id = "tide", title = "tide · approximate", tone = "info",
                value = ("%s about %s"):format(ahead[1].kind, Units.clock(ahead[1].time)),
                detail = detail .. "a model sea level, about 8 km wide: not for navigation" }
        end
    end
    return cards
end

-- The two charts on the activity's "today" view, over the next 24 hours.
-- `value` is metric; `unit` (a Units kind) converts it and ends the title.
Sailing.charts = {
    { title = "wind", unit = "wind", kind = "column", value = function(hour) return hour.wind or 0 end },
    { title = "gusts", unit = "wind", kind = "line", value = function(hour) return hour.gust or 0 end },
}

return Sailing
