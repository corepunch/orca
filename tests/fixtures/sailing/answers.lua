-- Synthetic answers for the Sailors' Weather services, in the shapes each
-- provider documents (decoded JSON as Lua tables, NDBC as its text). The
-- network to these hosts was blocked when they were written, so they are
-- not recordings: replace them with recorded answers when one is made
-- (Research/services-sailing.md, "Fixtures"). The clock is
-- tests/weather_fixture.lua's NOW, 2026-11-18 06:00 UTC.
local NOW = 1794981600
local A = {}

-- api.weather.gov /alerts/active?point=37.8100,-122.4600 (GeoJSON).
function A.nwsCrissy()
    local function feature(p) return { type = "Feature", properties = p } end
    return { type = "FeatureCollection", title = "Current watches, warnings, and advisories", features = {
        feature { event = "Small Craft Advisory", status = "Actual", messageType = "Alert", severity = "Minor",
            senderName = "NWS San Francisco CA", headline = "Small Craft Advisory issued November 17 at 8:45PM PST",
            sent = "2026-11-17T20:45:00-08:00", effective = "2026-11-17T20:45:00-08:00",
            onset = "2026-11-17T21:00:00-08:00", ends = "2026-11-18T15:00:00-08:00",
            expires = "2026-11-18T04:00:00-08:00" },
        -- Over before now: dropped.
        feature { event = "Gale Warning", status = "Actual", messageType = "Alert", severity = "Moderate",
            senderName = "NWS San Francisco CA", onset = "2026-11-17T06:00:00-08:00", ends = "2026-11-17T18:00:00-08:00" },
        -- Not a sailor's event: dropped.
        feature { event = "Heat Advisory", status = "Actual", messageType = "Alert", severity = "Moderate",
            senderName = "NWS San Francisco CA", onset = "2026-11-18T00:00:00-08:00", ends = "2026-11-18T20:00:00-08:00" },
        -- A test message: dropped.
        feature { event = "Storm Warning", status = "Test", messageType = "Alert", senderName = "NWS San Francisco CA",
            onset = "2026-11-18T00:00:00-08:00", ends = "2026-11-18T20:00:00-08:00" },
        -- Later today: caps the evening.
        feature { event = "Gale Warning", status = "Actual", messageType = "Alert", severity = "Moderate",
            senderName = "NWS San Francisco CA", headline = "Gale Warning issued November 17 at 8:45PM PST",
            sent = "2026-11-17T20:45:00-08:00", onset = "2026-11-18T10:00:00-08:00",
            ends = "2026-11-19T04:00:00-08:00" },
    } }
end

-- feeds.meteoalarm.org/api/v1/warnings/feeds-germany: CAP alerts as JSON.
local function cap(identifier, level, kind, area, onset, expires, languages)
    local infos = {}
    for _, language in ipairs(languages or { "de-DE" }) do
        infos[#infos + 1] = { language = language, category = { "Met" },
            event = language == "en-GB" and "wind gusts" or "STURMBÖEN",
            senderName = "Deutscher Wetterdienst", headline = language == "en-GB" and "Official WARNING of WIND GUSTS"
                or "Amtliche WARNUNG vor STURMBÖEN", onset = onset, effective = onset, expires = expires,
            parameter = { { valueName = "awareness_level", value = level }, { valueName = "awareness_type", value = kind } },
            area = { { areaDesc = area, geocode = { { valueName = "EMMA_ID", value = "DE000" } } } } }
    end
    return { uuid = identifier, alert = { identifier = identifier, sender = "opendata@dwd.de",
        sent = "2026-11-18T04:30:00+00:00", status = "Actual", msgType = "Alert", scope = "Public", info = infos } }
end
function A.meteoalarmGermany()
    return { warnings = {
        cap("a1", "3; orange; Severe", "1; Wind", "Bodenseekreis", "2026-11-18T05:00:00+00:00", "2026-11-18T18:00:00+00:00",
            { "de-DE", "en-GB" }),
        -- Expired: dropped.
        cap("a2", "2; yellow; Moderate", "1; Wind", "Kreis Nordfriesland", "2026-11-17T05:00:00+00:00",
            "2026-11-17T18:00:00+00:00"),
        -- Elsewhere in the country.
        cap("a3", "2; yellow; Moderate", "3; Thunderstorm", "Kreis Görlitz", "2026-11-18T05:00:00+00:00",
            "2026-11-18T12:00:00+00:00"),
        -- Rain is not a sailor's warning type: dropped.
        cap("a4", "2; yellow; Moderate", "10; Rain", "Bodenseekreis", "2026-11-18T05:00:00+00:00",
            "2026-11-18T12:00:00+00:00"),
        -- Green is "no warning": dropped.
        cap("a5", "1; green; Minor", "1; Wind", "Bodenseekreis", "2026-11-18T05:00:00+00:00", "2026-11-18T12:00:00+00:00"),
    } }
end

-- aviationweather.gov/api/data/metar?bbox=…&format=json&hours=2 around
-- Tarifa: Gibraltar twice (the newer kept), Tangier light and variable.
function A.metarTarifa()
    return {
        { icaoId = "LXGB", name = "Gibraltar, GI", lat = 36.151, lon = -5.349, obsTime = NOW - 78 * 60,
            reportTime = "2026-11-18T04:42:00.000Z", wdir = 240, wspd = 10, rawOb = "LXGB 180442Z 24010KT" },
        { icaoId = "LXGB", name = "Gibraltar, GI", lat = 36.151, lon = -5.349, obsTime = NOW - 18 * 60,
            reportTime = "2026-11-18T05:42:00.000Z", wdir = 250, wspd = 14, wgst = 21, rawOb = "LXGB 180542Z 25014G21KT" },
        { icaoId = "GMTT", name = "Tangier/Ibn Batouta, MA", lat = 35.727, lon = -5.917, obsTime = NOW - 30 * 60,
            reportTime = "2026-11-18T05:30:00.000Z", wdir = "VRB", wspd = 3, rawOb = "GMTT 180530Z VRB03KT" },
    }
end

-- api.pioupiou.fr/v1/live-with-meta/all near Lake Garda; km/h.
local function station(id, name, lat, lon, minutesAgo, avg, max, heading, stateName)
    return { id = id, meta = { name = name }, location = { latitude = lat, longitude = lon, success = true },
        measurements = { date = os.date("!%Y-%m-%dT%H:%M:%S.000Z", NOW - minutesAgo * 60), wind_heading = heading,
            wind_speed_avg = avg, wind_speed_max = max, wind_speed_min = avg / 2 },
        status = { state = stateName or "on" } }
end
function A.pioupiouGarda()
    return { doc = "http://developers.pioupiou.fr/api/live/", license = "http://developers.pioupiou.fr/data-licensing",
        attribution = "(c) contributors of the Pioupiou wind network <http://pioupiou.fr>", data = {
            station(1001, "Torbole", 45.873, 10.874, 4, 27.78, 37.04, 180),
            station(1002, "Riva harbour", 45.871, 10.870, 3, 40, 50, 180, "off"),
            station(1003, "Malcesine", 45.760, 10.810, 10, 20, 26, 190),
            station(1004, "Arco", 45.918, 10.886, 120, 10, 14, 170),
        } }
end
function A.pioupiouOne() return { data = station(1003, "Malcesine", 45.760, 10.810, 10, 20, 26, 190) } end

-- ndbc.noaa.gov/data/realtime2/FTPC1.txt and 46026.txt: newest row first,
-- m/s, "MM" missing.
A.ndbcFTPC1 = [[
#YY  MM DD hh mm WDIR WSPD GST  WVHT   DPD   APD MWD   PRES  ATMP  WTMP  DEWP  VIS PTDY  TIDE
#yr  mo dy hr mn degT m/s  m/s     m   sec   sec degT   hPa  degC  degC  degC  nmi  hPa    ft
2026 11 18 05 54 270  6.2  8.1    MM    MM    MM  MM 1016.0  11.8    MM   9.1   MM   MM    MM
2026 11 18 05 48 270  6.0  7.9    MM    MM    MM  MM 1016.0  11.8    MM   9.1   MM   MM    MM
]]
A.ndbc46026 = [[
#YY  MM DD hh mm WDIR WSPD GST  WVHT   DPD   APD MWD   PRES  ATMP  WTMP  DEWP  VIS PTDY  TIDE
#yr  mo dy hr mn degT m/s  m/s     m   sec   sec degT   hPa  degC  degC  degC  nmi  hPa    ft
2026 11 18 05 40 290  8.0   MM   1.8    12   7.5 280 1015.2  12.1  13.0   9.8   MM   MM    MM
2026 11 18 05 30 280  7.5  9.5   1.7    12   7.4 280 1015.3  12.1  13.0   9.8   MM   MM    MM
]]

return A
