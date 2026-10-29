-- A preview datasource: the provider contract served from deterministic
-- fixtures, for authoring, editor previews and offline tests. Pages and
-- declarations are unchanged; only the operations behind them differ.
--
--   local provider = Fixtures.provider({
--       accounts = { { id = "a1", name = "Checking" } },
--       entries = function(params) return rows[params.AccountId] end,
--   }, { computed = computed, filters = filters })
--
-- An operation the fixtures name returns those rows (a function receives
-- the operation's arguments). Query parameters filter table rows by the
-- matching snake_case field (AccountId → account_id), and pages are cut by
-- the query's PageSize with the row index as cursor. Any other operation
-- succeeds without effect, so command buttons work in a preview.
local Fixtures = {}

local function field(param)
    return (param:gsub("(%l)(%u)", "%1_%2"):lower())
end

local function serve(rows, params, page)
    local matched = {}
    for _, row in ipairs(rows) do
        local keep = true
        for name, value in pairs(params or {}) do
            local key = field(name)
            if value ~= nil and value ~= "" and row[key] ~= nil and row[key] ~= value then keep = false end
        end
        if keep then matched[#matched + 1] = row end
    end
    if not page or not page.size then return matched end
    local first = tonumber(page.cursor or 0) or 0
    local items = {}
    for index = first + 1, math.min(#matched, first + page.size) do items[#items + 1] = matched[index] end
    local last = first + #items
    return { items = items, cursor = last < #matched and tostring(last) or nil, hasMore = last < #matched }
end

-- `provider` (optional) supplies computed fields, filters, handlers, media
-- and any real operations; fixtures fill in the rest.
function Fixtures.provider(fixtures, provider)
    provider = provider or {}
    local own = provider.operations or {}
    provider.operations = setmetatable({}, { __index = function(_, name)
        if own[name] then return own[name] end
        local fixture = fixtures[name]
        if type(fixture) == "function" then return fixture end
        if type(fixture) == "table" then
            return function(params, page) return serve(fixture, params, page) end
        end
        return function() return true end
    end })
    return provider
end

return Fixtures
