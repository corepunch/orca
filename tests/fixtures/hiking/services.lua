-- Synthetic answers in the documented shapes of the US services the hiking
-- edition reads (all public domain; none recorded, as this sandbox cannot
-- reach them):
--
--   fire        WFIGS current perimeters query (ArcGIS, f=json, returnCentroid)
--   fireNone    the same query with nothing within 30 km
--   fireError   ArcGIS's 200 answer for a failed query
--   starts      WFIGS current incident locations (points, outSR=4326)
--   snotel      NRCS AWDB /services/v1/data, SNWD in inches, one null day
--   nps         NPS /api/v1/alerts?parkCode=yose
--   lightning   Open-Meteo icon_d2 lightning_potential, one spike at 14:00 UTC
local Fixture = {}

-- About 12 km north-east of the Mount Tallac trailhead (38.9227, -120.0714).
Fixture.fire = {
    objectIdFieldName = "OBJECTID", geometryType = "esriGeometryPolygon", spatialReference = { wkid = 4326 },
    fields = {},
    features = {
        { attributes = { poly_IncidentName = "Cascade", attr_IncidentSize = 2300, attr_PercentContained = 45,
            attr_ModifiedOnDateTime_dt = 1794981600000 }, -- 2026-11-18 06:00 UTC, ms
          centroid = { x = -119.9731, y = 38.9991 } },
    },
}
Fixture.fireNone = { objectIdFieldName = "OBJECTID", features = {} }
Fixture.fireError = { error = { code = 400, message = "Cannot perform query. Invalid query parameters.", details = {} } }

-- A new start about 5 km west of the trailhead, and the Cascade fire's point.
Fixture.starts = {
    geometryType = "esriGeometryPoint", spatialReference = { wkid = 4326 },
    features = {
        { attributes = { IncidentName = "Fallen Leaf", IncidentSize = 3, PercentContained = 0,
            ModifiedOnDateTime_dt = 1794974400000 }, geometry = { x = -120.1293, y = 38.9227 } },
        { attributes = { IncidentName = "CASCADE", IncidentSize = 2300, PercentContained = 45 },
            geometry = { x = -119.9731, y = 38.9991 } },
    },
}

Fixture.snotel = {
    { stationTriplet = "463:CA:SNTL",
      data = { {
          stationElement = { elementCode = "SNWD", ordinal = 1, durationName = "DAILY", storedUnitCode = "in" },
          values = {
              { date = "2026-11-14", value = 25 },
              { date = "2026-11-15" }, -- no reading that day (null)
              { date = "2026-11-16", value = 29 },
              { date = "2026-11-17", value = 33 },
          },
      } } },
}

Fixture.nps = {
    total = "2", limit = "20", start = "0",
    data = {
        { id = "a1", title = "Half Dome cables are down for the season", category = "Park Closure", parkCode = "yose",
            url = "https://www.nps.gov/yose/planyourvisit/halfdome.htm",
            description = "The cables are down until late May.", lastIndexedDate = "2026-11-17 10:00:00.0" },
        { id = "a2", title = "Shuttle runs on the winter schedule", category = "Information", parkCode = "yose",
            url = "", description = "" },
    },
}

function Fixture.lightning(series, iso)
    return { latitude = 45.98, longitude = 7.66, elevation = 4478, utc_offset_seconds = 0, timezone = "GMT",
        hourly = { time = series(48, iso), lightning_potential = series(48, function(i) return i == 14 and 5.5 or 0 end) } }
end

return Fixture
