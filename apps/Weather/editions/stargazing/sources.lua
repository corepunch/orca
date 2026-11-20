-- Extra answers for a stargazer, each an optional request (README,
-- "sources"), so the forecast stands when one fails:
--
--   air     Open-Meteo Air Quality (CAMS): aerosol optical depth and dust,
--           the smoke and haze that make a "clear" sky milky (hour.aod, hour.dust);
--   models  cloud cover from several weather models, to say whether they
--           agree (hour.models = { ecmwf_ifs025 = 20, … }).
local Sources = {}

-- The models asked for their cloud cover. Kept in one place: a model that
-- retires is dropped here, and parsing reads only what came back.
Sources.MODELS = { "ecmwf_ifs025", "gfs_seamless", "icon_seamless" }

local function place(spot) return ("latitude=%s&longitude=%s"):format(spot.lat, spot.lon) end

-- Copies each row of an hourly block onto the forecast hour with the same
-- local time, through `apply(hour, index)`.
local function byTime(json, forecast, apply)
    local block = type(json) == "table" and json.hourly
    if type(block) ~= "table" or type(block.time) ~= "table" then return end
    local at = {}
    for _, hour in ipairs(forecast.hours) do at[hour.time] = hour end
    for index, time in ipairs(block.time) do
        local hour = at[time]
        if hour then apply(hour, block, index) end
    end
end

Sources.air = {
    id = "air", optional = true,
    url = function(spot, Forecast)
        return Forecast.url("airQuality", place(spot) .. "&hourly=aerosol_optical_depth,dust&timezone=auto&forecast_days=5")
    end,
    parse = function(json, forecast)
        byTime(json, forecast, function(hour, block, index)
            hour.aod = block.aerosol_optical_depth and block.aerosol_optical_depth[index]
            hour.dust = block.dust and block.dust[index]
        end)
    end,
}

Sources.models = {
    id = "models", optional = true,
    url = function(spot, Forecast)
        return Forecast.url("forecast", place(spot) .. "&hourly=cloud_cover&models=" .. table.concat(Sources.MODELS, ",")
            .. "&timezone=auto&forecast_days=7")
    end,
    parse = function(json, forecast)
        byTime(json, forecast, function(hour, block, index)
            local models
            for _, id in ipairs(Sources.MODELS) do
                local value = block["cloud_cover_" .. id] and block["cloud_cover_" .. id][index]
                if type(value) == "number" then models = models or {}; models[id] = value end
            end
            hour.models = models
        end)
    end,
}

-- How the models see an hour's sky: the share of them that call it clear
-- (cloud under 30%), and their spread. nil with fewer than two models.
function Sources.agreement(hour)
    local count, clear, low, high = 0, 0, math.huge, -math.huge
    for _, value in pairs(hour.models or {}) do
        count = count + 1
        if value < 30 then clear = clear + 1 end
        low, high = math.min(low, value), math.max(high, value)
    end
    if count < 2 then return nil end
    return { count = count, clear = clear, chance = clear / count, spread = high - low }
end

return Sources
