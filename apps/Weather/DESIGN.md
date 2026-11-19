# Weather: one app for every activity (design proposal)

Status: **built** (place hub, good for, activity pages, places, charts first
for hourly, a verdict band per activity). Live tiles, choosing which
activities to follow and units are not built yet. The seven single-activity apps that came before
(Weather, Fishing, Surf, Sailing, Hiking, Garden, Stargazing on a shared
WeatherKit) were folded into this one app. This document records the
references behind the design. Historical facts, other apps' patterns and our own decisions
are kept apart.

## References

### Amazing Weather HD (EizSoft, Windows Phone 7/8, 2012–2017)

This was the best-reviewed third-party weather app on Windows Phone ("easily
our top", [Windows Central][wc-hd]). Its data came from Weather Underground.
The service shut down on 17 March 2017 ([sunset][wc-sunset]).

What its screenshots show ([Clinton Fitch review, 2014][cf];
[Windows Central v4.0][wc-hd]):

- **A city is a Pivot item.** The header strip reads "Paris Berlin Hel…",
  with the next cities dimmed. Swiping sideways changes the place, not the
  kind of data.
- **One dense "now" page:**
  - the city name and date, then a huge temperature ("16°");
  - "Feels like", the condition ("Light Rain"), wind with an arrow, and the UV index;
  - a left column of humidity, pressure, visibility, precipitation and dew point;
    sunrise and sunset sit on the right;
  - a **five-day strip** at the bottom: lowercase "mon tue wed" headers with an
    accent underline, then an icon, high/low and a rain % with a drop glyph.
- **The app bar** has hourly, radar and pin buttons. Detail pages are Pivots:
  - **day detail:** day/night text and a rain/snow chart;
  - **charts:** 36 hours of rain, temperature and wind, accent on dark grey;
  - **hourly:** a list of cards.
- **Three themes:**
  - Metro light: white background with an accent-coloured temperature;
  - Metro dark;
  - animated HD photo or video backgrounds; a double-tap hides the data.
- **Tiles and lock screen:**
  - small, medium and wide tiles, each with a front and a back; a wide tile shows five days;
  - a transparent tile option on Windows Phone 8.1;
  - a dynamic lock screen ([v4.1][wc-41], [8.1 tiles][wc-81]).

Bing Weather on WP8, for comparison, was a **Panorama**: today · daily ·
hourly · maps. A day opened its own nested day/night Panorama
([All About Windows Phone][aawp]).

### Apps that rate activities

- **The Weather Channel** comes closest to our product.
  - **Today feed:** "Today's Activities Forecast" is one sentence plus a card
    per activity, each with Poor/Fair/Good.
  - **Activity page:** a Poor/Fair/Good bar along a 24-hour timeline, a
    best-window sentence, detail tiles and "next best days".
  - **Choosing activities:** you add the activities you care about
    ([activity hub][twc-hub], [announcement][twc-blog]).
- **AccuWeather Health & Activities:** you choose which indices to follow.
  Every activity shares one scale: Poor, Fair, Good, Very Good, Excellent,
  from 0 to 10 ([API index categories][accu-api]).
- **Windy.app profiles** (Sail, Kite, Surf, Fish, …) are lenses over one forecast:
  - a profile changes which rows the table shows;
  - you switch it by swiping on the spot page or from a "current profile" button
    ([custom profiles][windy-profile]).
- **Clear Outside** (astronomy) puts a traffic-light row over an hourly
  grid: green clear, orange possible, red clouded. Cloud layers, moon, wind
  and frost rows follow ([guide][clear-outside]).
- **Weather Underground Smart Forecasts:** 13 presets or your own ideal and
  acceptable ranges; bars mark the best times ([How-To Geek][wu]).
- **Surf-forecast** fades its 0–10 stars as onshore wind grows
  ([FAQ][surf-forecast]). **Fishing Points** rates each day with 1–3 stars and
  marks major and minor bite times ([help][fishing-points]).

## Proposed design

### The two axes: place and activity

A forecast belongs to a **place**, and an **activity** is a way of reading
it. Amazing Weather swipes between places; Windy.app swaps profiles over the
same forecast; The Weather Channel lists every activity on one card. We
combine them:

```text
Places (Pivot, as in Amazing Weather)   ← swipe: Lake Tahoe · Ocean Beach · Zermatt …
 └─ Place hub (Panorama)
     now            Amazing Weather–style conditions and 5-day strip
     good for       one row per followed activity: verdict + best window   → tap
     hourly · week  everyday weather
 └─ Activity page (separate page, Back returns)
     Pivot: now · hourly · week · conditions   (≤ 4 items)
```

**Recommendation:** use one Panorama per place, with the places as a list or
picker rather than a Pivot around the Panorama. The repository's WP guidance
forbids nesting Pivot inside Panorama, and Panorama should own its sideways
swipe. Amazing Weather's sideways swipe between cities therefore becomes a
**places** section and a pin per place.

### The hub, section by section

1. **Title:** the place name, lowercase and very large: "lake tahoe".
2. **now:**
   - a huge temperature, then feels like, the condition, and wind with its direction;
   - a two-column block: humidity, pressure, visibility, precipitation and dew
     point on the left; sunrise, sunset and the moon on the right;
   - Amazing Weather's five-day strip.
3. **good for:** The Weather Channel's activity cards in Metro dress.
   - One row per followed activity: its name in lowercase, the verdict in its
     tone, and the best window ("best 06–08h").
   - The current verdict tile shrinks to a coloured bar at the row's start.
   - Activities that don't fit the place are left out, for example surf at a lake.
   - Tapping a row opens that activity's page.
4. **hourly:** one row per hour; on the right, a coloured cell for each
   followed activity (Clear Outside's traffic-light row). This is optional.
5. **week:** the days, each with the best activity and its window ("sat:
   good for hiking 10–15h").

### The activity page

Today's profile panorama moves onto its own page as a **Pivot**: now ·
hourly · week · conditions. That's four items, within the WP Pivot limit. The
page keeps the activity's reasons, notes and conditions. The verdict scale
stays good/fair/poor, because the colours and words already work. A 5-step
scale like AccuWeather's is possible but not proposed.

### Places carry what they're good for

Each place lists its activities and the facts their rules need: a coast's
`facing`, the fact that a lake has no sea swell, a summit's height, a dark sky.
The separate spot lists of the seven profiles merge into one list of places.
Search and device location can add places later; neither exists yet.

### Settings

You choose which activities to follow, as in The Weather Channel and
AccuWeather. Units and theme come later, Metro dark first. Per-activity
accent colours mark the activity pages (the lens identity Windy.app needs).
The hub keeps the app accent.

### Later

- Live tiles and the lock screen: Amazing Weather's strongest feature, and
  not available in ORCA yet.
- A photo or animated background theme.
- Charts.

## Keep from the current kit

- The profiles' rules, reasons and tests; profiles become activities.
- One forecast request per place. Activities are lenses over it and add no
  requests. Surf and Sailing add the marine request only for coastal places.
- The data-scope pattern: one report query with rows told apart by `Kind`,
  extended with an activity column.

[wc-hd]: https://www.windowscentral.com/must-have-app-amazing-weather-hd-updated-windows-phone-8
[wc-sunset]: https://www.windowscentral.com/sun-will-set-amazing-weather-hd-march-17
[wc-41]: https://www.windowscentral.com/amazing-weather-hd-updated-dynamic-lock-screens-and-more-live-tile-options
[wc-81]: https://www.windowscentral.com/amazing-weather-hd-updated-transparent-live-tiles
[cf]: https://www.clintonfitch.com/?p=876
[aawp]: https://allaboutwindowsphone.com/flow/item/18107_Bing_Weather_offers_forecasts_.php
[twc-hub]: https://weather.com/activity-hub/running/33b2f813c363234c94d71ad881357dee41d3539311938686cfad33b57a46c807
[twc-blog]: https://www.weathercompany.com/blog/weather-tailored-to-your-outdoor-adventures/
[accu-api]: https://developer.accuweather.com/documentation/index-categories
[windy-profile]: https://media.windy.app/custom_profile
[clear-outside]: https://www.sfnpcc.org/blog/2023/8/16/how-to-navigate-clear-outside-weather-app
[wu]: https://www.howtogeek.com/304478/how-to-get-a-schedule-for-your-ideal-weather-conditions-in-weather-underground/
[surf-forecast]: https://www.surf-forecast.com/pages/faq
[fishing-points]: https://stgfp.zendesk.com/hc/en-us/articles/26490957857298-Understanding-Fish-Activity
