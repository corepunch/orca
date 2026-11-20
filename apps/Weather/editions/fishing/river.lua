-- River flow for river places (FI11): Open-Meteo's Flood API gives daily
-- discharge from GloFAS on a grid of about 5 km. It takes the largest river
-- in the cell, so a small stream may read as its bigger neighbour; the card
-- says so. A place opts in with `river = true` (places.lua).
local River = {}

River.FLOOD = "https://flood-api.open-meteo.com/v1/flood"

function River.url(spot, Forecast)
    if not (spot and spot.river) then return nil end
    local query = ("latitude=%s&longitude=%s&daily=river_discharge&past_days=7&forecast_days=7"):format(spot.lat, spot.lon)
    -- The flood endpoint joins Forecast.endpoints when the foundation adds it.
    if Forecast and Forecast.endpoints and Forecast.endpoints.flood then return Forecast.url("flood", query) end
    return River.FLOOD .. "?" .. query
end

function River.parse(json)
    local d = json.daily or {}
    local days = {}
    for index, date in ipairs(d.time or {}) do
        local value = d.river_discharge and d.river_discharge[index]
        if value then days[#days + 1] = { date = date, discharge = value } end
    end
    if #days < 3 then error("no river discharge here") end
    return { days = days }
end

River.source = { id = "river", optional = true, url = River.url,
    parse = function(json, forecast) forecast.river = River.parse(json) end }

-- The river on a date: { discharge, usual (the median of the days before),
-- ratio, trend = "rising" | "falling" | "steady", state = "high" | "low" |
-- "normal" }, or nil.
function River.on(forecast, date)
    local river = forecast.river
    if not river then return nil end
    local today, before = nil, {}
    for index, day in ipairs(river.days) do
        if day.date == date then today = index; break end
        before[#before + 1] = day.discharge
    end
    if not today or #before < 3 then return nil end
    table.sort(before)
    local usual = before[(#before + 1) // 2]
    local now, yesterday = river.days[today].discharge, river.days[today - 1].discharge
    local ratio = usual > 0 and now / usual or 1
    local change = yesterday > 0 and now / yesterday or 1
    return { discharge = now, usual = usual, ratio = ratio,
        trend = change >= 1.15 and "rising" or change <= 0.87 and "falling" or "steady",
        state = ratio >= 2 and "high" or ratio <= 0.5 and "low" or "normal" }
end

return River
