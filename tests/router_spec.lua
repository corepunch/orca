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

-- Run all
test_dispatch_calls_correct_handler()
test_dispatch_nil_for_unknown_route()
test_resolve_returns_route()
test_resolve_nil_for_unknown()
test_register_routes_auto_discovery()
test_multiple_routes()

-- ---------------------------------------------------------------------------
-- Test 7: url_for returns the URL for a named route
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
-- Test 8: url_for passes through a path string unchanged
-- ---------------------------------------------------------------------------
local function test_url_for_passthrough()
  local owner = {}
  local router = Router(owner)

  test.expect_eq(router:url_for("/dashboard"), "/dashboard", "url_for('/dashboard') should return path unchanged")
  print("PASS: test_url_for_passthrough")
end

test_url_for_named()
test_url_for_passthrough()

print("All router tests passed.")
