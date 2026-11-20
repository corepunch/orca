-- Place search (P1) through the Open-Meteo Geocoding API: a name or a
-- postcode, with the country and region shown. The URL goes through
-- Forecast.url, so an API key moves it to the commercial host with the
-- rest. Offline, or when nothing is found, the app's own places that match
-- still show, so search never leaves the page empty without saying why.
local Forecast = require "Weather/forecast"
local MyPlaces = require "Weather/myplaces"
local Search = { COUNT = 10 }

-- Percent-encoding for a query value (UTF-8 bytes as %XX).
function Search.encode(text)
    return (text:gsub("[^%w%-%._~]", function(c) return ("%%%02X"):format(c:byte()) end))
end

-- The trimmed query, or nil when it is too short to ask (Open-Meteo answers
-- nothing to one character).
function Search.clean(text)
    text = tostring(text or ""):gsub("%s+", " "):match("^%s*(.-)%s*$")
    if utf8.len(text) == nil or utf8.len(text) < 2 then return nil end
    return text
end

function Search.url(text)
    return Forecast.url("geocoding", ("name=%s&count=%d&language=en&format=json"):format(Search.encode(text), Search.COUNT))
end

-- Lowercase without the common accents, so "zurich" finds "Zürich".
local FOLD = { ["á"] = "a", ["à"] = "a", ["â"] = "a", ["ä"] = "a", ["ã"] = "a", ["å"] = "a", ["é"] = "e", ["è"] = "e",
    ["ê"] = "e", ["ë"] = "e", ["í"] = "i", ["ì"] = "i", ["î"] = "i", ["ï"] = "i", ["ó"] = "o", ["ò"] = "o", ["ô"] = "o",
    ["ö"] = "o", ["õ"] = "o", ["ø"] = "o", ["ú"] = "u", ["ù"] = "u", ["û"] = "u", ["ü"] = "u", ["ñ"] = "n", ["ç"] = "c",
    ["ß"] = "ss", ["ł"] = "l", ["æ"] = "ae" }
function Search.fold(text)
    text = tostring(text or ""):lower()
    for from, to in pairs(FOLD) do text = text:gsub(from, to) end
    return (text:gsub("Ä", "a"):gsub("Ö", "o"):gsub("Ü", "u"))
end

-- Places the app already knows (saved and suggested) whose name or region
-- holds the text.
function Search.known(text)
    local needle, out = Search.fold(text), {}
    for _, place in ipairs(MyPlaces.all()) do
        local hay = Search.fold((place.title or "") .. " " .. (place.region or ""))
        if hay:find(needle, 1, true) then out[#out + 1] = place end
    end
    return out
end

-- Drafts from a decoded geocoding answer, in its order (best first).
-- `text` is what was asked, so a postcode found is shown.
function Search.parse(json, text)
    local drafts = {}
    for _, result in ipairs(type(json) == "table" and type(json.results) == "table" and json.results or {}) do
        local draft = MyPlaces.fromResult(result, text)
        if draft then drafts[#drafts + 1] = draft end
    end
    return drafts
end

-- Runs a search: `fetch(url, options)` as the forecast uses. Returns the
-- drafts found and, when the network failed, the reason.
function Search.run(text, fetch)
    local clean = Search.clean(text)
    if not clean then return {}, nil end
    local json, err = (fetch or Forecast.networkFetch)(Search.url(clean), Forecast.requestOptions())
    if not json then return {}, err or "No connection." end
    return Search.parse(json, clean), nil
end

return Search
