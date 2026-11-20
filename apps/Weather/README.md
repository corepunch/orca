# Weather

One forecast per place, read by every activity that fits there. Each
activity is a lens over the same forecast: it rates every hour **good**,
**fair** or **poor** and says why in plain words. The look is Windows Phone
(Panorama, Pivot, big lowercase type). The references behind it are in
[DESIGN.md](DESIGN.md).

| Activity | Question it answers | Extra data |
| --- | --- | --- |
| [outdoors](activities/outdoors.lua) | Is it nice to be outside? | — |
| [fishing](activities/fishing.lua) | Will the fish bite? Pressure trend, wind on the water, dawn and dusk, moon | — |
| [surf](activities/surf.lua) | Is it worth paddling out? Swell size, period and direction; offshore or onshore wind for the beach | Marine forecast |
| [sailing](activities/sailing.lua) | Is there a steady, usable wind? Wind band, gusts, offshore danger for kites | Marine forecast when there is sea |
| [hiking](activities/hiking.lua) | Is the trail safe and pleasant? Thunder and storm energy, ridge gusts, cold, fog, freezing level | Forecast at the place's height |
| [garden](activities/garden.lua) | Good for garden work, and what does the garden need? Frost ahead, wet or frozen soil, spraying wind, watering | Soil, evapotranspiration |
| [stars](activities/stargazing.lua) | Will the night be dark and clear? Darkness, clouds, the moon up or down, damp and haze | Cloud layers; tonight's card |

## Screens

**The place hub** (`Pages/Hub.xml`) is a Panorama titled with the place:

- **now**, after Amazing Weather: a huge temperature, feels like, the sky and
  the wind; then humidity, pressure, visibility, precipitation, dew point,
  sunrise, sunset and the moon; then a five-day strip.
- **good for**: one row per activity that fits the place. Each row shows its
  verdict in colour and its best window today, or tomorrow once today's has
  passed, with the reason that matters most. Tap a row to open that activity.
- **hourly**: charts first, as in Amazing Weather: the next 24 hours of
  temperature (columns) with wind (a line), then the chance of rain.
- **week**: every day, with its best activity and window.
- **places**: tap one to see its hub from the start. The section ends with
  a **settings** link.

In an edition the hub's first section opens with the edition activity's
cards ("frost tonight"); in the full app, with everyday outdoors' cards. A
small line under the conditions says how old the forecast is ("updated 25
min ago"), and says so plainly when it is the copy saved before the network
went away.

**An activity page** (`Pages/Activity.xml`) is a Pivot titled "fishing · lake
tahoe", with four views:

- **now**: a verdict tile with the score, the reasons, then the conditions the
  activity watches;
- **today**: a 24-hour verdict band (one cell per hour in its tone) and the
  two charts the activity names in `charts` (pressure and wind for fishing,
  swell and wind for surf, clouds and humidity for stars);
- **hourly**: the hour-by-hour list, one swipe further, as in Amazing Weather;
- **week**: each day's best window.

Tap the title line or swipe in from the left edge to go back. The page uses
the activity's accent colour. In an edition, the edition's own activity is
titled with the app's name when it fits ("surfers' weather · Bondi").

**Settings** (`Pages/Settings.xml`) is a Pivot built from declarations:
**units** (temperature, wind, heights, rain, pressure, distance, clock),
**activities** (each activity's own `settings`; an edition names it after its
activity) and **about** (the Open-Meteo attribution).

Verdict colours are state classes (`good`, `fair`, `poor`) in
`Styles/weather.css`. `good` uses the hub accent, or the activity's accent on
its page.

Every screen runs edge to edge (`RespectSafeArea = false` in `App.lua`) and
its Pivot or Panorama sets `ExtendBodyBehindHeader="true"`: the title and
headers sit on a frosted bar (`.pivot-bar, .panorama-bar` in
`Styles/weather.css`, as Chat's dark bar) and each section's scrolling body
scrolls under it and into the home-indicator area without being cut off. Give
a new page the same header and a scrolling body (`Overflow="hidden scroll"`).
Stargazers' red night mode tints the bar red-black in `night.css`.

## How it fits together

```text
App.lua            navigation: the hub stays alive; an activity page and settings open and close
places.lua         every place and what it is good for (facing, lake, …)
activities.lua     the activities in hub order, their accents, edition names, merged requests
activities/*.lua   one file per activity: rate, note, metrics, charts, window, focus, cards, …
provider.lua       places; hub, activity and settings rows; one cached and one saved forecast per place
forecast.lua       Open-Meteo forecast (+ marine, + sources) → hours, days and quarter hours
report.lua         hub rows and activity rows (WeatherRow, told apart by Kind)
score.lua          the scoring card: add points with a reason, cap, verdict; twilight, darkness
sky.lua            weather codes and icons, compass, angles, moon, dates, sun height, twilights
units.lua          every number as the user reads it (°C/°F, m/s…knots, m/ft, …, 24/12 h)
settings.lua       stored choices: units, clock and each activity's declared settings
storage.lua        small Lua-table files on the device, written atomically
Data/Hub.xml       Places, PlaceList, Report and a View per hub section
Data/Activity.xml  Report for the route's place and activity, a View per pivot item
Data/Settings.xml  the settings rows, split into units, activities and about
```

A place's forecast is fetched once and cached for ten minutes. The hub,
every activity page and going back all reuse it. A pull to refresh (or the
half-hourly poll) fetches it again. Places whose activities read the sea also
fetch the marine forecast. If that request fails, the rest still shows. The
last answer per place is saved on the device: the hub shows it at once on
start while the new one loads, and uses it, marked offline, when the network
fails.

## Add an activity

1. Add `activities/<id>.lua`. Copy the closest one; surf is a good model.
   Write `rate(hour, forecast)`:
   - Start a card with `Score.card(base)`.
   - Call `card:add(points, "the reason a person would give")` for each rule.
   - Call `card:cap(limit, reason)` for anything that rules the hour out,
     such as lightning or the dark.
2. Write `note(hour)` (the line under each hour), `metrics(hour, forecast)`
   (the conditions) and `charts`: two `{ title, unit, kind = "line" |
   "column", min, max, value = function(hour) }` for the "today" view.
   `value` is metric; `unit` is a Units kind (`"wind"`, `"temp"`,
   `"percent"`…) that converts it and ends the title ("wind kn"). Write
   every number with `Units` (below), never with a unit in the string.
   `Report.pressure`, `Report.wind`, `Report.sun` and `Report.moon` are
   ready-made.
3. Optional:
   - `needsMarine` adds the sea fields to each hour: `swellHeight`,
     `swellPeriod`, `swellDir`, `waveHeight` and `seaTemp`.
   - `window(hour)` picks which hours count towards a day's best window.
   - `focus(forecast)` picks the hour the verdict card shows.
4. Register it in `activities.lua` with an accent. Add matching rules to
   `Styles/weather.css`.
5. List it on the places in `places.lua` where it makes sense.
6. Add checks to `tests/test_weather_<id>.lua` (see "Tests" below).

An hour has these fields:

- **When:** `time`, `epoch` (UTC), `isDay`, and its `day` (`sunrise`, `sunset`, `max`, `min`).
- **Air:** `temp`, `feels`, `humidity`, `dewPoint`, `visibility`, `uv`.
- **Sky:** `code` (WMO), `cloud`, `cloudLow`/`Mid`/`High`.
- **Wind:** `wind`, `gust`, `windDir`.
- **Pressure and storms:** `pressure`, `pressureTrend` (hPa over 3 h),
  `freezingLevel`, `cape`.
- **Rain:** `rainChance`, `rain`.
- **Soil:** `soilMoisture`, `soilTemp`, `et0`.

- **Sun:** `sunAltitude` (degrees, at the hour's start); the day's
  `twilight` (see `Sky.twilight`).
- **Any column an activity asked for** under its API name
  (`hour.soil_temperature_6cm`, `hour.secondary_swell_wave_height`).

Units are °C, m/s, hPa and metres. `forecast.spot` is the place,
`forecast.epoch` is the time of the request, `forecast.updated` when the data
was fetched, `forecast.stale` is true for an offline copy, and
`forecast.elevation` is the height the forecast is for. `forecast.now` is the
current hour's index and `forecast.today` today's index in `forecast.days`
(earlier entries are past days). `forecast.quarters` holds minutely_15
rows and `forecast.missing[id]` an optional source that failed. Snow codes above 5 °C (3 °C for a day's low) are
reported as rain, because the model occasionally says otherwise.

## Run, test and capture

```bash
build/bin/orca apps/Weather
make test-weather
make test-weather-surf      # one per edition: fishing surf sailing hiking garden stargazing
```

### Editions: one app per activity

The same source also builds as a single-activity app. An edition keeps
everyday "outdoors" and its activity, only the places that offer it, and the
activity's accent on the hub. `package.lua` lists the editions under
`Editions`: fishing, surf, sailing, hiking, garden and stargazing.

```bash
make iphone APP=weather EDITION=surf     # "Surfers' Weather", org.orca.weather.surf
make ipad APP=weather EDITION=fishing    # "Fishers' Weather"
ORCA_EDITION=surf build/bin/orca apps/Weather
```

The iOS bundler checks the key against `Editions`, writes `build.lua` into
the bundled project and names the app after the edition. Each edition has its
own bundle id, so it installs beside the full app. Without `EDITION`, every
activity is in.

`make test-weather` runs `build/bin/orca -test=tests/test_weather.lua`;
`make test-weather-<key>` runs `tests/test_weather_<key>.lua`.

```bash
P=app/HubPage/HubPanorama
python3 tools/capture.py apps/Weather --wait 'Now*:now:now' \
    --shot "now=$P?SelectedIndex=0" --shot "good-for=$P?SelectedIndex=1" \
    --strip build/captures/Weather.png
```

The tests run the app on a synthetic forecast, so they need no network.

### Tests

`tests/weather_fixture.lua` is the shared harness. Start a file with
`local F = dofile("tests/weather_fixture.lua").init { edition = "surf" }`
(before any Weather module is required) and end it with `F.done("weather-surf")`.
It gives `F.check`, `F.NOW` (2026-11-18 06:00 UTC), `F.hour(fields)` and
`F.verdict(activity, fields, spot)` for hand-built hours, `F.says(result,
pattern)`, `F.weather(lat, extraColumns)` and `F.marine()` (a synthetic
week), `F.network()` (a fake fetch with `requests`, `failing`,
`marineFailing` and `answers[pattern]`), `F.storage(name, places)` (an
emptied scratch folder), `F.launch { places, net, clock, storage, region }`
(the app at "/", with `find`, `has`, `text`, `press`, `back`, `open`,
`settle`) and `F.checkEdition(key)` (the checks every edition shares).
Default units in tests are metric (region "NL").

Not built yet:

- searching for a place, device location and saved places;
- choosing which activities to follow;
- tides;
- live tiles;
- the moon's real rise and set times (stars estimates them from the phase,
  to about an hour).

## Extension points for editions

Wave 2 of [PLAN.md](PLAN.md) grows each edition in its own files
(`activities/<key>.lua`, `editions/<key>/`, `tests/test_weather_<key>.lua`).
These contracts let it do so without touching shared files. If one is
missing, ask under "Requests to the foundation" in PLAN.md.

**`fields`: more forecast columns.** An activity may declare

```lua
Garden.fields = { hourly = { "soil_temperature_6cm" }, daily = { "et0_fao_evapotranspiration_sum" },
    minutely15 = { "precipitation" }, marine = { "secondary_swell_wave_height" }, pastDays = 7, forecastDays = 16 }
```

`Activities.request(place)` merges the fields of every activity at the place
into one request (columns once each, the most past and forecast days).
Columns the parser does not name land on the hour or day under their API
name; `marine` columns need `needsMarine`. Past hours come before
`forecast.now` in order; past days before `forecast.today`
(`Report.daysAhead(forecast)` skips them).

**`sources`: extra requests.**

```lua
Hiking.sources = { { id = "air", optional = true,
    url = function(spot, Forecast) return Forecast.url("airQuality", "latitude=" .. spot.lat .. "&longitude=" .. spot.lon
        .. "&hourly=pm2_5") end,
    parse = function(json, forecast) … end } }
```

`url` may return nil to skip. `parse` adds what it reads to the forecast
(hours, days or its own field). A required source that fails fails the
forecast; an optional one is recorded in `forecast.missing[id]`. Answers are
saved with the forecast and replayed offline. Base URLs and the API key live
in `Forecast.endpoints` (`forecast`, `marine`, `airQuality`, `geocoding`,
`apiKey`); build URLs with `Forecast.url(name, query)`, which switches to the
commercial "customer-" host and adds `apikey` when a key is set.

**`cards(forecast, place)`: big answers.** Returns a list of
`{ id, title, value, detail, tone = "good" | "fair" | "poor" | "info", icon }`
(`icon` a path such as `Sky.icon(code)`, optional). They show on the
activity's "now" view between the verdict and the reasons
(`Templates/CardRow.xml`, rows `<place>:<activity>:card:<id>`), and the
edition activity's cards lead the hub's first section (rows
`<place>:card:<id>`). In the full app everyday outdoors leads.

**`settings`: declared choices.**

```lua
Fishing.settings = {
    { key = "moon", title = "moon in the score", kind = "toggle", default = false, detail = "anglers' lore" },
    { key = "species", title = "species", kind = "choice", default = "bass", options = { { "bass", "bass" }, "trout" } },
    { key = "depth", title = "depth", kind = "number", default = 2, min = 0, max = 20, step = 0.5,
        format = function(v) return Units.height(v) end },
}
Settings.get("fishing", "moon")          -- the stored value, else the default
```

They appear on the settings page under the activity's title. A choice steps
to the next option, a toggle flips, a number has less and more. Values are
saved at once. `Settings.set(group, key, value)`, `Settings.step(group, key,
direction)` and `Settings.revision()` (bumps on every change) are there for
code; the shared units are the group `"units"` (`temp`, `wind`, `height`,
`rain`, `pressure`, `distance`, `clock`).

**`Units`: every number on screen.** The rules stay metric; only text is
converted. `Units.temp(c)` "12°", `Units.tempDelta(dc)`, `Units.wind(ms)`,
`Units.gust(ms, gust)` "18 g 26 kn", `Units.height(m, decimals)` (one decimal
under 10), `Units.rain(mm)`, `Units.pressure(hpa, decimals)`,
`Units.distance(km)`, `Units.clock(iso or "06:00")`, `Units.span(fromIso,
toIso)` "06–08h" or "6–8 am", `Units.label(kind)` for chart titles,
`Units.value(kind, metric)` for chart values. Defaults: metric (m/s,
24 h); a US region gets °F, mph, feet, inches, inHg, miles and a 12-hour
clock; the UK mph and miles; Sailors' Weather knots everywhere. The region
is `ORCA_REGION`, else the preferred language's region ("en-US"), else
`LC_ALL`, `LC_MEASUREMENT` or `LANG`; with none, metric.

**`Storage`: files on the device.** `Storage.load(name)` and
`Storage.save(name, table)` keep plain tables (strings, numbers, booleans,
nested tables) as Lua source in `$HOME/Library/Application Support/<app
name>/<name>.lua`, one folder per edition. Writes are atomic; reads run with
no environment and an instruction budget, so a file holds data, never code.
Names are letters, digits, `-`, `_` and `.`. `Storage.new { dir = … }` makes a
store elsewhere; the app uses the one tests pass (`Storage.use`). Already
taken: `settings`, `forecast-<place id>`.

**`Sky.sunAltitude(epoch, lat, lon)` and `Sky.twilight(date, lat, lon,
offset)`.** The sun's height in degrees (NOAA's algorithm, no refraction)
and, for a local date, `{ sunrise, sunset, polar, civil, nautical,
astronomical }`. Each twilight is `{ dawn, dusk, dawnEpoch, duskEpoch }` in
local time, `nil` when the sun never gets that low (no full darkness in a
northern summer), or `{ dark = true }` when it never gets that high (polar
night); `polar` is `"day"` or `"night"` when the sun never sets or rises.
The forecast puts them on `day.twilight` and `hour.sunAltitude`.
`Score.twilight(hour, level)` is the minutes to the nearest sunrise or sunset
(or that level's dawn or dusk) and `Score.darkness(hour)` says `"day"`,
`"civil"`, `"nautical"`, `"astronomical"` or `"night"`.

**Stylesheet.** An edition's activity may set `stylesheet =
"Weather/editions/<key>/style.css"`. Every screen of that edition then uses
it. The sheet starts with `@import "../../Styles/weather.css";` and adds
to it, for example a red night theme under a state class the edition sets.

