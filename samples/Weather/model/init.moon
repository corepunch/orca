weather_api = require "model.weather"

-- Default location shown on the Home and Forecast screens.
DEFAULT_LAT = 37.7749
DEFAULT_LON = -122.4194

-- Saved locations shown on the Saved screen.
LOCATIONS = {
  { name: "San Francisco", lat: 37.7749, lon: -122.4194 }
  { name: "New York",      lat: 40.7128, lon: -74.0060  }
  { name: "Berlin",        lat: 52.5200, lon: 13.4050   }
}

-- Module-level cache – reset when the app restarts.
context = {}

-- Weather model.  All methods are called as class methods: Weather\current!
-- They yield the running coroutine while the HTTP request is in flight, so
-- they must be called from within a coroutine (content: is async when
-- triggered by a user event; use pcall to guard against the initial
-- synchronous render during startup).
class Weather
  -- Returns the current-conditions table for (lat, lon).
  current: (lat=DEFAULT_LAT, lon=DEFAULT_LON) =>
    key = "current:#{lat},#{lon}"
    return context[key] if context[key]
    data = weather_api.current lat, lon
    context[key] = data.current
    context[key]

  -- Returns the daily forecast table for (lat, lon).
  forecast: (lat=DEFAULT_LAT, lon=DEFAULT_LON) =>
    key = "forecast:#{lat},#{lon}"
    return context[key] if context[key]
    data = weather_api.forecast lat, lon
    context[key] = data.daily
    context[key]

  -- Human-readable label for a WMO weather code.
  description: (code) => weather_api.description code

  -- Cardinal direction string ("N", "NE", …) from a bearing in degrees.
  wind_dir: (degrees) => weather_api.wind_dir degrees

  -- Format a temperature value as "N °C" (rounded to nearest integer).
  format_temp: (t) => "#{math.floor(t + 0.5)} °C"

return {
  :Weather
  :LOCATIONS
  default_lat: DEFAULT_LAT
  default_lon: DEFAULT_LON
}
