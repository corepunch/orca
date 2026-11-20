-- The 16-day swell outlook and how far to trust it (SU7). Open-Meteo's
-- default marine model ("best match") reaches about 9 days; NOAA's GFS
-- Wave runs 16. The outlook asks GFS Wave for the swell too: past the
-- default model it fills the hours, and where both exist their agreement
-- says how sure the forecast is. Far days always read as low confidence.
local Outlook = { DAYS = 16, MODEL = "ncep_gfswave025" }

-- A point about 20 km out to sea from the beach: GFS Wave's quarter-degree
-- cells near a coast are often land (all zeros), and the swell offshore is
-- what the outlook needs anyway.
local function offshore(spot)
    if not spot.facing then return spot.lat, spot.lon end
    local rad = math.rad(spot.facing)
    return spot.lat + 0.2 * math.cos(rad), spot.lon + 0.2 * math.sin(rad) / math.max(0.2, math.cos(math.rad(spot.lat)))
end

-- An extra request (README, `sources`): optional, so a failure leaves the
-- default model's days.
Outlook.source = {
    id = "surf-outlook", optional = true,
    url = function(spot, Forecast)
        if not (spot.lat and spot.lon) then return nil end
        local lat, lon = offshore(spot)
        return Forecast.url("marine", ("latitude=%.3f&longitude=%.3f&hourly=wave_height,swell_wave_height,"
            .. "swell_wave_period,swell_wave_direction,wind_wave_height,wind_wave_period,wind_wave_direction"
            .. "&models=%s&cell_selection=sea&timezone=auto&forecast_days=%d")
            :format(lat, lon, Outlook.MODEL, Outlook.DAYS))
    end,
    parse = function(json, forecast) Outlook.merge(json, forecast) end,
}
Outlook.offshore = offshore

-- Puts the GFS sea on the hours: `gfsWave` (all waves together) everywhere,
-- and the swell and wind sea where the default model has none (`oneModel`).
-- A cell with no waves at all is land: then nothing is merged.
function Outlook.merge(json, forecast)
    local h = json.hourly or {}
    local function at(name, index) return h[name] and h[name][index] end
    local sea = false
    for _, height in pairs(h.wave_height or {}) do sea = sea or height > 0 end
    if not sea then forecast.missing["surf-outlook"] = "no sea in the long-range wave model here"; return end
    local byTime = {}
    for index, time in ipairs(h.time or {}) do byTime[time] = index end
    for _, hour in ipairs(forecast.hours or {}) do
        local index = byTime[hour.time]
        local wave = index and at("wave_height", index)
        if wave then
            hour.gfsWave = wave
            if hour.swellHeight == nil and hour.waveHeight == nil then
                hour.waveHeight, hour.oneModel = wave, true
                hour.swellHeight, hour.swellPeriod = at("swell_wave_height", index), at("swell_wave_period", index)
                hour.swellDir = at("swell_wave_direction", index)
                hour.wind_wave_height, hour.wind_wave_period = at("wind_wave_height", index), at("wind_wave_period", index)
                hour.wind_wave_direction = at("wind_wave_direction", index)
            end
        end
    end
end

local WORDS = { "high", "medium", "low" }

-- How sure a day's swell is: "high", "medium" or "low", and why. Far days
-- are less sure; two models that disagree, or only one model, lower it.
function Outlook.confidence(forecast, date)
    local ahead
    for index, day in ipairs(forecast.days or {}) do
        if day.date == date then ahead = index - (forecast.today or 1) end
    end
    if not ahead then return "low", "beyond the forecast" end
    local step = ahead <= 2 and 1 or ahead <= 5 and 2 or 3
    local why = ("%d %s out"):format(ahead, ahead == 1 and "day" or "days")
    local one, spread, count = false, 0, 0
    for _, hour in ipairs(forecast.hours or {}) do
        if hour.date == date then
            if hour.oneModel then one = true
            elseif hour.gfsWave and hour.waveHeight then
                spread = spread + math.abs(hour.gfsWave - hour.waveHeight) / math.max(0.3, hour.waveHeight, hour.gfsWave)
                count = count + 1
            end
        end
    end
    if ahead == 0 then why = "today" end
    if one then return "low", why .. ", one wave model" end
    if count > 0 and spread / count > 0.35 then
        return WORDS[math.min(3, step + 1)], why .. ", the wave models disagree"
    end
    if count > 0 and step > 1 and spread / count < 0.15 then why = why .. ", the wave models agree" end
    return WORDS[step], why
end

return Outlook
