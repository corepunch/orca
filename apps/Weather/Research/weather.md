# Weather — research

Summary:
- The market splits into free apps full of ads (The Weather Channel, AccuWeather), polished subscription apps (Carrot, Mercury, Hello Weather) and the default Apple Weather. Apple took over Dark Sky's next-hour rain, but people still say it is unreliable.
- People complain most about ads that get in the way of the forecast, paywalls on features that used to be free, accuracy (above all rain timing), location-data selling and notification noise.
- Our opening is a calm, ad-free, private app that answers "what will it be like and what should I do?" in plain words. The "good for" verdict with a best window is that answer. Today the app is missing the basics every competitor has: device location, place search, units, widgets, alerts and next-hour rain.

Scope: the full general-audience app and its everyday **outdoors** activity.
The six specialist editions are researched separately. Research date: November
2026. Prices and ratings change often and differ by storefront. Most came
from App Store listings or third-party trackers; they are marked *(unverified)*
where no first-party page confirmed them.

## Competitors

| App | Price (US, approx.) | Standout | Weak spots people name |
| --- | --- | --- | --- |
| **Apple Weather** | Free, built in | Next-hour rain (from Dark Sky) in some countries, 10-day hourly, maps, severe and rain notifications, widgets. iOS 26 adds severe alerts for places you are likely to visit. | Rain timing and "100% today but nothing hourly" mismatches; weaker outside North America; one data source. |
| **CARROT Weather** | Free app; Premium about $4.99/mo or $19.99/yr; Premium Ultra about $9.99/mo or $39.99/yr *(unverified)* | Snarky personality, a custom layout builder, a choice of data source, Watch, CarPlay, automatic Live Activities | Subscription-only since v5; tier confusion; the best alerts need Ultra |
| **The Weather Channel** | Free with ads; premium from about $4.99/mo or $29.99/yr *(unverified)* | Radar, 15-minute forecasts (premium), **activity forecasts** (running, cycling, hiking, camping, tennis/pickleball, golf, gardening) with best and worst times | The most ad-heavy; a May 2026 report of unskippable ads blocking the radar; bloat; settled a location-data lawsuit in 2020 |
| **AccuWeather** | Free with ads; Premium about $1.99/mo, Premium+ about $4.99/mo or $29.99/yr *(unverified)* | MinuteCast (minute-by-minute rain, free), Health & Activities indices on a 0–10 Poor…Excellent scale | Video ads, upsell, notifications; in 2017 it shared location with an ad firm even with location off |
| **Mercury Weather** | Free; subscription about $2.99/mo, $14.99/yr or $49.99 lifetime *(unverified)* | Beautiful charts, countdown widgets, rain and snow notifications (added 2024) | Several places is paid; little radar |
| **Hello Weather** | Free; Pro about $1.99/mo or $19.99/yr, lifetime available *(unverified)* | Clean colourful design, a choice of data provider, widgets | Watch complication moved behind the paywall, which annoyed users |
| **Weather Strip** | Free trial, then about $1.99/mo or $8.99/yr *(unverified)* | A dense one-row strip per day; lock-screen and StandBy widgets | Niche, information-dense |
| **Weawow** | Free, no ads; donations unlock extra providers | Many providers, photo backgrounds, custom widgets, no tracking | Small team; rating not verified |
| **Windy.com** | Free; Premium about $18.99–29.99/yr *(unverified, varies)* | Global model maps (ECMWF, GFS…), layers, comparing models | Overwhelming for casual users |
| **Yr (NRK / MET Norway)** | Free | Public-service, strong in the Nordics, clear meteograms | Small US presence; rating about 4.2 *(unverified)* |
| **Flowx** | Free; paid tiers under $20/yr | Model maps and a graph-first forecast | iOS availability unclear; RainViewer radar shutdown hit it |
| **Clime (NOAA Radar)** | Free with ads; subscription removes them | US radar and alerts | Ads |
| **MyRadar** | Free with in-app purchases | Fast radar, hurricane tracking | Price not verified |
| **Rain Alarm** | Free with in-app purchases; Pro $5.99 | Push warnings when rain approaches | Radar only |
| **Today Weather** | Free with in-app purchases | A choice of data source, rain and snow alarm, radar | Ads in the free tier |
| **Forecast Bar / RainViewer** | Freemium | Rain **Live Activity** on the lock screen and Dynamic Island | Live Activity is paid (Forecast Bar Gold) |

Notes:

- **After Dark Sky.** Apple bought Dark Sky in 2020. It shut the app down on 1 January 2023 and the API in March 2023. Its next-hour rain moved into Apple Weather, which offers it only in some countries; at launch these were the US, UK and Ireland. Developers got WeatherKit. Forum users still say Apple's rain timing lags Dark Sky, though others say Dark Sky was often wrong too. This is anecdotal.
  ([MacRumors][mr-ds], [9to5Mac][9to5-ds], [Apple forum][apple-rain], [MacRumors "weather app is so bad"][mr-bad])
- **Activity ratings already exist**: TWC's activity forecasts (Poor…Perfect for golf, best and worst times for running), AccuWeather's 0–10 indices and Weather Underground's Smart Forecasts. None of them is the *home* of the app; they sit below the feed. Our "good for" section can be the headline, with a reason and a best window.
  ([TWC blog, June 2026][twc-blog], [TWC App Store][twc-app])
- **Rain Live Activities** are now common in paid apps: Forecast Bar, RainViewer and Carrot ("automatic Live Activities").
  ([Forecast Bar][fb-la], [RainViewer][rv-la], [MacRumors Carrot CarPlay][mr-carrot-la])
- **Choosing the data source** (Carrot, Hello, Weawow, Today Weather) is a power-user trust feature. It answers "my app is wrong here".

Sources: [Sweet Setup][sweet], [iMore][imore], [Carrot pricing via MacRumors forum][mr-carrot], [App Pricing Lab: Carrot][apl-carrot], [Mercury App Store][mercury], [Hello App Store][hello], [Weather Strip App Store][strip], [Weawow listing][weawow], [Windy App Store][windy], [Yr App Store][yr], [Rain Alarm][rainalarm], [Today Weather][todayweather], [AccuWeather App Store][accu-app], [Flowx forum on RainViewer][flowx-rv].

## What people complain about

Category shares from one review-analysis blog (no stated method, treat as
indicative): accuracy 28%, ads 23%, subscriptions 19%, notifications 12%,
redesigns 9%, battery 5%, location and widgets 4% ([unstar][unstar-rank]).

1. **Ads that block the forecast.** People object less to ads existing than to when they appear: launch interstitials, auto-playing video, ads covering the hourly view or the radar during a storm. Some say ads remain even after paying.
   ([unstar TWC][unstar-twc], [mwm.ai on TWC 16.8.1][mwm-twc], [MacRumors "why did TWC ruin their app"][mr-twc], [r/assholedesign via mirror][reddit-ads], [feddit][feddit])
2. **Subscriptions and paywalls on things that used to be free.** Examples: Carrot going subscription-only, Hello Weather's complication, Mercury charging for more than one place, TWC's 15-minute forecast. Some users accept a fair price, and they say so; the anger is at bait-and-switch and confusing tiers.
   ([MacRumors Carrot 5 thread][mr-carrot], [Hello complication thread][mr-hello], [unstar][unstar-rank])
3. **Rain accuracy and contradictions.** Examples: "100% chance today" with dry hours, rain falling while the app says none, next-hour data unavailable. People also misread "% chance of rain". A University of Reading study found UK apps over-forecast rain.
   ([Apple Discussions][apple-wack], [Apple dev forum][apple-rain], [TidBITS][tidbits], [Which?][which], [Reading blog][reading])
4. **Location privacy.** AccuWeather sent location to Reveal Mobile even with location off (2017), and EPIC sued in 2018. The LA City Attorney sued TWC/IBM over selling location data; it settled in 2020 with clearer disclosures. People now distrust "Always" location prompts.
   ([EPIC v. AccuWeather][epic], [Engadget on TWC][engadget-twc], [Bass Berry][bassberry])
5. **Battery drain** from background location and widgets (TWC especially, sometimes Apple Weather). This is anecdotal.
   ([Apple Discussions TWC][apple-twc-batt], [Apple Discussions 2025][apple-batt25], [MacRumors widget drain][mr-widget])
6. **Notification noise**: duplicates, stale alerts, "thunderstorm in the next hour" that never comes. Users still want storm warnings; they want them *right*, not fewer.
   ([AppleVis Weather Gods][applevis])
7. **Clutter and redesigns.** News, video and lifestyle content push the forecast down. Big redesigns move things people knew where to find. Windy is "too much" for casual users.
   ([unstar][unstar-rank], [feddit][feddit])
8. **Missing Dark Sky.** Custom rain notifications, a clean "rain stopping in 12 min" line and the time-machine view are gone.
   ([9to5Mac][9to5-ds], [PhoneArena][pa-ds])
9. **Aggressive trials.** For example, a weekly $9.99 auto-renewal with a hard-to-find cancel (Weather Live).
   ([unstar][unstar-rank])

Reddit-specific evidence was thin in search (direct Reddit pages were not
returned; mirrors and aggregators were). Treat the tone as reliable and any
counts as indicative ([GummySearch][gummy], [r/ios mirror][rios]).

## Persona

### Primary: Maya, the commuter parent

- **Context:** 36, lives in a mid-size city (say Manchester or Denver). She walks two kids to school, then cycles or takes the train to work. Weekends are the park, football practice and errands.
- **Devices:** an iPhone (one or two years old) and sometimes an Apple Watch. She glances at the lock screen and a home-screen widget far more than she opens the app.
- **Goals:**
  - in five seconds: do the kids need coats, is there rain at 8:15 and 15:30, umbrella or not;
  - on the weekend: the best two hours for the park.
- **Frustrations:**
  - ads before the forecast;
  - "60% rain" that means nothing to her;
  - apps that nag for "Always" location;
  - push alerts that cry wolf;
  - paying for an app and then being asked for more.
- **Usage moments:** 06:45 in bed (today's line), 08:00 at the door (next hour), 15:00 at work (pick-up), Friday evening (the weekend).

### Secondary: Sam, the weekend outdoors generalist

- **Context:** 29, no kids. He runs three times a week, cycles and hikes now and then, and has barbecues in summer. He doesn't need a specialist edition, just "when is it nice to be out".
- **Devices:** iPhone and Watch. He uses StandBy at night.
- **Goals:** the best window today or this week; wind for cycling; heat and air quality for runs; UV.
- **Frustrations:**
  - Windy is too much;
  - TWC's activity cards are buried below news;
  - indices with no reason given.
- **Usage moments:** evening planning for tomorrow, the morning run check, Thursday planning for Saturday.

## User stories

Tags: priority [P0 must for launch / P1 strong / P2 later] and status vs current
code [have / partial / missing]. "Maya" and "Sam" are the personas above.

### A. At a glance, right now

1. As Maya, I want the current temperature, sky and feels-like on one screen so that I can dress in seconds. [P0][have]
2. As Maya, I want one plain sentence for the rest of today ("dry until 4, then showers; 14° max") so that I don't read charts at the door. [P0][missing]
3. As Maya, I want a "what to bring" line (umbrella, coat, sunscreen) so that I pack the kids' bags correctly. [P0][missing]
4. As Maya, I want "warmer / colder than yesterday" so that I can judge layers against what I wore. [P1][missing]
5. As Sam, I want wind speed with its direction so that I know which way the ride home will be hard. [P0][have]
6. As Sam, I want gusts shown next to wind so that I know whether the bike will be pushed around. [P1][partial]
7. As Maya, I want humidity, pressure, visibility and dew point tucked below so that the main view stays clean. [P1][have]
8. As Maya, I want sunrise and sunset so that I know if pick-up is in the dark. [P1][have]
9. As Sam, I want the UV index now and the day's peak so that I know when to wear sunscreen. [P0][partial]
10. As Maya, I want to see how old the forecast is ("updated 8 min ago") so that I can trust it. [P0][missing]
11. As Maya, I want the screen to open instantly with the last forecast so that I'm never staring at a spinner. [P0][missing]
12. As Sam, I want the moon phase so that I know how bright an evening walk will be. [P2][have]

### B. Rain and the next hours

13. As Maya, I want "rain starts in about 20 min, lasts 40 min" so that I can time leaving the house. [P0][missing]
14. As Maya, I want a 2-hour rain-intensity graph in 15-minute steps so that I can see the gap between showers. [P0][missing]
15. As Maya, I want the rain chance explained as words ("likely", "a few showers") as well as a percentage so that I understand it. [P1][missing]
16. As Maya, I want the expected amount (light, moderate, heavy, in mm or inches) so that I know if a hood will do. [P1][partial]
17. As Maya, I want snow told apart from rain, with its depth, so that I know whether school might close. [P1][partial]
18. As Sam, I want a 24-hour temperature chart so that I can see the warm part of the day. [P0][have]
19. As Sam, I want a 24-hour rain-chance chart so that I can pick a dry run. [P0][have]
20. As Sam, I want a 48-hour hourly list with icons so that I can plan tomorrow morning tonight. [P1][partial]
21. As Maya, I want the hourly rows for 08:00 and 15:00 highlighted (school runs I set) so that I check only what matters. [P2][missing]
22. As Maya, I want the app to say honestly when next-hour rain is less precise here so that I don't over-trust it. [P1][missing]
23. As Sam, I want thunder risk flagged per hour so that I avoid being caught on an open field. [P0][partial]
24. As Sam, I want fog and low visibility called out in the morning so that I ride with lights. [P2][partial]

### C. Planning the day and week

25. As Maya, I want a 7-day forecast with highs, lows and rain so that I can plan the week. [P0][have]
26. As Sam, I want up to 14 days, marked as less certain, so that I can pencil in a trip. [P2][missing]
27. As Maya, I want to tap a day for its hourly detail so that I can plan Saturday's party. [P1][missing]
28. As Sam, I want each day's best outdoor window ("sat 10–15h") so that I pick the best day. [P0][have]
29. As Sam, I want the weekend summarized on Thursday ("Saturday is the better day") so that I can commit to plans. [P1][missing]
30. As Maya, I want a day/night description ("cloudy morning, clearing by afternoon") so that the day reads like a story. [P1][missing]
31. As Sam, I want the hours of sunshine each day so that I know if it's worth heading out. [P2][missing]
32. As Maya, I want the five-day strip on the first screen so that I see the coming days without swiping. [P0][have]
33. As Sam, I want to know how sure the forecast is (models agree or not) so that I can judge weekend plans. [P2][missing]
34. As Maya, I want a frost or ice warning for the morning so that I leave early to scrape the car. [P1][missing]
35. As Sam, I want the heat of the day ("hottest 14–16h, 31°") so that I run before it. [P1][partial]
36. As Maya, I want last week's actual weather so that I can settle "was it really that wet?". [P2][missing]

### D. Outdoors "good for" verdicts

37. As Sam, I want a good/fair/poor verdict for being outside now so that I decide at a glance. [P0][have]
38. As Sam, I want the top reasons behind the verdict ("comfortable, gentle wind") so that I trust it. [P0][have]
39. As Sam, I want today's best window for outdoors on the hub so that I know when to go. [P0][have]
40. As Sam, I want a 24-hour verdict band so that I can see good and bad stretches. [P0][have]
41. As Sam, I want tomorrow's best window once today's has passed so that the hub stays useful in the evening. [P0][have]
42. As Sam, I want to choose which activities appear in "good for" so that the list fits my life. [P0][missing]
43. As Sam, I want everyday activity lenses (run, cycle, walk the dog, picnic, barbecue, drying laundry) so that the verdict fits what I'm doing. [P1][missing]
44. As Sam, I want to set my comfort range ("I run cold") so that "comfortable" means comfortable for me. [P1][missing]
45. As Maya, I want a "kids outside" lens (playground: wet, hot slides, UV, cold) so that I can plan the park. [P2][missing]
46. As Sam, I want the verdict to include air quality and pollen so that a "good" hour is not a smoggy one. [P1][missing]
47. As Sam, I want the verdict to say heat and cold in feels-like terms (wind chill, humidity) so that it matches my body. [P1][partial]
48. As Sam, I want to be told when there is no good window today and when the next one is so that I don't keep checking. [P1][partial]
49. As Sam, I want the verdict colour to also show as a word so that it works without colour vision. [P0][have]
50. As Sam, I want to share a verdict ("Saturday 10–15h good for a picnic") with friends so that we can plan together. [P2][missing]

### E. Health: air, pollen, UV, heat and cold

51. As Sam, I want the air quality index now and today (US AQI or European AQI by region) so that I decide whether to run. [P1][missing]
52. As Maya, I want pollen levels (grass, birch, ragweed) where available so that my son takes his antihistamine. [P1][missing]
53. As Maya, I want the app to say plainly when pollen isn't available here so that I don't think it's zero. [P1][missing]
54. As Sam, I want a UV peak time and a "burn time" hint so that I cover up at midday. [P1][partial]
55. As Maya, I want a heat warning in plain words ("too hot for the playground slides after 1pm") so that the kids stay safe. [P2][missing]
56. As Sam, I want the wind chill in winter so that I dress for my morning run. [P1][partial]
57. As Maya, I want humidity described as "muggy" or "dry" so that the number means something. [P2][missing]
58. As Sam, I want dust or smoke events highlighted so that I skip outdoor workouts. [P2][missing]
59. As Maya, I want the dew point explained ("sticky") or hidden so that it doesn't confuse. [P2][partial]
60. As Sam, I want sunrise and sunset golden-hour times so that I can plan an evening walk. [P2][partial]

### F. Places and location

61. As Maya, I want the forecast for where I am, from the device location, so that I don't add my city by hand. [P0][missing]
62. As Maya, I want to search any town or postcode so that I can add Grandma's place. [P0][missing]
63. As Maya, I want to save, rename, reorder and delete places so that my list is mine. [P0][missing]
64. As Maya, I want "approximate location" to be enough so that I don't have to share my exact position. [P0][missing]
65. As Sam, I want to drop a pin on a map for a trailhead so that I can forecast places without a name. [P2][missing]
66. As Maya, I want to swipe or tap quickly between my places so that I check home and school in seconds. [P1][partial]
67. As Sam, I want the forecast at the place's real height so that a hill gives hill weather. [P1][have]
68. As Maya, I want place names in my language so that the list reads naturally. [P2][missing]
69. As Sam, I want a "trip" place with dates so that I see only the days I'll be there. [P2][missing]
70. As Maya, I want the app to work fully if I never grant location so that it respects my choice. [P0][partial]

### G. Alerts and notifications

71. As Maya, I want a notification when rain will start within 15–30 min where I am so that I can bring the washing in. [P1][missing]
72. As Maya, I want official severe weather warnings (storms, flooding, heat) for my places so that my family stays safe. [P0][missing]
73. As Maya, I want an optional morning briefing at a time I pick so that I know the day before getting up. [P1][missing]
74. As Sam, I want a heads-up when a good window opens tomorrow so that I plan a run. [P2][missing]
75. As Maya, I want quiet hours and per-type toggles so that alerts never wake the baby. [P1][missing]
76. As Maya, I want no marketing or news notifications ever so that every alert matters. [P0][missing]
77. As Maya, I want an evening frost alert so that I cover the car or plants. [P2][missing]
78. As Sam, I want a "rain ending" notification so that I can head out the moment it stops. [P2][missing]
79. As Maya, I want alerts to name the place and time ("Home: rain at 15:20") so that I know which place it means. [P1][missing]
80. As Maya, I want the app to work without notifications so that alerts are a choice, not a requirement. [P0][have]

### H. Widgets and system surfaces

81. As Maya, I want a small home-screen widget with temperature and today's line so that I rarely open the app. [P0][missing]
82. As Maya, I want a lock-screen widget with next-hour rain so that I check it from bed. [P0][missing]
83. As Sam, I want a medium widget with today's best outdoor window so that "good for" lives on my home screen. [P1][missing]
84. As Maya, I want a Live Activity on the lock screen while rain is near so that I can watch it arrive. [P2][missing]
85. As Sam, I want a StandBy and a large widget with the hourly chart so that I see the day at a glance on the nightstand. [P2][missing]
86. As Sam, I want an Apple Watch complication with the verdict and temperature so that I check it mid-run. [P2][missing]
87. As Maya, I want Siri and Shortcuts ("Do I need an umbrella?") so that I can ask hands-free. [P2][missing]
88. As Maya, I want widgets to follow my Metro accent colour and light/dark theme so that my home screen looks consistent. [P2][missing]

### I. Settings, units and personalization

89. As Maya, I want °F or °C, mph, km/h, m/s or knots, and inches or mm so that numbers match where I live. [P0][missing]
90. As Maya, I want a 12- or 24-hour clock from my phone settings so that times look normal. [P0][missing]
91. As Sam, I want light and dark themes and a choice of accent colour so that it feels like my phone. [P1][missing]
92. As Sam, I want to reorder hub sections so that what I check first comes first. [P2][missing]
93. As Maya, I want sensible defaults by region (US: °F, mph) so that I configure nothing on day one. [P0][missing]
94. As Sam, I want to see the data source and model for my place so that I know where the forecast comes from. [P1][missing]

### J. Trust, privacy, reliability and accessibility

95. As Maya, I want no ads, no trackers and a privacy label of "Data Not Collected" so that I can trust the app with my location. [P0][partial]
96. As Maya, I want a clear, one-time price or a fair subscription that never takes back features so that I don't feel tricked. [P0][missing]
97. As Maya, I want the last forecast to show offline, marked as old, so that it works on the train. [P0][missing]
98. As Maya, I want a clear message and a retry when the service is down so that I know it's not my phone. [P0][have]
99. As Sam, I want VoiceOver labels and Dynamic Type so that the big lowercase type stays readable for everyone. [P1][missing]
100. As Maya, I want the app to use almost no battery in the background so that it never shows up in battery settings. [P0][partial]

Count: 100. By status: 19 have, 18 partial, 63 missing (tags are judgement calls against `forecast.lua`, `report.lua`, `activities/outdoors.lua`, `places.lua`, `provider.lua`, `Pages/Hub.xml`).

## Feature gaps

Ordered by priority. Sizes: S = days, M = 1–2 weeks, L = more, or needs native or engine work.

| # | Feature | Why | Data | Size | Likely files |
| --- | --- | --- | --- | --- | --- |
| 1 | **Device location + place search + saved places** | Every competitor has it. Without it the app is a demo. | Search: Open-Meteo Geocoding API (`/v1/search`, GeoNames-based; names, postcodes, `language`, `countryCode`; no reverse geocoding). Device: iOS Core Location with approximate accuracy accepted. Reverse name: iOS `CLGeocoder` natively; Nominatim's usage policy is unsuitable for app traffic. | L | `places.lua`, `provider.lua`, new `Pages/Places.xml` / search template, `App.lua`; native location and persistence (engine or plugin) |
| 2 | **Next-hour rain ("starts in 20 min")** | The thing people still miss from Dark Sky and the top door-step question. | Open-Meteo `minutely_15=precipitation,rain,snowfall,weather_code` with `forecast_minutely_15=8`. True 15-minute data only in North America and Central Europe; elsewhere it is interpolated from hourly, so the wording must soften there. Radar nowcasting (WeatherKit `forecastNextHour`) is more precise but needs a key and a signed JWT. | M | `forecast.lua` (new block), `report.lua` (`next` row), `Pages/Hub.xml`, a template |
| 3 | **Today in one line + what to bring** | Answers "what will it be like" without reading. Matches the plain-words house style. | Derived from existing hourly fields; add `precipitation_hours`, `uv_index_max`, `wind_gusts_10m_max` and `apparent_temperature_max/min` daily. | S | `report.lua`, `activities/outdoors.lua`, `Templates/HubNow.xml` |
| 4 | **Units, clock and region defaults** | US users will not read °C and m/s; it's a one-star review on day one. | Convert locally (rules stay metric); Open-Meteo also has `temperature_unit`, `wind_speed_unit`, `precipitation_unit`. Locale from the device. | S–M | `score.lua` (`fmt`), `sky.lua` (`clock`), `report.lua`, a settings page |
| 5 | **Persistent cache, offline view and "updated n min ago"** | Instant launch and trust on trains. Today's cache is 10 minutes in memory only. | None; store the last JSON per place. | S | `provider.lua`, `forecast.lua` (`epoch` already kept) |
| 6 | **Home and lock-screen widgets** | Most use happens here (Maya). A Metro "live tile" is also the brand's heritage. | The same cache, shared through an App Group. | L | native WidgetKit extension (Swift) reading a JSON snapshot that the app writes; iOS bundler |
| 7 | **Official severe weather alerts** | A safety basic; Apple Weather has it. | US: NWS `api.weather.gov/alerts` (free, public domain, needs a User-Agent). EU: MeteoAlarm feeds (check licence). Canada: ECCC. Or WeatherKit `weatherAlerts`, which covers many countries but needs a JWT. Open-Meteo has **no** warnings. | M–L | new `alerts.lua`, `report.lua`, Hub "now" banner |
| 8 | **Air quality and pollen in outdoors** | It makes "good for outdoors" honest on smoky or high-pollen days, and many TWC/AccuWeather indices include it. | Open-Meteo Air Quality API: `us_aqi`, `european_aqi`, `pm2_5`, `uv_index`, pollen (alder, birch, grass, mugwort, olive, ragweed). **Pollen is Europe only, in season, 4 days.** Global domain about 45 km, 3-hourly, 5 days. | S–M | `forecast.lua` (third, optional request), `activities/outdoors.lua`, `report.lua` details |
| 9 | **Choose and order activities; everyday lenses** | TWC and AccuWeather both let you pick. Run, cycle, dog walk, picnic and laundry make "outdoors" personal for the general audience. | Existing fields. | M | `activities.lua`, new `activities/*.lua`, a settings page, persistence |
| 10 | **Notifications: rain soon, morning briefing** | Retention, and the Dark Sky gap. | `minutely_15` (as #2). Reliable delivery needs a server push or a tolerant `BGAppRefreshTask`; local-only checks are best-effort. | L | native background task + `UNUserNotificationCenter`; possibly a small server |
| 11 | Day detail (tap a day → its hours) and 14–16 days | Planning parties and trips. | `forecast_days=16` (mark days 8+ as uncertain). | S–M | `report.lua`, `Pages/Day.xml`, `forecast.lua` (`DAYS`) |
| 12 | Yesterday comparison | A cheap, much-loved line ("4° colder than yesterday"). | `past_days=1`. | S | `forecast.lua`, `report.lua` |
| 13 | Comfort preferences ("I run cold") | Personal verdicts. | None. | S | `activities/outdoors.lua`, settings |
| 14 | Accessibility: VoiceOver labels, Dynamic Type, contrast | App Store review, a wider audience, Apple featuring. | None. | M | `Templates/*.xml`, `Styles/weather.css`, engine accessibility support (verify) |
| 15 | Hourly "good for" traffic-light row in the hub | Already proposed in DESIGN.md; cheap visual. | None. | S | `report.lua`, `Pages/Hub.xml` |
| 16 | Rain map / radar | Expected by many, but expensive. | RainViewer's free API was cut back in 2025 (composites patrons-only, zoom ≤10, "not for high-volume commercial"). US NOAA MRMS/NEXRAD tiles are public; global radar needs a paid provider (Tomorrow.io, Xweather, etc.). *Status uncertain as of Nov 2026.* | L | new map page; native map view |
| 17 | Live Activity for rain; Watch; Siri/Shortcuts | Polish and featuring potential. | As #2. | L | native extensions |

**Licensing before launch (cross-cutting, blocking):** the free Open-Meteo API
is for **non-commercial** use (600/min, 5,000/h, 10,000/day, 300,000/month).
An App Store app sold for money, or with a subscription, is commercial and
needs a paid plan: Standard 1M calls/month, Professional 5M. Prices were not
shown on the page. Whether a free, no-ads app counts is unclear; ask
info@open-meteo.com. Data is CC BY 4.0, so show attribution. Estimate calls:
1 forecast + optional marine + AQ per place per refresh × places × users.
([pricing][om-pricing], [docs][om-docs], [AQ][om-aq], [geocoding][om-geo])

## What we must avoid

- **Ads of any kind**, above all before or over the forecast or the radar.
- **Taking features back** behind a new tier, or paywalling the basics (more than one place, widgets, alerts). If there's a subscription, say what it pays for (data costs) and keep a lifetime option.
- **Dark-pattern trials** (weekly billing, hidden cancel).
- **Selling or sharing location.** No third-party SDKs. Ask for "When In Use" plus approximate location; never nag for "Always"; send only rounded coordinates to the API.
- **Notification spam**: no news, no "engagement" pushes, no duplicates, nothing during quiet hours, and no alert without a place and time.
- **False precision**: don't say "rain in 7 minutes" where the 15-minute data is interpolated; mark days 8+ as uncertain; explain percentages.
- **Clutter**: no news feed or video. The hub stays now · good for · hourly · week · places.
- **Unexplained scores**: every verdict keeps its reasons (our strength over the indices).
- **Battery drain**: no continuous background location; refresh on open, plus rare background tasks.
- **Silent failure**: always show the age of the data and a retry.
- **Big disruptive redesigns** after launch; evolve in place.

## Shared platform needs

What every edition (full, fishing, surf, sailing, hiking, garden, stargazing)
needs in common, best built once in the shared source:

- **Place search and saved places:** geocoding, persistence, reorder and rename. Editions add place facts such as `facing` and `lake`, perhaps by asking the user or from a map tap.
- **Device location:** approximate accuracy is enough. It must work fully without permission.
- **Units and locale:** temperature, wind (incl. knots for sail and surf), precipitation, height (ft/m for surf and hiking), 12/24 h clock and region defaults. Convert at display time; activity rules stay metric.
- **Persistent cache and offline:** the last forecast per place, its age, instant launch and a cap on request volume (it also controls Open-Meteo costs).
- **Widgets and lock screen:** one snapshot format (now, the next-hour line, today's verdict and window per followed activity) that every edition's WidgetKit extension reads. This is the "live tile" heritage.
- **Notifications:** one scheduler with per-type toggles, quiet hours and place/time wording. Editions add their own triggers (a good surf window, frost for garden, a clear night for stars).
- **Severe alerts:** one regional provider layer (NWS, MeteoAlarm, ECCC or WeatherKit).
- **Privacy:** no third-party SDKs; aim for the App Store label "Data Not Collected"; rounded coordinates; a plain privacy page.
- **Data licence and attribution:** an Open-Meteo commercial plan (and any radar or alerts provider), CC BY attribution on an "about" page, and a per-place note on the data source and model.
- **Settings and persistence:** followed activities, comfort ranges, theme and accent, saved in one store.
- **Accessibility:** VoiceOver labels on verdicts, charts and bands; Dynamic Type; verdict words never colour-only.
- **Monetization model:** one decision shared by all editions (e.g. paid up front per edition, or free with one optional subscription covering all), applied consistently.

[mr-ds]: https://www.macrumors.com/2023/01/01/dark-sky-shuts-down-tomorrow/
[9to5-ds]: https://9to5mac.com/2022/12/20/apple-weather-dark-sky-support-document/
[pa-ds]: https://www.phonearena.com/news/Apple-to-shut-down-Dark-Sky-app-publishes-a-guide-to-Weather-app-for-Dark-Sky-users_id144376
[apple-rain]: https://developer.apple.com/forums/thread/729736
[apple-wack]: https://discussions.apple.com/thread/254922784
[mr-bad]: https://forums.macrumors.com/threads/weather-app-is-so-bad.2415294/
[tidbits]: https://talk.tidbits.com/t/weather-in-ios-15-gains-precipitation-notifications-but-do-they-work-reliably/17278
[twc-blog]: https://www.weathercompany.com/blog/weather-tailored-to-your-outdoor-adventures/
[twc-app]: https://apps.apple.com/us/app/id295646461
[fb-la]: https://forecastbar.com/weather-live-activity/
[rv-la]: https://www.rainviewer.com/guide/tutorial-ios-live-activity.html
[mr-carrot-la]: https://forums.macrumors.com/threads/carrot-weather-introduces-carplay-app-and-automatic-live-activities.2447527/page-2
[sweet]: https://thesweetsetup.com/?p=6896
[imore]: https://www.imore.com/best-weather-apps-iphone
[mr-carrot]: https://forums.macrumors.com/threads/carrot-weather-5-0-launches-with-fresh-design-more-snarky-dialogue-and-customization-options.2282291/page-2
[apl-carrot]: https://apppricinglab.com/iap/apple/961390574
[mercury]: https://apps.apple.com/us/app/id1621800675?l=en-us
[hello]: https://apps.apple.com/us/app/hello-weather-worlds-best/id978393692
[mr-hello]: https://forums.macrumors.com/threads/annoyed-at-hello-weather-now-a-paywall-to-use-their-complication.2459934
[strip]: https://apps.apple.com/ca/app/id1528594026
[weawow]: https://apps.appfollow.io/ios/weather-widget-weawow/1209810737?country=gb
[windy]: https://apps.apple.com/nz/app/windy-com/id1161387262
[yr]: https://apps.apple.com/us/app/-/id490989206
[rainalarm]: https://apps.apple.com/us/app/id397676100
[todayweather]: https://apps.apple.com/app/id1214962662
[accu-app]: https://apps.apple.com/app/id300048137
[flowx-rv]: https://forum.flowx.io/t/rainviewer-will-discontinue-jan-2026/1993
[unstar-rank]: https://unstar.app/blog/weather-apps-ranked-by-user-complaints-2026
[unstar-twc]: https://unstar.app/blog/weather-channel-app-ads-before-alerts-radar-reviews-2026
[mwm-twc]: https://mwm.ai/articles/the-weather-channel-radar-v16-8-1-forces-unclosable-2-minute-ads-blocking-radar-may-2026
[mr-twc]: https://spy.macrumors.com/threads/why-did-the-weather-channel-ruin-their-app.2195950
[reddit-ads]: https://lr.eu.psf.lt/r/assholedesign/comments/1e4en01/imagine_having_to_watch_3_minutes_of_ads_during_a
[feddit]: https://feddit.it/post/555770/529934
[which]: https://www.which.co.uk/news/article/weather-apps-which-are-most-accurate-aL43y6N3kuUN
[reading]: https://blogs.reading.ac.uk/weather-and-climate-at-reading/2020/what-does-a-probability-of-rainfall-mean
[epic]: https://epic.org/documents/epic-v-accuweather/
[engadget-twc]: https://www.engadget.com/weather-channel-app-settles-la-lawsuit-074559520.html
[bassberry]: https://www.bassberry.com/news/privacy-perils-weather-apps-tracking-more-than-weather-patterns/
[apple-twc-batt]: https://discussions.apple.com/thread/252943798
[apple-batt25]: https://discussions.apple.com/thread/256080186
[mr-widget]: https://forums.macrumors.com/threads/weather-widget-battery-drain.2433754
[applevis]: https://applevis.com/comment/96622
[gummy]: https://gummysearch.com/tools/best-products/weather-apps/
[rios]: https://redlib.hbubli.cc/r/ios/comments/1hvs4py/ios_weather_apps
[om-docs]: https://open-meteo.com/en/docs
[om-pricing]: https://open-meteo.com/en/pricing
[om-aq]: https://open-meteo.com/en/docs/air-quality-api
[om-geo]: https://open-meteo.com/en/docs/geocoding-api
