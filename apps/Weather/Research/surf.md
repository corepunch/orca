# Surfers' Weather — research

Surfers pay for Surfline mostly because of its cams. They complain about its price, ads, metered free checks, cams that go down and forecasts that miss. Since Magicseaweed closed in 2023 there has been no simple, free, no-account forecast with a clear rating.
Surfers' Weather can win on that gap: an honest plain-words verdict for *your* break, with tides, a multi-swell breakdown, alerts and a widget, and no ads or metering. It cannot compete on cams.
The biggest missing pieces in our code are tides, finding or adding your own break, a per-spot profile, imperial units, alerts and widgets, and keeping the last forecast for offline use. The Open-Meteo **commercial licence** must be settled before we charge money.

Researched November 2026. Prices are US App Store prices unless noted, and they change often. Reddit itself could not be fetched (the site blocks automated clients), so Reddit sentiment comes through secondary sources and is marked as such.

## Competitors

| App | What it gives you | Price / paywall | Rating (App Store) | Stands out for |
| --- | --- | --- | --- | --- |
| **Surfline** (incl. former Magicseaweed) | Spot forecasts with human forecasters, its own nearshore model, 1,000+ cams, buoys, tides, alerts, session tracking, Apple Watch | Free tier with ads. Premium $15.99/mo or $119.99/yr, Premium with Ads $69.99/yr, Premium+ $149.99/yr ([App Store][sl-app]; [BreakFinder, Oct 2026][bf-worth]) | 4.7 from about 31K ratings ([App Store][sl-app]) | Cams and cam rewind; Premium+ adds a wave timeline, crowd insights and "Smart Cam" in the US, AU and UK ([App Store][sl-app]) |
| **Surf-Forecast.com** (Meteo365) | 7,000+ breaks, multi-swell display, tide times, Wavefinder map, links to webcams | Free; Premium $4.99/mo or $34.99/yr for hourly and 16-day forecasts, custom alerts and no ads ([App Store][sf-app]) | 4.9 from about 1K ratings ([App Store][sf-app]) | Coverage of obscure breaks; spot "consistency" guides showing how often a break is clean, blown out or small ([Surfd][surfd]); star rating that fades as onshore wind grows ([FAQ][sf-faq]) |
| **Windy.app** | 30,000+ spots, wind and wave forecasts, model comparison, wave alerts, offline favourites, nearby stations | Pro from about $9.99/mo to $59.99/yr; many price points listed ([App Store][windyapp]) | 4.7 from about 73K ratings ([App Store][windyapp]) | Custom profiles per sport; wave alerts on height, direction and period; offline mode |
| **Windy.com** | Animated global wind, swell and pressure maps from several models | Free; Premium about $25–35/yr ([LazySurfer][ls-best]) | not checked | The best free map for watching a swell come in; not break-specific ([Surfd][surfd]) |
| **Windguru** | Multi-model hour-by-hour tables of wind, gusts, wave height, period and direction | Free; Pro about $21/yr ([Surfd][surfd]); iPhone use is mostly a web app ([AlternativeTo][wg-alt]) | — (no official iOS app found) | Wind detail, popular in Europe; dated, dense interface ([LazySurfer][ls-best]) |
| **Swellnet** (Australia) | Cams, replays, daily reports, forecaster notes, local surfer "obs" at dawn and in the afternoon | Free 5-day forecast; Pro (about A$8.95/mo) for ad-free cams and 16 days; 24-hour and 7-day passes ([Surfd][surfd]; [App Store][swn-app]) | About 4.3–4.6, sources disagree ([AppGoblin][swn-goblin]) | Human local reports; short passes instead of a yearly lock-in |
| **Dawn Patrol** | Apple Watch session tracker (waves, speed, maps); forecasts on the watch and in widgets | Free tracking; Soul Surfer $3.49/mo or $27.49/yr adds forecasts, widgets and analytics ([App Store][dp-app]) | 4.6 from about 4.2K ratings ([App Store][dp-app]) | Watch-first; sessions shown against tide and wave conditions; complaints about missed waves |
| **Swell Intel** | Nearshore SWAN model for breaking height at the break, 16 days, published accuracy record | Free; Premium $2.99/mo ([Swell Intel][si]) | not checked | Publishes how accurate it has been (self-reported) |
| **LazySurfer** | Personal "predicted rating" learned from the sessions you log; NOAA buoys and NWS wind; offline cache | Free; Pro (price not found) ([LazySurfer][ls-best]) | not checked | Learns what *you* call good at *your* break; mostly US |
| **BreakFinder** | Free 7-day forecasts, a 0–100 score instead of stars | Free; Premium €4.99/mo ([AlternativeTo][sl-alt]) | not checked | Built to replace Magicseaweed ([BreakFinder][bf-msw]) |
| **Glassy Surf Report** | Free 7-day swell and wind, spot alerts, session log tied to the Glassy Zone wearable | Free | The listing looks out of date (iOS 10, v3.0.x) ([App Store][glassy]) | Probably abandoned |
| **Swell Info** | US, Mexico, Central America and Caribbean forecasts and reports | Free, with ads ([Surfd][surfd]) | not checked | Easy for novices; intrusive ads |
| **Magicseaweed (MSW)** | — | Closed in 2023; the site now redirects to Surfline ([Stab][stab-msw]) | The MSW app stayed at 4.8 until it was retired ([Unstar][unstar]) | Was free, needed no account, and had a 1–5 star rating anyone could read |

Notes:

- **"Deepswell" was not found** on the App Store or the web. It may not exist or may have been renamed. The newer small indie surf apps we did find are Swell Scope (drop a pin for your own break), A-Frame, Surfnerd ($7/mo), Wave Forecast (Open-Meteo and NOAA data) and Surf Watch ([App Store search results][indie]). Several of them use the same free data we do.
- Surfline's free tier is **more generous than older complaints suggest**. Its listing now advertises free 10-day forecasts, hourly ratings, its nearshore model, alerts and session tracking ([App Store][sl-app]). The surf-check meter (3 or 5 a week) people hated in 2021 seems to have been relaxed for signed-in users (developer reply, [App Store][sl-app]). Surfline's real moat is **cams**, which we cannot match.
- The general pattern is a free short forecast, with a paid long range, alerts and no ads. Cheap competitors sit at $20–35 a year, and Surfline at $70–150.

## What people complain about

### 1. Price and paywalls
- Premium costs about $120 a year and Premium+ $150. Features move to higher tiers, and some cams that used to be free now need a subscription ([Trustpilot][tp]; [BreakFinder][bf-worth]; [JustUseApp][jua]).
- Free use is metered with "surf checks" (3 to 5 a week, per reviews), and the free tier feels like a tease ([App Store][sl-app]; [splitmetrics][split]; [Mike Glezos critique][mg]).
- Surf-Forecast's app is called a bait and switch, because the website is free but the app charges for the next week ([JustUseApp][jua-sf]).
- Trial-to-paid charges with refunds refused (Windy.app, a single anecdote) ([AppTail][apptail]).

### 2. Ads, including for people who pay
- Long video ads before short cam clips, and ads on every cam refresh ([Google Play reviews via search][gp-sl]; [Mike Glezos][mg]).
- Ads still appear for some paying users ([Trustpilot][tp]).
- Swell Info's ads are described as intrusive ([Surfd][surfd]).

### 3. Forecasts that miss or feel generic
- Wave heights are wrong, generic across a whole region, or change during the day; some say this got worse after the MSW merger ([Trustpilot][tp]; [JustUseApp][jua]).
- An example: a hurricane swell on Long Island was under-called at 2–3 ft when it was about 5–6 ([Epic Surf review, App Store][epic]).
- Star ratings are a crowd average and do not say whether a spot will work *for you* ([LazySurfer][ls-best]; [LazySurfer beginners][ls-beg]).
- Windy and Windguru give offshore numbers, so you have to work out the break yourself ([Swell Intel][si]).

### 4. Cams down, removed or poor
- Cams stay offline for weeks, get removed, or are grainy or badly angled ([Trustpilot][tp]; [JustUseApp][jua]).

### 5. Clutter and the learning curve
- There is too much data and every section blends into the next. Beginners get no help reading swell or period ([Mike Glezos][mg]; [LazySurfer beginners][ls-beg]).
- Windguru and Surf-Forecast look dated and dense, with small text ([LazySurfer][ls-best]; [Surfd][surfd]).
- Stock photos of perfect waves sit behind poor conditions and mislead ([Mike Glezos][mg]).

### 6. Onboarding and finding your spot
- The home screen is empty after you grant location, and adding a spot means drilling through continent, country, state, city and beach ([Mike Glezos][mg]).
- Signup is pushed before you get any value ([Mike Glezos][mg]).
- Swellnet's first search is not intuitive ([Swellnet beta thread][swn-beta]).

### 7. Stability, speed and devices
- Crashes when replaying cams, slow loading, freezes and lost session data ([JustUseApp][jua]; [Trustpilot][tp]).
- Weak iPad support: limited landscape and missing tools ([JustUseApp][jua]).
- Surfline's app overrides background audio ([Surf-Forecast reviews][sf-app]).
- Watch trackers miss waves ([App Store][dp-app]).

### 8. Loss of free tools and consolidation
- When MSW closed, people lost a free, no-account forecast with stars, and their saved spots were not exported ([BreakFinder][bf-msw]; [Stab][stab-msw]).
- People feel Surfline is now nearly a monopoly ([Trustpilot][tp]).
- Forum users missed MSW's per-swell breakdown (Seabreeze forum, found via search; the page could not be loaded) and r/surfing's "MSW RIP" thread ([search result][rd-msw]; not read directly).

### 9. Crowds
- Surfers blame live cams and surf reports for crowding breaks ([Wikipedia: Surfline][wiki-sl]). This is a reason to **not** promote "secret" spots or crowd-driving features.

## Persona

### Primary: Marta, the before-work regular

- **Who:** 31, product designer in Lisbon. She has surfed for six years, at an intermediate to advanced level. Her home breaks are Ericeira (Ribamar, Foz do Lizandro) and Costa da Caparica, 35–45 minutes away by car. Her quiver is a 6'0" shortboard and a 7'6" mid-length.
- **Context:** She surfs two or three mornings a week before a 9:30 stand-up, plus longer weekend sessions. She takes one surf trip a year (Morocco, Canaries, Bali).
- **Goals:**
  - Decide the night before whether to set a 6:00 alarm.
  - Pick the right break for the tide and wind.
  - Not waste a 90-minute round trip on a blown-out or flat sea.
  - Know which board and which wetsuit to bring.
- **Frustrations:**
  - She pays for Surfline mainly for cams, but the cams she needs are often down.
  - She finds the ads and upsell noise annoying.
  - The forecast says "2–3 ft fair" for the whole coast, while she knows Ribamar needs a NW swell and low-to-mid tide.
  - Tide and swell live on different screens.
  - Units are mixed (feet in the app, metres from Portuguese surfers).
- **Devices:** an iPhone 15, an Apple Watch SE and CarPlay in the car. She uses widgets heavily and has a Lock Screen widget for her home break.
- **How she uses it:**
  - 21:30 in bed: a 30-second check of tomorrow's dawn window.
  - 06:00: a glance at the widget or watch to confirm wind and tide.
  - Sunday: plan the week.
  - Monthly: a deeper look at swells 7–14 days out for a weekend trip.

### Secondary: Jonas, the travelling improver

- **Who:** 26, from Munich, a beginner-intermediate on a foamie or longboard. He surfs only on one or two trips a year (Hossegor, Tarifa) and in the Eisbach wave once.
- **Needs:**
  - "Is it safe and fun *for me* today?" in plain words.
  - What period and offshore mean.
  - When the beach is too big for him.
  - Rip and lifeguard awareness.
  - Wetsuit advice.
  - Works with poor roaming data.
- **Frustrations:** dense tables, the paywall during a one-week trip, and no idea which beach in town suits his level.

## User stories

Priority: **P0** must, **P1** should, **P2** could. Coverage: **have** means the current code does it, **partial** means it does some of it, **missing** means it does not.
"Marta" is the primary persona and "Jonas" the secondary.

### A. The verdict: should I go?

1. As Marta, I want one clear verdict for my break right now so that I decide in seconds. [P0] [have]
2. As Marta, I want the reason behind the verdict in plain words ("offshore wind grooms the faces") so that I trust it. [P0] [have]
3. As Marta, I want today's best window ("best 07–09h") so that I know when to be in the water. [P0] [have]
4. As Marta, I want tomorrow's dawn window shown in the evening so that I can set my alarm. [P0] [partial — the window switches to tomorrow only once today's has passed]
5. As Marta, I want an hour-by-hour colour band for the day so that I see when it turns on and off. [P0] [have]
6. As Marta, I want the verdict to account for the tide at my break so that "good" means good at the time I can surf. [P0] [missing]
7. As Jonas, I want the verdict adjusted to my skill level so that "good" never means overhead and dumping for me. [P0] [missing]
8. As Marta, I want a finer scale than three steps (e.g. 0–10 or five words) so that I can tell "fair" from "nearly good". [P1] [partial — the score exists, only three tones are shown]
9. As Marta, I want the verdict to say what would make it better ("cleans up when the wind turns at 10h") so that I can plan. [P2] [missing]
10. As Marta, I want a one-line daily summary ("Small and clean early, onshore by noon") so that I get the gist without numbers. [P1] [missing]
11. As Jonas, I want the verdict to tell me which board suits the hour (longboard, mid-length or shortboard) so that I bring the right one. [P2] [partial — the size reasons mention longboards]
12. As Marta, I want the score to stay steady between refreshes unless the forecast truly changed so that I don't feel whiplash. [P1] [partial]

### B. Swell and waves

13. As Marta, I want swell height, period and direction so that I can judge the swell myself. [P0] [have]
14. As Marta, I want every swell train (primary, secondary, wind-swell) listed separately so that I see a long-period NW groundswell under the local chop. [P0] [missing]
15. As Marta, I want the expected breaking (face) height at my break, not only open-ocean height, so that the number matches what I see. [P0] [missing]
16. As Marta, I want heights in feet or metres, my choice, so that they match how my friends talk. [P0] [missing]
17. As Jonas, I want body-relative words (knee, waist, head high, overhead) so that I understand the size. [P0] [partial — only in the reasons]
18. As Marta, I want peak period as well as mean period so that I can spot powerful groundswell. [P1] [missing]
19. As Marta, I want a swell chart for the next days so that I see the swell build and fade. [P0] [partial — a 24-hour column chart only]
20. As Marta, I want to know if the swell direction suits my break's window (e.g. W–NW) so that I don't drive to a beach the swell misses. [P0] [partial — only a rough "heading away" penalty]
21. As Marta, I want swell energy or power shown so that I can compare a 1 m @ 16 s swell with a 2 m @ 7 s one. [P2] [missing]
22. As Marta, I want a note when a new swell arrives ("new NW groundswell fills in Thursday afternoon") so that I catch the first day. [P1] [missing]
23. As Jonas, I want a short explainer of period, swell and wind-swell so that I learn to read forecasts. [P1] [missing]
24. As Marta, I want the combined sea height (waves) beside the swell so that I see how messy it is. [P1] [have]

### C. Wind

25. As Marta, I want wind as offshore, cross-shore or onshore for my beach so that I know if it is clean. [P0] [have]
26. As Marta, I want wind speed in knots, km/h or mph so that it matches my habit. [P0] [missing — m/s only]
27. As Marta, I want gusts beside the average wind so that I know how choppy it gets. [P1] [have]
28. As Marta, I want the morning glassy spell and the time the sea breeze kicks in so that I go before it. [P0] [partial — the hourly verdict shows it, nothing calls it out]
29. As Marta, I want a wind direction arrow drawn relative to the beach so that I read it at a glance. [P1] [missing]
30. As Marta, I want "light onshore" treated more gently than strong onshore so that small clean-ish days are not rated poor. [P1] [have]
31. As Marta, I want wind at the coast, not inland, so that the forecast matches the beach. [P1] [partial — the place point is used; the grid cell may fall on land]

### D. Tides

32. As Marta, I want today's high and low tide times and heights so that I plan around them. [P0] [missing]
33. As Marta, I want a tide curve under the hourly band so that I see swell, wind and tide together. [P0] [missing]
34. As Marta, I want to set my break's best tide (low, mid, high, pushing, dropping) so that the verdict uses it. [P0] [missing]
35. As Marta, I want a warning when the tide fills in and the break shuts down so that I time my session. [P1] [missing]
36. As Marta, I want spring and neap tides marked by the moon phase so that I expect big tidal swings. [P2] [partial — the moon phase is shown]
37. As Jonas, I want a warning about the strong currents a big tide range brings so that I stay safe. [P2] [missing]
38. As Marta, I want the tide on the week view so that I pick the weekend morning with the right tide. [P1] [missing]
39. As Marta, I want tides to work offline for the coming days so that I can check on the beach without signal. [P1] [missing]

### E. Spots and places

40. As Marta, I want to search for my break by name so that I can add it in seconds. [P0] [missing]
41. As Marta, I want to drop a pin for a break that is not listed so that my secret reef works too. [P0] [missing]
42. As Marta, I want to set which way my beach faces by turning a compass on the map so that offshore is correct. [P0] [missing — `facing` is hard-coded]
43. As Marta, I want my saved breaks first and in my own order so that the app opens on my home break. [P0] [missing]
44. As Jonas, I want nearby breaks from my location so that I find spots on a trip. [P0] [missing]
45. As Marta, I want to compare my three local breaks side by side for tomorrow so that I pick where to drive. [P0] [missing]
46. As Marta, I want a per-break profile (best swell window, best wind, best tide, size range, beginner-friendly) so that ratings fit each break. [P0] [missing]
47. As Jonas, I want a spot's level (beginner, intermediate, expert) and hazards (rocks, rips, reef) so that I choose safely. [P1] [missing]
48. As Marta, I want my spots and settings kept on the device and in iCloud so that a new phone does not lose them (the lesson from MSW). [P1] [missing]
49. As Marta, I want more than the seven built-in surf places so that the app is useful where I live. [P0] [missing]
50. As Marta, I want to share a spot (and its profile) with a friend so that we use the same settings. [P2] [missing]
51. As Marta, I want "best break today among my saved ones" on the hub so that the app makes the choice for me. [P1] [missing]
52. As Jonas, I want the travel time to each saved break so that I weigh a better break against the drive. [P2] [missing]

### F. Planning ahead

53. As Marta, I want each day's best window for the week so that I can plan sessions. [P0] [have]
54. As Marta, I want up to 14–16 days of swell outlook so that I can plan a weekend trip. [P1] [missing — 7 days]
55. As Marta, I want a confidence level that drops further out so that I don't trust day 12 like day 1. [P1] [missing]
56. As Marta, I want to see where the models disagree so that I know when the forecast is shaky. [P2] [missing]
57. As Marta, I want the week view to show the best hour and its tide and wind at a glance so that I don't open each day. [P1] [partial]
58. As Marta, I want to add a session to my calendar from a window so that I block the time. [P2] [missing]
59. As Jonas, I want a trip planner for a date range at one place so that I can prepare. [P2] [missing]
60. As Marta, I want "what changed since yesterday's forecast" so that I see upgrades and downgrades. [P2] [missing]

### G. Alerts, widgets, watch

61. As Marta, I want a notification when my break will be good tomorrow morning so that I don't need to check. [P0] [missing]
62. As Marta, I want to set my own alert rules (swell ≥ 1 m, period ≥ 10 s, offshore) so that alerts match my taste. [P1] [missing]
63. As Marta, I want at most one alert a day and quiet hours so that the app never nags. [P0] [missing]
64. As Marta, I want a home-screen widget with my break's verdict and best window so that I see it without opening the app. [P0] [missing]
65. As Marta, I want a Lock Screen widget so that I see it on wake-up. [P1] [missing]
66. As Marta, I want an Apple Watch complication so that I check at dawn without my phone. [P2] [missing]
67. As Marta, I want a Siri or Shortcuts answer ("How's Ribamar?") so that I ask hands-free while driving. [P2] [missing]
68. As Marta, I want a CarPlay or glanceable summary so that I can check while parked. [P2] [missing]
69. As Marta, I want alerts for a fresh long-period swell days ahead so that I can plan time off. [P1] [missing]
70. As Marta, I want a "conditions just turned good" alert during the day so that I catch a quick evening session. [P2] [missing]

### H. Safety and comfort

71. As Marta, I want lightning to rule out the hour so that I never paddle out into a storm. [P0] [have]
72. As Marta, I want darkness to rule out the hour, with first light shown, so that dawn patrol starts on time. [P0] [partial — sunrise is shown, first light is not]
73. As Marta, I want the water temperature and a wetsuit suggestion so that I pack the right suit. [P0] [have]
74. As Marta, I want air temperature and wind chill for after the session so that I bring a poncho and warm clothes. [P1] [partial — on the hub only]
75. As Marta, I want a UV warning for midday sessions so that I use sunscreen. [P1] [partial — on the hub only]
76. As Marta, I want a water-quality warning after heavy rain (runoff) so that I avoid getting sick. [P1] [missing]
77. As Jonas, I want a "too big for you" warning tied to my level so that I don't get into trouble. [P0] [missing]
78. As Jonas, I want rip-current advice and a reminder to surf near lifeguards on big or tidal days so that I stay safe. [P1] [missing]
79. As Marta, I want the hazards of strong offshore wind (hard to paddle, being blown out to sea) called out so that I judge risk. [P1] [have]
80. As Marta, I want fog and low-visibility warnings so that I know when it is unsafe to surf alone. [P2] [missing — visibility is fetched, surf does not use it]

### I. Personal: sessions and learning

81. As Marta, I want to log a session with a one-tap rating so that the app learns what I call good. [P1] [missing]
82. As Marta, I want my verdict tuned from my logs at each break so that ratings become personal. [P2] [missing]
83. As Marta, I want to see past sessions next to the conditions at the time so that I learn my break. [P1] [missing]
84. As Marta, I want "days like this one in my log" so that I recall how similar days went. [P2] [missing]
85. As Marta, I want my quiver (boards) saved so that suggestions use my real boards. [P2] [missing]
86. As Marta, I want Apple Health or Dawn Patrol sessions imported so that I don't log twice. [P2] [missing]
87. As Jonas, I want tips per verdict ("small, clean: great longboard day") so that I improve my reading. [P1] [partial]
88. As Marta, I want my settings (units, level, alert times) asked once in a short setup so that the app fits me from the first day. [P1] [missing]
89. As Marta, I want my notes on a spot ("best on the push, 2 h before high") kept with it so that I remember. [P2] [missing]
90. As Marta, I want yesterday's conditions (what actually happened) so that I compare them with the forecast. [P2] [missing]

### J. Trust, price and quality

91. As Marta, I want no ads, ever, so that the app feels calm and fast. [P0] [have]
92. As Marta, I want no account and no metered checks so that I can open it as often as I like. [P0] [have]
93. As Marta, I want a fair one-time or low yearly price for extras so that I don't feel trapped. [P1] [missing — no pricing yet]
94. As Marta, I want the last forecast shown offline with "updated 2 h ago" so that it works at the beach car park. [P0] [missing — in-memory 10-minute cache only]
95. As Marta, I want the app to open to my break's verdict in under a second so that it beats the cam apps. [P0] [partial — waits for the network on cold start]
96. As Marta, I want to see where the data comes from (Open-Meteo, the model names) so that I can judge it. [P1] [missing — and attribution is required by the licence]
97. As Marta, I want real conditions, not stock photos of perfect waves, so that nothing misleads me. [P0] [have — no photos]
98. As Marta, I want the app to leave my music playing so that it doesn't interrupt the drive. [P1] [have, needs checking on a device]
99. As Marta, I want it to work well on iPad in landscape so that I can plan trips on the big screen. [P2] [partial — iPad builds exist; the layout is not checked]
100. As Marta, I want an honest note when the swell data is missing or the spot sits too close to the shore for the wave model so that I am not misled. [P0] [partial — "No swell data" caps the score]

## Feature gaps

Ordered by value to the persona against cost. Sizes: **S** is under a day, **M** a few days, **L** a week or more, or engine work.

1. **Tides: times, a curve, and per-break tide preference**
   - **Why:** stories 6, 32–39. Every serious competitor has tides ([Surf-Forecast][sf-app], [Surfline][sl-app]), and they are the most-missed fact for a break.
   - **Data:**
     - Open-Meteo Marine has `sea_level_height_msl` (which includes tides) hourly, and 15-minutely in Europe and North America. It runs at about 8 km and is "not suitable for coastal navigation" ([Marine API][om-marine]). That is fine for the shape and the high/low *times*, but heights are rough.
     - Exact heights would need NOAA CO-OPS (free, US only) or a paid global API: Stormglass (10 free requests a day), TideCheck (50 free a day, Pro $29/mo) or WorldTides (price not checked) ([search][tides]).
     - Start with Open-Meteo, find highs and lows from the curve, and label them "approximate".
   - **Size:** M.
   - **Files:** `forecast.lua` (MARINE fields, `hour.tide`, finding highs and lows), `activities/surf.lua` (tide rule, metrics, a third chart), `places.lua` (`tide = "low-mid"`), `report.lua`, `Pages/Activity.xml`, `Templates/MetricRow.xml` or a new tide template, `Styles/weather.css`, `tests/test_weather.lua`.

2. **Find, add and save your own breaks (search, pin, facing, location)**
   - **Why:** stories 40–44, 48–49, plus the onboarding complaints ([Mike Glezos][mg]). Seven hard-coded places cannot ship.
   - **Data:**
     - The Open-Meteo Geocoding API is free, but it finds towns, not breaks. That is fine with "drop a pin and turn the beach compass".
     - We also need a curated starter list of well-known breaks with `facing`. That is our own data, so check licensing before copying any spot database.
     - Device location needs the engine's location API (README lists it as not built).
   - **Size:** L.
   - **Files:** `places.lua` (move to stored places), `provider.lua`, `App.lua` (routes), `Data/Hub.xml`, `Data/Routes.xml`, a new add-place page and templates, `Templates/PlaceRow.xml`, persistence in the engine or the app.

3. **Per-break profile that drives the rating**
   - **What:** a swell direction window, best wind, tide, a min/max size, and the break's level.
   - **Why:** stories 7, 20, 34, 46–47, 77; the "generic regional rating" complaint ([Trustpilot][tp]; [LazySurfer][ls-best]).
   - **Data:** none new; it is our own fields.
   - **Size:** M.
   - **Files:** `places.lua` (`swellWindow = {250, 330}`, `tide`, `level`, `size = {0.6, 2.5}`), `activities/surf.lua` (replace the `> 100°` rule), `score.lua` if new caps are needed, tests.

4. **Multi-swell breakdown and peak period**
   - **Why:** stories 14, 18, 22, 24. MSW's per-swell view is what users miss most (forum, via search), and Surf-Forecast's multi-swell display is praised ([App Store][sf-app]).
   - **Data:**
     - Open-Meteo Marine: `secondary_swell_wave_*` (some models), `tertiary_swell_wave_*` (GFS only), `wind_wave_height/period/direction` and `swell_wave_peak_period` ([Marine API][om-marine]). All free.
     - Uncertain: how often the secondary swell is filled for the default "best match" model. Test it per region.
   - **Size:** S–M.
   - **Files:** `forecast.lua` (MARINE string, parse), `activities/surf.lua` (`metrics`, rate the dominant train by energy), `README.md` (hour fields).

5. **Units: ft, knots, mph and °F**
   - **Why:** stories 16, 26. The US, UK and Australian markets talk in feet and knots.
   - **Data:** Open-Meteo supports `wind_speed_unit=kn|mph|kmh`, `temperature_unit=fahrenheit` and marine `length_unit=imperial` ([Forecast API][om-docs]; [Marine API][om-marine]).
   - **Approach:** keep the fetch metric and convert in `Score.fmt`. The rules stay in metric.
   - **Size:** S–M.
   - **Files:** `score.lua` (`fmt`), `report.lua`, `activities/surf.lua` (notes), a settings page, persistence.

6. **Breaking-height estimate and body-relative size words**
   - **Why:** stories 15, 17; "the forecast number doesn't match what I see" ([Swell Intel][si]).
   - **Data:**
     - Derived from swell height, period and the break's exposure, using a simple empirical formula (for example, a factor of roughly 1.2–1.6 on swell height for long periods). The formula needs validation, so label the result "estimate".
     - True nearshore models (SWAN, as Swell Intel and Surfline use) are not available from Open-Meteo.
   - **Size:** S (heuristic) or L (a real model, out of scope).
   - **Files:** `activities/surf.lua`, `places.lua` (`exposure` factor), tests.

7. **Offline and instant start (keep the last forecast on disk)**
   - **What:** keep the last forecast, show "updated 2 h ago", then refresh in the background.
   - **Why:** stories 39, 94, 95; offline mode is a selling point of Windy.app and LazySurfer ([App Store][windyapp]; [LazySurfer][ls-best]).
   - **Data:** none new.
   - **Size:** M.
   - **Files:** `provider.lua` (persistent cache), `forecast.lua` (serialise), a hub "updated" line in `report.lua` and `Templates/HubNow.xml`; engine file storage, if one is exposed to Lua.

8. **Alerts: "tomorrow at dawn will be good"**
   - **What:** a single daily summary alert plus custom rules, with quiet hours.
   - **Why:** stories 61–63, 69–70. Surfline, Surf-Forecast and Windy.app all sell alerts ([App Store][sl-app], [App Store][sf-app], [App Store][windyapp]).
   - **Data:** the same forecast.
   - **Engine:** needs iOS local notifications and background app refresh in ORCA. Push from a server would need a backend, which we avoid. On iOS, background fetch timing is not guaranteed, so schedule an evening check plus a local notification, and treat delivery as best effort.
   - **Size:** L, mostly engine work.
   - **Files:** the engine platform layer, `App.lua`, a settings page.

9. **Home-screen and Lock Screen widget (later a watch complication)**
   - **Why:** stories 64–66. Live tiles were Amazing Weather's best feature (DESIGN.md), and Dawn Patrol charges for widgets ([App Store][dp-app]).
   - **Data:** the same forecast.
   - **Engine:** WidgetKit needs a native extension. ORCA has none today (README: "live tiles" not built).
   - **Size:** L.
   - **Files:** engine and iOS bundler (`make iphone`), with a small JSON snapshot written by `provider.lua`.

10. **Compare my breaks / best break today**
    - **What:** one row per saved break, with tomorrow's best window and verdict.
    - **Why:** stories 45, 51. Surfline advertises "spot comparisons" ([App Store][sl-app]).
    - **Data:** one forecast per break, roughly doubling calls. The free tier allows 10,000 a day for the whole app *only for non-commercial use* (see 13).
    - **Size:** M.
    - **Files:** `provider.lua`, `report.lua` (a new row kind), `Pages/Hub.xml` (the "good for" section becomes "my breaks" in the surf edition), `Templates/GoodForRow.xml`.

11. **First light and dawn-patrol card**
    - **What:** civil twilight, plus "tomorrow 06:40 first light, glassy, mid tide rising".
    - **Why:** stories 4, 28, 72.
    - **Data:** compute civil twilight from the sun's position locally; it is pure maths, with no API needed.
    - **Size:** S.
    - **Files:** `sky.lua`, `activities/surf.lua` (`focus`, `window`), `report.lua`.

12. **Longer outlook with confidence (up to 16 days)**
    - **Why:** stories 54–56. 16 days is a paid feature everywhere else ([Surfd][surfd]; [App Store][sf-app]).
    - **Data:**
      - The weather API allows `forecast_days` up to 16 ([Forecast API][om-docs]).
      - The Marine API page says `forecast_days` accepts 0–8, while ECMWF WAM runs 15 days and GFS Wave 16 ([Marine API][om-marine]). Whether the Marine endpoint can serve more than 8 days is **uncertain**; check it.
      - Confidence: spread between ECMWF WAM and GFS Wave, or the Ensemble API for wind ([Forecast API][om-docs]).
    - **Size:** M.
    - **Files:** `forecast.lua` (DAYS, models), `report.lua` (the week view), `Pages/Activity.xml`.

13. **Open-Meteo commercial licence and attribution (business blocker)**
    - **Why:** the free API "is for non-commercial use". Commercial use needs a paid plan and an API key with the `customer-` URL prefix. Every user must credit the model providers and Open-Meteo ([Pricing][om-price]; [Marine API][om-marine]). Any App Store app that charges money or carries ads is very likely commercial, so confirm with info@open-meteo.com.
    - **Size:** S of code (API key, URL prefix, an "about the data" screen) plus a monthly cost.
    - **Files:** `forecast.lua` (URLs), a new about view (story 96).

14. **Water-quality warning after rain**
    - **What:** "more than 10 mm in the last 72 h near a river mouth: runoff risk".
    - **Why:** story 76. It is a common public-health guideline in places like Southern California (the 72-hour rule). **Check the threshold** per region before shipping.
    - **Data:** Open-Meteo `past_days` precipitation ([Forecast API][om-docs]). Free.
    - **Size:** S.
    - **Files:** `forecast.lua` (`past_days=3`), `activities/surf.lua`, `places.lua` (a `runoff` flag).

15. **Skill level and a short setup**
    - **What:** level, units, home break and alert time, asked once.
    - **Why:** stories 7, 77, 88; the beginner complaints ([LazySurfer beginners][ls-beg]).
    - **Size:** M.
    - **Files:** a new setup page, `activities/surf.lua` (size caps by level), settings persistence.

16. **Session log with a one-tap rating, then personal tuning**
    - **Why:** stories 81–84, 89. This is LazySurfer's whole pitch ([LazySurfer][ls-best]).
    - **Data:** local storage only; HealthKit import later.
    - **Size:** M (log), then L (tuning).
    - **Files:** new pages, persistence, and `score.lua` hooks.

17. **Beginner explainers and level-aware tips**
    - **Why:** stories 23, 87; "no help reading charts" ([Mike Glezos][mg]).
    - **Size:** S.
    - **Files:** `activities/surf.lua` (the reason text), a small "how to read" page.

18. **Wind drawn against the beach, plus coastal grid-cell checks**
    - **Why:** stories 29, 31, 100.
    - **Data:** the marine `cell_selection=sea` option exists ([Marine API][om-marine]). The weather API takes the same parameter; check whether `sea` or `nearest` gives better coastal wind.
    - **Size:** S.
    - **Files:** `forecast.lua` (`query`), `Templates/MetricRow.xml` or a new arrow glyph, `sky.lua`.

Not recommended: live cams (expensive hardware and hosting, and the crowding concerns), community reports at launch (needs moderation and a backend), and "secret spot" sharing.

## What we must avoid

- **Metering or a crippled free tier.** Never count "surf checks" or lock tomorrow's forecast. If we charge, sell extras (alerts, widgets, 16 days, sessions) at a fair, honest price, or one time, and say clearly what is free ([App Store][sl-app]; [Mike Glezos][mg]; [JustUseApp][jua-sf]).
- **Ads, especially video ads and ads for paying users** ([Trustpilot][tp]; [Mike Glezos][mg]).
- **Dark-pattern trials.** Trials that convert silently, prices that are hard to find, refunds refused ([AppTail][apptail]; [BreakFinder][bf-worth]).
- **Forced signup before any value.** No account is needed to see a forecast ([BreakFinder][bf-msw]; [Mike Glezos][mg]).
- **Data dumps without a verdict, or a verdict without reasons.** Keep the verdict, its reason and the numbers in that order ([LazySurfer beginners][ls-beg]).
- **One regional rating for all breaks.** Ratings must use the break's facing, swell window and tide ([Trustpilot][tp]).
- **Overconfident numbers.** Say "about" and "estimate" for derived wave faces and tides from coarse models; show confidence far out; never present Open-Meteo tides as navigation data ([Marine API][om-marine]).
- **Misleading imagery.** No stock photos of perfect waves ([Mike Glezos][mg]).
- **Nagging notifications.** At most one summary a day by default, with quiet hours.
- **Locking in user data.** Let people export their spots and sessions; MSW users lost theirs ([BreakFinder][bf-msw]).
- **Heavy, slow, crashy builds.** Surfline's app is 217 MB ([App Store][sl-app]). Open fast, from the cache.
- **Interrupting audio** ([App Store][sf-app]).
- **Crowd-driving features.** No "secret spot" leaderboards or "crowd now" for small breaks ([Wikipedia][wiki-sl]).
- **Fear.** Safety is told calmly and concretely ("Lightning: get out of the water."), never with alarm.

[sl-app]: https://apps.apple.com/us/app/surfline-wave-surf-reports/id393782096
[sf-app]: https://apps.apple.com/us/app/id6450721636
[sf-faq]: https://www.surf-forecast.com/pages/faq
[windyapp]: https://apps.apple.com/us/app/windyapp/id997079492
[dp-app]: https://apps.apple.com/app/id1161014179
[glassy]: https://apps.apple.com/us/app/glassy-surf-report/id569457357
[swn-app]: https://apps.apple.com/app/id1066432845
[swn-goblin]: https://appgoblin.info/apps/1066432845
[swn-beta]: https://www.swellnet.com/comment/841490
[indie]: https://apps.apple.com/us/app/-/id6747609653
[bf-worth]: https://breakfinder.surf/en/blog/is-surfline-worth-it
[bf-msw]: https://breakfinder.surf/en/blog/magicseaweed-shutdown-where-surfers-went
[ls-best]: https://lazysurfer.app/compare/best-surf-forecasting-app-2026.html
[ls-beg]: https://lazysurfer.app/compare/best-surf-app-for-beginners-2026.html
[surfd]: https://surfd.com/guides/best-surf-forecasts
[si]: https://www.swell-intel.com/compare/surf-forecast-apps
[mg]: https://www.mikeglezos.com/app-critique-surfline
[jua]: https://justuseapp.com/en/app/393782096/surfline-surfing-companion/reviews
[jua-sf]: https://justuseapp.com/en/app/6450721636/surf-forecast-com/reviews
[tp]: https://www.trustpilot.com/review/surfline.com
[split]: https://splitmetrics.com/apps/surfline-wave-surf-reports/id393782096
[gp-sl]: https://play.google.com/store/apps/details?id=com.surfline.android&hl=en_US
[epic]: https://apps.apple.com/app/id1336528409
[apptail]: https://apptail.io/app/windyapp-wind-tides-radar-oXg/greece
[unstar]: https://unstar.app/developer/surfline-wavetrak-ios-329198084?country=gb
[stab-msw]: https://stabmag.com/elsewhere/breaking-magicseaweed-is-no-longer-with-us/
[rd-msw]: https://redlib.belloworld.it/r/surfing/comments/12yfs4g/magic_seaweed_rip/jhnnyvy
[wiki-sl]: https://en.wikipedia.org/wiki/Surfline
[wg-alt]: https://alternativeto.net/software/windguru/about
[sl-alt]: https://alternativeto.net/software/surfline/?p=2
[tides]: https://tidecheck.com/developers
[om-marine]: https://open-meteo.com/en/docs/marine-weather-api
[om-docs]: https://open-meteo.com/en/docs
[om-price]: https://open-meteo.com/en/pricing
