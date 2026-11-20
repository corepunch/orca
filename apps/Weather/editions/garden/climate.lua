-- Local frost dates and hardiness (GA9), from 30 years of daily minimum
-- temperatures in Open-Meteo's Historical Weather API (ERA5-Land
-- reanalysis, about 10 km). One request per place, ever: the answer is
-- boiled down and kept with Storage ("climate-<lat>_<lon>"), and the hub
-- never waits for it. The first time a place is seen its cards mark it
-- wanted; the request rides along with the next forecast load, which runs
-- while the hub already shows the saved forecast. A failed request is
-- retried a day later.
--
-- Frost here is an air frost: a daily minimum of 0 °C or less at 2 m.
-- Ground frost comes earlier in autumn and later in spring, and the 10 km
-- grid smooths frost pockets away, so real dates can be worse.
--
--   last spring frost   the last frost before midsummer, per year
--   first autumn frost  the first after it
--   odds                50 % is the median year; "1 year in 10" is the
--                       10th or 90th percentile (a frost-free spring counts
--                       as the earliest, a frost-free autumn as the latest)
--   hardiness           USDA zone from the mean of each year's coldest
--                       night; the RHS rating of plants that survive it
local Forecast = require "Weather/forecast"
local Storage = require "Weather/storage"
local Units = require "Weather/units"
local Climate = {}

Climate.YEARS = 30
Climate.RETRY = 86400

local memory = {}

function Climate.key(spot)
    if not (spot and spot.lat and spot.lon) then return nil end
    return ("climate-%.1f_%.1f"):format(spot.lat, spot.lon)
end

-- What is kept for a place: { frost = …, zone = … } once known, else
-- { wanted = true, tried = epoch }. nil before the place was seen.
function Climate.saved(spot)
    local key = Climate.key(spot)
    if not key then return nil end
    if memory[key] == nil then memory[key] = Storage.load(key) or false end
    return memory[key] or nil
end

local function keep(spot, value)
    local key = Climate.key(spot)
    if not key then return end
    memory[key] = value
    Storage.save(key, value)
end

-- Forget what is in memory (tests start over with an empty folder).
function Climate.reset() memory = {} end

-- Marks a place wanted the first time its cards are drawn.
function Climate.want(spot)
    if Climate.key(spot) and not Climate.saved(spot) then keep(spot, { wanted = true }) end
end

-- The 30 whole years before the current one.
function Climate.years(epoch)
    local year = tonumber(os.date("!%Y", epoch or os.time()))
    return year - Climate.YEARS, year - 1
end

-- The source's URL, or nil when nothing is needed (known, unseen, or tried
-- less than a day ago).
function Climate.url(spot, now)
    local saved = Climate.saved(spot)
    if not saved or saved.frost or not saved.wanted then return nil end
    now = now or os.time()
    if saved.tried and now - saved.tried < Climate.RETRY then return nil end
    keep(spot, { wanted = true, tried = now })
    local first, last = Climate.years(now)
    return Forecast.url("archive", ("latitude=%s&longitude=%s&start_date=%d-01-01&end_date=%d-12-31"
        .. "&daily=temperature_2m_min&models=era5_land&timezone=auto"):format(spot.lat, spot.lon, first, last))
end

-- Days into the year, for a non-leap year (29 February counts as 1 March).
local CUMULATIVE = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 }
local function dayOfYear(m, d) return CUMULATIVE[m] + math.min(d, m == 2 and 28 or 31) + (m == 2 and d == 29 and 1 or 0) end
local MONTHS = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" }

-- "22 apr" (or "apr 22" where the US reads °F) for a day of the year.
function Climate.dateText(day)
    if not day then return "—" end
    day = (math.floor(day + 0.5) - 1) % 365 + 1
    local month = 12
    while CUMULATIVE[month] >= day do month = month - 1 end
    local d = day - CUMULATIVE[month]
    return Units.current("temp") == "f" and ("%s %d"):format(MONTHS[month], d) or ("%d %s"):format(d, MONTHS[month])
end

local function percentile(sorted, q)
    if #sorted == 0 then return nil end
    return sorted[math.max(1, math.min(#sorted, math.ceil(q * #sorted)))]
end

-- USDA zone for a mean coldest night, °C: zone 1 starts at −60 °F and each
-- zone is 10 °F wide, "a" its colder half.
function Climate.usda(c)
    local f = c * 9 / 5 + 32 + 60
    local zone = math.max(1, math.min(13, math.floor(f / 10) + 1))
    local half = (f % 10) < 5 and "a" or "b"
    if f < 0 then half = "a" elseif f >= 130 then half = "b" end
    return zone .. half
end

-- The RHS rating of the plants that come through that night outdoors.
local RHS = { { -20, "H7" }, { -15, "H6" }, { -10, "H5" }, { -5, "H4" }, { 1, "H3" }, { 5, "H2" }, { 10, "H1c" },
    { 15, "H1b" } }
function Climate.rhs(c)
    for _, entry in ipairs(RHS) do
        if c < entry[1] then return entry[2] end
    end
    return "H1a"
end

-- Frost dates and hardiness from daily dates and minimums. `lat` picks the
-- hemisphere: south of the equator the frost season runs July to June.
-- nil when fewer than ten years are complete.
function Climate.compute(dates, mins, lat)
    local south = (lat or 0) < 0
    local seasons, yearsMin, order = {}, {}, {}
    for index, date in ipairs(dates or {}) do
        local t = mins[index]
        local y, m, d = date:match("^(%d+)-(%d+)-(%d+)")
        if t and y then
            y, m, d = tonumber(y), tonumber(m), tonumber(d)
            yearsMin[y] = math.min(yearsMin[y] or math.huge, t)
            local doy = dayOfYear(m, d)
            -- A season starts at midwinter: 1 January north, 1 July south.
            local season, day = y, doy
            if south then
                if m >= 7 then day = doy - 181 else season, day = y - 1, doy + 184 end
            end
            local s = seasons[season]
            if not s then s = { days = 0 }; seasons[season] = s; order[#order + 1] = season end
            s.days = s.days + 1
            if t <= 0 then
                if day < 183 then s.last = day else s.first = s.first or day end
            end
        end
    end
    local lasts, firsts, lengths, frostless = {}, {}, {}, 0
    for _, season in ipairs(order) do
        local s = seasons[season]
        if s.days >= 360 then
            lasts[#lasts + 1] = s.last or -math.huge
            firsts[#firsts + 1] = s.first or math.huge
            if not s.last and not s.first then frostless = frostless + 1 end
            lengths[#lengths + 1] = math.min(365, (s.first or 366) - (s.last or 0))
        end
    end
    if #lasts < 10 then return nil end
    table.sort(lasts); table.sort(firsts); table.sort(lengths)
    local coldest, count = 0, 0
    for _, low in pairs(yearsMin) do coldest, count = coldest + low, count + 1 end
    coldest = coldest / count
    local function finite(v) return v ~= math.huge and v ~= -math.huge and v or nil end
    -- Back to days of the calendar year.
    local shift = south and 181 or 0
    local function calendar(v) return v and (v + shift - 1) % 365 + 1 or nil end
    return {
        years = #lasts, frostless = frostless, coldest = coldest, zone = Climate.usda(coldest), rhs = Climate.rhs(coldest),
        lastSpring = { p10 = calendar(finite(percentile(lasts, 0.1))), p50 = calendar(finite(percentile(lasts, 0.5))),
            p90 = calendar(finite(percentile(lasts, 0.9))) },
        firstAutumn = { p10 = calendar(finite(percentile(firsts, 0.1))), p50 = calendar(finite(percentile(firsts, 0.5))),
            p90 = calendar(finite(percentile(firsts, 0.9))) },
        season = percentile(lengths, 0.5),
    }
end

-- The source (Garden.sources): parses the archive answer and keeps the result.
Climate.source = {
    id = "climate", optional = true,
    url = function(spot) return Climate.url(spot) end,
    parse = function(json, forecast)
        local daily = json.daily or {}
        local spot = forecast.spot or { lat = json.latitude, lon = json.longitude }
        local result = Climate.compute(daily.time, daily.temperature_2m_min or {}, json.latitude or spot.lat)
        if not result then error("fewer than ten whole years of history") end
        local first, last = (daily.time[1] or ""):sub(1, 4), (daily.time[#daily.time] or ""):sub(1, 4)
        result.from, result.to = tonumber(first), tonumber(last)
        keep(spot, { frost = true, climate = result })
        forecast.climate = result
    end,
}

-- The place's climate, or nil while it is not known.
function Climate.get(forecast, spot)
    if forecast and forecast.climate then return forecast.climate end
    local saved = Climate.saved(spot or forecast and forecast.spot)
    return saved and saved.climate
end

-- Rows for the activity page: frost dates, the frost-free season, hardiness.
-- Britain and Ireland, where plant labels carry RHS ratings.
local function british(spot)
    return spot and spot.lat and spot.lat >= 49.8 and spot.lat <= 61 and spot.lon >= -11 and spot.lon <= 2
end

function Climate.metrics(forecast, spot)
    spot = spot or forecast and forecast.spot
    local c = Climate.get(forecast, spot)
    if not c then
        return { { label = "frost dates", value = "not yet",
            note = "worked out once from 30 years of records, after the next refresh" } }
    end
    local years = c.from and ("%d–%d"):format(c.from, c.to) or ("%d years"):format(c.years)
    local rows = {}
    local spring, autumn = c.lastSpring, c.firstAutumn
    if not spring.p50 and not autumn.p50 then
        rows[#rows + 1] = { label = "frost dates", value = "frost-free most years", tone = "good",
            note = ("air frost in %d of %d years · %s"):format(c.years - c.frostless, c.years, years) }
    else
        rows[#rows + 1] = { label = "last spring frost", value = Climate.dateText(spring.p50),
            note = ("10 %% chance of frost after %s · 90 %% after %s · %s"):format(Climate.dateText(spring.p90),
                Climate.dateText(spring.p10), years) }
        rows[#rows + 1] = { label = "first autumn frost", value = Climate.dateText(autumn.p50),
            note = ("10 %% chance of frost before %s · 90 %% before %s · air frost; ground frost comes sooner"):format(
                Climate.dateText(autumn.p10), Climate.dateText(autumn.p90)) }
        if c.season then
            rows[#rows + 1] = { label = "frost-free season", value = ("about %d days"):format(math.floor(c.season + 0.5)),
                note = "in the middle year; frost pockets have fewer" }
        end
    end
    rows[#rows + 1] = { label = "hardiness", value = "zone " .. c.zone,
        note = ("coldest night of an average winter %s%s · approximate"):format(Units.temp(c.coldest),
            british(spot) and (" · plants rated %s or hardier"):format(c.rhs) or "") }
    return rows
end

return Climate
