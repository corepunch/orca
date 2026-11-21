-- Synthetic answers in the documented shapes of the space-weather services
-- Stargazers' Weather reads (no live network in tests). Times are built
-- around `now` (a UTC epoch) so freshness can be tested. Shapes, from the
-- services' documentation and examples (verify against the live feeds
-- before release, see Research/services-stargazing.md):
--
--   SWPC products/solar-wind/mag-2-hour.json, plasma-2-hour.json: a header
--     row, then one row of strings a minute, nulls in spacecraft gaps.
--   SWPC json/ovation_aurora_latest.json: { "Observation Time",
--     "Forecast Time", "Data Format", coordinates = { { lon 0–359, lat
--     −90…90, percent }, … } } on the whole 1° grid.
--   SWPC products/alerts.json: { { product_id, issue_datetime, message } }.
--   GFZ kp.gfz.de/app/json/?index=Hp30: { meta, datetime = { … }, Hp30 = { … }, status = { … } }.
local Space = {}

local function stamp(epoch, sep) return os.date("!%Y-%m-%d" .. (sep or " ") .. "%H:%M:%S", math.floor(epoch)) end

-- Two hours of magnetometer rows, one a minute up to `now`: `bz(minutesAgo)`
-- gives Bz (nil for a null); `bt` follows.
function Space.mag(now, bz)
    local rows = { { "time_tag", "bx_gsm", "by_gsm", "bz_gsm", "lon_gsm", "lat_gsm", "bt" } }
    for ago = 119, 0, -1 do
        local value = bz(ago)
        rows[#rows + 1] = { stamp(now - ago * 60) .. ".000", value and "1.20" or nil, value and "-2.10" or nil,
            value and ("%.2f"):format(value) or nil, value and "300.1" or nil, value and "-40.2" or nil,
            value and ("%.2f"):format(math.abs(value) + 2) or nil }
    end
    return rows
end

-- Two hours of plasma rows: `speed(minutesAgo)` (nil for a null).
function Space.plasma(now, speed)
    local rows = { { "time_tag", "density", "speed", "temperature" } }
    for ago = 119, 0, -1 do
        local value = speed(ago)
        rows[#rows + 1] = { stamp(now - ago * 60) .. ".000", value and "4.50" or nil, value and ("%.1f"):format(value) or nil,
            value and "120000" or nil }
    end
    return rows
end

-- The OVATION grid: every cell 0 except `cells[lat][lon]` (integer degrees,
-- lon 0–359), observed `age` seconds before `now`.
function Space.ovation(now, cells, age)
    local list = {}
    for lon = 0, 359 do
        for lat = -90, 90 do
            list[#list + 1] = { lon, lat, cells[lat] and cells[lat][lon] or 0 }
        end
    end
    local observed = now - (age or 300)
    return { ["Observation Time"] = stamp(observed, "T") .. "Z", ["Forecast Time"] = stamp(observed + 1800, "T") .. "Z",
        ["Data Format"] = "[Longitude, Latitude, Aurora]", coordinates = list, type = "MultiPoint" }
end

-- SWPC's alert feed: a G2 watch for tomorrow (serial 3001), an earlier G1
-- watch (serial 2990) and its cancellation, and a K-index warning that ran
-- out yesterday. Message text after SWPC's real messages.
function Space.alerts(now)
    local function issued(epoch) return stamp(epoch) .. ".000" end
    local function spoken(epoch) return os.date("!%Y %b %d %H%M UTC", math.floor(epoch)) end
    local function day(epoch) return os.date("!%b %d", math.floor(epoch)) end
    local day0, day1, day2 = now, now + 86400, now + 2 * 86400
    return {
        { product_id = "A20F", issue_datetime = issued(now - 2 * 3600),
            message = "Space Weather Message Code: WATA20\r\nSerial Number: 3001\r\nIssue Time: " .. spoken(now - 2 * 3600)
                .. "\r\n\r\nWATCH: Geomagnetic Storm Category G2 Predicted\r\n\r\nHighest Storm Level Predicted by Day:\r\n"
                .. day(day0) .. ":  None (Below G1)   " .. day(day1) .. ":  G2 (Moderate)   " .. day(day2) .. ":  G1 (Minor)\r\n\r\n"
                .. "THIS SUPERSEDES ANY/ALL PRIOR WATCHES IN EFFECT\r\n\r\nNOAA Space Weather Scale descriptions can be found at"
                .. "\r\nwww.swpc.noaa.gov/noaa-scales-explanation" },
        { product_id = "A20F", issue_datetime = issued(now - 5 * 3600),
            message = "Space Weather Message Code: WATA50\r\nSerial Number: 2990\r\nIssue Time: " .. spoken(now - 5 * 3600)
                .. "\r\n\r\nWATCH: Geomagnetic Storm Category G3 Predicted\r\n\r\nHighest Storm Level Predicted by Day:\r\n"
                .. day(day0) .. ":  G3 (Strong)   " .. day(day1) .. ":  G1 (Minor)   " .. day(day2) .. ":  None (Below G1)\r\n" },
        { product_id = "A20F", issue_datetime = issued(now - 4 * 3600),
            message = "Space Weather Message Code: WATA99\r\nSerial Number: 2995\r\nIssue Time: " .. spoken(now - 4 * 3600)
                .. "\r\n\r\nCANCEL WATCH: Geomagnetic Storm Category G3 Predicted\r\nCancel Serial Number: 2990\r\n" },
        { product_id = "K05W", issue_datetime = issued(now - 30 * 3600),
            message = "Space Weather Message Code: WARK05\r\nSerial Number: 1500\r\nIssue Time: " .. spoken(now - 30 * 3600)
                .. "\r\n\r\nWARNING: Geomagnetic K-index of 5 expected\r\nValid From: " .. spoken(now - 30 * 3600)
                .. "\r\nValid To: " .. spoken(now - 20 * 3600) .. "\r\nWarning Condition: Onset\r\n" },
    }
end

-- Six hours of Hp30 to `now`, from `values` (oldest first, one a half hour).
function Space.hp30(now, values)
    local start = now - now % 1800 - (#values - 1) * 1800
    local times, status = {}, {}
    for index = 1, #values do
        times[index] = stamp(start + (index - 1) * 1800, "T") .. "Z"
        status[index] = "nowcast"
    end
    return { meta = { source = "GFZ German Research Centre for Geosciences", license = "CC BY 4.0" },
        datetime = times, Hp30 = values, status = status }
end

-- NOAA's Kp forecast as a list of objects, the same Kp all night.
function Space.kp(now, kp)
    local list, start = {}, now - now % (3 * 3600) - 6 * 3600
    for step = 0, 16 do
        list[#list + 1] = { time_tag = os.date("!%Y-%m-%dT%H:%M:%S", math.floor(start + step * 3 * 3600)), kp = kp,
            observed = "predicted", noaa_scale = kp >= 5 and "G1" or nil }
    end
    return list
end

return Space
