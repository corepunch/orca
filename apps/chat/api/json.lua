-- Strict JSON at the remote service boundary. No native modules are required,
-- so injected HTTP transports use exactly the same wire format in tests.
local M = {}
local escapes = { ['"'] = '\\"', ['\\'] = '\\\\', ['\b'] = '\\b', ['\f'] = '\\f',
  ['\n'] = '\\n', ['\r'] = '\\r', ['\t'] = '\\t' }

local function quote(value)
  return '"' .. value:gsub('[%z\1-\31\\"]', function(c)
    return escapes[c] or string.format("\\u%04x", c:byte())
  end) .. '"'
end

function M.encode(value)
  local seen = {}
  local function encode(item, depth)
    if depth > 128 then error("JSON nesting limit exceeded", 0) end
    local kind = type(item)
    if kind == "nil" then return "null"
    elseif kind == "string" then return quote(item)
    elseif kind == "boolean" then return item and "true" or "false"
    elseif kind == "number" then
      if item ~= item or item == math.huge or item == -math.huge then error("Invalid JSON number", 0) end
      return string.format("%.17g", item)
    elseif kind ~= "table" then error("Unsupported JSON value", 0) end
    if seen[item] then error("Cyclic JSON value", 0) end
    seen[item] = true
    local fields, count, array = {}, 0, #item > 0
    for key in pairs(item) do
      count = count + 1
      if type(key) ~= "number" or key % 1 ~= 0 or key < 1 or key > #item then array = false end
    end
    if array and count ~= #item then array = false end
    if array then
      for _, child in ipairs(item) do fields[#fields + 1] = encode(child, depth + 1) end
    else
      for key, child in pairs(item) do
        if type(key) ~= "string" then error("JSON object keys must be strings", 0) end
        fields[#fields + 1] = quote(key) .. ":" .. encode(child, depth + 1)
      end
    end
    seen[item] = nil
    return (array and "[" or "{") .. table.concat(fields, ",") .. (array and "]" or "}")
  end
  return encode(value, 0)
end

local function utf8(code)
  if code < 128 then return string.char(code) end
  if code < 2048 then return string.char(192 + math.floor(code / 64), 128 + code % 64) end
  if code < 65536 then
    return string.char(224 + math.floor(code / 4096), 128 + math.floor(code / 64) % 64, 128 + code % 64)
  end
  return string.char(240 + math.floor(code / 262144), 128 + math.floor(code / 4096) % 64,
    128 + math.floor(code / 64) % 64, 128 + code % 64)
end

function M.decode(source)
  if type(source) ~= "string" then error("Expected JSON string", 0) end
  local pos, length = 1, #source
  local function fail() error("Invalid JSON at byte " .. pos, 0) end
  local function skip()
    while source:sub(pos, pos):match("[ \n\r\t]") do pos = pos + 1 end
  end
  local function hex()
    local digits = source:sub(pos, pos + 3)
    if #digits ~= 4 or not digits:match("^%x%x%x%x$") then fail() end
    pos = pos + 4
    return tonumber(digits, 16)
  end
  local function stringValue()
    pos = pos + 1
    local parts, start = {}, pos
    while pos <= length do
      local c = source:sub(pos, pos)
      if c == '"' then
        parts[#parts + 1] = source:sub(start, pos - 1)
        pos = pos + 1
        return table.concat(parts)
      elseif c == "\\" then
        parts[#parts + 1] = source:sub(start, pos - 1)
        pos = pos + 1
        c = source:sub(pos, pos)
        local map = { ['"'] = '"', ["\\"] = "\\", ["/"] = "/", b = "\b", f = "\f", n = "\n", r = "\r", t = "\t" }
        if c == "u" then
          pos = pos + 1
          local code = hex()
          if code >= 55296 and code <= 56319 then
            if source:sub(pos, pos + 1) ~= "\\u" then fail() end
            pos = pos + 2
            local low = hex()
            if low < 56320 or low > 57343 then fail() end
            code = 65536 + (code - 55296) * 1024 + low - 56320
          elseif code >= 56320 and code <= 57343 then fail() end
          parts[#parts + 1] = utf8(code)
        else
          if not map[c] then fail() end
          parts[#parts + 1], pos = map[c], pos + 1
        end
        start = pos
      else
        if c:byte() < 32 then fail() end
        pos = pos + 1
      end
    end
    fail()
  end
  local parse
  parse = function(depth)
    if depth > 128 then fail() end
    skip()
    local c = source:sub(pos, pos)
    if c == '"' then return stringValue() end
    if c == "{" or c == "[" then
      local object, result, index = c == "{", {}, 1
      local close = object and "}" or "]"
      pos = pos + 1
      skip()
      if source:sub(pos, pos) == close then pos = pos + 1; return result end
      while true do
        local key = index
        if object then
          if source:sub(pos, pos) ~= '"' then fail() end
          key = stringValue()
          skip()
          if source:sub(pos, pos) ~= ":" then fail() end
          pos = pos + 1
        end
        result[key] = parse(depth + 1)
        index = index + 1
        skip()
        c = source:sub(pos, pos)
        pos = pos + 1
        if c == close then return result end
        if c ~= "," then fail() end
        skip()
      end
    end
    for literal, value in pairs { ["true"] = true, ["false"] = false } do
      if source:sub(pos, pos + #literal - 1) == literal then pos = pos + #literal; return value end
    end
    if source:sub(pos, pos + 3) == "null" then pos = pos + 4; return nil end
    local number = source:sub(pos):match("^%-?%d+%.?%d*[eE]?[%+%-]?%d*")
    if not number then fail() end
    -- Lua patterns have no alternation; validate each numeric component.
    local mantissa, exponent = number:match("^(.-)[eE](.*)$")
    mantissa = mantissa or number
    local unsigned = mantissa:gsub("^%-", "")
    if unsigned:match("^0%d") or not (unsigned:match("^%d+$") or unsigned:match("^%d+%.%d+$"))
        or (exponent and not exponent:match("^[%+%-]?%d+$")) then fail() end
    local value = tonumber(number)
    if not value or value == math.huge or value == -math.huge then fail() end
    pos = pos + #number
    return value
  end
  local result = parse(0)
  skip()
  if pos <= length then fail() end
  return result
end

return M
