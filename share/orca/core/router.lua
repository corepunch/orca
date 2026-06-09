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

local function compile_route_pattern(url)
  local names = {}
  local parts = { "^" }
  local i = 1
  while i <= #url do
    local ch = url:sub(i, i)
    if ch == ":" then
      local name = url:sub(i + 1):match("^([%a_][%w_]*)")
      if name then
        i = i + #name + 1
        local class = "[^/]+"
        if url:sub(i, i) == "[" then
          local close = url:find("]", i, true)
          if close then class = url:sub(i + 1, close - 1); i = close + 1 end
        end
        names[#names + 1] = name; parts[#parts + 1] = "(" .. class .. ")"
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

local function build_route(name, url, handler)
  local pattern, names = compile_route_pattern(url)
  return {
    name     = name or url,
    url      = url,
    handler  = handler,
    pattern  = pattern,
    names    = names,
    is_dynamic = #names > 0 or url:find("*", 1, true) ~= nil,
  }
end

local function fill_path_params(path, params)
  if type(path) ~= "string" then return nil end
  if type(params) ~= "table" then
    return (path:find(":[%a_][%w_]*") or path:find("*", 1, true)) and nil or path
  end
  local ok = true
  local function replace_param(name)
    local v = params[name]; if v == nil then ok = false; return "" end
    return encode_component(v)
  end
  path = path:gsub(":([%a_][%w_]*)%b[]", replace_param)
  path = path:gsub(":([%a_][%w_]*)", replace_param)
  path = path:gsub("%*", function()
    return params.splat ~= nil and tostring(params.splat) or ""
  end)
  return ok and path or nil
end

local Router = Widget:extend {
  __init = function(self, owner)
    self.owner       = owner
    self.routes      = {}
    self.routes_by_path = {}
    self.named_routes   = {}
    self:registerRoutes(owner)
  end,

  add = function(self, name, url, handler)
    assert(type(url) == "string",    "route url must be a string")
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

    local best_path = nil
    local best_score = -1
    local best_len = -1
    for _, route in ipairs(routes) do
      local path = fill_path_params(route.url, params)
      if path then
        local score = #route.names
        local len = #route.url
        if score > best_score or (score == best_score and len > best_len) then
          best_score = score
          best_len = len
          best_path = path
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
        local captures = { path:match(candidate.pattern) }
        if #captures > 0 then
          local params = {}
          for i, key in ipairs(candidate.names) do params[key] = captures[i] end
          return { name = candidate.name, url = candidate.url, handler = candidate.handler, params = params, path = path }
        end
      end
    end
    return nil
  end,

  dispatch = function(self, req, route, ...)
    route = route or self:resolve(req)
    local owner = self.owner or self
    owner.params = route and route.params or {}
    if type(req) == "table" then req.params = owner.params end
    return route and route.handler(owner, req, ...) or nil
  end,
}

return Router