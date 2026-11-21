# Weather apps: product plan

One source tree, seven App Store apps. The full **Weather** app is for
everyone. Six editions each serve one activity: **Fishers'**, **Surfers'**,
**Sailors'**, **Hikers'**, **Gardeners'** and **Stargazers' Weather**
(`EDITION=<key>`, see [README](README.md#editions-one-app-per-activity)).
The editions share the forecast and the Metro look. Beyond that they may
differ as much as their users do.

The research behind every item is in [Research/](Research/). Each file holds
the competitors, the public complaints, a persona and 100 user stories:

| App | Persona | Research |
| --- | --- | --- |
| Weather | Maya, a commuter parent; Sam, a weekend generalist | [weather.md](Research/weather.md) |
| Fishers' | a weekend lake and shore angler | [fishing.md](Research/fishing.md) |
| Surfers' | a dawn-patrol intermediate surfer | [surf.md](Research/surf.md) |
| Sailors' | a kiter and a dinghy sailor | [sailing.md](Research/sailing.md) |
| Hikers' | a day hiker and a mountaineer | [hiking.md](Research/hiking.md) |
| Gardeners' | Margaret (UK allotment); Dan (US lawn and beds) | [garden.md](Research/garden.md) |
| Stargazers' | a visual observer and an astrophotographer | [stargazing.md](Research/stargazing.md) |

Story numbers below (`S12`) refer to the app's own research file.

## 0. Decisions before launch (not code)

1. **Open-Meteo licence.** Every research file found the same blocker. The
   free API is for non-commercial use only. Its terms count an app with a
   subscription or ads as commercial, and free use is capped at about 10,000
   calls a day. A paid app needs the commercial API (its own host and an API
   key). Ensemble data needs the Professional tier. Ask Open-Meteo whether a
   free app with no ads counts. `forecast.lua` keeps the base URL and key in
   one place for this reason (F4).
2. **Business model.** Across all seven markets the loudest complaint is the
   same: features that were free moved behind a paywall, and ads cover the
   forecast. Recommended: no ads, no account, no tracking, and an honest
   one-off or small yearly price. Never take away something that was free.
3. **Attribution.** Show "Weather data by Open-Meteo.com" plus the sources
   Open-Meteo names (on the settings page, F3).
4. **Tides and observations.** Open-Meteo's sea level is about 8 km and "not
   for coastal navigation". Exact tides need NOAA (free, US only) or a paid
   global source. Decide per edition. Until then, label them "approximate".

## 1. Principles (from "what we must avoid")

- **A verdict with its reasons, always.** Never a bare number, and never a
  wall of numbers without a verdict.
- **Honest wording.** Say "estimate" for heuristics such as seeing, breaking
  height, storm clock and blight, and "approximate" for coarse data. Present
  lore as lore: the solunar effect has no evidence behind it.
- **Safety is never paywalled.** That covers thunder, gale, frost, darkness
  and cold water.
- **Calm notifications.** At most one a day per kind, with quiet hours, and
  each kind can be switched off.
- **Private by default.** Places, logs and profiles stay on the device, can
  be exported, and are never shared or put on a public map. No spot-burning.
- **Works offline.** Show the last forecast with its age, never silently stale.
- **Units the user reads:** knots for wind sports; °F, mph and inches in the
  US. The rules stay metric inside; only what is shown is converted.

## 2. How the work is split

```text
Wave 1  Foundation (one agent)        shared modules and extension points, F1–F9
Wave 2  Seven agents in parallel      one per app, each in its own files only
Wave 3  Places (one agent)            search, saved places, place editor
Wave 4  Native iOS (engine)           location, local notifications, widgets
```

**File ownership in wave 2.** Each edition agent owns these and edits nothing
else:

- `activities/<key>.lua`
- `editions/<key>/` (new modules, templates, pages, data)
- `tests/test_weather_<key>.lua`

The Weather agent owns `activities/outdoors.lua`, `report.lua`,
`Pages/Hub.xml`, `Data/Hub.xml`, `Templates/HubNow.xml` and
`tests/test_weather.lua`. Anything shared that an edition needs goes through
the extension points below. If one is missing, write the need into this file
under "Requests to the foundation" instead of editing shared files.

**Running and committing.** Build once (`make unite`). Run a single test
directly: `build/bin/orca -test=tests/test_weather_<key>.lua`. Commit only
your own paths: `git add <paths> && git commit -m … -- <paths>`. Retry if git
reports `index.lock`. Never stage other people's files: the checkout has
unrelated `apps/encarta` work in progress.

## 3. Wave 1: foundation (F1–F9)

These modules let every edition grow without touching shared files.

- **F1 `storage.lua`, local files.** `Storage.load(name)` and
  `Storage.save(name, table)` keep plain Lua tables. Writes are atomic (a
  temporary file, then rename). Reads go through `load(text, name, "t", {})`,
  so a file can hold no code. Location: `$HOME/Library/Application
  Support/<Name>/` on iOS and macOS, with one folder per edition so editions
  never share files. `options.dir` overrides it for tests. Values are strings,
  numbers, booleans and nested tables only.
- **F2 `units.lua` and `settings.lua`.**
  - Settings are stored with F1 and cover units, the clock, and each
    activity's own values.
  - Units cover temperature (°C/°F), wind (m/s, km/h, mph, kn, Bft), height
    (m/ft), rain (mm/in), pressure (hPa/inHg) and the clock (24 h/12 h).
  - The defaults come from the edition and the region: Sailors' uses knots;
    a US region uses °F, mph and inches.
  - The functions are `Units.temp(c)`, `Units.wind(ms)`, `Units.gust(ms, gust)`
    (shown as "18 g 26"), `Units.height(m)`, `Units.rain(mm)`,
    `Units.pressure(hpa)`, `Units.clock("06:00")` and `Units.label(kind)`.
  - Every existing activity's display text and chart titles move to `Units`
    in this step, so wave 2 starts with units done.
- **F3 settings page.** A `/settings` route with a Pivot, built from
  declarations rather than written by hand:
  - shared units and clock;
  - each activity's `settings` list:
    `{ key, title, kind = "choice" | "number" | "toggle", options, default, min, max, step }`,
    read with `Settings.get(activityId, key)`;
  - an "about" item with the data attribution.
  - It opens from a "settings" link at the end of the hub's places section.
- **F4 forecast extension points.** An activity may declare
  `fields = { hourly = {…}, daily = {…}, minutely15 = {…}, pastDays = n, forecastDays = n }`.
  `forecast.lua` merges the fields of the edition's activities into one
  request. Any column it does not know lands on the hour or day under its API
  name (for example `hour.soil_temperature_6cm`). Past hours keep their
  order, and `forecast.now` still points at the current hour.
  An activity may also declare
  `sources = { { id, url = function(spot) … end, optional = true, parse = function(json, forecast) … end } }`
  for extra requests such as air quality, NOAA SWPC Kp or NOAA tides. The base
  URLs and an optional API key live in one table (decision 0.1).
- **F5 offline forecast.** The provider saves the last decoded forecast per
  place with F1. When the network fails it uses that copy and sets
  `forecast.stale = true`. Both pages show "updated 2 h ago" (a `kind =
  "age"` row) and say clearly when the forecast is old. It starts from the
  saved copy, then refreshes.
- **F6 sun position.** `Sky.sunAltitude(epoch, lat, lon)` and
  `Sky.twilight(date, lat, lon, offset)` return civil, nautical and
  astronomical dawn and dusk, or `nil` when the sun never gets low enough
  (summer at high latitude). `Score.twilight` uses them. Hiking, stargazing,
  surf, fishing and garden all depend on this.
- **F7 cards.** An activity may provide `cards(forecast, place)`, a list of
  `{ id, title, value, detail, tone, icon }`. These are big answers such as
  "tonight", "frost tonight", "storm clock", "tides" or "water this week".
  Cards appear on the activity's "now" view above the reasons, with a new
  `Templates/CardRow.xml`. In an edition, the edition activity's cards also
  lead the hub's first section, so Gardeners' opens on "frost tonight", not
  on generic weather.
- **F8 test harness.** Move the synthetic forecast, fake fetch and clock from
  `tests/test_weather.lua` into `tests/weather_fixture.lua`. Add Makefile
  rules `test-weather-<key>` for the six editions and add them to
  `APP_LUA_TESTS`. Start each edition's test file with the activity checks
  that exist today.
- **F9 activity page title line.** When there is room, show the edition name
  instead of "weather" (for example "surfers' weather"). Keep Back.

**Done when:** `make test-weather` and all six `test-weather-<key>` pass, and
every edition still builds and runs on desktop with `ORCA_EDITION=<key>`.

## 4. Wave 2: per-app backlogs

Priorities: **P0** to launch, **P1** soon after, **P2** later. Size: S, M, L.
"Native" means it needs wave 4.

### Weather (full app): `outdoors.lua`, hub

| ID | Feature | Pri | Size | Notes |
| --- | --- | --- | --- | --- |
| W1 | Next-hour rain: "rain starts in about 20 min" | P0 | M | `minutely_15`. Real 15-minute data only in North America and Central Europe; elsewhere it is interpolated, so the wording softens there |
| W2 | Today in one line, plus what to bring (umbrella, coat, sunscreen) | P0 | S | add daily `precipitation_hours`, `uv_index_max`, `wind_gusts_10m_max`, `apparent_temperature_max/min` |
| W3 | Air quality and pollen in the outdoors verdict | P1 | S–M | Air Quality API as an F4 source. Pollen is Europe only, in season, 4 days |
| W4 | "4° colder than yesterday" | P1 | S | `pastDays = 1` |
| W5 | Tap a day to see its hours; 14–16 days with "less certain" beyond day 7 | P1 | S–M | `forecastDays = 16` |
| W6 | Everyday lenses: run, cycle, dog walk, picnic, laundry | P1 | M | new activities, full app only |
| W7 | Choose and order the "good for" activities | P1 | M | an F3 setting |
| W8 | Comfort setting ("I run cold") | P2 | S | an outdoors setting |
| W9 | An hourly "good for" traffic-light row on the hub | P2 | S | DESIGN.md already proposes it |
| W10 | Official severe weather warnings (US NWS, MeteoAlarm, Canada) | P1 | M–L | done for the US, Canada, Norway and Germany through `services/warnings.lua` (SV-W1 to SV-W4 below); MeteoAlarm, the UK, Australia, New Zealand and Japan are links until their licence or key allows reading |
| W11 | Accessibility: VoiceOver labels, Dynamic Type, contrast | P1 | M | partial, see [README](README.md#tests): engine accessibility properties and an iOS VoiceOver bridge; labels on colour-only bands, reasons and rows; Dynamic Type through `TextScale`, 0.9 to 1.35. Not done: VoiceOver scrolling to content below the fold (three-finger scroll does not reach engine scroll views), live Dynamic Type on an open page, a contrast audit |
| W12 | Rain radar | P2 | L | no free global radar (RainViewer's free API was cut back in 2025); a radar line for Germany is SV-W5, a map is SV-W8 |

#### Services (existing free services, [research](Research/services-weather.md))

Shared modules in `services/`, reusable by every edition (`Warnings.sources()`, `Warnings.cards`). Tests: `tests/test_weather_services.lua` (`make test-weather-services`), offline on synthetic answers.

| ID | Item | Pri | Status | Notes |
| --- | --- | --- | --- | --- |
| SV-W1 | Warning registry, coverage covered / link / none | P0 | done | `services/warnings.lua`; NWS moved to `services/nws.lua`; a place's country picks first, else coarse boxes. Link and none show a calm card, never "no warnings" |
| SV-W2 | Canada (ECCC GeoMet OGC API) | P0 | done | `services/eccc.lua`; bbox request, point in polygon at card time, French names for French readers. Not verified live (host blocked in the build session) |
| SV-W3 | MeteoAlarm (Europe) | P1 | link only | licence to confirm in writing (meteoalarm@geosphere.at); then the JSON feed per country and an EMMA region lookup |
| SV-W4 | Norway (MET MetAlerts 2.0) | P1 | done | `services/metno.lua`, by point; needs the place's country (NO) |
| SV-W5 | Radar "rain nearby" line (Germany, Bright Sky) | P1 | done | `services/radar.lua`; DWD warnings via Bright Sky too (`services/dwd.lua`). Distance and trend only, no direction yet, never minutes |
| SV-W6 | NWS HeatRisk | P2 | follow-up | touches run and dog walk |
| SV-W7 | Lightning potential (ICON-D2) | P2 | follow-up | needs a `score.lua` helper shared with hiking (HI3) |
| SV-W8 | Radar map | P2 | follow-up | needs an engine map/tile view |

### Fishers' Weather: `fishing.lua`

| ID | Feature | Pri | Size | Notes |
| --- | --- | --- | --- | --- |
| FI1 | Water temperature: sea from marine data, big lakes from `soil_temperature_0cm` over water cells, or the user's own reading | P0 | S–M | today fishing never asks for marine data |
| FI2 | Pressure history and fronts: 48 h back and 24 h ahead, "front passes ~14:00" | P0 | S–M | `pastDays = 2` |
| FI3 | Honest scoring: rules worded as anglers' experience, moon off by default with a toggle | P0 | S | the current solunar wording reads as fact |
| FI4 | Safety: wind builds later, wind onto or off the bank (`facing`), dawn fog, cold water, swell cap for rock anglers | P0 | S–M | |
| FI5 | Tides for coastal places: NOAA in the US, approximate Open-Meteo sea level elsewhere, with station and distance; tide stage in the score | P1 | L | decision 0.4 |
| FI6 | Private catch log: two taps, offline, saves the hour's conditions, free CSV export | P1 | L | F1; `editions/fishing/log.lua` |
| FI7 | "Your best conditions", learned from the log | P2 | M | after FI6 |
| FI8 | Real moonrise and moonset, major and minor periods shown as tradition | P1 | S | daily `moonrise`/`moonset` |
| FI9 | Species profiles (bass, trout, pike, carp, sea bass) that change the rules | P2 | M | an F3 setting |
| FI10 | Compare my waters: the best place this morning | P1 | M | provider |
| FI11 | River flow and level from Open-Meteo flood data | P2 | M | F4 source |
| FI12 | Link to local fishing rules | P2 | S | per place |

#### Fishers' Weather: services

Existing free services ([services-fishing.md](Research/services-fishing.md)),
in `editions/fishing/gauges.lua`, `tides.lua`, `water.lua` and `rules.lua`.
Response shapes come from the services' documentation; each needs one live
check before release (the hosts were unreachable when this was built).

| ID | Service | Pri | Status |
| --- | --- | --- | --- |
| SV-FI1 | USGS gauge: latest flow, level and water temperature, a week for trend and usual; "silent since" after 6 h | P0 | built. Place field `gauge = { usgs = … }`, or the nearest gauge within 10 km of a US river place. Picking a gauge in the place editor is a follow-up |
| SV-FI2 | NOAA CO-OPS water temperature at the place's tide station, ahead of the marine model | P0 | built; stale (over 6 h) or sensorless stations fall back |
| SV-FI3 | Environment Agency, PEGELONLINE, OPW gauges, one reading shape | P1 | built (`gauge = { ea = … }`, `pegel` or `opw`). EA typical range, OPW trend from saved readings and Hub'Eau (France) are follow-ups |
| SV-FI4 | NOAA GLERL GLSEA Great Lakes average surface temperature | P1 | built; by `greatLake` or the lake's name inside the lakes' box |
| SV-FI5 | GBIF "recorded near here" species | P2 | follow-up: needs a log-page source and a 30-day cache |
| SV-FI6 | NY and WA stocking (Socrata) | P2 | follow-up: confirm each portal's terms first |
| SV-FI7 | Algal blooms, link-only | P2 | built: a conditions row on lakes and rivers, EPA link in the US, "warm and calm" first |

### Surfers' Weather: `surf.lua`

| ID | Feature | Pri | Size | Notes |
| --- | --- | --- | --- | --- |
| SU1 | Several swells shown apart (primary and secondary swell, wind sea), plus peak period | P0 | S–M | marine `secondary_swell_*`, `wind_wave_*` |
| SU2 | Estimated breaking height in body words (knee, waist, head high), labelled "estimate" | P0 | S | |
| SU3 | A profile per break: swell window, best wind, size range, best tide | P0 | M | `places.lua` fields; an F3 setting for skill |
| SU4 | Dawn-patrol card: first light, the first good hour tomorrow | P0 | S | F6, F7 |
| SU5 | Tides: times, a curve, and each break's best tide in the rating | P1 | M–L | decision 0.4 |
| SU6 | Best break today among my breaks | P1 | M | provider |
| SU7 | 16-day outlook with a confidence word | P1 | M | check whether marine data goes past 8 days |
| SU8 | Water-quality warning after heavy rain (runoff) | P1 | S | `pastDays` rain sum |
| SU9 | Skill level and a short first-run setup | P1 | S | F3 |
| SU10 | Session log with a one-tap rating, later personal tuning | P2 | M | F1 |
| SU11 | Wind drawn against the beach | P2 | S | |

#### Surfers' services

Existing services ([Research/services-surf.md](Research/services-surf.md)),
each an optional source in `Surf.sources`; a failed or silent one never
reads as "safe" or "no warnings". Tests use synthetic answers in each
service's documented shape (`tests/test_weather_surf.lua`); none of these
hosts was reachable from the build machine.

| ID | Service | Pri | Status |
| --- | --- | --- | --- |
| SV-SU1 | NWS beach warnings (rip current, beach hazards, high surf, tsunami) | P0 | done: `editions/surf/hazards.lua`, source `surf-hazards`; the `hazard` card leads the hub (`rank = 0`) and caps the hour while in force, harder for beginners ("surf near a lifeguard") |
| SV-SU2 | Live NDBC buoy against the forecast | P0 | done: `editions/surf/buoys.lua`, source `surf-buoy` (CoastWatch ERDDAP JSON, six hours); "swell arriving" on a period jump; late (> 3 h), silent or failed buoys say so on the page. Buoys for Ocean Beach (46026) and Malibu (46025); the nearest station for other breaks is a follow-up. ERDDAP's lag was not measured |
| SV-SU3 | Measured water temperature (CO-OPS, else the buoy) | P0 | done: source `surf-water`; the water row says "measured at …" or "model". Stations 9414290 (San Francisco) and 9410840 (Santa Monica) |
| SV-SU4 | Official bathing-water status | P1 | partly: `editions/surf/waterquality.lua` with Ireland's EPA adapter (CC BY 4.0), dormant until a break has an Irish ID: its alert list's field names are unverified. The rain rule stays, worded "no official reading here". England's EA forecast (path unverified, no forecasts Oct–Apr) and NSW Beachwatch (endpoint and terms unconfirmed) are links only |
| SV-SU5 | Sun and current on the surf page | P1 | done: `editions/surf/comfort.lua`, source `surf-current` (its own small marine request, because the answer names the current's unit); card `comfort`, page only |
| SV-SU6 | Pages about the break (cams, water quality, sharks, surf-zone forecast) | P1 | done as text: `links` in `breaks.lua`, card `links`, page only. Cards cannot open an outside page yet |
| SV-SU7 | NWS Surf Zone Forecast rip risk | P2 | follow-up: text layouts differ by office and no recorded product was available |
| SV-SU8 | Buoys outside the US (UK CCO, Irish Marine Institute) | P2 | follow-up: CCO needs a key tied to a Referer; the Irish ERDDAP dataset was not verified |

Requests to the foundation from Surfers' services:

- A card link that opens an outside page (for example `act` returning
  `{ url = … }`), so SV-SU6's links can be tapped.
- In the full app the surf page asks `api.weather.gov` for alerts again
  when outdoors already has them (`forecast.warnings`); a shared alert
  source with per-activity filters would save the request.

### Sailors' Weather: `sailing.lua`

| ID | Feature | Pri | Size | Notes |
| --- | --- | --- | --- | --- |
| SA1 | Knots first, with gusts as "18 g 26" | P0 | S | done by F2; check every string |
| SA2 | Rider profile: sport, level, wind range, kite and sail sizes; "rig the 9 m" | P0 | M | an F3 setting; replaces the fixed 3/5/11/15 m/s |
| SA3 | Gust realism: squalls from `cape` and pressure trend, direction shifts, lulls | P0 | S | |
| SA4 | Model agreement: "sure / likely / uncertain" per hour, plus a model strip | P1 | M | `models=`; don't hard-code model names (HRRR and NAM retire November 2026) |
| SA5 | Sea-breeze hint: an afternoon onshore build on sunny days | P1 | S–M | |
| SA6 | Tides and wind-against-tide hours | P1 | M | decision 0.4 |
| SA7 | "Where should I go this weekend": rank my spots | P1 | M | provider |
| SA8 | Safe direction sector per spot (offshore danger for kites) | P0 | S | `places.lua` |
| SA9 | Live stations and buoys against the forecast | P2 | L | done: METAR, OpenWindMap, NDBC (services SV-SA2, SV-SA4, SV-SA5); bias per spot is SV-SA7 |
| SA10 | Gale and storm warnings at saved spots | P1 | M | done: NWS (US) and MeteoAlarm (Europe), services SV-SA1, SV-SA3 |

#### Sailors' Weather services

From [services-sailing.md](Research/services-sailing.md). Code in
`editions/sailing/warnings.lua`, `live.lua` and `links.lua`; every source is
optional; credit in `Sailing.attribution`.
Tests run on synthetic answers in the documented shapes
(`tests/fixtures/sailing/answers.lua`): the hosts were blocked when this was
built, so each still needs one live recording.

| ID | Service | Pri | Status | Notes |
| --- | --- | --- | --- | --- |
| SV-SA1 | NWS marine warnings (api.weather.gov) | P0 | done | Gale, Storm, Special Marine and similar cap every profile; a Small Craft Advisory caps learners and dinghies. Point query only: the `?zone=` second call waits for a Crissy recording |
| SV-SA2 | Nearest airport METAR (aviationweather.gov) | P0 | done | "observed now", distance, age, against the forecast hour; not live after 90 min. Airport vs shore ("inland") is not known, so the card says "airport, X km away" |
| SV-SA3 | MeteoAlarm warnings, Europe | P1 | done | country feed; area by CAP polygon, a curated area-name table per spot, or the place's own names; another district is "elsewhere", and a found place with no match reads "check your area", never "none" |
| SV-SA4 | OpenWindMap / Pioupiou stations | P1 | done | `live-with-meta/all` inside its dense area (France, the Alps), or one station when the place sets `windStation`; offline and older than 30 min skipped; a station within 10 km leads |
| SV-SA5 | NDBC shore stations and buoys | P1 | done | `format = "text"`; curated stations for Crissy (FTPC1, 46026) and Islamorada (MLRF1); the offshore buoy adds a waves card |
| SV-SA6 | Station and forecast links | P1 | partial | a "check by hand" card (NDBC pages, NWS text, MeteoAlarm, OpenWindMap, Met Office for GB). Shown as text: no open-a-web-page action yet; no free "station link" field in the place editor; Holfuy and Windguru ids not seeded (unverified) |
| SV-SA7 | Forecast-vs-observed bias per spot | P2 | follow-up | needs stored pairs (Storage) |
| SV-SA8 | DMI and KNMI observations | P2 | follow-up | needs keys in `Forecast.endpoints` |

### Hikers' Weather: `hiking.lua`

| ID | Feature | Pri | Size | Notes |
| --- | --- | --- | --- | --- |
| HI1 | Summit and trailhead side by side, from one multi-point request; fix the valley coordinates | P0 | S–M | `editions/hiking/places`; F4 |
| HI2 | Ridge wind at summit height from pressure levels | P0 | M | `wind_speed_850/700/600hPa` + geopotential |
| HI3 | Storm clock: the first likely thunder hour and "be off the summit by", labelled "estimate" | P0 | M | `cape`, `lifted_index`, codes 95–99 |
| HI4 | Daylight budget: first and last light, daylight left, turnaround from start time and hike length | P0 | S–M | F6; an F3 setting |
| HI5 | Trail conditions: snow depth, last 72 h of snow and rain, freeze–thaw, heat, UV | P1 | S | `pastDays = 3`, `snow_depth` |
| HI6 | Model confidence and 16 days | P1 | M | |
| HI7 | Official warnings and avalanche bulletin links | P1 | M | with W10 |
| HI8 | Smoke and air quality on the trail | P2 | S | Air Quality source |

#### Hikers' Weather: services

Existing services from [services-hiking.md](Research/services-hiking.md)
(issue #503). Each is an optional source in `editions/hiking/services.lua`;
the rating stays weather-only, and credits come from `Hiking.attribution`.

| ID | Feature | Pri | Status |
| --- | --- | --- | --- |
| SV-HI1 | US avalanche danger card (avalanche.org map layer) | P0 | built, **off** (`Services.CLEARED.avalancheOrg`) until the National Avalanche Center agrees in writing; Tahoe keeps the link meanwhile |
| SV-HI2 | Alpine danger at the summit's elevation band (EAWS CAAML v6) | P0 | built: SLF on (CC BY 4.0), Euregio **off** until its licence is settled, Varsom link only; both peaks wait for their micro-region ids from regions.avalanches.org (not reachable here) |
| SV-HI3 | Wildfire near the trail (WFIGS perimeters and new starts) | P1 | built, on for US peaks |
| SV-HI4 | Measured snow near the top (SNOTEL) | P1 | built for Tahoe (Echo Peak); SLF IMIS later (endpoint to verify) |
| SV-HI5 | National park alerts (NPS) | P1 | built, on only with `Forecast.endpoints.npsKey` (none committed) |
| SV-HI6 | Huts, shelters and water (OSM, precomputed) | P2 | follow-up: needs a maintainer's Overpass run per peak |
| SV-HI7 | Lightning potential in the storm clock (ICON-D2) | P2 | built for Garda and Zermatt; threshold `Trail.LIGHTNING` to calibrate |
| SV-HI8 | Official links where no open API can be used | P2 | built: MeteoAlarm, USFS forest orders, Euregio and Varsom links; Avalanche Canada, BRA and SAIS wait for a curated peak there |

### Gardeners' Weather: `garden.lua`

| ID | Feature | Pri | Size | Notes |
| --- | --- | --- | --- | --- |
| GA1 | Tonight's frost with ground frost: "possible" from 4 °C and "likely" from 2 °C on clear, calm nights | P0 | M | today it only warns at air ≤ 1 °C |
| GA2 | Weekly water balance: rain against evaporation over the last and next 7 days; "water / skip / rain coming"; L/m² | P0 | M | `pastDays = 7`, daily ET0 |
| GA3 | Soil temperature at sowing depth, 7-day trend, "warm enough to sow" for common crops, lawn trigger | P0 | M | `soil_temperature_6cm/18cm`; `editions/garden/crops.lua` |
| GA4 | Job windows: spray, mow, plant out, feed, harvest before bad weather | P0 | M | cards |
| GA5 | Garden-first hub: frost tonight, water this week, best time today | P0 | S | F7 |
| GA6 | Potato blight (Hutton criteria) and mildew risk | P1 | S | |
| GA7 | Heat, gust, hail and snow warnings for plants | P1 | S | |
| GA8 | Root-zone moisture instead of 0–1 cm | P1 | S | `soil_moisture_3_to_9cm`, `9_to_27cm` |
| GA9 | Local frost dates with 10/50/90 % odds, and hardiness zone | P1 | M | historical API once, cached with F1 |
| GA10 | Growing degree days since a date | P2 | M | done with SV-GA2: `editions/garden/degree.lua`, "growing season" card |
| GA11 | Plot settings: frost pocket, soil type, what I protect | P1 | M | F3 |
| GA12 | 16 days | P2 | S | |
| GA13 | Moon gardening, optional and labelled as tradition | P2 | S | |

#### Gardeners' Weather: services

Existing services ([Research/services-garden.md](Research/services-garden.md)).
Each is an optional source (asked for the hub in Gardeners' Weather and,
in the full app, when the garden page opens), credited through the
activity's `attribution`; a failed service never reads as "no warning".

| ID | Service | Status |
| --- | --- | --- |
| SV-GA1 | NWS frost, freeze, heat and high-wind alerts, US (`editions/garden/warnings.lua`) | done: card `official` (rank 0, unmodified, issuer named); the frost card says "NWS: … in force", "no NWS frost alert in force … our estimate" or "NWS alerts not reached" |
| SV-GA2 | Growing degree days from the archive (`degree.lua`, `climate.lua`) | done: second archive source `gdd` (once a day, after the hub), 30-year average season from the frost-date request, "N days ahead", crabgrass window for US lawns (rule of thumb, source *verify*) |
| SV-GA3 | CAMS pollen, Europe (`jobs.lua`) | done: source `pollen`; a note on mowing, the best time today and the hour; no own card |
| SV-GA4 | U.S. Drought Monitor category at the point (`water.lua`) | done: source `drought` (FeatureServer point query, at most once a day, kept 8 days); D1+ adds a line to the water card. FeatureServer owner to confirm |
| SV-GA5 | Hosepipe-ban pointer, UK (`water.lua`) | done as text (Water UK's address): no card link can open a web page yet |
| SV-GA6 | Met Office Weather Warnings, UK | link only ("official warnings" row): NSWWS Open Licence and style guide unread, and the feed names regions, not points |
| SV-GA7 | USA-NPN Spring Index and pest stages | not started: For-Profit policy to clear first |
| SV-GA8 | Reference links: USDA map, RHS ratings, AHDB BlightWatch | done as text on the hardiness and blight rows |

### Stargazers' Weather: `stargazing.lua`

| ID | Feature | Pri | Size | Notes |
| --- | --- | --- | --- | --- |
| ST1 | Real darkness (astronomical, nautical, civil); "no full darkness tonight" | P0 | S–M | F6; replaces "90 min after sunset" |
| ST2 | Real moonrise and moonset, moon height per hour, moon-free dark window | P0 | S–M | daily `moonrise/moonset/moon_phase`; height computed locally |
| ST3 | Tonight as one night, dusk to dawn, not split at midnight | P0 | S–M | |
| ST4 | High cloud and haze rated apart from clear; aerosols | P0 | S–M | `cloud_cover_high`; Air Quality AOD as an F4 source |
| ST5 | Dew and frost on the optics: "dew heaters from 23:00" | P0 | S | |
| ST6 | Red night mode that switches on at dusk, with no white flash | P0 | M | `editions/stargazing/night.css` and a state class; also check the launch screen |
| ST7 | Model agreement: "chance of clear" | P1 | M | `models=` |
| ST8 | Seeing and transparency, labelled "estimate" | P1 | M–L | jet stream, lapse rate; `editions/stargazing/seeing.lua` |
| ST9 | Aurora chance from Kp, the oval, darkness, cloud and latitude | P1 | M | NOAA SWPC as an F4 source |
| ST10 | Night-long chart from dusk to dawn: cloud layers, moon height, darkness | P1 | M | |
| ST11 | Fog and low-cloud risk | P1 | S | `boundary_layer_height` |
| ST12 | Meteor showers (a yearly table) and planets up tonight | P2 | S/M | done (SV-ST4, SV-ST5): `planets.lua`, `events.lua`; lunar eclipses too, solar eclipses not yet |
| ST13 | Deep-sky and planet scores shown apart | P2 | S | partial: the planets card is apart from the night's verdict; no planet score yet |
| ST14 | Bortle class per curated site | P2 | S | not built: SV-ST7 (Black Marble estimate); the World Atlas is CC BY-NC |

#### Stargazers' services

Existing services reused ([Research/services-stargazing.md](Research/services-stargazing.md)).
Each is an optional source or local code, credited through the activity's
`attribution`; a failed one is recorded in `forecast.missing` and the
aurora card says the data is missing, never "no aurora".

| ID | Service | Status |
| --- | --- | --- |
| SV-ST1 | Solar wind now (SWPC mag/plasma 2-hour) | done: `Aurora.solarWind`, `windText`; asked where the aurora can matter on a dark night; rows over 15 min old count as missing |
| SV-ST2 | OVATION 30-minute nowcast (SWPC) | done: asked only dark and in reach (`Aurora.worth`), cut to the cells within 10° before it is saved; replaces the Kp reason for the hour going on |
| SV-ST3 | SWPC alerts, GFZ Hp30 | done: watches, warnings and alerts with cancellations; "activity is rising now" from Hp30 |
| SV-ST4 | Planets tonight (Schlyter) | done: `planets.lua`, a card; checked against published conjunctions, oppositions, elongations and magnitudes |
| SV-ST5 | Meteor showers and lunar eclipses (static tables) | done: `events.lua`, cards rated by radiant height, moonlight and cloud; solar eclipses need Besselian elements (follow-up) |
| SV-ST6 | ISS and Tiangong passes (CelesTrak + SGP4) | deferred: L, and CelesTrak's terms for an app with many users are unconfirmed |
| SV-ST7 | Bortle estimate from NASA Black Marble | deferred (P2): needs an offline tool and a data file |
| SV-ST8 | Second-opinion links | deferred (P2): the app has no way to open a web page yet (request to the foundation) |

## 5. Wave 3: places (one agent)

| ID | Feature | Pri | Size | Status |
| --- | --- | --- | --- | --- |
| P1 | Place search through the Open-Meteo Geocoding API (name or postcode, with country) | P0 | M | done, 2277736e: offline it finds the app's own places |
| P2 | Saved places with F1: add, remove, reorder, home first; the curated places become suggestions | P0 | M | done, 2277736e: the first run shows the suggestions |
| P3 | Place editor for what an activity needs: a shore-facing compass, lake or sea, summit height, safe sector, break profile | P0 | M | done, 2277736e: activities declare `placeFields`; no rename yet |
| P4 | Activities that fit a place are guessed from its kind and confirmed by the user | P1 | S | done, 2277736e |
| P5 | Compare saved places for one activity ("best place today") as a shared view that SU6, SA7 and FI10 use | P1 | M | done, 93063191: a shared helper (`compare.lua`), not a new page |

## 6. Wave 4: native iOS (engine work)

| ID | Feature | Pri | Size | Status |
| --- | --- | --- | --- | --- |
| N1 | Device location with approximate accuracy accepted, Lua API `orca.location`; reverse geocoding through `CLGeocoder` | P0 | M | done, 039abc5e: "use my location" on the hub and the search page, one fix, saved only through the editor (`MyPlaces.fromLocation`); `Capabilities = { "location" }` in package.lua adds the usage string |
| N2 | Local notifications and background refresh (`BGAppRefreshTask`), Lua API for scheduling; alert rules in Lua (`alerts.lua`) | P1 | L | done, 2f0b4292: seven calm kinds for the home place, off until asked, quiet hours, one per kind a day; `background.lua` refreshes best effort ([README](README.md#tests)) |
| N3 | Home and lock-screen widgets: a WidgetKit extension reading a snapshot the app writes into an App Group | P1 | L | partial: `widget.lua` writes the snapshot through `orca.notifications.writeWidgetSnapshot`, `Widget/WeatherWidget.swift` reads it, `WIDGETS=1` builds and embeds it (simulator checked: the app writes `widget.json` into its group). Needs the owner: App Groups on the team and a widget target in Xcode for device builds ([docs/IOS.md](../../docs/IOS.md#widgets)); no reload from the app (WidgetCenter is Swift-only), the widget reads every 30 min |
| N4 | Dark launch screen for Stargazers' (no white flash) | P0 | S | done, 4d13ba66: package.lua `LaunchBackground = "#000000"` gives every Weather build a black launch screen and dark system sheets |
| N5 | Live Activity for rain, Apple Watch | P2 | L | not started |

## 7. Status

Each agent updates its own rows here: `done` with the commit, or `blocked:
why`.

| Wave | Status |
| --- | --- |
| Research | done (Research/*.md) |
| 1 Foundation | done: F1, F2, F6 in 850a5a31; F2–F5, F7, F9 in c9a8f894; F8 in 76b480de. Contracts in [README](README.md#extension-points-for-editions) |
| 2 Apps | done. Weather W1–W10 except W11 (engine has no accessibility properties), W12 skipped: bee80f81, deca9fde, e42e0d37, 68412dba. Fishers' FI1–FI12 (FI2, FI5, FI11, FI12 partial): 0bd69837, d70eb45b. Surfers' SU1–SU11 (SU6, SU9, SU10 partial): ed5e2cfa, 9eec452e. Sailors' SA1–SA8 (SA9, SA10 skipped): 42328fd6. Hikers' HI1–HI8 (HI7 needs a User-Agent to work live): 07010bfc. Gardeners' GA1–GA9, GA11–GA13 (GA10 skipped): e8387067, b16276bd. Stargazers' ST1–ST9, ST11 (ST10 partial; ST12–ST14 skipped): 8420eb49. Tests: 880 checks pass |
| 2.5 Integration | done: I1–I11 (section 8). I1, I4 in ee252388; I2, I3 in 5c861e16; I5–I7 in 8b7204f4; I8–I10 in f4a83259; I11 with this row. Tests: 919 checks pass |
| 3 Places | done: P1–P4 in 2277736e, P5 in 93063191 (section 5). Device location is wave 4's: `MyPlaces.fromLocation(lat, lon, name, region)` makes the draft. Tests: 991 checks pass |
| 4 Native | N1, N2, N4 done; W11 and N3 partial; N5 not started (section 6). Tests: weather 403, alerts 40, editions 614 checks pass |

## 8. Wave 2.5: integration (one agent, shared files)

The requests the wave-2 agents could not make themselves, grouped. One agent
owns every shared file for this wave.

| ID | Change | From | Pri | Status |
| --- | --- | --- | --- | --- |
| I1 | **Fewer requests per place.** The full app merges every edition's `sources` and fields: about 9 requests per place at Tahoe, and quarter-hours for all 16 days. Fetch an activity's optional sources only for the lead activity on the hub, and for the activity whose page is open. Add `minutely15Steps` (`forecast_minutely_15`). Request `models=` in the main call when an activity asks (`fields.models`). Lakes don't ask for marine data (`needsMarine` may be a function of the place) | Weather, sailing, fishing, hiking | P0 | done, ee252388 (numbers below); `fields.models` is supported but no edition moved onto it, see below |
| I2 | **A calm hub.** Cards say where they show: `cards(forecast, place, clock, where)` with `where = "hub"` or `"activity"`, or a card's `hub = false`. The hub shows at most three edition cards, ranked by tone (poor and warnings first), and the rest stay on the activity page | garden, surf, sailing, hiking | P0 | done, 5c861e16; a card may also set `rank` (Surfers' first-run setup leads with `rank = 0`) |
| I3 | **Actions and edition pages.** A card or row may carry an `action` handled by `activity.act(id, …)`, and an activity may declare `page` (routes, provider, link text) that the app routes to. This wires up Fishers' log, waters and patterns (d70eb45b; see the fishing agent's diff), Surfers' one-tap session log and setup | fishing, surf | P0 | done, 5c861e16: Fishers' log opens from the fishing page; Surfers' level and session log are card links |
| I4 | **Headers for sources** (`source.headers` → `network.fetch(url, { headers })`) with a generic app User-Agent and no personal contact. This makes the NWS warnings work live for hikers and the full app (`api.weather.gov` answers 403 without one) | hiking, Weather | P0 | done, ee252388: the engine already sends `headers` (and `nocookies`); every request carries "<AppName>/1.0 (ORCA)" and no cookies. Checked live: api.weather.gov answered with warnings |
| I5 | **Night-aware days and charts:** `activity.nightOf(hour)` for grouping in `bestOfDay` and the week; `bestWindow` checks that hours follow each other by `epoch`; `activity.chartHours(forecast)` so charts can start before now (fishing pressure 48 h back, stars dusk to dawn) | stargazing, fishing | P1 | done, 8b7204f4: stargazing groups by night and charts dusk to dawn; fishing charts two days back |
| I6 | **Edition page classes:** a supported `lead.pageClasses()` hook in `screenFor` and on refresh, replacing red mode's side effect through the stylesheet | stargazing | P1 | done, 8b7204f4: red mode moved onto `pageClasses` and a `stylesheet` function |
| I7 | `activity.dayNote(day, forecast)` on day rows (surf's confidence); a third chart slot or a tide curve | surf | P2 | done, 8b7204f4: surf's confidence on the week, and a tide chart as the third |
| I8 | Endpoints table: add `flood`, `archive` and NOAA tides, so a commercial key switches every host | fishing, garden | P1 | done, f4a83259: only Open-Meteo hosts move with the key |
| I9 | Test fixture: synthetic places carry their real `utc_offset_seconds` and longitude; `Fixture.storage` clears extra files | stargazing, garden | P1 | done, f4a83259: `F.network { placeClock = true }`, `F.weather(lat, extra, lon)`, `F.storage(name, places, extra)` |
| I10 | CSS: accents for the lenses (run, cycle, dog walk, picnic, laundry); `.wx-poor` to at least 4.5:1 contrast on black | Weather | P2 | done, f4a83259: lime, pink, taupe, olive and steel; poor text #f01a00 (4.85:1) |
| I11 | README: document `/day`, answer rows, `everywhere`/`followed`, the `clock` argument to `cards`, the outdoors settings, and every hook added here | Weather | P1 | done (README, with this table) |

**I1, requests per place.** Counted with the test fixture's fake network
over each build's own places: the hub's refresh, then opening the place's
first activity. Fresh storage, so nothing is replayed.

| Build | Places | Hub before | Hub after | Activity page after |
| --- | --- | --- | --- | --- |
| Weather (full) | 34 | 5.29 | 2.65 | +1.44 |
| Weather at Lake Tahoe | 1 | 11 | 3 | +0 (fishing at a lake) |
| Fishers' | 6 | 2.33 | 1.67 | +0 |
| Surfers' | 7 | 3.00 | 3.00 | +0 |
| Sailors' | 8 | 4.12 | 3.75 | +0 |
| Hikers' | 11 | 4.27 | 4.27 | +0 |
| Gardeners' | 9 | 1.11 | 1.11 | +0 |
| Stargazers' | 8 | 4.00 | 4.00 | +0 |

Before, the page added nothing because the hub had fetched everything. The
full app gains most: its hub now asks for the forecast, air quality and (in
the US) warnings, and the other activities' sources wait for their page.
Lakes lose the marine request (Fishers', Sailors'). An edition's lead is its
own activity, so its sources stay on the hub; what remains is those:
models, summit points, air and warnings (hiking), models, air and Kp
(stars), models, tides and spot ranking (sailing), the long wave outlook
(surf). The quarter hours shrink from 16 days to 12 steps, which lightens
the full app's main call.

Decision for the user: moving sailing's, hiking's and stargazing's model
requests into the main call (`fields.models`) would save one request each,
but Open-Meteo returns every column for every model, about five times the
main answer (tens of thousands of values instead of a few hundred). They
stay separate sources until that trade is wanted.

## Requests to the foundation

Edition agents add needs here instead of editing shared files.

Notes from the foundation:

- Reading a file outside the bundle needs `io.open(path, "r+b")`: ORCA's
  `io.open` wrapper serves plain `"r"`/`"rb"` reads from its package file
  system, which may not see the file. `Storage` already does this.
- F9's "when there is room" is a character budget (34, the app name and the
  place), not a measured width.
- The region for default units comes from `core.getPreferredLanguage()`;
  when the app bundle declares localizations without a region it can be
  only "en", which falls back to metric. Wave 4 may want the device region
  (`NSLocale.current.region`) exposed to Lua. Done in cdb615db:
  `core.getRegion()` is the device region, and `core.getMeasurementSystem()`
  and `core.uses24HourClock()` follow the phone's own settings
  (`Units.device()`).

Notes from wave 3 (places):

- Postcodes are matched worldwide: "10115" finds New York before Berlin.
  Open-Meteo takes `countryCode=`; passing the device region (wave 4's
  `NSLocale.current.region`, see above) would fix it. Done in cdb615db: a
  query with a digit asks for the device's country only, and a name lists
  that country's places first (`Search.prefer`).
- The geocoder cannot tell a seaside town from an inland one, and has no
  shore direction: a found coast asks the user for its facing. A reverse
  lookup of the nearest coastline would need another data source.
- Renaming a place (hiking S65) and a context menu on place rows (Windows
  Phone's long press) are not built; the editor's links do the rest.
