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
- **places**: tap one to see its hub from the start.

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
the activity's accent colour.

Verdict colours are state classes (`good`, `fair`, `poor`) in
`Styles/weather.css`. `good` uses the hub accent, or the activity's accent on
its page.

## How it fits together

```text
App.lua            navigation: the hub stays alive; an activity page opens and closes
places.lua         every place and what it is good for (facing, lake, …)
activities.lua     the activities in hub order, with their accents
activities/*.lua   one file per activity: rate, note, metrics, window, focus
provider.lua       places; hub and activity reports; one cached forecast per place
forecast.lua       Open-Meteo forecast (+ marine for coasts) → hours and days
report.lua         hub rows and activity rows (WeatherRow, told apart by Kind)
score.lua          the scoring card: add points with a reason, cap, verdict
sky.lua            weather codes and icons, compass, angles, moon, dates
Data/Hub.xml       Places, PlaceList, Report and a View per hub section
Data/Activity.xml  Report for the route's place and activity, a View per pivot item
```

A place's forecast is fetched once and cached for ten minutes. The hub,
every activity page and going back all reuse it. A pull to refresh (or the
half-hourly poll) fetches it again. Places whose activities read the sea also
fetch the marine forecast. If that request fails, the rest still shows.

## Add an activity

1. Add `activities/<id>.lua`. Copy the closest one; surf is a good model.
   Write `rate(hour, forecast)`:
   - Start a card with `Score.card(base)`.
   - Call `card:add(points, "the reason a person would give")` for each rule.
   - Call `card:cap(limit, reason)` for anything that rules the hour out,
     such as lightning or the dark.
2. Write `note(hour)` (the line under each hour), `metrics(hour, forecast)`
   (the conditions) and `charts`: two `{ title, kind = "line" | "column",
   min, max, value = function(hour) }` for the "today" view. `Report.pressure`, `Report.wind`, `Report.sun` and
   `Report.moon` are ready-made.
3. Optional:
   - `needsMarine` adds the sea fields to each hour: `swellHeight`,
     `swellPeriod`, `swellDir`, `waveHeight` and `seaTemp`.
   - `window(hour)` picks which hours count towards a day's best window.
   - `focus(forecast)` picks the hour the verdict card shows.
4. Register it in `activities.lua` with an accent. Add matching rules to
   `Styles/weather.css`.
5. List it on the places in `places.lua` where it makes sense.
6. Add checks to `tests/test_weather.lua`.

An hour has these fields:

- **When:** `time`, `epoch` (UTC), `isDay`, and its `day` (`sunrise`, `sunset`, `max`, `min`).
- **Air:** `temp`, `feels`, `humidity`, `dewPoint`, `visibility`, `uv`.
- **Sky:** `code` (WMO), `cloud`, `cloudLow`/`Mid`/`High`.
- **Wind:** `wind`, `gust`, `windDir`.
- **Pressure and storms:** `pressure`, `pressureTrend` (hPa over 3 h),
  `freezingLevel`, `cape`.
- **Rain:** `rainChance`, `rain`.
- **Soil:** `soilMoisture`, `soilTemp`, `et0`.

Units are °C, m/s, hPa and metres. `forecast.spot` is the place,
`forecast.epoch` is the time of the request, and `forecast.elevation` is the
height the forecast is for. Snow codes above 5 °C (3 °C for a day's low) are
reported as rain, because the model occasionally says otherwise.

## Run, test and capture

```bash
build/bin/orca apps/Weather
make test-weather
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

`make test-weather` runs `build/bin/orca -test=tests/test_weather.lua`.

```bash
P=app/HubPage/HubPanorama
python3 tools/capture.py apps/Weather --wait 'Now*:now:now' \
    --shot "now=$P?SelectedIndex=0" --shot "good-for=$P?SelectedIndex=1" \
    --strip build/captures/Weather.png
```

The test runs the app on a synthetic forecast, so it needs no network.

Not built yet:

- searching for a place, device location and saved places;
- choosing which activities to follow;
- units other than metric;
- tides;
- live tiles;
- the moon's real rise and set times (stars estimates them from the phase,
  to about an hour).
