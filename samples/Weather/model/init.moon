weather_api = require "model.weather"
Settings    = require "model.settings"

-- Default location shown on the Home and Forecast screens.
DEFAULT_LAT = 37.7749
DEFAULT_LON = -122.4194

-- Saved locations shown on the Saved screen.
LOCATIONS = {
  { name: "San Francisco", admin1: "California", country: "United States", lat: 37.7749, lon: -122.4194, timezone: "America/Los_Angeles" }
  { name: "New York",      admin1: "New York",   country: "United States", lat: 40.7128, lon: -74.0060,  timezone: "America/New_York" }
  { name: "Berlin",        admin1: "Berlin",     country: "Germany",       lat: 52.5200, lon: 13.4050,   timezone: "Europe/Berlin" }
}

LOCATION_CATALOG = {
  { name: "London",        admin1: "England",          country: "United Kingdom", lat: 51.5072, lon: -0.1276,  timezone: "Europe/London" }
  { name: "London",        admin1: "Ontario",          country: "Canada",         lat: 42.9849, lon: -81.2453, timezone: "America/Toronto" }
  { name: "London",        admin1: "Kentucky",         country: "United States",  lat: 37.1290, lon: -84.0833, timezone: "America/New_York" }
  { name: "Paris",         admin1: "Ile-de-France",    country: "France",         lat: 48.8566, lon: 2.3522,   timezone: "Europe/Paris" }
  { name: "Tokyo",         admin1: "Tokyo",            country: "Japan",          lat: 35.6762, lon: 139.6503, timezone: "Asia/Tokyo" }
  { name: "Sydney",        admin1: "New South Wales",  country: "Australia",      lat: -33.8688, lon: 151.2093, timezone: "Australia/Sydney" }
  { name: "Madrid",        admin1: "Community of Madrid", country: "Spain",       lat: 40.4168, lon: -3.7038,  timezone: "Europe/Madrid" }
  { name: "Los Angeles",   admin1: "California",       country: "United States",  lat: 34.0522, lon: -118.2437, timezone: "America/Los_Angeles" }
  { name: "Chicago",       admin1: "Illinois",         country: "United States",  lat: 41.8781, lon: -87.6298, timezone: "America/Chicago" }
  { name: "Rome",          admin1: "Lazio",            country: "Italy",          lat: 41.9028, lon: 12.4964,  timezone: "Europe/Rome" }
  { name: "Amsterdam",     admin1: "North Holland",    country: "Netherlands",    lat: 52.3676, lon: 4.9041,   timezone: "Europe/Amsterdam" }
  { name: "Singapore",     admin1: "",                 country: "Singapore",      lat: 1.3521,  lon: 103.8198, timezone: "Asia/Singapore" }
}

-- Module-level cache – reset when the app restarts.
context = {}
context_meta = {}
current_location = LOCATIONS[1]
CACHE_TTL_SECONDS = 10 * 60

cache_get = (key) ->
  data = context[key]
  return nil unless data
  ts = context_meta[key] or 0
  return data if os.time! - ts <= CACHE_TTL_SECONDS
  context[key] = nil
  context_meta[key] = nil
  nil

cache_set = (key, value) ->
  context[key] = value
  context_meta[key] = os.time!
  value

clear_cache = ->
  context = {}
  context_meta = {}

location_subtitle = (loc) ->
  parts = {}
  parts[#parts + 1] = loc.admin1 if loc.admin1 and #loc.admin1 > 0
  parts[#parts + 1] = loc.country if loc.country and #loc.country > 0
  table.concat parts, ", "

location_key = (loc) -> "#{loc.name}:#{loc.lat},#{loc.lon}"

copy_location = (loc) ->
  {
    name: loc.name
    admin1: loc.admin1
    country: loc.country
    lat: loc.lat
    lon: loc.lon
    timezone: loc.timezone
  }

find_saved_location = (loc) ->
  key = location_key loc
  for saved in *LOCATIONS
    return saved if location_key(saved) == key
  nil

add_location = (loc) ->
  saved = find_saved_location loc
  unless saved
    saved = copy_location loc
    LOCATIONS[#LOCATIONS + 1] = saved
  current_location = saved
  saved

set_current_location = (loc) ->
  current_location = find_saved_location(loc) or add_location loc

get_current_location = -> current_location or LOCATIONS[1]

search_locations = (query) ->
  q = string.lower(query or "")
  return {} if #q == 0

  results = {}
  for loc in *LOCATION_CATALOG
    haystack = string.lower "#{loc.name} #{loc.admin1 or ""} #{loc.country or ""}"
    if string.find haystack, q, 1, true
      results[#results + 1] = loc
      break if #results >= 12
  results

-- Weather model.  All methods are called as class methods: Weather\current!
-- They yield the running coroutine while the HTTP request is in flight, so
-- they must be called from within a coroutine (content: is async when
-- triggered by a user event; use pcall to guard against the initial
-- synchronous render during startup).
class Weather
  -- Returns the current-conditions table for (lat, lon).
  current: (lat=nil, lon=nil) =>
    unless lat and lon
      loc = get_current_location!
      lat, lon = loc.lat, loc.lon
    key = "current:#{lat},#{lon}"
    cached = cache_get key
    return cached if cached
    data = weather_api.current lat, lon
    cache_set key, data.current

  -- Returns the daily forecast table for (lat, lon).
  forecast: (lat=nil, lon=nil) =>
    unless lat and lon
      loc = get_current_location!
      lat, lon = loc.lat, loc.lon
    key = "forecast:#{lat},#{lon}"
    cached = cache_get key
    return cached if cached
    data = weather_api.forecast lat, lon
    cache_set key, data.daily

  -- Returns the hourly forecast table for (lat, lon) — 48 hours (2 days).
  hourly: (lat=nil, lon=nil) =>
    unless lat and lon
      loc = get_current_location!
      lat, lon = loc.lat, loc.lon
    key = "hourly:#{lat},#{lon}"
    cached = cache_get key
    return cached if cached
    data = weather_api.hourly lat, lon
    cache_set key, data.hourly

  refresh: =>
    clear_cache!

  alerts: (lat=nil, lon=nil) =>
    current = @current lat, lon
    forecast = @forecast lat, lon
    alerts = {}

    uv = current.uv_index or 0
    if uv >= 8
      alerts[#alerts + 1] = {
        level: "warning"
        title: "High UV Index"
        detail: "UV index is #{math.floor(uv + 0.5)}. Limit midday sun exposure."
      }

    wind = current.wind_speed_10m or 0
    if wind >= 14
      alerts[#alerts + 1] = {
        level: "warning"
        title: "Strong Wind"
        detail: "Sustained wind is #{math.floor(wind + 0.5)} m/s. Secure loose outdoor items."
      }

    today_precip = ((forecast.precipitation_probability_max or {})[1] or 0)
    if today_precip >= 70
      alerts[#alerts + 1] = {
        level: "advisory"
        title: "Rain Likely Today"
        detail: "Precipitation chance is #{math.floor(today_precip + 0.5)}%."
      }

    max_temp = ((forecast.temperature_2m_max or {})[1] or 0)
    min_temp = ((forecast.temperature_2m_min or {})[1] or 0)
    if max_temp >= 35
      alerts[#alerts + 1] = {
        level: "advisory"
        title: "Heat Advisory"
        detail: "High temperature may reach #{@format_temp max_temp}."
      }
    if min_temp <= 0
      alerts[#alerts + 1] = {
        level: "advisory"
        title: "Cold Advisory"
        detail: "Low temperature may drop to #{@format_temp min_temp}."
      }

    if #alerts == 0
      alerts[#alerts + 1] = {
        level: "ok"
        title: "All Clear"
        detail: "No severe weather conditions detected right now."
      }

    alerts

  -- Human-readable label for a WMO weather code.
  description: (code) => weather_api.description code

  -- SVG icon path for a WMO weather code.
  icon: (code) => weather_api.icon code

  -- Cardinal direction string ("N", "NE", …) from a bearing in degrees.
  wind_dir: (degrees) => weather_api.wind_dir degrees

  -- Format a temperature value using the currently selected unit.
  -- Returns "N °C" (Celsius) or "N °F" (Fahrenheit), rounded to the nearest integer.
  format_temp: (t) =>
    if Settings\get_unit! == "fahrenheit"
      "#{math.floor(t * 9/5 + 32 + 0.5)} °F"
    else
      "#{math.floor(t + 0.5)} °C"

return {
  :Weather
  :LOCATIONS
  :Settings
  :add_location
  :set_current_location
  :get_current_location
  :search_locations
  :location_subtitle
  :clear_cache
  format_time: weather_api.format_time
  default_lat: DEFAULT_LAT
  default_lon: DEFAULT_LON
}
