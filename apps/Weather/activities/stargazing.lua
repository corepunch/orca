-- Stargazing: will the night sky be dark and clear? Stargazers need a
-- fully dark sky, no clouds (thin high cloud and smoke included), the moon
-- down or thin, dry optics, still air and no fog. Its best windows are at
-- night, unlike every other profile, and a night runs from sunset to
-- sunrise across midnight (editions/stargazing/night.lua). The sun's and
-- moon's heights are computed for the place, so darkness is real at every
-- latitude; hand-built hours without a place fall back to estimates.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Moon = require "Weather/editions/stargazing/moon"
local Night = require "Weather/editions/stargazing/night"
local Seeing = require "Weather/editions/stargazing/seeing"
local Sources = require "Weather/editions/stargazing/sources"
local Aurora = require "Weather/editions/stargazing/aurora"
local RedMode = require "Weather/editions/stargazing/redmode"

local Stargazing = {
    id = "stargazing", title = "stars", needsMarine = false,
}

-- Upper winds and temperatures for seeing, water vapour for transparency,
-- the mixing height for fog.
Stargazing.fields = {
    hourly = { "wind_speed_250hPa", "temperature_850hPa", "temperature_700hPa", "temperature_500hPa",
        "geopotential_height_850hPa", "geopotential_height_700hPa", "geopotential_height_500hPa",
        "total_column_integrated_water_vapour", "boundary_layer_height" },
}
Stargazing.sources = { Sources.air, Sources.models, Aurora.source }

local REDMODE = { { "auto", "dusk to dawn" }, { "on", "always" }, { "off", "off" } }
Stargazing.settings = {
    { key = "redmode", title = "red night mode", kind = "choice", default = "auto", options = REDMODE,
        detail = "red type on black keeps your eyes used to the dark",
        format = function(value)
            for _, entry in ipairs(REDMODE) do if entry[1] == value then return entry[2] end end
            return tostring(value)
        end },
}

-- Red night mode through the app's hooks: every screen opens in the sheet
-- it asks for, and each drawn page (a forecast, or the settings after a
-- change) tells it where and when it is, so the screens follow at dusk.
function Stargazing.stylesheet() return RedMode.stylesheet() end
function Stargazing.pageClasses(forecast)
    if forecast then RedMode.follow(forecast) end
    return RedMode.on() and "night" or ""
end

-- Whether the forecast's own day and night (`isDay`) agree with the sun
-- computed for its place and clock. They always do for a real answer; a
-- wrong time zone or a mislabelled place would put "midnight" in daylight,
-- and then the forecast's own day and night are the safer guide.
local function agrees(forecast)
    if forecast.agrees ~= nil then return forecast.agrees end
    local spot, wrong, count = forecast.spot, 0, 0
    for _, hour in ipairs(forecast.hours or {}) do
        if hour.epoch and hour.isDay ~= nil then
            local altitude = Sky.sunAltitude(hour.epoch + 1800, spot.lat, spot.lon)
            count = count + 1
            if (hour.isDay and altitude < -10) or (not hour.isDay and altitude > 10) then wrong = wrong + 1 end
        end
    end
    forecast.agrees = wrong <= count / 10
    return forecast.agrees
end

-- The place to compute the sky for, or nil (hand-built hours, or a
-- forecast whose clock does not fit its place).
local function spotOf(forecast)
    local spot = forecast and forecast.spot
    if not (spot and spot.lat and spot.lon) then return nil end
    return agrees(forecast) and spot or nil
end

-- How dark an hour is ("day", "civil", "nautical", "astronomical" or
-- "night"), from the sun at the middle of the hour; nil without a place.
local function darkness(hour, forecast)
    local spot = spotOf(forecast)
    if spot and hour.epoch then return Night.level(Sky.sunAltitude(hour.epoch + 1800, spot.lat, spot.lon)) end
    local untrusted = forecast and forecast.spot and forecast.agrees == false
    return not untrusted and hour.sunAltitude and Night.level(hour.sunAltitude) or nil
end
Stargazing.darkness = darkness

-- The moon at the middle of an hour: { altitude, up, lit, name }. Without
-- a place, the old estimate from the phase (good to about an hour).
local function moonAt(hour, forecast)
    local spot, epoch = spotOf(forecast), hour.epoch
    if spot and epoch then
        local phase = Moon.phase(epoch + 1800)
        local altitude = Moon.altitude(epoch + 1800, spot.lat, spot.lon)
        return { altitude = altitude, up = altitude > Moon.HORIZON, lit = phase.lit, name = phase.name }
    end
    local moon = Sky.moon(epoch or (forecast and forecast.epoch) or os.time())
    moon.up = Sky.moonUp(moon.phase, Sky.hour(hour.time) or 0)
    return moon
end
Stargazing.moonAt = moonAt

-- Clouds that hide stars outright (low and middle), apart from the high
-- veil that only dims them. Total cover when the layers are missing.
local function blocking(hour)
    if hour.cloudLow or hour.cloudMid then return math.max(hour.cloudLow or 0, hour.cloudMid or 0) end
    return hour.cloud or 100
end

local function clearAndCalm(hour) return blocking(hour) < 30 and (hour.wind or 0) < 3 end

-- Dew or frost on the optics: { frost, sure, text } or nil. Glass under a
-- clear, calm sky cools a degree or two below the air, so dew comes early.
function Stargazing.dew(hour)
    local gap = hour.temp and hour.dewPoint and hour.temp - hour.dewPoint
    if not gap then
        if (hour.humidity or 0) < 90 then return nil end
        return { sure = false, frost = false, text = "Damp air: dew on lenses." }
    end
    local margin = gap - (clearAndCalm(hour) and 1.5 or 0)
    if margin > 3 then return nil end
    local frost, sure = hour.dewPoint < 0, margin <= 1
    local text = frost and (sure and "Frost on the optics: heaters on, caps on." or "Frost likely on the optics: heaters on.")
        or (sure and "Dew is certain: optics fog without a heater." or "Dew likely on the optics: heaters on.")
    return { frost = frost, sure = sure, text = text, gap = gap }
end

-- Fog now ("fog"), fog likely to form ("risk"), or nil. Calm, saturated air
-- with a shallow mixed layer near the ground; the models miss much of it.
function Stargazing.fog(hour)
    if hour.code == 45 or hour.code == 48 or (hour.visibility and hour.visibility < 1000) then return "fog" end
    local gap = hour.temp and hour.dewPoint and hour.temp - hour.dewPoint
    if not gap or gap > 1 or (hour.wind or 0) >= 3 then return nil end
    local shallow = hour.boundary_layer_height
    if (shallow and shallow < 200) or (hour.cloudLow or 0) >= 50 or (hour.visibility and hour.visibility < 5000) then
        return "risk"
    end
end

-- Where the aurora would show at this hour, or nil: only after dark.
local function aurora(hour, forecast, level)
    local spot = spotOf(forecast)
    if not (spot and hour.kp) or not (level == "night" or level == "astronomical" or level == "nautical") then return nil end
    return Aurora.reach(spot.lat, spot.lon, hour.kp)
end

local function poleward(spot) return spot and spot.lat < 0 and "south" or "north" end

function Stargazing.rate(hour, forecast)
    local card = Score.card(50)
    if Sky.isStorm(hour.code) then card:cap(5, "Thunderstorm.") end
    local level = darkness(hour, forecast)
    if level == "day" then card:cap(5, "Daylight.")
    elseif level == "civil" then card:cap(20, "Twilight: the sky is still bright.")
    elseif level == "nautical" then card:cap(40, "Nautical twilight: only bright stars and planets show.")
    elseif level == "astronomical" then card:cap(65, "Not fully dark: the sun is less than 18° below the horizon.")
    elseif level == nil then
        if hour.isDay then card:cap(5, "Daylight.")
        elseif Score.twilight(hour) <= 90 then card:cap(35, "Twilight: the sky is not fully dark yet.") end
    end

    local below, high = blocking(hour), hour.cloudHigh or 0
    if below >= 60 then card:add(-35, "Cloudy: nothing to see.")
    elseif below >= 30 then card:add(-15, "Partly cloudy: stars only in the gaps.")
    elseif below >= 10 then card:add(5, "A few low clouds drifting by.")
    elseif high < 10 then card:add(25, "Clear sky.")
    else card:add(20, "Clear below the high cloud.") end
    if below < 60 then
        if high >= 60 then card:add(-25, "A veil of high cloud: only the moon and planets shine through.")
        elseif high >= 30 then card:add(-10, "Thin high cloud: a milky sky for faint objects.")
        elseif high >= 10 then card:add(-4, "A little high cloud dims faint objects.") end
    end
    if Sky.isWet(hour.code) or Sky.isSnow(hour.code) then card:add(-15, "Rain or snow: keep the telescope covered.") end

    -- Other weather models: say so when they see it differently.
    local models = Sources.agreement(hour)
    if models then
        if below < 30 and models.chance <= 0.34 then card:add(-10, "Most weather models expect cloud.")
        elseif below >= 60 and models.chance >= 0.67 then card:add(10, "Other weather models expect clear spells.")
        elseif models.spread >= 50 then card:note("The weather models disagree about clouds: uncertain.") end
    end

    -- Smoke, dust and water vapour make a cloudless sky milky.
    local aod = hour.aod
    if aod and aod >= 0.4 then card:add(-15, "Smoke or dust in the air: a milky sky.")
    elseif aod and aod >= 0.2 then card:add(-7, "Haze in the air dims faint stars.")
    elseif (hour.visibility or 20000) < 10000 then card:add(-8, "Hazy air dims the stars.") end
    if (hour.total_column_integrated_water_vapour or 0) >= 25 then card:add(-4, "Moist air: a softer, less clear sky.") end

    local moon = moonAt(hour, forecast)
    if moon.altitude then
        -- A moon high up lights the sky more than one on the horizon.
        local light = moon.up and moon.lit * math.max(0.15, math.min(1, (moon.altitude + 2) / 32)) or 0
        if not moon.up then card:add(10, "The moon is down: a dark sky.")
        elseif light >= 0.45 then card:add(-20, "Bright moon high up: faint stars wash out.")
        elseif light >= 0.2 then card:add(-10, "The moon is up and lights the sky.")
        elseif light >= 0.06 then card:add(-4, "A low or thin moon adds a little light.")
        else card:add(6, "Only a thin moon: the sky stays dark.") end
    elseif moon.up and moon.lit > 0.6 then card:add(-20, "Bright moon up: faint stars wash out.")
    elseif moon.up and moon.lit > 0.25 then card:add(-8, "The moon is up.")
    else card:add(10, "Moon down or thin: a dark sky.") end

    local fog = Stargazing.fog(hour)
    if fog == "fog" then card:cap(20, "Fog: nothing to see.")
    elseif fog == "risk" then card:add(-10, "Fog may form: calm, saturated air. Valley sites fog first.") end
    local dew = Stargazing.dew(hour)
    if dew then card:add(dew.sure and -8 or -4, dew.text) end
    if (hour.wind or 0) > 8 then card:add(-6, "Windy: a shaky telescope.")
    elseif (hour.gust or 0) > 12 then card:add(-4, "Gusts shake the telescope.") end
    if (hour.temp or 10) < 0 then card:note("Below freezing: dress warmly.") end
    local reach = below < 60 and aurora(hour, forecast, level)
    if reach then
        card:note(reach == "overhead" and "Aurora likely (estimate from Kp)."
            or ("Aurora possible low in the %s (estimate from Kp)."):format(poleward(spotOf(forecast))))
    end
    return card
end

-- Dark hours only: full darkness or the last of astronomical twilight.
function Stargazing.window(hour, forecast)
    local level = darkness(hour, forecast)
    if level then return level == "night" or level == "astronomical" end
    return not hour.isDay and Score.twilight(hour) > 90
end

-- A night belongs to the evening it began: hours before noon count for the
-- day before, so "today" means tonight, across midnight.
function Stargazing.nightOf(hour)
    local h = Sky.hour(hour.time)
    if not (h and h < 12 and hour.date) then return hour.date end
    local start = Sky.epochOf(hour.date)
    return start and os.date("!%Y-%m-%d", start - 86400) or hour.date
end

-- The charts run over tonight, dusk to dawn (the night going on, or the
-- next one), not the next 24 hours.
function Stargazing.chartHours(forecast)
    local frame = spotOf(forecast) and Night.tonight(forecast)
    if not (frame and frame.from) then return nil end
    local first, last
    for index, hour in ipairs(forecast.hours) do
        if hour.epoch and hour.epoch + 3600 > frame.from and hour.epoch < frame.to then
            first, last = first or index, index
        end
    end
    return first, last
end

-- The card shows the first dark hour from now: tonight, or right now.
function Stargazing.focus(forecast)
    for index = forecast.now, #forecast.hours do
        if Stargazing.window(forecast.hours[index], forecast) then return index end
    end
    return forecast.now
end

local function moonText(moon)
    if not moon.up then return "moon down" end
    return moon.altitude and ("moon up %d°"):format(math.floor(moon.altitude + 0.5)) or "moon up"
end

local DARK_WORDS = { day = "day", civil = "twilight", nautical = "twilight", astronomical = "almost dark", night = "dark" }

local function cloudText(hour)
    local text = Score.fmt(hour.cloud, "%")
    if (hour.cloudHigh or 0) >= 10 then text = text .. (" (high %s)"):format(Score.fmt(hour.cloudHigh, "%")) end
    return text
end

function Stargazing.note(hour, forecast)
    local level = darkness(hour, forecast)
    local text = ("clouds %s · %s · %s"):format(cloudText(hour), moonText(moonAt(hour, forecast)), Units.temp(hour.temp))
    return level and DARK_WORDS[level] .. " · " .. text or text
end

-- Text for a span of epochs: "21:40–02:10".
local function between(from, to, offset)
    return Units.clock(Night.iso(from, offset)) .. "–" .. Units.clock(Night.iso(to, offset))
end
Stargazing.between = between

-- The hours of tonight from now, each rated, and the longest run of good
-- ones in a row: { hours, best, run = { first, last, count } }.
local function judge(forecast, frame, from)
    local rated, best, run, current = {}, nil, nil, nil
    for _, hour in ipairs(Night.hours(forecast, frame, from)) do
        local result = Stargazing.rate(hour, forecast):result()
        rated[#rated + 1] = { hour = hour, result = result }
        if not best or result.score > best.result.score then best = rated[#rated] end
        if result.verdict == "good" then
            if current and current.last.epoch == hour.epoch - 3600 then current.last, current.count = hour, current.count + 1
            else current = { first = hour, last = hour, count = 1 } end
            if not run or current.count > run.count then run = { first = current.first, last = current.last, count = current.count } end
        else current = nil end
    end
    return { hours = rated, best = best, run = run }
end
Stargazing.judge = judge

-- go, maybe or stay in, for a judged night.
local function decide(night)
    if night.run and night.run.count >= 2 then return "go", "good" end
    local fair = 0
    for _, entry in ipairs(night.hours) do if entry.result.verdict ~= "poor" then fair = fair + 1 end end
    if night.run or fair >= 2 then return "maybe", "fair" end
    return "stay in", "poor"
end
Stargazing.decide = decide

-- The nights after tonight with a "go", as "thu 19, sat 21".
local function goodNights(forecast, tonight)
    local names = {}
    for _, frame in ipairs(Night.week(forecast)) do
        if frame ~= tonight and frame.sunset and decide(judge(forecast, frame)) == "go" then
            names[#names + 1] = Sky.weekday(frame.sunset:sub(1, 10))
        end
    end
    return names
end

local function agreementText(night)
    local count, clear, models = 0, 0, 0
    for _, entry in ipairs(night.hours) do
        local agreement = Sources.agreement(entry.hour)
        if agreement and Stargazing.window(entry.hour, night.forecast) then
            count, clear, models = count + 1, clear + agreement.chance, agreement.count
        end
    end
    if count == 0 then return nil end
    local chance = math.floor(clear / count * 100 / 5 + 0.5) * 5
    if chance == 0 then return ("none of %d weather models sees it clear"):format(models) end
    if chance == 100 then return ("all %d weather models see it clear"):format(models) end
    return ("about %d%% chance of clear (%d weather models)"):format(chance, models)
end

-- The reason heard most often across the night's hours.
local function commonReason(night)
    local counts, top = {}, nil
    for _, entry in ipairs(night.hours) do
        local reason = entry.result.reasons[1]
        if reason then
            counts[reason.text] = (counts[reason.text] or 0) + 1
            if not top or counts[reason.text] > counts[top] then top = reason.text end
        end
    end
    return top
end

local function tonightCard(forecast, frame, night)
    local word, tone = decide(night)
    local detail
    if word == "go" then
        local run = night.run
        detail = ("clear and dark %s · %d h in a row"):format(between(run.first.epoch, run.last.epoch + 3600, forecast.offset),
            run.count)
    elseif word == "maybe" then
        local reason = night.best.result.reasons[1]
        detail = ("best from %s · %s"):format(Units.clock(night.best.hour.time), reason and reason.text or "")
    elseif night.best then
        detail = commonReason(night) or ""
    else
        detail = "the night is over"
    end
    local chance = agreementText(night)
    if chance then detail = detail .. " · " .. chance end
    if word ~= "go" then
        local later = goodNights(forecast, frame)
        detail = detail .. (#later > 0 and " · next good night: " .. later[1] or "")
    end
    return { id = "tonight", title = "tonight", value = word, detail = detail, tone = tone,
        icon = night.best and Sky.icon(night.best.hour.code, false) or nil }
end

local function darknessCard(forecast, frame)
    local offset = forecast.offset
    if frame.dark then
        local detail = frame.sunset and frame.sunrise and ("sunset %s · sunrise %s"):format(Units.clock(frame.sunset),
            Units.clock(frame.sunrise)) or "the sun stays down all day"
        return { id = "darkness", title = "full darkness", value = between(frame.dark.from, frame.dark.to, offset),
            detail = detail, tone = "info" }
    end
    local deepest = { astronomical = "astronomical twilight", nautical = "nautical twilight", civil = "civil twilight" }
    return { id = "darkness", title = "full darkness", value = "not tonight", tone = "fair",
        detail = ("the sun stays less than 18° down: the sky never gets past %s"):format(deepest[frame.level] or "twilight") }
end

local function moonCard(forecast, frame)
    local offset, moon = forecast.offset, frame.moon
    local parts = { ("%s, %d%% lit"):format(moon.name, math.floor(moon.lit * 100 + 0.5)) }
    if frame.moonRise then parts[#parts + 1] = "rises " .. Units.clock(Night.iso(frame.moonRise, offset)) end
    if frame.moonSet then parts[#parts + 1] = "sets " .. Units.clock(Night.iso(frame.moonSet, offset)) end
    local value, tone
    local free = 0
    for _, span in ipairs(frame.moonFree) do free = free + (span.to - span.from) end
    if frame.moonUp == "none" then value, tone = "down all night", "good"
    elseif moon.lit < 0.15 then value, tone = "a thin moon", "good"
    elseif #frame.moonFree > 0 then
        local first = frame.moonFree[1]
        value = "moon-free " .. between(first.from, first.to, offset)
        if #frame.moonFree > 1 then value = value .. " +" end
        tone = free >= 2 * 3600 and "good" or "fair"
    elseif frame.dark then value, tone = "up all the dark hours", "poor"
    else value, tone = frame.moonUp == "all" and "up all night" or "up tonight", "fair" end
    return { id = "moon", title = "moon", value = value, detail = table.concat(parts, " · "), tone = tone }
end

-- The first night hour with dew or frost, and with fog.
local function firstOf(night, test)
    for _, entry in ipairs(night.hours) do
        local found = test(entry.hour)
        if found then return entry.hour, found end
    end
end

local function dewCard(night)
    local hour, dew = firstOf(night, Stargazing.dew)
    if not hour then
        local driest = night.hours[1] and night.hours[1].hour
        return { id = "dew", title = "dew", value = "dry optics", tone = "good",
            detail = driest and driest.temp and driest.dewPoint and ("the air stays %s or more above the dew point")
                :format(Units.tempDelta(3)) or "dry air tonight" }
    end
    local what = dew.frost and "frost" or "dew"
    return { id = "dew", title = dew.frost and "frost" or "dew", tone = dew.sure and "poor" or "fair",
        value = hour.epoch <= night.from and "heaters on now" or ("heaters from %s"):format(Units.clock(hour.time)),
        detail = hour.dewPoint and ("%s likely: air %s, dew point %s at %s"):format(what, Units.temp(hour.temp),
            Units.temp(hour.dewPoint), Units.clock(hour.time)) or "damp air" }
end

local function fogCard(night)
    local hour, fog = firstOf(night, Stargazing.fog)
    if not hour then return nil end
    local when = hour.epoch <= night.from and "now" or "from " .. Units.clock(hour.time)
    return { id = "fog", title = "fog", tone = fog == "fog" and "poor" or "fair",
        value = (fog == "fog" and "fog %s" or "possible %s"):format(when),
        detail = "calm, saturated air: valley sites fog first, and forecasts often miss it" }
end

-- Seeing and transparency at the night's best hour, labelled as estimates.
local function skyCard(forecast, night)
    local entry = night.run and { hour = night.run.first } or night.best
    if not entry then return nil end
    local seeing, clarity = Seeing.seeing(entry.hour, forecast.elevation), Seeing.transparency(entry.hour)
    local why = {}
    if seeing and seeing.why then why[#why + 1] = seeing.why end
    if clarity.why then why[#why + 1] = clarity.why end
    return { id = "sky", title = "seeing · transparency (estimate)", tone = "info",
        value = (seeing and seeing.word or "—") .. " · " .. clarity.word,
        detail = ("at %s · %s"):format(Units.clock(entry.hour.time), #why > 0 and table.concat(why, ", ") or "from model winds and moisture") }
end

local function auroraCard(forecast, night)
    local spot, best, clear = spotOf(forecast), nil, false
    for _, entry in ipairs(night.hours) do
        local reach = aurora(entry.hour, forecast, darkness(entry.hour, forecast))
        if reach and (not best or reach == "overhead" or entry.hour.kp > best.hour.kp) then
            best = { hour = entry.hour, reach = reach }
        end
        if reach and blocking(entry.hour) < 60 then clear = true end
    end
    if not best then return nil end
    local where = best.reach == "overhead" and "likely" or "low in the " .. poleward(spot)
    return { id = "aurora", title = "aurora (estimate)", tone = clear and (best.reach == "overhead" and "good" or "fair") or "poor",
        value = clear and where or "behind clouds",
        detail = ("Kp %.0f forecast around %s; best away from town lights, looking %s"):format(best.hour.kp,
            Units.clock(best.hour.time), poleward(spot)) }
end

-- Tonight's big answers: the verdict for the whole night, darkness, the
-- moon, dew, fog, the sky's quality, the aurora and the next good nights.
function Stargazing.cards(forecast)
    local frame = spotOf(forecast) and Night.tonight(forecast)
    if not frame then return {} end
    if frame.polar == "day" then
        return { { id = "tonight", title = "tonight", value = "no night", tone = "poor",
            detail = "the sun does not set: stars wait for later in the season" } }
    end
    local night = judge(forecast, frame, forecast.epoch)
    night.forecast, night.from = forecast, forecast.epoch
    local cards = { tonightCard(forecast, frame, night), darknessCard(forecast, frame), moonCard(forecast, frame) }
    if #night.hours > 0 then
        cards[#cards + 1] = dewCard(night)
        cards[#cards + 1] = fogCard(night)
        -- Seeing and transparency matter only when there is something to see.
        if decide(night) ~= "stay in" then cards[#cards + 1] = skyCard(forecast, night) end
        cards[#cards + 1] = auroraCard(forecast, night)
    end
    local list = {}
    for index = 1, #cards do if cards[index] then list[#list + 1] = cards[index] end end
    return list
end

function Stargazing.metrics(hour, forecast)
    local moon = moonAt(hour, forecast)
    local spot = spotOf(forecast)
    local moonNote = Score.fmt(moon.lit * 100, "% lit · ") .. (moon.altitude and (moon.up
        and ("%d° up"):format(math.floor(moon.altitude + 0.5)) or "below the horizon")
        or (moon.up and "about up now" or "about below the horizon"))
    if spot then
        local events = Moon.events(hour.date or hour.time:sub(1, 10), spot.lat, spot.lon, forecast.offset)
        if events.rise then moonNote = moonNote .. " · rises " .. Units.clock(events.rise) end
        if events.set then moonNote = moonNote .. " · sets " .. Units.clock(events.set) end
    end
    local seeing, clarity = Seeing.seeing(hour, forecast and forecast.elevation), Seeing.transparency(hour)
    local metrics = {
        { label = "clouds", value = Score.fmt(hour.cloud, "%"),
            note = ("low %s · middle %s · high %s"):format(Score.fmt(hour.cloudLow, "%"), Score.fmt(hour.cloudMid, "%"),
                Score.fmt(hour.cloudHigh, "%")) },
        { label = "moon", value = moon.name, note = moonNote },
        { label = "seeing (estimate)", value = seeing and seeing.word or "—",
            note = seeing and (seeing.why or "") .. (seeing.jet and (" · jet stream " .. Units.wind(seeing.jet)) or "") },
        { label = "transparency (estimate)", value = clarity.word, note = clarity.why },
        { label = "dew point", value = Units.temp(hour.dewPoint),
            note = hour.temp and hour.dewPoint and ("air %s, %s above it"):format(Units.temp(hour.temp),
                Units.tempDelta(hour.temp - hour.dewPoint)) or nil },
        { label = "visibility", value = Units.distance((hour.visibility or 0) / 1000),
            note = hour.aod and ("aerosols %.2f (0.1 is clean air)"):format(hour.aod) or nil },
        Report.wind(hour),
        Report.sun(hour),
    }
    local models = Sources.agreement(hour)
    if models then
        table.insert(metrics, 2, { label = "weather models", value = ("%d of %d clear"):format(models.clear, models.count),
            note = models.spread >= 50 and "they disagree: uncertain" or "they broadly agree" })
    end
    return metrics
end

-- The two charts on the activity's "today" view, over tonight
-- (`chartHours`): the clouds, and how high the moon stands (0 when down).
Stargazing.charts = {
    { title = "clouds", unit = "percent", kind = "column", min = 0, max = 100, value = function(hour) return hour.cloud or 0 end },
    { title = "moon height °", kind = "line", min = 0, max = 90,
        value = function(hour, forecast) return math.max(0, moonAt(hour, forecast).altitude or 0) end },
}

return Stargazing
