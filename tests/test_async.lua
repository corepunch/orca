local test = require "orca.test"
-- Headless tests for share/orca/core/async.lua
-- Tests the Lua-level async helpers: capture_errors, yield_error, respond_to,
-- set_error_handler, and async.run (stubbing orca.async for synchronous testing).
--
-- Run via: $(TARGET) -test=tests/test_async.lua

local orca = require "orca"

-- Stub orca.async for synchronous headless testing:
-- Instead of posting a coroutine event, just call fn() directly so we can
-- assert outcomes without needing a running event loop.
local original_raw_async = orca.async
orca.async = function(fn, ...) fn(...) end

local async = require "orca.core.async"

-- Restore after the module was loaded (async.lua captures the stub above).
-- We keep the stub in place for the duration of the tests.

-- ---------------------------------------------------------------------------
-- Test 1: async.run calls the function
-- ---------------------------------------------------------------------------
local function test_run_calls_function()
  local called = false
  async.run(function() called = true end)
  test.expect(called, "async.run should call the function")
  print("PASS: test_run_calls_function")
end

-- ---------------------------------------------------------------------------
-- Test 2: async.run routes errors to the error handler (not re-raises)
-- ---------------------------------------------------------------------------
local function test_run_routes_errors()
  local captured_err = nil
  async.set_error_handler(function(err) captured_err = err end)

  -- This should NOT propagate the error upward
  local ok = pcall(function()
    async.run(function() error("boom") end)
  end)

  async.set_error_handler(nil)  -- reset to default

  test.expect(ok, "async.run should not propagate errors to caller")
  test.expect(captured_err ~= nil, "error handler should have been called")
  test.expect(captured_err:find("boom") ~= nil, "error message should contain 'boom'")
  print("PASS: test_run_routes_errors")
end

-- ---------------------------------------------------------------------------
-- Test 3: async.run passes arguments to the function
-- ---------------------------------------------------------------------------
local function test_run_passes_args()
  local got_a, got_b
  async.run(function(a, b) got_a = a; got_b = b end, "hello", 42)
  test.expect_eq(got_a, "hello", "first arg should be 'hello'")
  test.expect_eq(got_b, 42, "second arg should be 42")
  print("PASS: test_run_passes_args")
end

-- ---------------------------------------------------------------------------
-- Test 4: set_error_handler with nil resets to default (no crash)
-- ---------------------------------------------------------------------------
local function test_set_error_handler_nil_resets()
  async.set_error_handler(function(err) end)  -- suppress output
  async.set_error_handler(nil)                 -- reset to default
  -- Default handler writes to stderr — just ensure it doesn't crash
  local ok = pcall(function()
    async.run(function() error("test_reset") end)
  end)
  test.expect(ok, "reset error handler should not crash")
  print("PASS: test_set_error_handler_nil_resets")
end

-- ---------------------------------------------------------------------------
-- Test 5: capture_errors — success path returns fn's return value
-- ---------------------------------------------------------------------------
local function test_capture_errors_success()
  local handler = async.capture_errors(function(self, req)
    return "success"
  end)
  local result = handler({}, {})
  test.expect_eq(result, "success", "capture_errors success should return fn result")
  print("PASS: test_capture_errors_success")
end

-- ---------------------------------------------------------------------------
-- Test 6: capture_errors — yield_error returns { error = msg }
-- ---------------------------------------------------------------------------
local function test_capture_errors_yield_error()
  local handler = async.capture_errors(function(self, req)
    async.yield_error("invalid input")
    return "never reached"
  end)
  local result = handler({}, {})
  test.expect(type(result) == "table", "yield_error result should be a table")
  test.expect_eq(result.error, "invalid input", "result.error should carry the message")
  print("PASS: test_capture_errors_yield_error")
end

-- ---------------------------------------------------------------------------
-- Test 7: capture_errors — unexpected errors are re-raised
-- ---------------------------------------------------------------------------
local function test_capture_errors_reraises_unexpected()
  local handler = async.capture_errors(function(self, req)
    error("unexpected crash")
  end)
  local ok, err = pcall(handler, {}, {})
  test.expect(not ok, "unexpected errors should be re-raised")
  test.expect(err:find("unexpected crash") ~= nil, "re-raised error should contain original message")
  print("PASS: test_capture_errors_reraises_unexpected")
end

-- ---------------------------------------------------------------------------
-- Test 8: respond_to — dispatches GET by default
-- ---------------------------------------------------------------------------
local function test_respond_to_default_get()
  local handler = async.respond_to {
    GET  = function(self, req) return "get_response" end,
    POST = function(self, req) return "post_response" end,
  }
  -- No method in req → defaults to GET
  local result = handler({}, {})
  test.expect_eq(result, "get_response", "respond_to with no method should use GET")
  print("PASS: test_respond_to_default_get")
end

-- ---------------------------------------------------------------------------
-- Test 9: respond_to — dispatches by req.method
-- ---------------------------------------------------------------------------
local function test_respond_to_dispatch_by_method()
  local handler = async.respond_to {
    GET  = function(self, req) return "get" end,
    POST = function(self, req) return "post" end,
  }
  test.expect_eq(handler({}, { method = "GET"  }), "get",  "GET dispatch")
  test.expect_eq(handler({}, { method = "POST" }), "post", "POST dispatch")
  print("PASS: test_respond_to_dispatch_by_method")
end

-- ---------------------------------------------------------------------------
-- Test 10: respond_to — method matching is case-insensitive
-- ---------------------------------------------------------------------------
local function test_respond_to_case_insensitive()
  local handler = async.respond_to {
    DELETE = function(self, req) return "deleted" end,
  }
  local result = handler({}, { method = "delete" })
  test.expect_eq(result, "deleted", "respond_to should normalise method to uppercase")
  print("PASS: test_respond_to_case_insensitive")
end

-- ---------------------------------------------------------------------------
-- Test 11: respond_to — unknown method returns { error = ... }
-- ---------------------------------------------------------------------------
local function test_respond_to_unknown_method()
  local handler = async.respond_to {
    GET = function(self, req) return "ok" end,
  }
  local result = handler({}, { method = "PATCH" })
  test.expect(type(result) == "table", "unknown method should return a table")
  test.expect(result.error ~= nil, "unknown method should set result.error")
  print("PASS: test_respond_to_unknown_method")
end

-- ---------------------------------------------------------------------------
-- Test 12: yield_error inside respond_to + capture_errors pipeline
-- ---------------------------------------------------------------------------
local function test_pipeline_yield_error()
  local handler = async.capture_errors(async.respond_to {
    POST = function(self, req)
      if not req.body then
        async.yield_error("missing body")
      end
      return "ok"
    end,
  })
  local result = handler({}, { method = "POST" })
  test.expect_eq(result.error, "missing body", "pipeline should propagate yield_error")
  print("PASS: test_pipeline_yield_error")
end

-- Run all
test_run_calls_function()
test_run_routes_errors()
test_run_passes_args()
test_set_error_handler_nil_resets()
test_capture_errors_success()
test_capture_errors_yield_error()
test_capture_errors_reraises_unexpected()
test_respond_to_default_get()
test_respond_to_dispatch_by_method()
test_respond_to_case_insensitive()
test_respond_to_unknown_method()
test_pipeline_yield_error()

-- Restore orca.async
orca.async = original_raw_async

print("All async tests passed.")
