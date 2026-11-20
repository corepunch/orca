-- Outdoors, the everyday activity: is it nice to be outside? Comfortable temperature, a
-- dry sky, a gentle wind and some sun make a good hour.
--
-- It also gives the full app's first answers (`cards`, research stories 2, 3
-- and 13): the next two hours of rain from the quarter hours, the rest of the
-- day in one line, and what to bring.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"

local Weather = {
    id = "outdoors", title = "outdoors", needsMarine = false,
    fields = {
        daily = { "precipitation_hours", "uv_index_max", "wind_gusts_10m_max", "apparent_temperature_max",
            "apparent_temperature_min" },
        minutely15 = { "precipitation", "snowfall" },
        pastDays = 1, -- "4° colder than yesterday"
        forecastDays = 16, -- days 8 and on say "less certain"
    },
}

function Weather.rate(hour)
    local card = Score.card(55)
    if Sky.isStorm(hour.code) then card:cap(15, "Thunderstorm: stay indoors.") end
    local feels = hour.feels or hour.temp
    if feels == nil then card:note("No temperature reading.")
    elseif feels < 0 then card:add(-25, "Freezing: wrap up warm.")
    elseif feels < 10 then card:add(-10, "Chilly: bring a jacket.")
    elseif feels <= 26 then card:add(12, "Comfortable temperature.")
    elseif feels <= 32 then card:add(-6, "Warm: find some shade.")
    else card:add(-20, "Hot: drink water and stay out of the sun.") end
    if Sky.isSnow(hour.code) then card:add(Sky.isHeavy(hour.code) and -25 or -10, "Snow: boots and a warm coat.")
    elseif Sky.isHeavy(hour.code) then card:add(-25, "Heavy rain.")
    elseif Sky.isWet(hour.code) then card:add(-12, "Wet: take an umbrella.")
    elseif (hour.rainChance or 0) >= 50 then card:add(-6, "Rain is likely.") end
    if (hour.wind or 0) > 10 then card:add(-15, "Windy.") elseif (hour.wind or 0) < 5 then card:add(4, "Gentle wind.") end
    if hour.isDay and (hour.cloud or 100) < 50 and not Sky.isWet(hour.code) then card:add(6, "Sunny spells.") end
    if (hour.uv or 0) >= 8 then card:note("Very high UV: use sunscreen.") end
    return card
end

function Weather.window(hour) return hour.isDay end

function Weather.note(hour)
    return ("%s · rain %s · wind %s"):format(Units.temp(hour.temp), Score.fmt(hour.rainChance, "%"),
        Units.wind(hour.wind))
end

function Weather.metrics(hour, forecast)
    return {
        { label = "feels like", value = Units.temp(hour.feels), note = "air " .. Units.temp(hour.temp) },
        { label = "rain", value = Score.fmt(hour.rainChance, "%"), note = Units.rain(hour.rain) .. " this hour" },
        Report.wind(hour),
        { label = "humidity", value = Score.fmt(hour.humidity, "%") },
        { label = "uv index", value = Score.fmt(hour.uv) },
        Report.pressure(hour),
        Report.sun(hour),
        Report.moon(forecast),
    }
end

-- The two charts on the activity's "today" view, over the next 24 hours.
-- `value` is metric; `unit` (a Units kind) converts it and ends the title.
Weather.charts = {
    { title = "temperature", unit = "temp", kind = "column", value = function(hour) return hour.temp or 0 end },
    { title = "chance of rain", unit = "percent", kind = "column", min = 0, max = 100,
        value = function(hour) return hour.rainChance or 0 end },
}

-- Next-hour rain -------------------------------------------------------------

-- Open-Meteo's quarter hours come from real 15-minute models only over North
-- America and Central Europe; elsewhere they are hourly data spread out, so
-- the minutes would be false precision and the wording stays rough.
local PRECISE = {
    { south = 24, north = 60, west = -135, east = -60 }, -- HRRR (US) and HRDPS (Canada)
    { south = 43, north = 57, west = -5, east = 20 },    -- ICON-D2 and AROME
}
function Weather.preciseNowcast(spot)
    for _, box in ipairs(PRECISE) do
        if spot and spot.lat and spot.lon and spot.lat >= box.south and spot.lat <= box.north
            and spot.lon >= box.west and spot.lon <= box.east then return true end
    end
    return false
end

local WET_QUARTER = 0.1 -- mm in a quarter hour; less is a few drops

-- "about 20 min", "about 1 h 15 min", rounded to five minutes.
local function about(seconds)
    local minutes = math.max(5, math.floor(seconds / 300 + 0.5) * 5)
    if minutes < 60 then return ("about %d min"):format(minutes) end
    local rest = minutes % 60
    return rest == 0 and ("about %d h"):format(minutes // 60) or ("about %d h %d min"):format(minutes // 60, rest)
end

-- Rain per hour, as people say it (the usual 2.5 and 7.6 mm an hour).
local function intensity(mmPerHour)
    return mmPerHour < 2.5 and "light" or mmPerHour < 7.6 and "moderate" or "heavy"
end

-- The next two hours of rain, from the quarter hours: a card
-- `{ value, detail, tone }`, or nil without quarter hours. Each quarter's
-- precipitation fell in the fifteen minutes before its time.
function Weather.nextHour(forecast, now)
    now = now or forecast.epoch
    local ahead = {}
    for _, quarter in ipairs(forecast.quarters or {}) do
        if quarter.epoch and quarter.precipitation and quarter.epoch > now and quarter.epoch <= now + 2 * 3600 + 900 then
            ahead[#ahead + 1] = quarter
        end
    end
    if #ahead < 4 then return nil end
    local function wet(q) return q and q.precipitation >= WET_QUARTER end
    local first, last
    for index, quarter in ipairs(ahead) do
        if wet(quarter) then first = first or index; last = index
        elseif first then break end
    end
    if not first then return { value = "dry for the next 2 hours", detail = "no rain in the next two hours", tone = "good" } end
    local peak, snow = 0, false
    for index = first, last do
        peak = math.max(peak, ahead[index].precipitation * 4)
        snow = snow or (ahead[index].snowfall or 0) > 0
    end
    local what, strength = snow and "snow" or "rain", intensity(peak)
    local tone = strength == "light" and "fair" or "poor"
    local starts = ahead[first].epoch - 900 - now
    local stops = last < #ahead and ahead[last].epoch - now or nil
    if not Weather.preciseNowcast(forecast.spot) then
        local rough = "timing is approximate here"
        if starts <= 0 then
            return { value = what .. " now, " .. (stops and "easing within 2 hours" or "for a while"),
                detail = strength .. " · " .. rough, tone = tone }
        end
        return { value = ("%s likely %s"):format(what, starts <= 3600 and "within the hour" or "in 1 to 2 hours"),
            detail = ("%s, around %s · %s"):format(strength, Units.clock(os.date("!%H:%M", ahead[first].epoch - 900
                + (forecast.offset or 0))), rough), tone = tone }
    end
    if starts <= 0 then
        return { value = stops and ("%s stops in %s"):format(what, about(stops)) or what .. " for the next 2 hours",
            detail = strength .. (stops and ", then dry for a while" or ""), tone = tone }
    end
    local lasts = stops and stops - starts
    return { value = ("%s starts in %s"):format(what, about(starts)),
        detail = strength .. (lasts and ", for " .. about(lasts) or ", and goes on past 2 hours"), tone = tone }
end

-- The rest of the day ----------------------------------------------------------

local function wetHour(hour)
    return Sky.isWet(hour.code) or Sky.isStorm(hour.code) or (hour.rainChance or 0) >= 60
end

-- What wet hours bring, in a word: thunderstorms, snow, showers or rain.
local function wetWord(hours)
    local storm, snow, showers = false, false, false
    for _, hour in ipairs(hours) do
        storm = storm or Sky.isStorm(hour.code)
        snow = snow or Sky.isSnow(hour.code)
        showers = showers or (hour.code or 0) >= 80 and (hour.code or 0) <= 82
    end
    return storm and "thunderstorms" or snow and "snow" or showers and "showers" or "rain"
end

-- The hours the day's line is about: from now to 22:00 today, or all of
-- tomorrow's waking hours once fewer than three are left today.
local function wakingHours(forecast)
    local hours, now = forecast.hours, forecast.hours[forecast.now]
    local function collect(date, from)
        local list = {}
        for index = from, #hours do
            local hour = hours[index]
            if hour.date == date and (Sky.hour(hour.time) or 0) >= 7 and (Sky.hour(hour.time) or 0) <= 22 then
                list[#list + 1] = hour
            elseif hour.date > date then break end
        end
        return list
    end
    local list = collect(now.date, forecast.now)
    if #list >= 3 then return list, "today", now.day end
    for index = forecast.now, #hours do
        if hours[index].date > now.date then
            local tomorrow = collect(hours[index].date, index)
            return tomorrow, "tomorrow", hours[index].day
        end
    end
    return list, "today", now.day
end

-- Runs of wet and dry hours, in order: { wet, from, to, hours }.
local function runs(hours)
    local list = {}
    for _, hour in ipairs(hours) do
        local w, run = wetHour(hour), list[#list]
        if run and run.wet == w then run.to = hour; run.hours[#run.hours + 1] = hour
        else list[#list + 1] = { wet = w, from = hour, to = hour, hours = { hour } } end
    end
    return list
end

-- "dry until 16:00, then showers": the day's rain story from its runs.
local function rainStory(list)
    if #list == 0 then return nil end
    if #list == 1 then return (list[1].wet and wetWord(list[1].hours) or "dry") .. " all day" end
    local function at(hour) return Units.clock(hour.time) end
    if #list == 2 then
        if list[1].wet then return ("%s until %s, then dry"):format(wetWord(list[1].hours), at(list[2].from)) end
        return ("dry until %s, then %s"):format(at(list[2].from), wetWord(list[2].hours))
    end
    local wet, longest = {}, nil
    for _, run in ipairs(list) do
        if run.wet then
            for _, hour in ipairs(run.hours) do wet[#wet + 1] = hour end
            if not longest or #run.hours > #longest.hours then longest = run end
        end
    end
    if longest.from == longest.to then return ("%s at times, most around %s"):format(wetWord(wet), at(longest.from)) end
    return ("%s at times, most %s"):format(wetWord(wet), Units.span(longest.from.time, longest.to.time))
end

-- The day in one line and what to bring: two cards for `cards`.
function Weather.today(forecast)
    if not forecast.hours[forecast.now] then return {} end
    local hours, title, day = wakingHours(forecast)
    return Weather.summary(hours, day, title)
end

-- A whole day's waking hours in one line and what to bring (the day page).
function Weather.daySummary(forecast, date)
    local hours, day = {}, nil
    for _, hour in ipairs(forecast.hours) do
        local h = Sky.hour(hour.time) or 0
        if hour.date == date and h >= 7 and h <= 22 then hours[#hours + 1] = hour; day = hour.day end
    end
    return Weather.summary(hours, day, "the day")
end

-- The line and the things to bring for a list of hours (`day` its day).
function Weather.summary(hours, day, title)
    if #hours == 0 then return {} end
    day = day or {}
    local list = runs(hours)
    local low, high, uv, gust, wetHours = math.huge, -math.huge, 0, 0, {}
    for _, hour in ipairs(hours) do
        local feels = hour.feels or hour.temp
        if feels then low, high = math.min(low, feels), math.max(high, feels) end
        uv, gust = math.max(uv, hour.uv or 0), math.max(gust, hour.gust or 0)
        if wetHour(hour) then wetHours[#wetHours + 1] = hour end
    end
    gust = math.max(gust, day.wind_gusts_10m_max or 0)
    local line = rainStory(list) .. "; up to " .. Units.temp(day.max or high)
    if gust >= 14 then line = line .. ", windy" end
    local feelsLine = low <= high and ("feels %s to %s"):format(Units.temp(low), Units.temp(high)) or ""
    local cards = { { id = "today", title = title, value = line, tone = "info",
        detail = feelsLine .. (uv >= 3 and " · uv up to " .. Score.fmt(uv) or "") } }
    -- What to bring, with the reason for each thing.
    local bring, why = {}, {}
    if #wetHours > 0 then
        local word = wetWord(wetHours)
        bring[#bring + 1] = word == "snow" and "boots" or gust >= 15 and "a hood" or "an umbrella"
        why[#why + 1] = word .. " from " .. Units.clock(wetHours[1].time)
            .. (word ~= "snow" and gust >= 15 and ", too windy for an umbrella" or "")
    end
    if low < 0 then bring[#bring + 1] = "a warm coat, hat and gloves"
    elseif low < 10 then bring[#bring + 1] = "a coat"
    elseif low < 16 then bring[#bring + 1] = "a light jacket" end
    if low < 16 then why[#why + 1] = "feels " .. Units.temp(low) .. " at the coldest" end
    if uv >= 3 then
        bring[#bring + 1] = "sunscreen"
        why[#why + 1] = "uv " .. Score.fmt(uv) .. " at its peak"
    end
    if high >= 30 then
        bring[#bring + 1] = "water"
        why[#why + 1] = "feels " .. Units.temp(high) .. " at the hottest"
    end
    cards[2] = { id = "bring", title = "bring", tone = "info",
        value = #bring > 0 and table.concat(bring, ", ") or "nothing extra",
        detail = #why > 0 and table.concat(why, " · ") or "dry and mild" }
    return cards
end

-- The full app's first answers, ahead of the conditions on the hub.
function Weather.cards(forecast, place, clock)
    local cards = {}
    local next = Weather.nextHour(forecast, clock)
    if next then cards[1] = { id = "next-hour", title = "next 2 hours", value = next.value, detail = next.detail,
        tone = next.tone } end
    for _, card in ipairs(Weather.today(forecast)) do cards[#cards + 1] = card end
    return cards
end

return Weather
