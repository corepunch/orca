local Widget = require "orca.core.widget"

local function encode_component(value, is_query)
  value = tostring(value)
  if is_query then value = value:gsub(" ", "+") end
  return value:gsub("([^%w%-_%.~])", function(c)
    return string.format("%%%02X", string.byte(c))
  end)
end

local function encode_query_string(params)
  if type(params) ~= "table" then
    return params and tostring(params) or ""
  end
  local parts = {}
  for key, value in pairs(params) do
    if value ~= nil then
      parts[#parts + 1] = encode_component(key, true) .. "=" .. encode_component(value, true)
    end
  end
  table.sort(parts)
  return table.concat(parts, "&")
end

local function split_path_and_query(path)
  if type(path) ~= "string" then return path, nil end
  local route_path, query = path:match("^([^?]*)%??(.*)$")
  return route_path, query ~= "" and query or nil
end

local function normalize_request(req)
  if type(req) == "table" then
    req.path = req.path or req.url or req.route
    req.params = req.params or {}
    return req
  end
  return {
    path = req,
    params = {}
  }
end

local function compile_route_pattern(url)
  local names = {}
  local parts = { "^" }
  local i = 1
  while i <= #url do
    local ch = url:sub(i, i)
    if ch == "(" then
      -- skip: optional groups handled by expand_route_patterns, not inline
      i = i + 1
    elseif ch == ")" then
      i = i + 1
    elseif ch == ":" then
      local name = url:sub(i + 1):match("^([%a_][%w_]*)")
      if name then
        i = i + #name + 1
        local class = "[^/]+"
        if url:sub(i, i) == "[" then
          local close = url:find("]", i, true)
          if close then class = url:sub(i + 1, close - 1); i = close + 1 end
        end
        names[#names + 1] = name
        parts[#parts + 1] = "(" .. class .. ")"
      else
        parts[#parts + 1] = "%:"; i = i + 1
      end
    elseif ch == "*" then
      names[#names + 1] = "splat"; parts[#parts + 1] = "(.*)"; i = i + 1
    else
      parts[#parts + 1] = ch:gsub("([^%w])", "%%%1"); i = i + 1
    end
  end
  parts[#parts + 1] = "$"
  return table.concat(parts), names
end

-- Expand a URL template with optional groups into a list of concrete variants.
-- "/adventure/:game(/:session)" → { "/adventure/:game", "/adventure/:game/:session" }
-- Works recursively so nested optionals work too.
local function expand_route_patterns(url)
  local s, e = url:find("%b()")
  if not s then return { url } end
  local before = url:sub(1, s - 1)
  local inside = url:sub(s + 1, e - 1)
  local after  = url:sub(e + 1)
  local results = {}
  -- variant without the optional segment
  for _, v in ipairs(expand_route_patterns(before .. after)) do
    results[#results + 1] = v
  end
  -- variant with the optional segment
  for _, v in ipairs(expand_route_patterns(before .. inside .. after)) do
    results[#results + 1] = v
  end
  return results
end

local function build_route(name, url, handler)
  local pattern, names = compile_route_pattern(url)
  local variants = expand_route_patterns(url)
  -- Build a sub-pattern per concrete variant (sorted longest first so
  -- the most-specific variant wins during resolve).
  table.sort(variants, function(a, b) return #a > #b end)
  local sub_patterns = {}
  for _, v in ipairs(variants) do
    local p, n = compile_route_pattern(v)
    sub_patterns[#sub_patterns + 1] = { pattern = p, names = n, url = v }
  end
  return {
    name         = name or url,
    url          = url,
    handler      = handler,
    pattern      = pattern,   -- kept for is_dynamic detection
    names        = names,
    is_dynamic   = #names > 0 or url:find("*", 1, true) ~= nil
                   or url:find("(", 1, true) ~= nil,
    sub_patterns = sub_patterns,
  }
end

-- Expand one optional group segment, returning the filled text or "" if any
-- enclosed param is missing. Handles the plain-param and splat sub-cases only
-- (no nested groups — those are handled by the outer recursive call in
-- fill_path_params).
local function expand_optional(segment, params)
  -- Check whether every :param inside this segment is present.
  local ok = true
  local result = segment
    :gsub(":([%a_][%w_]*)%b[]", function(name)
      local v = params[name]; if v == nil then ok = false; return "" end
      return encode_component(v)
    end)
    :gsub(":([%a_][%w_]*)", function(name)
      local v = params[name]; if v == nil then ok = false; return "" end
      return encode_component(v)
    end)
    :gsub("%*", function()
      return params.splat ~= nil and tostring(params.splat) or ""
    end)
  return ok and result or ""
end


local function fill_path_params(path, params)
  if type(path) ~= "string" then return nil end
  if type(params) ~= "table" then
    return (path:find(":[%a_][%w_]*") or path:find("*", 1, true)
            or path:find("(", 1, true)) and nil or path
  end

  -- Expand optional groups into concrete variants, try longest first.
  local variants = expand_route_patterns(path)
  table.sort(variants, function(a, b) return #a > #b end)
  for _, variant in ipairs(variants) do
    local ok = true
    local function replace_param(name)
      local v = params[name]; if v == nil then ok = false; return "" end
      return encode_component(v)
    end
    local result = variant
      :gsub(":([%a_][%w_]*)%b[]", replace_param)
      :gsub(":([%a_][%w_]*)",     replace_param)
      :gsub("%*", function()
        return params.splat ~= nil and tostring(params.splat) or ""
      end)
    if ok then return result end
  end
  return nil
end

local Router = Widget:extend {
  __init = function(self, owner)
    self.owner          = owner
    self.routes         = {}
    self.routes_by_path = {}
    self.named_routes   = {}
    self:registerRoutes(owner)
  end,

  normalize_request = function(self, req)
    return normalize_request(req)
  end,

  add = function(self, name, url, handler)
    assert(type(url) == "string",       "route url must be a string")
    assert(type(handler) == "function", "route handler must be a function")
    local route = build_route(name, url, handler)
    self.routes[#self.routes + 1] = route
    self.routes_by_path[url] = route
    if name and name ~= url then
      if not self.named_routes[name] then
        self.named_routes[name] = {}
      end
      self.named_routes[name][#self.named_routes[name] + 1] = route
    end
    table.sort(self.routes, function(a, b)
      if a.is_dynamic ~= b.is_dynamic then return not a.is_dynamic end
      if #a.names     ~= #b.names     then return #a.names < #b.names end
      return #a.url > #b.url
    end)
    return handler
  end,

  url_for = function(self, name, params, query)
    local function with_query(p)
      if query == nil then return p end
      local qs = encode_query_string(query)
      return qs ~= "" and p .. "?" .. qs or p
    end

    if type(name) == "string" and name:sub(1, 1) == "/" then
      local path = fill_path_params(name, params)
      return path and with_query(path)
    end

    local routes = self.named_routes[name]
    if not routes then return nil end

    local best_path, best_score, best_len = nil, -1, -1
    for _, route in ipairs(routes) do
      local path = fill_path_params(route.url, params)
      if path then
        local score, len = #route.names, #route.url
        if score > best_score or (score == best_score and len > best_len) then
          best_score, best_len, best_path = score, len, path
        end
      end
    end

    return best_path and with_query(best_path)
  end,

  match = function(self, name, url, handler)
    return self:add(name, url, handler)
  end,

  registerRoutes = function(self, target)
    local mt = getmetatable(target)
    if not mt then return end
    for k, v in pairs(mt) do
      if type(k) == "table" and type(v) == "function" then
        for route_name, route_path in pairs(k) do
          if type(route_path) == "string" and route_path:sub(1, 1) == "/" then
            self:add(route_name, route_path, v)
          end
        end
      elseif type(k) == "string" and k:sub(1, 1) == "/" and type(v) == "function" then
        self:add(k, k, v)
      end
    end
  end,

  resolve = function(self, req)
    local path = type(req) == "table" and (req.path or req.url or req.route) or req
    if type(path) ~= "string" then return nil end
    path = split_path_and_query(path)
    if not path then return nil end

    local route = self.routes_by_path[path]
    if route then
      return { name = route.name, url = route.url, handler = route.handler, params = {}, path = path }
    end

    for _, candidate in ipairs(self.routes) do
      if candidate.is_dynamic then
        for _, sp in ipairs(candidate.sub_patterns) do
          local captures = { path:match(sp.pattern) }
          if #captures > 0 or path:match(sp.pattern) ~= nil then
            local params = {}
            for i, key in ipairs(sp.names) do
              if captures[i] ~= nil then params[key] = captures[i] end
            end
            return { name = candidate.name, url = candidate.url, handler = candidate.handler, params = params, path = path }
          end
        end
      end
    end
    return nil
  end,

  dispatch = function(self, req, route, ...)
    req = self:normalize_request(req)
    route = route or self:resolve(req)
    local owner = self.owner or self
    req.params = route and route.params or {}
    if route and route.name ~= nil then req.route_name = route.name end
    return route and route.handler(owner, req, ...) or nil
  end,
}

return Router