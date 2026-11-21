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
- **week**: every day, with its best activity and window. Tap a day to open
  its hours.
- **places**: my places (the first is home; each has **edit**), **add a
  place**, then the suggestions (each has **keep**) and a **settings** link.
  Tap a place to see its hub from the start. On the first run nothing is
  saved, so the hub opens on the first suggestion.

In an edition the hub's first section opens with the edition activity's
cards ("frost tonight"): at most three, the most urgent (warnings and poor
answers first), and the rest stay on the activity's page. In the full app
everyday outdoors' cards follow the temperature as compact **answers**
(rows of Kind `answer`: official warnings, the next two hours, today in one
line, what to bring). A small line under the conditions says how old the
forecast is ("updated 25 min ago"), and says so plainly when it is the copy
saved before the network went away.

**A day** (`Pages/Day.xml`, route `/day`, `Report.day`) opens from the hub's
week: the sky, high and low, each activity's `daySummary(forecast, date)`
cards, the day's readings, what each followed activity is good for then,
and its hours. Back returns to the hub; an activity opened from a day goes
back to that day.

**An activity page** (`Pages/Activity.xml`) is a Pivot titled "fishing · lake
tahoe", with four views:

- **now**: a verdict tile with the score, the activity's cards (each may
  carry links that act, such as "log a session: good"), a link to the
  activity's own page when it has one ("log a catch"), the reasons, then the
  conditions the activity watches;
- **today**: a 24-hour verdict band (one cell per hour in its tone) and up
  to three charts the activity names in `charts` (pressure two days back and
  wind for fishing; surf, wind and the tide for surf; clouds and the moon
  from dusk to dawn for stars);
- **hourly**: the hour-by-hour list, one swipe further, as in Amazing Weather;
- **week**: each day's best window.

Tap the title line or swipe in from the left edge to go back. The page uses
the activity's accent colour. In an edition, the edition's own activity is
titled with the app's name when it fits ("surfers' weather · Bondi").

**Add a place** (`Pages/Search.xml`, route `/search`) searches a name or a
postcode and lists what each result is and where ("peak · Valais,
Switzerland"); before a search it lists the suggestions. Offline it says so
and still finds the app's own places. A result opens **the place editor**
(`Pages/Place.xml`, route `/place`): where the place is, its kind (town,
coast, lake, river, mountain), in the full app a "good for" switch per
activity, and each of its activities' own fields. Rows step like settings.
A new place has "save this place" and "save as home"; a saved one "make this
home", "move up", "move down" and "remove" (a second tap confirms; its
saved forecast goes too). Changes to a saved place are kept at once.

**Settings** (`Pages/Settings.xml`) is a Pivot built from declarations:
**units** (temperature, wind, heights, rain, pressure, distance, clock),
**activities** (each activity's own `settings`; an edition names it after its
activity), **appearance**, and **about** (the Open-Meteo attribution).
Under **appearance**, tap **background photos** to show or hide the hub's
photo. The choice is saved per app. Red night mode in Stargazers’ Weather
keeps the hub black.

Everyday outdoors' own settings: how you feel the cold (`comfort`: "I run
cold" moves "comfortable" about 3°), and in the full app the "good for"
order (`order`: as listed or best first) and one switch per other activity
(`follow_<id>`). An activity with `everywhere = true` fits every place (the
everyday lenses: run, cycle, dog walk, picnic, laundry); `followed = false`
starts it switched off. `Report.followed(activities)` keeps the ones the
user follows; the hub's "good for", its week and a day page list only those.

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
places.lua         the curated places and what each is good for (facing, lake, …): the suggestions
myplaces.lua       my places: saved per edition, home first; drafts, guesses and the editor's fields
search.lua         place search through Open-Meteo's geocoding
placepages.lua     the place rows, the search and the place editor (operations and handlers)
compare.lua        compare my places for one activity
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
Data/Day.xml       Report.day for the route's place and date
Data/Settings.xml  the settings rows, split into units, activities and about
Data/Search.xml    search results, notes and the suggestions
Data/Place.xml     the place editor's facts, fields and links
editions/<key>/    an edition's own modules, pages and data (its `page` routes)
services/          shared existing services: the official-warning registry and its providers, DWD radar
```

`services/` holds the outside services more than one app can reuse.
`services/warnings.lua` is the official-warning registry: `Warnings.coverage(spot)`
says `"covered"` (a provider is read: NWS, Environment Canada, MET Norway,
DWD via Bright Sky), `"link"` (an official site to see: the Met Office,
BoM, MeteoAlarm until its licence is confirmed, …) or `"none"`;
`Warnings.sources()` gives one optional source per provider, each asked
only where coverage picks it; `Warnings.cards(forecast, now, { note = true })`
turns `forecast.warnings` into cards and, where nothing is read, a calm
"see the Met Office" card, never "no warnings"; `Warnings.attribution()`
credits them. `services/radar.lua` is the Bright Sky radar line for
Germany. The full app's everyday outdoors uses both; an edition may add
`Warnings.sources()` to its own `sources`. The API is in the header of
`services/warnings.lua`.

A place's forecast is fetched once and cached for ten minutes. The hub,
every activity page and going back all reuse it. A pull to refresh (or the
half-hourly poll) fetches it again. Places whose activities read the sea also
fetch the marine forecast. If that request fails, the rest still shows. The
hub asks only for the lead activity's optional sources (everyday outdoors in
the full app, the edition's own otherwise); an activity page adds its own on
top of the cached forecast, once. Every request carries a generic
User-Agent with the app's name ("HikersWeather/1.0 (ORCA)") and no cookies. The
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
   (the conditions) and `charts`: two or three `{ title, unit, note, kind =
   "line" | "column", min, max, value = function(hour) }` for the "today"
   view (`note` follows the unit: "pressure hPa · 2 days back").
   `value` is metric; `unit` is a Units kind (`"wind"`, `"temp"`,
   `"percent"`…) that converts it and ends the title ("wind kn"). Write
   every number with `Units` (below), never with a unit in the string.
   `Report.pressure`, `Report.wind`, `Report.sun` and `Report.moon` are
   ready-made.
3. Optional:
   - `needsMarine` adds the sea fields to each hour: `swellHeight`,
     `swellPeriod`, `swellDir`, `waveHeight` and `seaTemp`. It may be a
     function of the place (`not place.lake and "optional"`), so a lake asks
     no marine forecast; `"optional"` lets the forecast stand without it.
   - `window(hour)` picks which hours count towards a day's best window.
   - `focus(forecast)` picks the hour the verdict card shows.
   - `nightOf(hour, forecast)` groups hours by the date their night began,
     so a night activity's "today" runs past midnight (best windows always
     follow hours by `epoch`, across midnight).
   - `chartHours(forecast)` returns the first and last hour index the
     "today" charts draw (default: now and the next 23). Longer charts tick
     every 12 hours and mark "now".
   - `dayNote(day, forecast)`: a word for the week's day rows ("high
     confidence"), in place of the plain "less certain" after a week.
   - `everywhere`, `followed`: see Settings above.
4. Register it in `activities.lua` with an accent. Add matching rules to
   `Styles/weather.css`.
5. List it on the places in `places.lua` where it makes sense, add it to
   the guesses in `myplaces.lua` (`GOOD_FOR`), and declare `placeFields`
   if it needs facts about a place (below).
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

The offscreen capture has no notch. To see a page as on an iPhone, give the
screen its insets first (`app?SafeAreaTop=47&SafeAreaBottom=34`), and scroll
a body with `ContentOffset` (negative Y scrolls down):

```bash
python3 tools/capture.py apps/Weather --wait 'Now*' \
    --shot "now=app?SafeAreaTop=47&SafeAreaBottom=34" \
    --shot "scrolled=app/**/NowBody?ContentOffset=0 -300"
```

A step that opens another page gets a new screen; set its insets again.

### Tests

`tests/weather_fixture.lua` is the shared harness. Start a file with
`local F = dofile("tests/weather_fixture.lua").init { edition = "surf" }`
(before any Weather module is required) and end it with `F.done("weather-surf")`.
It gives `F.check`, `F.NOW` (2026-11-18 06:00 UTC), `F.hour(fields)` and
`F.verdict(activity, fields, spot)` for hand-built hours, `F.says(result,
pattern)`, `F.weather(lat, extraColumns)` and `F.marine()` (a synthetic
week), `F.network()` (a fake fetch with `requests`, `failing`,
`marineFailing`, `answers[pattern]` and `options`; `F.network { placeClock =
true }` answers each place in its own clock and longitude, as
`F.weather(lat, extra, lon)` does), `F.storage(name, places, extra)` (a
scratch folder emptied of the app's and editions' files, and of
`extra(place)`), `F.launch { places, net, clock, storage, region, location }` (the app
at "/", with `find`, `has`, `text`, `press`, `back`, `open`, `cards`,
`settle`) and `F.checkEdition(key)` (the checks every edition shares).
Default units in tests are metric (region "NL").

Not built yet:

- live tiles (wave 4).

`F.storage` empties the saved places (`places`) too, so every launch starts
on the first run; `ui.choose(id)` taps a place row.

**Use my location** (`locate.lua`, N1). Where `orca.location` can ask
(an iPhone or iPad build whose package.lua declares `Capabilities = {
"location" }`), the hub's places section and the search page offer "use my
location". It looks once (`location.current()`, approximate accepted),
names the fix with the system geocoder and shows it like a search result
("your location · North Holland, Netherlands · approximate"); the editor
saves it as any found place, with its country. Denied, unsupported, no fix
or no name each give one calm line; search always stays. Tests pass
`location` (a stand-in module) to `F.launch`.

**Notifications** (`alerts.lua`, N2). Off until the user turns them on
(settings, "notifications" under the activities); turning them on asks the
system once. They are for the home place only: at most one of each kind a
day, never in quiet hours (22:00 to 07:00 by default; an alert due then
waits for their end, or is dropped when that is too late to help), and each
kind has its own switch. The kinds, each where its activity is in the app:

| Kind | Activity | Said | Rule |
| --- | --- | --- | --- |
| `rain` | outdoors | 30 min before | dry now, ≥ 60 % and ≥ 0.3 mm in the next 3 h |
| `brief` | outdoors | 07:00 | the day's high and low, rain, what to bring (off by default) |
| `frost` | garden | 17:00 | tonight ≤ 0.5 °C, or ≤ 2 °C (likely) / ≤ 4 °C (possible) clear and calm |
| `storm` | hiking | 3 h before | the first daylight hour with codes 95–99, or CAPE ≥ 1000 J/kg and ≥ 40 % (an estimate) |
| `wind` | sailing | 19:00 the evening before, or 3 h before | two good hours in a row by Sailing's own rating |
| `dawn` | surf | 20:00 | Surfers' dawn-patrol card is good for tomorrow |
| `clear` | stargazing | 2 h before | two dark hours (sun 12° down) under 20 % cloud |

Per edition: an edition keeps outdoors and its own activity, so it offers
`rain`, `brief` and its own kind (Gardeners' `frost`, Hikers' `storm`,
Sailors' `wind`, Surfers' `dawn`, Stargazers' `clear`; Fishers' has no kind
of its own yet). The full app offers all seven. The switches start with
`rain` and the edition's own kind on and the rest off; the master switch
starts off everywhere.

The settings group is `alerts`: `on` (the master switch), one toggle per
kind (`rain`, `brief`, …) and `quietFrom` / `quietTo` (whole local hours of
the home place; `from` equal to `to` means no quiet hours). Turning `on` off
cancels everything planned; changing a kind or quiet hours plans again at
once.
The plan remembers what went out in storage (`alerts`, ids
`"<kind>:<local date>"`), so a kind is said at most once a local day even
when the forecast is planned again.

Background refresh is best effort. iOS decides when (and whether) to run
it, from the person's usage and Low Power Mode; Weather asks for about every
three hours (`Alerts.REFRESH`) and never counts on it. What is planned is
scheduled as local notifications, which go out without the app running, so
an alert planned at the last launch still arrives when no refresh comes; it
is only less fresh.

```lua
Alerts.candidates(forecast, place, now)  -- { kind, title, body, at, before } per rule
Alerts.plan(candidates, { now, offset, setting, delivered })  -- what to deliver, ids "<kind>:<date>"
Alerts.sync(forecast, place, { now, storage, notifier })  -- plan and (re)schedule; storage "alerts"
Alerts.refresh { home, fetch, now, storage, notifier }    -- the background hook's work
```

An activity may add its own (`activity.alerts(forecast, place, now)`,
items with a `kind` from `Alerts.KINDS`). The app syncs whenever the home
place's fresh forecast is drawn. `background.lua` is the background refresh
(iOS `BGAppRefreshTask`, best effort): without screens it loads the home
place's forecast and syncs again. Tests pass `notifier` (a stand-in for
`orca.notifications`) to `F.launch`; `tests/test_weather_alerts.lua` checks
every rule, the plan and the hook.

**Widgets** (`widget.lua`, N3). Whenever the home place's fresh forecast is
drawn, and after a background refresh, the app writes a snapshot for the
home-screen and lock-screen widget: `{ version, app, edition, place, temp,
sky, activity, verdict, tone, line, updated }`, the lead activity's verdict
now and its best window today ("fishing best 06–08h", else the sky). It
goes through `orca.notifications.writeWidgetSnapshot` into the build's App
Group (`widget.json`); `Widget/WeatherWidget.swift` (small, medium,
rectangular and inline) reads it every half hour and never fetches. Each
edition is its own app with its own group, so each has its own widget.
Only `make ios-sim APP=weather WIDGETS=1` builds the extension;
[docs/IOS.md](../../docs/IOS.md#widgets) lists what a device build needs.
A successful write also asks for a background refresh, so the widget stays
current with notifications off. `tests/test_weather_alerts.lua` checks the
snapshot and both writers.

**VoiceOver and Dynamic Type** (W11). On iOS the engine mirrors each drawn
screen into VoiceOver elements (`Node2D.AccessibilityLabel`, `…Hint`,
`…Traits`, `…Hidden`; `source/ios_accessibility.m`). A button reads its own
and its children's text, plain text reads as itself, and a Panorama or
Pivot reads its title, then each header before its section. Weather adds
labels where text alone is not enough: a band cell says its hour and verdict
(`Spoken`, "09:00 good"), a reason its tone first (`Toned`, "poor: …"), and
"good for", day, hour, detail and strip rows read as one sentence
(`Summary`) with a hint where they open a page; charts are named after
their series; "less" and "more" hint the setting they change. A new
template whose meaning sits in a colour or an icon needs a label too.
`orca.UIKit.describeAccessibility(screen)` lists what VoiceOver would read;
`tests/test_weather.lua` checks the hub with it.

Type follows the person's text size: `App.textScale()` turns
`core.getTextScale()` (Settings, Display and Text Size) into
`page.TextScale` for every page it opens, held between 0.9 and 1.35
(`App.TEXT_SCALE`) because the Metro pages do not reflow beyond that; the
accessibility sizes stop at 1.35. A change made while the app runs reaches
the pages opened after it.

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

`Activities.request(place, focus)` merges the fields of every activity at
the place into one request (columns once each, the most past and forecast
days). Columns the parser does not name land on the hour or day under their
API name; `marine` columns need `needsMarine`. Two more fields:

- `minutely15Steps = 12` stops the quarter hours after that many steps from
  now (`forecast_minutely_15`), so a nowcast does not pull 16 days of them.
- `models = { "gfs_seamless", … }` (or a function of the place) asks those
  weather models in the main call (`models=best_match,…`). The best match
  keeps the plain names; every other model's columns land under their
  suffixed names (`hour.wind_speed_10m_gfs_seamless`). Open-Meteo returns
  every column for every model, so this multiplies the answer; a separate
  source asking a model's one or two columns is often lighter. Past hours come before
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
forecast; an optional one is recorded in `forecast.missing[id]`. A required
source is always asked; an optional one only for the lead activity (the
hub) and for the activity whose page is open, which fetches it on top of the
cached forecast. Rate an hour well without it. Answers are saved with the
forecast and replayed offline. `headers = { Accept = "…" }` (or a function
of the spot) adds request headers; every request already carries the app's
User-Agent and no cookies (`Forecast.requestOptions`). Base URLs and the API
key live in `Forecast.endpoints` (`forecast`, `marine`, `airQuality`,
`geocoding`, `flood`, `archive`, `noaaTides`, `apiKey`); build URLs with
`Forecast.url(name, query)`, which switches every Open-Meteo host to the
commercial "customer-" host and adds `apikey` when a key is set.
`format = "text"` hands `parse` the body as a string instead of decoded
JSON (NOAA NDBC's fixed-width buoy files, CSV). A host that is not
Open-Meteo may keep its base URL in the edition's own module.

**`attribution`: credit for a service.** An activity that reads another
service lists `{ { "Tides by NOAA CO-OPS", "Public domain, tidesandcurrents.noaa.gov." } }`
(or a function returning it). `Provider.about()` puts each title once on
the settings' "about", after Open-Meteo's line and before the privacy line.
The research behind each service and its licence is in
`Research/services-<key>.md`.

**`cards(forecast, place, clock, where)`: big answers.** Returns a list of
`{ id, title, value, detail, tone = "good" | "fair" | "poor" | "info", icon }`
(`icon` a path such as `Sky.icon(code)`, optional). `clock` is the time now
(it may be nil); `where` is `"hub"` or `"activity"`. They show on the
activity's "now" view between the verdict and the reasons
(`Templates/CardRow.xml`, rows `<place>:<activity>:card:<id>`), and the
edition activity's cards lead the hub's first section (rows
`<place>:card:<id>`): at most three (`Report.HUB_CARDS`), the most urgent by
`rank` (lower first) or else tone (poor, fair, good, info), shown in the
activity's own order. `hub = false` keeps a card on the activity page. In
the full app everyday outdoors leads, as answer rows.

**Card links and `act(id, context)`.** A card may carry `actions = { {
"log:good", "good" }, … }` (at most three) or one `action = { id, text }`;
each shows as a link under it. A tap calls the activity's
`act(id, { place, forecast, clock, where, card })`; a returned route path
("/settings", the activity's page) opens; otherwise the cards redraw.
Surfers' level setup and one-tap session log work this way.

**`page`: an activity's own page.** `page = { routes = "Weather/editions/
<key>/Data/Routes.xml", path = "/<key>/…", link = "log a catch", provider =
function(options) … end }`. The app routes those paths, and the activity
page shows `link`. `provider(options)` returns a data provider (operations,
filters, handlers) over the app's store; `options` gives `forecastFor(id)`,
`places`, `place()` (the place shown), `clock()` and `back()`.

**`pageClasses(forecast)` and a `stylesheet` function.** The lead activity
may return extra state classes for every page ("night"), and `stylesheet`
may be a function returning the sheet. The app dresses each new screen, and
after every drawn page (a forecast, the settings after a change) the open
ones, swapping the classes it added before. `forecast` is the one just
drawn, or nil. Stargazers' red night mode works this way.

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
is `ORCA_REGION`, else the device's region (`core.getRegion()`, the phone's
Language and Region setting, apart from the app's language), else the
preferred language's region ("en-US"), else `LC_ALL`, `LC_MEASUREMENT` or
`LANG`; with none, metric. Without `ORCA_REGION` the phone's own measurement
system and 24-hour switch (`core.getMeasurementSystem()`,
`core.uses24HourClock()`, through `Units.device()`) win over the region's
usual ones. Place search asks for the device's country only for a postcode
and otherwise lists that country's places first (`Search.prefer`).

**`Storage`: files on the device.** `Storage.load(name)` and
`Storage.save(name, table)` keep plain tables (strings, numbers, booleans,
nested tables) as Lua source in `$HOME/Library/Application Support/<app
name>/<name>.lua`, one folder per edition. Writes are atomic; reads run with
no environment and an instruction budget, so a file holds data, never code.
Names are letters, digits, `-`, `_` and `.`. `Storage.new { dir = … }` makes a
store elsewhere; the app uses the one tests pass (`Storage.use`). Already
taken: `settings`, `places`, `forecast-<place id>`.

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

## Places

**My places** (`myplaces.lua`). The saved places are kept with Storage
(`places`, one file per edition), in the user's order; the first is home.
The curated places (`places.lua`) are suggestions: a kept suggestion stores
only its id and what the user changed, so the editions' tables keyed by id
(peaks, spots, breaks, rules) still find it. A found place stores its names,
coordinates, height and country too. Its id is stable: `geo-<GeoNames id>`
from search, `at-<lat>_<lon>` from a location, with `-2`… when taken. Every
per-place file (`forecast-<id>`, logs) and the files keyed by coordinates
(`climate-…`, `stargazing-place`) work for it unchanged.

```lua
MyPlaces.mine()                -- saved places, or the suggestions before any is saved
MyPlaces.all()                 -- saved places, then the suggestions (the hub's list)
MyPlaces.find(id)              -- a saved or curated place
MyPlaces.add(draftOrId, home)  -- save a draft, or keep a suggestion by id
MyPlaces.set(id, key, value)   -- the user's own value (`place.own` lists them)
MyPlaces.move(id, delta), MyPlaces.makeHome(id), MyPlaces.remove(id)
MyPlaces.fromResult(json)      -- a draft from a geocoding result
MyPlaces.fromLocation(lat, lon, name, region, country)  -- a draft at a device fix
```

A resolved place is a plain table: the curated or found fields, the user's
values on top, `kind`, and `lake` or `river` from the kind the user chose.

**Guessed activities** (`MyPlaces.guess(kind, elevation, population)`). The
kind comes from the result's GeoNames feature code (a beach or bay is a
coast; `LK` a lake; `STM` a river; `MT`, `PK` a mountain), else from the
edition (surf and sailing: a coast; hiking: a mountain), else a town. A coast
is good for surf, sailing and fishing, a lake for fishing and sailing, a
river for fishing, a mountain for hiking, a town for the garden; from 1000 m
hiking too, and the stars except in a city over 200,000 people. The editor
shows the guess to confirm, and changing the kind guesses again. In an
edition a place is added for the edition's activity.

**`placeFields`: what an activity needs to know about a place.** The editor
lists the fields of every activity the place offers, under its name; a key
two activities declare (`facing`) shows once.

```lua
Surf.placeFields = {
    { key = "facing", title = "the shore faces", kind = "compass", needed = "not set: …" },
    { key = "breakTide", title = "best tide", kind = "choice", options = { { "all", "any tide" }, … },
        default = function(place) return Breaks.of(place).tide end },
}
```

`kind` is `"compass"` (eight points, less and more turn it), `"choice"`,
`"toggle"` or `"number"` (`min`, `max`, `step`, `start` for the first tap,
`format`). `default` is a value or a function of the place; `needed` replaces
the detail while there is no value. The value lands on the place under its
key. Today: surf (`facing`, `breakLevel`, `breakTide`), sailing (`facing`,
`safeFrom`, `safeTo`), fishing (`facing`) and hiking (`summit`,
`trailhead`). `Breaks.of`, `Spots.sector` and the hiking `Sources.peak` read
the user's values first, then their tables, then a guess from `facing`.
The provider rates a cached forecast again when an edit leaves its requests
the same, so turning a shore asks nothing.

**`compare.lua`: compare my places for one activity.**
`Compare.places(activity)` lists my places offering it,
`Compare.saved(place, epoch)` replays the forecast kept on the device (no
request), and `Compare.rank(entries, best, { tie })` rates entries with
`best(entry)` (`{ result, span, hour, later }`) and sorts them: sooner, then
higher, then the tie-break or the title; places whose forecast failed come
last. Surfers' best break (`Surf.compare`, `Surf.others`), Sailors' weekend
(`Rank.near`, `Rank.rank`) and Fishers' waters (`editions/fishing/compare.lua`)
use it. None asks more than before: surf uses saved forecasts, sailing one
multi-point request, fishing the cached forecast per water on its own page.

**Search** (`search.lua`). `Search.run(text, fetch)` asks
`Forecast.url("geocoding", "name=…&count=10&language=en&format=json")`, so a
commercial key moves it with the rest; it sends only what was typed.
`Search.known(text)` finds the app's own places (accents folded), which
offline search falls back to.

