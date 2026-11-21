-- Where the hike starts and where it tops out, for each hiking place
-- (places.lua is shared, so the heights live here, looked up by place id).
-- The forecast for the place itself is often a town or valley floor; these
-- two points are asked for at their own heights (Open-Meteo `elevation=`).
--
--   top, start   { name, lat, lon, elevation (m) }
--   nws          a US place: official warnings from api.weather.gov
--   official     { { name, where } }: human forecasts and warnings to read
--   avalanche    { name, where }: the regional bulletin, shown when there is snow;
--                `center` (avalanche.org centre id) and `zone` (its zone name, when
--                the top falls outside every polygon) for a US centre (SV-HI1);
--                `caaml = { provider, region, treeline }` for an EAWS bulletin
--                (SV-HI2): "slf" or "euregio", the EAWS micro-region id from
--                regions.avalanches.org, the treeline height (m) when known
--   station      { kind = "snotel", id = triplet, name, elevation (m) }: measured
--                snow near the top (SV-HI4)
--   nps          a National Park Service park code: park alerts (SV-HI5)
--   forest       { name, where }: the forest or park order to check near a fire
--   iconD2       inside ICON-D2's domain: lightning potential (SV-HI7)
--
-- A CAAML region id stays empty until it is read off regions.avalanches.org
-- (not reachable when this was written): without one the bulletin source
-- asks nothing and the card keeps its link.
--
-- Heights are the published summit and trailhead heights; coordinates
-- were checked against Open-Meteo's 90 m elevation model (November 2026).
-- Some "tops" are the high point of the usual day hike, not a summit
-- (Base Las Torres).
return {
    tahoe = {
        top = { name = "Mount Tallac", lat = 38.9058, lon = -120.0991, elevation = 2968 },
        start = { name = "Mount Tallac trailhead", lat = 38.9227, lon = -120.0714, elevation = 1960 },
        nws = true,
        official = { { "NWS Reno", "weather.gov/rev" } },
        avalanche = { "Sierra Avalanche Center", "sierraavalanchecenter.org", center = "SAC", zone = "Central Sierra Nevada" },
        -- Echo Peak SNOTEL (CA), 7,800 ft, about 10 km south of the top.
        station = { kind = "snotel", id = "463:CA:SNTL", name = "Echo Peak", elevation = 2377 },
        forest = { "Lake Tahoe Basin forest orders", "fs.usda.gov/ltbmu" },
    },
    yosemite = {
        top = { name = "Half Dome", lat = 37.7461, lon = -119.5332, elevation = 2694 },
        start = { name = "Happy Isles", lat = 37.7323, lon = -119.5578, elevation = 1230 },
        nws = true,
        official = { { "NWS Hanford", "weather.gov/hnx" }, { "Yosemite conditions", "nps.gov/yose" } },
        nps = "yose",
        forest = { "Yosemite conditions", "nps.gov/yose/planyourvisit/conditions.htm" },
    },
    ["mauna-kea"] = {
        top = { name = "Mauna Kea summit", lat = 19.8207, lon = -155.4681, elevation = 4207 },
        start = { name = "Visitor Information Station", lat = 19.7615, lon = -155.4560, elevation = 2800 },
        nws = true,
        official = { { "NWS Honolulu", "weather.gov/hfo" }, { "Mauna Kea Weather Center", "mkwc.ifa.hawaii.edu" } },
    },
    brecon = {
        top = { name = "Pen y Fan", lat = 51.8838, lon = -3.4363, elevation = 886 },
        start = { name = "Storey Arms", lat = 51.8710, lon = -3.4730, elevation = 440 },
        official = { { "MWIS", "mwis.org.uk" }, { "Met Office", "metoffice.gov.uk" } },
    },
    snowdon = {
        top = { name = "Yr Wyddfa", lat = 53.0685, lon = -4.0763, elevation = 1085 },
        start = { name = "Pen-y-Pass", lat = 53.0806, lon = -4.0216, elevation = 359 },
        official = { { "MWIS", "mwis.org.uk" }, { "Met Office", "metoffice.gov.uk" } },
    },
    garda = {
        top = { name = "Monte Altissimo di Nago", lat = 45.8106, lon = 10.8881, elevation = 2078 },
        start = { name = "Bocca di Navene", lat = 45.7880, lon = 10.8726, elevation = 1420 },
        official = { { "Meteotrentino", "meteotrentino.it" }, { "MeteoAlarm", "meteoalarm.org" } },
        avalanche = { "Euregio avalanche report", "avalanche.report", caaml = { provider = "euregio" } },
        iconD2 = true,
    },
    zermatt = {
        top = { name = "Matterhorn", lat = 45.9763, lon = 7.6586, elevation = 4478 },
        start = { name = "Schwarzsee", lat = 45.9910, lon = 7.7085, elevation = 2583 },
        official = { { "MeteoSwiss", "meteoswiss.admin.ch" }, { "MeteoAlarm", "meteoalarm.org" } },
        avalanche = { "SLF bulletin", "slf.ch", caaml = { provider = "slf" } },
        iconD2 = true,
    },
    lofoten = {
        top = { name = "Fløya", lat = 68.2483, lon = 14.6012, elevation = 590 },
        start = { name = "Svolvær", lat = 68.2420, lon = 14.5800, elevation = 30 },
        official = { { "Yr (Met Norway)", "yr.no" }, { "MeteoAlarm", "meteoalarm.org" } },
        avalanche = { "Varsom", "varsom.no" },
    },
    fuji = {
        top = { name = "Mount Fuji", lat = 35.3606, lon = 138.7274, elevation = 3776 },
        start = { name = "Yoshida 5th Station", lat = 35.3954, lon = 138.7330, elevation = 2305 },
        official = { { "Japan Meteorological Agency", "jma.go.jp" } },
    },
    tekapo = {
        top = { name = "Mount John", lat = -43.9858, lon = 170.4650, elevation = 1029 },
        start = { name = "Lake Tekapo village", lat = -44.0045, lon = 170.4740, elevation = 720 },
        official = { { "MetService", "metservice.com" } },
    },
    paine = {
        top = { name = "Base Las Torres", lat = -50.9450, lon = -72.9650, elevation = 880 },
        start = { name = "Welcome Center", lat = -50.9615, lon = -72.8650, elevation = 150 },
        official = { { "Meteochile", "meteochile.gob.cl" } },
    },
}
