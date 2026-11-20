-- "Where should I go this weekend?" (SA7): the other sailing spots within
-- driving range, in one extra request (Open-Meteo takes comma-separated
-- latitudes and longitudes and answers with a list), each rated with the
-- rider's profile and compared with this spot's own forecast.
--
-- The spots are the edition's places (places.lua, read when the request is
-- built, since it loads the activities). Saved spots (wave 3) should feed
-- `Rank.places` instead.
local Forecast = require "Weather/forecast"
local Sky = require "Weather/sky"
local Rank = {}

-- Nil means the edition's places; tests set a list.
Rank.places = nil

local HOURLY = "weather_code,temperature_2m,wind_speed_10m,wind_gusts_10m,wind_direction_10m,is_day,pressure_msl,cape,"
    .. "visibility,precipitation,cloud_cover"

local function candidates()
    if Rank.places then return Rank.places end
    local list = {}
    for _, place in ipairs(require "Weather/places") do
        for _, id in ipairs(place.activities or {}) do
            if id == "sailing" then list[#list + 1] = place; break end
        end
    end
    return list
end

-- Great-circle distance in km.
function Rank.distance(a, b)
    local r, rad = 6371, math.rad
    local dlat, dlon = rad(b.lat - a.lat), rad(b.lon - a.lon)
    local h = math.sin(dlat / 2) ^ 2 + math.cos(rad(a.lat)) * math.cos(rad(b.lat)) * math.sin(dlon / 2) ^ 2
    return 2 * r * math.asin(math.min(1, math.sqrt(h)))
end

-- The other spots within `km` of a place, nearest first.
function Rank.near(place, km)
    local list = {}
    for _, other in ipairs(candidates()) do
        if other.id ~= place.id and other.lat and other.lon then
            local d = Rank.distance(place, other)
            if d <= km then list[#list + 1] = { place = other, km = d } end
        end
    end
    table.sort(list, function(a, b) return a.km < b.km end)
    return list
end

-- `range()` gives the km to compare within (a setting).
function Rank.source(range)
    return {
        id = "spots", optional = true,
        url = function(spot)
            if not (spot and spot.lat and spot.lon) then return nil end
            local near = Rank.near(spot, range())
            if #near == 0 then return nil end
            local lats, lons = {}, {}
            for index, entry in ipairs(near) do lats[index], lons[index] = entry.place.lat, entry.place.lon end
            return Forecast.url("forecast", ("latitude=%s&longitude=%s&hourly=%s&wind_speed_unit=ms&timezone=auto"
                .. "&forecast_days=7"):format(table.concat(lats, ","), table.concat(lons, ","), HOURLY))
        end,
        -- `forecast.spots`: { place, forecast } per answer, matched to the
        -- nearest candidate (the API snaps coordinates to its grid).
        parse = function(json, forecast)
            if type(json) ~= "table" then return end
            local answers = json.hourly and { json } or json
            local spots = {}
            for _, answer in ipairs(answers) do
                if type(answer) == "table" and type(answer.hourly) == "table" and answer.latitude then
                    local best, bestKm
                    for _, place in ipairs(candidates()) do
                        local km = Rank.distance(place, { lat = answer.latitude, lon = answer.longitude or 0 })
                        if km < 25 and (not bestKm or km < bestKm) then best, bestKm = place, km end
                    end
                    if best then
                        local parsed = Forecast.parse(answer, nil, forecast.epoch, best)
                        parsed.spot = best
                        if parsed.current then spots[#spots + 1] = { place = best, forecast = parsed } end
                    end
                end
            end
            forecast.spots = spots
        end,
    }
end

-- The dates to compare: the coming Saturday and Sunday, or what is left of
-- the weekend when it is one.
function Rank.dates(forecast)
    local now = forecast.hours[forecast.now]
    if not now then return {} end
    local seen, dates = {}, {}
    for _, hour in ipairs(forecast.hours) do
        if hour.date >= now.date and not seen[hour.date] then seen[hour.date] = true; dates[#dates + 1] = hour.date end
    end
    local weekend = {}
    for _, date in ipairs(dates) do
        local day = Sky.weekday(date):sub(1, 3)
        if day == "sat" or day == "sun" then weekend[#weekend + 1] = date end
        if day == "sun" and #weekend > 0 then break end
    end
    return weekend
end

-- Each spot's best window over the dates, best first:
-- { place, date, result, span, km }. `best(forecast, date)` rates a day.
function Rank.rank(place, forecast, dates, best)
    local entries = { { place = place, forecast = forecast, km = 0 } }
    for _, other in ipairs(forecast.spots or {}) do
        entries[#entries + 1] = { place = other.place, forecast = other.forecast, km = Rank.distance(place, other.place) }
    end
    local ranked = {}
    for _, entry in ipairs(entries) do
        local top
        for _, date in ipairs(dates) do
            local result, span = best(entry.forecast, date)
            if result and (not top or result.score > top.result.score) then
                top = { place = entry.place, date = date, result = result, span = span, km = entry.km }
            end
        end
        if top then ranked[#ranked + 1] = top end
    end
    table.sort(ranked, function(a, b)
        if a.result.score ~= b.result.score then return a.result.score > b.result.score end
        return a.km < b.km
    end)
    return ranked
end

return Rank
