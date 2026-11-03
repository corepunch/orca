-- Locale-backed application strings. Catalogs contain source keys or semantic IDs;
-- messages/usernames and other service content never pass through this module.
local core = require "orca.core"
local Localization = {}
local Instance = {}
Instance.__index = Instance

local function normalize(language)
    return tostring(language or ""):gsub("_", "-"):match("^[^%.:@]+") or "en"
end

local function category(language, count)
    if type(count) ~= "number" or count % 1 ~= 0 then return "other" end
    count = math.abs(count)
    if language:match("^ru") then
        local tens, units = count % 100, count % 10
        if units == 1 and tens ~= 11 then return "one" end
        if units >= 2 and units <= 4 and (tens < 12 or tens > 14) then return "few" end
        return "many"
    end
    return count == 1 and "one" or "other"
end

local function entries(catalog)
    local result = {}
    for key, value in pairs(catalog) do
        if type(value) == "string" then
            result[#result + 1] = { Key = key, Value = value, Type = "Text" }
        end
    end
    table.sort(result, function(a, b) return a.Key < b.Key end)
    return result
end

function Instance:setLanguage(language)
    local requested = normalize(language or (core.getPreferredLanguage and core.getPreferredLanguage())
        or os.getenv("ORCA_LANGUAGE") or os.getenv("LANG") or self.fallback)
    local lower = requested:lower()
    local selected
    for tag in pairs(self.catalogs) do if tag:lower() == lower then selected = tag; break end end
    local base = lower:match("^[^-]+")
    if not selected then
        for tag in pairs(self.catalogs) do if tag:lower() == base then selected = tag; break end end
    end
    self.language = selected or self.fallback
    self.catalog = {}
    for key, value in pairs(self.catalogs[self.fallback] or {}) do self.catalog[key] = value end
    for key, value in pairs(self.catalogs[self.language] or {}) do self.catalog[key] = value end
    self.dictionary = core.Locale { Language = self.language, Entries = entries(self.catalog) }
    -- Roots can have been removed by application navigation while Lua still
    -- holds their wrappers. Apply only explicitly live views, never old graphs.
    return self.language
end

function Instance:t(key, params)
    if type(key) ~= "string" then return key end
    if type(params) == "number" then params = { count = params } end
    params = params or {}
    local value = self.catalog[key]
    if type(value) == "table" then
        value = value[category(self.language, params.count)] or value.other or value.one
    elseif value ~= nil and core.getLocalizedString then
        value = core.getLocalizedString(self.dictionary, key)
    end
    value = value or key
    return (value:gsub("%%{([%w_]+)}", function(name)
        return params[name] ~= nil and tostring(params[name]) or "%{" .. name .. "}"
    end))
end

function Instance:apply(root)
    if not root then return root end
    local dictionary = self.roots[root]
    if dictionary then dictionary.Entries = entries(self.catalog); dictionary.Language = self.language
    else
        dictionary = core.Locale { Name = "__localization", Language = self.language,
            Entries = entries(self.catalog) }
        root:addChild(dictionary)
        self.roots[root] = dictionary
    end
    local function visit(node)
        -- TextResourceID is resolved by native text layout, including rows
        -- instantiated later. Only presentation-only chrome is source matched.
        local saved = self.originals[node]
        if not saved then saved = {}; self.originals[node] = saved end
        for _, property in ipairs { "PlaceholderText" } do
            local value = saved[property] or node[property]
            if type(value) == "string" and self.catalog[value] ~= nil then
                saved[property] = value
                node[property] = self:t(value)
            end
        end
        for child in node.children do if child ~= dictionary then visit(child) end end
    end
    visit(root)
    return root
end

function Localization.new(options)
    options = options or {}
    local self = setmetatable({ catalogs = options.catalogs or {}, fallback = options.fallback or "en",
        roots = setmetatable({}, { __mode = "k" }), originals = setmetatable({}, { __mode = "k" }) }, Instance)
    self:setLanguage(options.locale or options.language)
    return self
end

Localization.normalize = normalize
Localization.pluralCategory = category
return Localization
