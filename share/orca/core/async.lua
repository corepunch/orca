-- share/orca/core/async.lua
-- Lua-level async helpers for orca.async.
--
-- orca.async (the C primitive) creates a coroutine and posts
-- kEventResumeCoroutine; if the body throws, the error goes to stderr and
-- the coroutine silently dies.  This module wraps that primitive with
-- pcall-based error handling and provides Lapis-inspired helpers for
-- ergonomic async control flow.
--
-- Usage:
--   local async = require "orca.core.async"
--   async.run(function() ... end)          -- safe, errors routed to handler
--   async.set_error_handler(function(err) ... end)
--
--   local handler = async.capture_errors(function(self, req)
--     if bad then async.yield_error("oops") end
--     return "ok"
--   end)
--   local result = handler(self, req)
--   if result and result.error then ... end
--
--   local handler = async.respond_to {
--     GET  = function(self, req) return "hello" end,
--     POST = function(self, req) return "created" end,
--   }

local orca = require "orca"

-- Save the original C-level primitive so we can wrap it without recursion.
local _raw_async = orca.async

local M = {}

-- Default error handler: print to stderr.
local _error_handler = function(err)
  io.stderr:write("async error: " .. tostring(err) .. "\n")
end

-- Set a custom error handler invoked when an async.run body throws.
-- Pass nil to reset to the default stderr handler.
function M.set_error_handler(fn)
  if fn == nil then
    _error_handler = function(err)
      io.stderr:write("async error: " .. tostring(err) .. "\n")
    end
  else
    assert(type(fn) == "function", "error handler must be a function")
    _error_handler = fn
  end
end

-- async.run(fn, ...) — start fn in a coroutine via orca.async.
-- Unlike calling orca.async directly, any error thrown by fn is caught by
-- pcall and routed to the registered error handler instead of silently
-- dying with a bare stderr message.
function M.run(fn, ...)
  assert(type(fn) == "function", "async.run: expected a function")
  local nargs = select("#", ...)
  local args = { ... }
  _raw_async(function()
    local ok, err = pcall(fn, table.unpack(args, 1, nargs))
    if not ok then
      _error_handler(err)
    end
  end)
end

-- Sentinel type used to distinguish yield_error throws from plain errors.
local _YIELD_ERROR = {}

-- yield_error(msg) — raise a structured error that capture_errors will
-- catch and return as { error = msg }.  Use this inside a capture_errors
-- body to signal application-level failures without a full stack trace.
function M.yield_error(msg)
  error(setmetatable({ message = msg }, { __index = _YIELD_ERROR }), 2)
end

-- capture_errors(fn) — return a wrapper around fn(self, ...) that:
--   • catches yield_error() throws and returns { error = msg }
--   • re-raises any other (unexpected) errors
-- Inspired by the Lapis capture_errors helper.
function M.capture_errors(fn)
  assert(type(fn) == "function", "capture_errors: expected a function")
  return function(self, ...)
    local ok, result = pcall(fn, self, ...)
    if ok then
      return result
    end
    -- Check if it's a structured yield_error
    if type(result) == "table" and getmetatable(result) and
       getmetatable(result).__index == _YIELD_ERROR then
      return { error = result.message }
    end
    -- Unexpected error: re-raise with the original message
    error(result, 2)
  end
end

-- respond_to(handlers) — return a dispatcher function that calls the
-- correct handler based on req.method (defaults to "GET").
-- Inspired by Lapis respond_to.
-- Example:
--   ["/submit"] = async.respond_to {
--     GET  = function(self, req) return render_form() end,
--     POST = function(self, req) return process_form(req) end,
--   }
function M.respond_to(handlers)
  assert(type(handlers) == "table", "respond_to: expected a table of handlers")
  return function(self, req, ...)
    local method = (type(req) == "table" and type(req.method) == "string")
                   and req.method:upper() or "GET"
    local handler = handlers[method]
    if not handler then
      return { error = "method not allowed: " .. method }
    end
    return handler(self, req, ...)
  end
end

return M
