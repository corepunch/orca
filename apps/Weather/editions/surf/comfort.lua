-- Comfort in the water (SV-SU5): the sun and the current, on the surf page
-- as well as the hub's everyday rows. The current is Open-Meteo's marine
-- model (about 8 km cells): the drift along the coast, not the rips at a
-- sandbar, and the card says so. It is a small source of its own because
-- the answer names its unit (`hourly_units`), which the shared marine call
-- does not keep.
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Breaks = require "Weather/editions/surf/breaks"
local Comfort = { STRONG = 0.5, MODERATE = 0.25 } -- m/s

local TO_MS = { ["m/s"] = 1, ["km/h"] = 1 / 3.6, kn = 0.514444, mph = 0.44704 }

Comfort.source = {
    id = "surf-current", optional = true,
    url = function(spot, Forecast)
        if not (spot.lat and spot.lon) then return nil end
        return Forecast.url("marine", ("latitude=%s&longitude=%s&hourly=ocean_current_velocity,ocean_current_direction"
            .. "&cell_selection=sea&timezone=auto&forecast_days=3"):format(spot.lat, spot.lon))
    end,
    parse = function(json, forecast)
        local h = assert(type(json) == "table" and type(json.hourly) == "table" and json.hourly, "no current data")
        local units = type(json.hourly_units) == "table" and json.hourly_units or {}
        local scale = TO_MS[units.ocean_current_velocity or "km/h"] or 1 / 3.6
        local at = {}
        for index, time in ipairs(h.time or {}) do at[time] = index end
        for _, hour in ipairs(forecast.hours or {}) do
            local i = at[hour.time]
            local speed = i and h.ocean_current_velocity and tonumber(h.ocean_current_velocity[i])
            if speed then
                hour.current = speed * scale
                hour.currentDir = h.ocean_current_direction and tonumber(h.ocean_current_direction[i])
            end
        end
    end,
}

-- "strong", "moderate" or "weak", and whether it runs along the beach.
function Comfort.current(hour, profile)
    if not hour.current then return nil end
    local word = hour.current >= Comfort.STRONG and "strong" or hour.current >= Comfort.MODERATE and "moderate" or "weak"
    local facing = profile.wind and (profile.wind + 180) % 360
    local along = facing and hour.currentDir and Sky.angle(hour.currentDir, facing) >= 45
        and Sky.angle(hour.currentDir, facing) <= 135
    return word, along
end

-- The highest UV in the light hours left today, and its hour.
local function peakUv(forecast)
    local now = forecast.hours[forecast.now]
    local best
    for index = forecast.now, #forecast.hours do
        local hour = forecast.hours[index]
        if hour.date ~= now.date then break end
        if hour.uv and (not best or hour.uv > best.uv) then best = hour end
    end
    return best
end

function Comfort.card(forecast, hour)
    local now = forecast.hours and forecast.hours[forecast.now]
    if not now then return nil end
    hour = hour or now
    local profile = Breaks.of(forecast.spot)
    local parts, value, tone = {}, nil, "info"
    local word, along = Comfort.current(hour, profile)
    if word then
        parts[#parts + 1] = ("%s current%s, %s (model, not rips)"):format(word, along and " along the beach" or "",
            Units.wind(hour.current, 1))
        if word == "strong" then
            value, tone = "strong current", "fair"
            parts[#parts] = parts[#parts] .. ": you drift fast; line up on a mark on the beach"
        end
    end
    local sun = peakUv(forecast)
    if sun and sun.uv >= 3 then
        local strong = sun.uv >= 6
        parts[#parts + 1] = ("UV %d around %s: %s"):format(math.floor(sun.uv + 0.5), Units.clock(sun.time),
            strong and "sunscreen on face and lips, a rash vest or wetsuit top" or "sunscreen")
        value = value or ("UV %d"):format(math.floor(sun.uv + 0.5))
        if strong then tone = "fair" end
    end
    if #parts == 0 then return nil end
    return { id = "comfort", title = "sun and current", value = value or word .. " current", tone = tone, hub = false,
        detail = table.concat(parts, " · ") }
end

return Comfort
