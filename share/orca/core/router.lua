local Widget = require "orca.core.widget"

local Router = Widget:extend {
  __init = function(self, owner)
    self.owner = owner
    self.routes = {}
    self:registerRoutes(owner)
  end,

  add = function(self, name, url, handler)
    assert(type(url) == "string", "route url must be a string")
    assert(type(handler) == "function", "route handler must be a function")
    self.routes[url] = {
      name = name or url,
      handler = handler
    }
    return handler
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
