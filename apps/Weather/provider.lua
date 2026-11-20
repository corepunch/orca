-- The data provider: places, the place hub, one activity's report and the
-- settings page. A place's forecast is fetched once and kept for ten
-- minutes, so opening an activity or going back reuses it. The last answer
-- per place is also saved (Storage, "forecast-<place>"): the hub shows it at
-- once while the new one loads, and when the network fails the pages show
-- it with its age, marked `stale`. Options inject `fetch(url)`, `clock()`
-- and `storage` so tests run on recorded forecasts.
local Forecast = require "Weather/forecast"
local Report = require "Weather/report"
local Activities = require "Weather/activities"
local Settings = require "Weather/settings"
local Provider = {}

local CACHE_SECONDS = 10 * 60

local function kind(name) return function(record) return record.values.Kind == name end end

function Provider.new(places, options)
    options = options or {}
    local byId, cache = {}, {}
    for _, place in ipairs(places) do byId[place.id] = place end
    local clock = options.clock or os.time
    local storage = options.storage
    local provider

    local function marine(place) return Activities.needsMarine(place) and "optional" end

    -- The forecast saved on the device for a place, as of now, or nil.
    local function saved(place, now)
        local raw = storage and storage:load("forecast-" .. place.id)
        local forecast = raw and Forecast.restore(raw, place, Activities.request(place), now)
        if forecast then forecast.spot = place end
        return forecast
    end

    -- The place's forecast, from the cache while it is fresh. When the
    -- network fails, the last copy (in memory, else on the device) stands in.
    local function forecastFor(placeId, refresh)
        local place = byId[placeId]
        if not place then return nil, { code = "no_place", message = "Choose a place first." } end
        local cached = cache[placeId]
        local now = clock()
        if cached and not refresh and now - cached.at < CACHE_SECONDS then return cached.forecast, nil, place end
        local forecast, err, raw = Forecast.load(place, marine(place), options.fetch, now, Activities.request(place))
        if not forecast then
            local last = cached and cached.forecast or saved(place, now)
            if not last then return nil, { code = "network", message = err } end
            last.stale = true
            cache[placeId] = { forecast = last, at = cached and cached.at or now - CACHE_SECONDS }
            return last, nil, place
        end
        forecast.spot = place
        cache[placeId] = { forecast = forecast, at = now }
        if storage then storage:save("forecast-" .. place.id, raw) end
        return forecast, nil, place
    end

    local function hub(place, forecast)
        return Report.hub(place, forecast, Activities.at(place), clock(), Activities.lead())
    end

    provider = {
        places = places,
        computed = {
            scoreText = function(values) return ("%d"):format(math.floor(values.Score or 0)) end,
            hasIcon = function(values) return (values.Icon or "") ~= "" end,
            hasNote = function(values) return (values.Note or "") ~= "" end,
            hasDetail = function(values) return (values.Detail or "") ~= "" end,
        },
        filters = {
            nowRows = kind("now"), detailRows = kind("detail"), stripRows = kind("strip"), activityRows = kind("activity"),
            reasonRows = kind("reason"), hourRows = kind("hour"), dayRows = kind("day"), metricRows = kind("metric"),
            cardRows = kind("card"), ageRows = kind("age"), aboutRows = kind("about"),
            unitRows = function(record) return record.values.Kind == "setting" and record.values.Activity == Settings.UNITS end,
            activitySettingRows = function(record)
                return record.values.Kind == "setting" and record.values.Activity ~= Settings.UNITS
            end,
        },
        -- The hub shows the saved forecast at once while the first request runs.
        caches = {
            savedHub = function(params)
                local place = byId[params.PlaceId]
                local forecast = place and (cache[place.id] and cache[place.id].forecast or saved(place, clock()))
                return forecast and hub(place, forecast) or nil
            end,
        },
        handlers = {},
        operations = {
            places = function() return places end,
            -- A refresh (pull or poll) refetches; the first load may use the
            -- cache, and so does a redraw after the settings change.
            hub = function(params, _, _, query)
                local refresh = query and query.state and query.state.HasLoaded and not provider.redrawing
                provider.redrawing = nil
                local forecast, err, place = forecastFor(params.PlaceId, refresh)
                if not forecast then return nil, err end
                return hub(place, forecast)
            end,
            activity = function(params)
                local activity = Activities.byId[params.ActivityId]
                if not activity then return nil, { code = "no_activity", message = "Unknown activity." } end
                local forecast, err, place = forecastFor(params.PlaceId)
                if not forecast then return nil, err end
                return Report.rows(activity, place, forecast, clock())
            end,
            settings = function() return Provider.settingsRows() end,
        },
    }
    provider.forecastFor = forecastFor
    return provider
end

-- Open-Meteo's terms ask for this line and its sources (PLAN.md 0.3).
Provider.ABOUT = {
    { "Weather data by Open-Meteo.com", "Forecasts from national weather services: DWD, NOAA, Météo-France, ECMWF, "
        .. "JMA, MET Norway, the UK Met Office, Environment Canada and others, under CC BY 4.0." },
    { "your data stays here", "Your places and settings are kept on this device only." },
}

-- The settings page: the shared units, each activity's own settings and
-- "about" (WeatherRow, Kind "setting" or "about").
function Provider.settingsRows()
    local rows = Settings.rows(Activities.list)
    for index, about in ipairs(Provider.ABOUT) do
        rows[#rows + 1] = { id = "settings:about:" .. index, kind = "about", order = #rows + 1, title = about[1],
            detail = about[2], tone = "info", score = 0 }
    end
    local edition = Activities.edition and Activities.byId[Activities.edition]
    rows[#rows + 1] = { id = "settings:about:edition", kind = "about", order = #rows + 1,
        title = Activities.appName:lower(), tone = "info", score = 0,
        detail = edition and ("everyday outdoors and %s, at the places that offer it"):format(edition.title)
            or "every activity at every place" }
    return rows
end

return Provider
