local test = require "orca.test"
-- Headless tests for orca.core.router
-- Covers: route registration, dispatch to correct handler,
-- nil dispatch for unknown routes, and auto-discovery of "/" routes.

local Router = require "orca.core.router"

-- ---------------------------------------------------------------------------
-- Test 1: dispatch calls the correct handler for a registered route
-- ---------------------------------------------------------------------------
local function test_dispatch_calls_correct_handler()
  local owner = {}
  local router = Router(owner)
  local called_with = nil
  local handler = function(self, req)
    called_with = req
    return "result"
  end
  router:add("/", "/", handler)

  local result = router:dispatch("/")
  test.expect_eq(called_with, "/", "handler should receive the request")
  test.expect_eq(result, "result", "dispatch should return handler return value")
  print("PASS: test_dispatch_calls_correct_handler")
end

-- ---------------------------------------------------------------------------
-- Test 2: dispatch returns nil for an unregistered route
-- ---------------------------------------------------------------------------
local function test_dispatch_nil_for_unknown_route()
  local owner = {}
  local router = Router(owner)
  router:add("/home", "/home", function() return "home" end)

  local result = router:dispatch("/unknown")
  test.expect_eq(result, nil, "dispatch should return nil for unknown route")
  print("PASS: test_dispatch_nil_for_unknown_route")
end

-- ---------------------------------------------------------------------------
-- Test 3: resolve returns the route table for a known path
-- ---------------------------------------------------------------------------
local function test_resolve_returns_route()
  local owner = {}
  local router = Router(owner)
  local handler = function() end
  router:add("home", "/home", handler)

  local route = router:resolve("/home")
  test.expect(route ~= nil, "resolve should find registered route")
  test.expect_eq(route.handler, handler, "resolve should return correct handler")
  test.expect_eq(route.name, "home", "resolve should return correct name")
  test.expect_eq(route.url, "/home", "resolve should return the route pattern")
  print("PASS: test_resolve_returns_route")
end

-- ---------------------------------------------------------------------------
-- Test 4: resolve returns nil for unknown path
-- ---------------------------------------------------------------------------
local function test_resolve_nil_for_unknown()
  local owner = {}
  local router = Router(owner)
  router:add("/a", "/a", function() end)

  test.expect_eq(router:resolve("/b"), nil, "resolve should return nil for unknown path")
  print("PASS: test_resolve_nil_for_unknown")
end

-- ---------------------------------------------------------------------------
-- Test 5: auto-discovery of "/" routes from a class-like metatable
-- ---------------------------------------------------------------------------
local function test_register_routes_auto_discovery()
  local root_handler  = function(self) return "root"  end
  local about_handler = function(self) return "about" end

  local base = {
    ["/"]       = root_handler,
    ["/about"]  = about_handler,
    other       = function() return "should not be a route" end,
  }
  local instance = setmetatable({}, base)

  local router = Router(instance)
  test.expect_eq(router:dispatch("/"),       "root",  "/ route should dispatch")
  test.expect_eq(router:dispatch("/about"),  "about", "/about route should dispatch")
  test.expect_eq(router:dispatch("/other"),  nil,     "non-/ key should not be a route")
  print("PASS: test_register_routes_auto_discovery")
end

-- ---------------------------------------------------------------------------
-- Test 6: multiple routes coexist independently
-- ---------------------------------------------------------------------------
local function test_multiple_routes()
  local owner = {}
  local router = Router(owner)
  router:add("/a", "/a", function(self) return "a" end)
  router:add("/b", "/b", function(self) return "b" end)

  test.expect_eq(router:dispatch("/a"), "a", "/a route")
  test.expect_eq(router:dispatch("/b"), "b", "/b route")
  print("PASS: test_multiple_routes")
end

-- ---------------------------------------------------------------------------
-- Test 7: dispatch sets owner.params from dynamic route captures
-- ---------------------------------------------------------------------------
local function test_dispatch_dynamic_params()
  local owner = {}
  local router = Router(owner)
  router:add("adventure", "/adventure/:game/:session", function(self)
    return self.params.game .. ":" .. self.params.session
  end)

  test.expect_eq(router:dispatch("/adventure/zork1/s1"), "zork1:s1", "dispatch should capture dynamic route params")
  print("PASS: test_dispatch_dynamic_params")
end

-- Run all
test_dispatch_calls_correct_handler()
test_dispatch_nil_for_unknown_route()
test_resolve_returns_route()
test_resolve_nil_for_unknown()
test_register_routes_auto_discovery()
test_multiple_routes()
test_dispatch_dynamic_params()

-- ---------------------------------------------------------------------------
-- Test 8: url_for returns the URL for a named route
-- ---------------------------------------------------------------------------
local function test_url_for_named()
  local owner = {}
  local router = Router(owner)
  router:add("home", "/", function() end)
  router:add("about", "/about", function() end)

  test.expect_eq(router:url_for("home"), "/", "url_for('home') should return '/'")
  test.expect_eq(router:url_for("about"), "/about", "url_for('about') should return '/about'")
  print("PASS: test_url_for_named")
end

-- ---------------------------------------------------------------------------
-- Test 9: url_for interpolates params into named route paths
-- ---------------------------------------------------------------------------
local function test_url_for_named_with_params()
  local owner = {}
  local router = Router(owner)
  router:add("adventure", "/adventure/:game/:session", function() end)

  test.expect_eq(router:url_for("adventure", { game = "zork1", session = "abc123" }), "/adventure/zork1/abc123", "url_for should interpolate named route params")
  test.expect_eq(router:url_for("adventure", { game = "zork1" }), nil, "url_for should return nil when params are missing")
  print("PASS: test_url_for_named_with_params")
end

-- ---------------------------------------------------------------------------
-- Test 10: url_for interpolates params into direct path templates
-- ---------------------------------------------------------------------------
local function test_url_for_passthrough()
  local owner = {}
  local router = Router(owner)

  test.expect_eq(router:url_for("/dashboard"), "/dashboard", "url_for('/dashboard') should return path unchanged")
  test.expect_eq(router:url_for("/adventure/:game/:session", { game = "zork1", session = "s1" }), "/adventure/zork1/s1", "url_for should interpolate params in direct path templates")
  print("PASS: test_url_for_passthrough")
end

-- ---------------------------------------------------------------------------
-- Test 11: url_for returns nil for an undefined route name
-- ---------------------------------------------------------------------------
local function test_url_for_unknown_returns_nil()
  local owner = {}
  local router = Router(owner)
  router:add("home", "/", function() end)

  test.expect_eq(router:url_for("missing"), nil, "url_for should return nil for undefined route names")
  print("PASS: test_url_for_unknown_returns_nil")
end

-- ---------------------------------------------------------------------------
-- Test 12: url_for selects fillable pattern when a route name is reused
-- ---------------------------------------------------------------------------
local function test_url_for_named_reused_patterns()
  local owner = {}
  local router = Router(owner)
  router:add("Adventure", "/adventure/:game", function() end)
  router:add("Adventure", "/adventure/:game/:session", function() end)

  test.expect_eq(router:url_for("Adventure", { game = "zork1" }), "/adventure/zork1", "url_for should pick the one-segment pattern when only game is provided")
  test.expect_eq(router:url_for("Adventure", { game = "zork1", session = "s1" }), "/adventure/zork1/s1", "url_for should pick the two-segment pattern when session is provided")
  print("PASS: test_url_for_named_reused_patterns")
end

test_url_for_named()
test_url_for_named_with_params()
test_url_for_passthrough()
test_url_for_unknown_returns_nil()
test_url_for_named_reused_patterns()

print("All router tests passed.")
