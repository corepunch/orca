test = require "orca.test"
-- Headless tests for orca.core2.router
-- Covers: route registration, dispatch to correct handler,
-- nil dispatch for unknown routes, and auto-discovery of "/" routes.
--
-- Run via: $(TARGET) -test=tests/run_core2_specs.lua

Router = require "orca.core2.router"

-- ---------------------------------------------------------------------------
-- Test 1: dispatch calls the correct handler for a registered route
-- ---------------------------------------------------------------------------
test_dispatch_calls_correct_handler = ->
  owner = {}
  router = Router owner
  called_with = nil
  handler = (self, req) ->
    called_with = req
    "result"
  router\add "/", "/", handler

  result = router\dispatch "/"
  test.expect_eq called_with, "/", "handler should receive the request"
  test.expect_eq result, "result", "dispatch should return handler return value"
  print "PASS: test_dispatch_calls_correct_handler"

-- ---------------------------------------------------------------------------
-- Test 2: dispatch returns nil for an unregistered route
-- ---------------------------------------------------------------------------
test_dispatch_nil_for_unknown_route = ->
  owner = {}
  router = Router owner
  router\add "/home", "/home", -> "home"

  result = router\dispatch "/unknown"
  test.expect_eq result, nil, "dispatch should return nil for unknown route"
  print "PASS: test_dispatch_nil_for_unknown_route"

-- ---------------------------------------------------------------------------
-- Test 3: resolve returns the route table for a known path
-- ---------------------------------------------------------------------------
test_resolve_returns_route = ->
  owner = {}
  router = Router owner
  handler = ->
  router\add "home", "/home", handler

  route = router\resolve "/home"
  test.expect route ~= nil, "resolve should find registered route"
  test.expect_eq route.handler, handler, "resolve should return correct handler"
  test.expect_eq route.name, "home", "resolve should return correct name"
  print "PASS: test_resolve_returns_route"

-- ---------------------------------------------------------------------------
-- Test 4: resolve returns nil for unknown path
-- ---------------------------------------------------------------------------
test_resolve_nil_for_unknown = ->
  owner = {}
  router = Router owner
  router\add "/a", "/a", ->

  test.expect_eq (router\resolve "/b"), nil, "resolve should return nil for unknown path"
  print "PASS: test_resolve_nil_for_unknown"

-- ---------------------------------------------------------------------------
-- Test 5: auto-discovery of "/" routes from a class-like metatable
-- ---------------------------------------------------------------------------
test_register_routes_auto_discovery = ->
  root_handler = (self) -> "root"
  about_handler = (self) -> "about"

  -- Simulate a MoonScript-style app instance: "/" keys on the metatable
  base = {
    "/":      root_handler,
    "/about": about_handler,
    other:    -> "should not be a route",
  }
  instance = setmetatable {}, base

  router = Router instance
  test.expect_eq (router\dispatch "/"),      "root",  "/ route should dispatch"
  test.expect_eq (router\dispatch "/about"), "about", "/about route should dispatch"
  test.expect_eq (router\dispatch "/other"), nil,     "non-/ key should not be a route"
  print "PASS: test_register_routes_auto_discovery"

-- ---------------------------------------------------------------------------
-- Test 6: multiple routes coexist independently
-- ---------------------------------------------------------------------------
test_multiple_routes = ->
  owner = {}
  router = Router owner
  router\add "/a", "/a", (self) -> "a"
  router\add "/b", "/b", (self) -> "b"

  test.expect_eq (router\dispatch "/a"), "a", "/a route"
  test.expect_eq (router\dispatch "/b"), "b", "/b route"
  print "PASS: test_multiple_routes"

-- Run all
test_dispatch_calls_correct_handler!
test_dispatch_nil_for_unknown_route!
test_resolve_returns_route!
test_resolve_nil_for_unknown!
test_register_routes_auto_discovery!
test_multiple_routes!
