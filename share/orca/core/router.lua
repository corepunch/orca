local Widget = require "orca.core.widget"

local function encode_query_component(value)
  value = tostring(value)
  value = value:gsub(" ", "+")
  value = value:gsub("([^%w%-_%.~])", function(c)
    return string.format("%%%02X", string.byte(c))
  end)
  return value
end

local function encode_query_string(params)
  if type(params) ~= "table" then
    return params and tostring(params) or ""
  end

  local parts = {}
  for key, value in pairs(params) do
    if value ~= nil then
      parts[#parts + 1] = encode_query_component(key) .. "=" .. encode_query_component(value)
    end
  end
  table.sort(parts)
  return table.concat(parts, "&")
end

local Router = Widget:extend {
  __init = function(self, owner)
    self.owner = owner
    self.routes = {}
    self.named_routes = {}
    self:registerRoutes(owner)
  end,

  add = function(self, name, url, handler)
    assert(type(url) == "string", "route url must be a string")
    assert(type(handler) == "function", "route handler must be a function")
    self.routes[url] = {
      name = name or url,
      handler = handler
    }
    if name and name ~= url then
      self.named_routes[name] = url
    end
    return handler
  end,

  url_for = function(self, name, params, query)
    if type(name) == "string" and string.byte(name, 1) == 47 then
      local path = name
      if query ~= nil then
        local query_string = encode_query_string(query)
        if query_string ~= "" then
          path = path .. "?" .. query_string
        end
      end
      return path
    end

    local path = self.named_routes[name]
    if not path then
      return nil
    end

    if query ~= nil then
      local query_string = encode_query_string(query)
      if query_string ~= "" then
        path = path .. "?" .. query_string
      end
    end

    return path
  end,

  match = function(self, name, url, handler)
    return self:add(name, url, handler)
  end,

  registerRoutes = function(self, target)
    local mt = getmetatable(target)
    if not mt then return end
    for k, v in pairs(mt) do
      if type(k) == "table" then
        for _k, _v in pairs(k) do
          if type(_v) == "string" and string.byte(_v, 1) == 47 and type(v) == "function" then
            self:add(_k, _v, v)
          end
        end
      elseif type(k) == "string" and string.byte(k, 1) == 47 and type(v) == "function" then
        self:add(k, k, v)
      end
    end
  end,

  resolve = function(self, req)
    local path = req
    if type(req) == "table" then
      path = req.path or req.url or req.route
    end
    if type(path) ~= "string" then return nil end
    return self.routes[path]
  end,

  dispatch = function(self, req, ...)
    local route = self:resolve(req)
    if not route then return nil end
    local owner = self.owner or self
    return route.handler(owner, req, ...)
  end,
}

return Router
