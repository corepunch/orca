-- Synthetic answers for Fishers' Weather's services, built from each
-- service's documented response shape (Research/services-fishing.md) and
-- trimmed to a few readings. The values are made up; none was recorded
-- live, since the hosts were unreachable when they were written. Each
-- should be checked against one live answer before release.
-- F.NOW is 2026-11-18 06:00 UTC.
local S = {}

-- USGS OGC API, latest-continuous: one feature per gauge and parameter.
-- Here discharge and gage height, and no water temperature.
S.usgsLatest = { type = "FeatureCollection", features = {
    { type = "Feature", geometry = { type = "Point", coordinates = { -77.1278, 38.9497 } },
        properties = { monitoring_location_id = "USGS-01646500", parameter_code = "00060", statistic_id = "00011",
            time = "2026-11-18T00:20:00-05:00", value = "1240", unit_of_measure = "ft^3/s", approval_status = "Provisional" } },
    { type = "Feature", geometry = { type = "Point", coordinates = { -77.1278, 38.9497 } },
        properties = { monitoring_location_id = "USGS-01646500", parameter_code = "00065", statistic_id = "00011",
            time = "2026-11-18T00:20:00-05:00", value = "3.81", unit_of_measure = "ft", approval_status = "Provisional" } },
    -- A second gauge further off, as a bounding box brings it.
    { type = "Feature", geometry = { type = "Point", coordinates = { -77.25, 39.0 } },
        properties = { monitoring_location_id = "USGS-01646502", parameter_code = "00060", statistic_id = "00011",
            time = "2026-11-18T00:15:00-05:00", value = "900", unit_of_measure = "ft^3/s", approval_status = "Provisional" } },
} }

-- The same gauge with its water temperature (00010) too.
S.usgsWithTemp = { type = "FeatureCollection", features = {
    S.usgsLatest.features[1], S.usgsLatest.features[2],
    { type = "Feature", geometry = { type = "Point", coordinates = { -77.1278, 38.9497 } },
        properties = { monitoring_location_id = "USGS-01646500", parameter_code = "00010", statistic_id = "00011",
            time = "2026-11-18T00:20:00-05:00", value = "9.5", unit_of_measure = "degC", approval_status = "Provisional" } },
} }

S.usgsEmpty = { type = "FeatureCollection", features = {}, numberReturned = 0 }

-- USGS continuous: a week of discharge, steady at 2,000 ft³/s, then
-- falling to 1,240 over the last day.
function S.usgsWeek(now)
    local features = {}
    for day = 7, 1, -1 do
        local value = day == 1 and 1500 or 2000
        local epoch = now - 40 * 60 - day * 86400 -- the latest reading's time, `day` days back
        features[#features + 1] = { type = "Feature", properties = { monitoring_location_id = "USGS-01646500",
            parameter_code = "00060", time = os.date("!%Y-%m-%dT%H:%M:%S+00:00", epoch), value = tostring(value),
            unit_of_measure = "ft^3/s", approval_status = "Provisional" } }
    end
    return { type = "FeatureCollection", features = features }
end

-- Environment Agency flood monitoring: station readings, newest first.
S.ea = { items = {
    { ["@id"] = "x", dateTime = "2026-11-18T05:45:00Z", value = 1.12,
        measure = "http://environment.data.gov.uk/flood-monitoring/id/measures/1491TH-level-stage-i-15_min-mASD" },
    { ["@id"] = "x", dateTime = "2026-11-18T05:45:00Z", value = 42.5,
        measure = "http://environment.data.gov.uk/flood-monitoring/id/measures/1491TH-flow--i-15_min-m3_s" },
    { ["@id"] = "x", dateTime = "2026-11-17T05:45:00Z", value = 0.80,
        measure = "http://environment.data.gov.uk/flood-monitoring/id/measures/1491TH-level-stage-i-15_min-mASD" },
    { ["@id"] = "x", dateTime = "2026-11-17T05:45:00Z", value = 20.1,
        measure = "http://environment.data.gov.uk/flood-monitoring/id/measures/1491TH-flow--i-15_min-m3_s" },
    { ["@id"] = "x", dateTime = "2026-11-14T05:45:00Z", value = 18.0,
        measure = "http://environment.data.gov.uk/flood-monitoring/id/measures/1491TH-flow--i-15_min-m3_s" },
    { ["@id"] = "x", dateTime = "2026-11-12T05:45:00Z", value = 19.0,
        measure = "http://environment.data.gov.uk/flood-monitoring/id/measures/1491TH-flow--i-15_min-m3_s" },
} }

-- PEGELONLINE station with current W, Q and WT (local time, +01:00).
S.pegel = { uuid = "00000000-0000-0000-0000-000000000000", number = "0000000", shortname = "TESTPEGEL",
    longname = "TESTPEGEL", km = 100.0, agency = "TEST", longitude = 9.17, latitude = 47.66,
    water = { shortname = "TEST", longname = "TESTFLUSS" },
    timeseries = {
        { shortname = "W", longname = "WASSERSTAND ROHDATEN", unit = "cm", equidistance = 15,
            currentMeasurement = { timestamp = "2026-11-18T06:45:00+01:00", value = 312.0, stateMnwMhw = "normal",
                stateNswHsw = "normal" } },
        { shortname = "Q", longname = "ABFLUSS", unit = "m3/s", equidistance = 15,
            currentMeasurement = { timestamp = "2026-11-18T06:45:00+01:00", value = 250.0 } },
        { shortname = "WT", longname = "WASSERTEMPERATUR", unit = "°C", equidistance = 15,
            currentMeasurement = { timestamp = "2026-11-18T06:45:00+01:00", value = 8.4 } },
    } }

-- PEGELONLINE W measurements: a level rising 30 cm in the last day.
S.pegelWeek = {
    { timestamp = "2026-11-12T06:45:00+01:00", value = 280.0 },
    { timestamp = "2026-11-15T06:45:00+01:00", value = 282.0 },
    { timestamp = "2026-11-17T06:45:00+01:00", value = 282.0 },
    { timestamp = "2026-11-18T06:45:00+01:00", value = 312.0 },
}

-- OPW waterlevel.ie latest: level and temperature sensors of one station,
-- the voltage of another (left out).
S.opw = { type = "FeatureCollection", features = {
    { type = "Feature", geometry = { type = "Point", coordinates = { -8.6, 52.7 } },
        properties = { station_ref = "0000099999", station_name = "Test Bridge", sensor_ref = "0001", region_id = 3,
            datetime = "2026-11-18T05:30:00Z", value = "0.512", err_code = 99 } },
    { type = "Feature", geometry = { type = "Point", coordinates = { -8.6, 52.7 } },
        properties = { station_ref = "0000099999", station_name = "Test Bridge", sensor_ref = "0002", region_id = 3,
            datetime = "2026-11-18T05:30:00Z", value = "7.9", err_code = 99 } },
    { type = "Feature", geometry = { type = "Point", coordinates = { -9.0, 53.0 } },
        properties = { station_ref = "0000088888", station_name = "Other Weir", sensor_ref = "0003", region_id = 3,
            datetime = "2026-11-18T05:30:00Z", value = "12.1", err_code = 99 } },
} }

-- NOAA CO-OPS water_temperature, time_zone=gmt, units=metric.
S.coops = { metadata = { id = "0000000", name = "Test Harbor", lat = "24.95", lon = "-80.60" },
    data = { { t = "2026-11-18 05:36", v = "18.3", f = "0,0,0" }, { t = "2026-11-18 05:48", v = "18.4", f = "0,0,0" } } }
S.coopsStale = { metadata = S.coops.metadata, data = { { t = "2026-11-17 18:00", v = "19.0", f = "0,0,0" } } }
S.coopsNoSensor = { error = { message = "No data was found. This product may not be offered at this station "
    .. "at the requested time." } }

-- NOAA GLERL ERDDAP tabledap: daily lake averages; the columns are put
-- out of their usual order, so the parser must find them by name.
S.glerl = { table = {
    columnNames = { "time", "Ontario", "Erie", "Superior", "St. Clair" },
    columnTypes = { "String", "float", "float", "float", "float" },
    columnUnits = { "UTC", "degree_C", "degree_C", "degree_C", "degree_C" },
    rows = {
        { "2026-11-08T12:00:00Z", 11.0, 12.6, 6.0, 10.0 },
        { "2026-11-11T12:00:00Z", 10.6, 11.9, 5.8, 9.4 },
        { "2026-11-17T12:00:00Z", 10.1, 10.4, 5.5, 8.0 },
    } } }

return S
