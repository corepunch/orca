-- Open-Meteo weather API service.
-- Free to use, no API key required: https://open-meteo.com/
network = require "orca.network"

BASE_URL = "https://api.open-meteo.com/v1/forecast"

-- WMO Weather interpretation codes
DESCRIPTIONS =
  [0]:  "Clear sky"
  [1]:  "Mainly clear",    [2]:  "Partly cloudy",       [3]:  "Overcast"
  [45]: "Fog",             [48]: "Icy fog"
  [51]: "Light drizzle",   [53]: "Drizzle",              [55]: "Heavy drizzle"
  [61]: "Light rain",      [63]: "Rain",                 [65]: "Heavy rain"
  [71]: "Light snow",      [73]: "Snow",                 [75]: "Heavy snow"
  [80]: "Light showers",   [81]: "Showers",              [82]: "Violent showers"
  [85]: "Snow showers",    [86]: "Heavy snow showers"
  [95]: "Thunderstorm",    [96]: "Thunderstorm w/ hail", [99]: "Thunderstorm w/ heavy hail"

-- SVG icon filenames for WMO weather codes
ICONS =
  [0]:  "sun",           [1]:  "sun",           [2]:  "partly-cloudy"
  [3]:  "cloud"
  [45]: "fog",           [48]: "fog"
  [51]: "drizzle",       [53]: "drizzle",       [55]: "drizzle"
  [61]: "rain",          [63]: "rain",          [65]: "rain"
  [71]: "snow",          [73]: "snow",          [75]: "snow"
  [80]: "rain",          [81]: "rain",          [82]: "rain"
  [85]: "snow",          [86]: "snow"
  [95]: "thunderstorm",  [96]: "thunderstorm",  [99]: "thunderstorm"

DIRECTIONS = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" }

description = (code) ->
  DESCRIPTIONS[code] or "Code #{code}"

icon = (code) ->
  name = ICONS[code] or "cloud"
  "assets/icons/weather/#{name}.svg"

wind_dir = (degrees) ->
  idx = math.floor((degrees + 22.5) / 45) % 8
  DIRECTIONS[idx + 1]

current = (lat, lon) ->
  url = BASE_URL ..
    "?latitude=#{lat}" ..
    "&longitude=#{lon}" ..
    "&current=temperature_2m,apparent_temperature,wind_speed_10m,wind_direction_10m,weather_code" ..
    "&wind_speed_unit=ms" ..
    "&timezone=auto"
  response = network.fetch url
  response\json!

forecast = (lat, lon) ->
  url = BASE_URL ..
    "?latitude=#{lat}" ..
    "&longitude=#{lon}" ..
    "&daily=temperature_2m_max,temperature_2m_min,weather_code" ..
    "&wind_speed_unit=ms" ..
    "&timezone=auto"
  response = network.fetch url
  response\json!

return { :description, :icon, :wind_dir, :current, :forecast }
