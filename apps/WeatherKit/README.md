# WeatherKit

One forecast, many weather apps. WeatherKit holds everything the apps share:
the Open-Meteo fetch, the scoring helpers, the data scope and a Windows Phone
panorama. Each app adds a **profile** that answers one question about every
hour: is it good, fair or poor for *this* activity, and why?

| App | Question it answers | Extra data |
| --- | --- | --- |
| [Weather](../Weather/profile.lua) | Is it nice to be outside? | — |
| [Fishing](../Fishing/profile.lua) | Will the fish bite? Pressure trend, wind on the water, dawn and dusk, moon | — |
| [Surf](../Surf/profile.lua) | Is it worth paddling out? Swell size and period, offshore or onshore wind for each beach | Marine forecast |
| [Sailing](../Sailing/profile.lua) | Is there a steady, usable wind? Wind band, gusts, offshore danger for kites | Marine forecast when there is sea |
| [Hiking](../Hiking/profile.lua) | Is the trail safe and pleasant? Thunder and storm energy, ridge gusts, cold, fog, freezing level | Forecast at the spot's height |
| [Garden](../Garden/profile.lua) | Good for garden work, and what does the garden need? Frost ahead, wet or frozen soil, wind for spraying, watering | Soil, evapotranspiration |
| [Stargazing](../Stargazing/profile.lua) | Will the night be dark and clear? Darkness, clouds, the moon up or down, damp and haze | Cloud layers; tonight's card |

## The screen

`Pages/Forecast.xml` is a panorama titled with the profile's name:

- **now**: the spot, a verdict tile (score and good/fair/poor), the temperature
  and sky, and the strongest reasons behind the verdict.
- **hourly**: the next 24 hours, each with its verdict and the conditions
  that decided it.
- **week**: every day's best window ("best 06–08h") and its verdict.
- **conditions**: the readings the profile cares about.
- **spots**: tap one to forecast there.

Verdict colours are state classes (`good`, `fair`, `poor`) in
`Styles/weather.css`; `good` uses the app's `accent` from its `ThemeLibrary`.

## How it fits together

```text
Fishing/App.lua      return require("WeatherKit/App")(require "Fishing/profile")
Fishing/profile.lua  spots, rate(hour), note(hour), metrics(hour), window(hour)
WeatherKit/App.lua   store, routes, screen; one route "/"
  provider.lua       spots → WeatherSpot; report → WeatherRow rows
  forecast.lua       Open-Meteo forecast (+ marine) → hours and days
  report.lua         hours and days + profile → now, reason, hour, day, metric rows
  score.lua          the scoring card: add points with a reason, cap, verdict
  sky.lua            weather codes, compass, angles, moon phase, dates
Data/Forecast.xml    Spots, SpotList, Report (follows the selected spot), and
                     one View per panorama section, filtered by row Kind
```

One request feeds the whole panorama: the report is a single query whose rows
carry a `Kind`, and each section is a view over it. That keeps one loading and
error state and one refresh. The report refreshes every 30 minutes while it is
showing.

## Write another profile

1. Copy `apps/Surf` to `apps/<Name>`. Rename the package, pick an accent and
   keep the `WeatherKit` project reference.
2. In `profile.lua`, list spots and write `rate(hour, forecast)`. Start a card
   with `Score.card(base)`, then call `card:add(points, "the reason a person
   would give")` for each rule. Call `card:cap(limit, reason)` for anything
   that rules the hour out, such as lightning or the dark.
3. Write `note(hour)` (one line under each hour) and `metrics(hour, forecast)`
   (the conditions section). `Report.pressure`, `Report.wind`, `Report.sun` and
   `Report.moon` are ready-made.
4. Set `needsMarine = true` (or `"optional"`, so a failed marine request
   still shows the rest) to receive `swellHeight`, `swellPeriod`,
   `swellDir`, `waveHeight` and `seaTemp` on every hour.
5. Optional: `window(hour)` limits which hours count towards a day's best
   window (daylight for most; the dark for stargazing), and `focus(forecast)`
   picks the hour index the verdict card shows instead of now (Stargazing
   shows tonight's first dark hour).
6. Add checks to `tests/test_weather_profiles.lua`. Then run
   `sh tools/xcode/generate.sh <Name>` for an iPhone project.

An hour has `time`, `temp`, `feels`, `code` (WMO), `wind`, `gust`, `windDir`,
`pressure`, `pressureTrend` (hPa over 3 h), `cloud`, `rainChance`, `rain`,
`humidity`, `dewPoint`, `visibility`, `uv`, `isDay`, `cloudLow`/`Mid`/`High`,
`freezingLevel`, `cape`, `soilMoisture`, `soilTemp`, `et0`, `epoch` (UTC) and its `day` (`sunrise`, `sunset`,
`max`, `min`). Units are °C, m/s, hPa and metres. `forecast.spot` is the
selected spot, `forecast.epoch` the time of the request and `forecast.elevation`
the height the forecast is for. Snow codes at temperatures above 5 °C (3 °C
for a day's low) are reported as rain: the model occasionally says otherwise.

## Run and test

```bash
build/bin/orca apps/Fishing
build/bin/orca -test=tests/test_weather_profiles.lua   # or: make test-weather-profiles
```

The test runs every app on a synthetic forecast, so it needs no network.

Not yet: searching for a place, device location, saved spots, units other
than metric, tides, and the moon's real rise and set times (stargazing
estimates whether it is up from its phase, to about an hour). Open-Meteo has no tide forecast.
