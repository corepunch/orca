local test = require "orca.test"
-- Headless tests for the orca.parsers.json module.
-- Covers: parse/decode of numbers, strings, booleans, null, arrays, and
-- nested objects; encode round-trips for arrays and objects.
-- No renderer or display required.
--
-- Run with: $(TARGET) -test=tests/test_parsers.lua

local json = require "orca.parsers.json"

-- ---------------------------------------------------------------------------
-- parse: numbers
-- ---------------------------------------------------------------------------
local function test_parse_number()
  local n = json.parse("42")
  test.expect_eq(type(n), "number", "parse integer: type is number")
  test.expect_eq(n, 42, "parse integer: value is 42")

  local f = json.parse("3.14")
  test.expect_eq(type(f), "number", "parse float: type is number")
  test.expect(math.abs(f - 3.14) < 0.001, "parse float: value ≈ 3.14")

  local neg = json.parse("-7")
  test.expect_eq(neg, -7, "parse negative integer: value is -7")
  print("PASS: test_parse_number")
end

-- ---------------------------------------------------------------------------
-- parse: string values
-- ---------------------------------------------------------------------------
local function test_parse_string()
  local s = json.parse('"hello"')
  test.expect_eq(type(s), "string", "parse string: type is string")
  test.expect_eq(s, "hello", "parse string: value is 'hello'")

  local empty = json.parse('""')
  test.expect_eq(empty, "", "parse empty string: value is ''")
  print("PASS: test_parse_string")
end

-- ---------------------------------------------------------------------------
-- parse: booleans
-- ---------------------------------------------------------------------------
local function test_parse_boolean()
  local t = json.parse("true")
  test.expect_eq(type(t), "boolean", "parse true: type is boolean")
  test.expect(t == true, "parse true: value is true")

  local f = json.parse("false")
  test.expect_eq(type(f), "boolean", "parse false: type is boolean")
  test.expect(f == false, "parse false: value is false")
  print("PASS: test_parse_boolean")
end

-- ---------------------------------------------------------------------------
-- parse: null → nil
-- ---------------------------------------------------------------------------
local function test_parse_null()
  local n = json.parse("null")
  test.expect(n == nil, "parse null: value is nil")
  print("PASS: test_parse_null")
end

-- ---------------------------------------------------------------------------
-- parse: flat array
-- ---------------------------------------------------------------------------
local function test_parse_array()
  local arr = json.parse('[1, 2, 3]')
  test.expect_eq(type(arr), "table", "parse array: type is table")
  test.expect_eq(#arr, 3, "parse array: length is 3")
  test.expect_eq(arr[1], 1, "parse array: arr[1] = 1")
  test.expect_eq(arr[2], 2, "parse array: arr[2] = 2")
  test.expect_eq(arr[3], 3, "parse array: arr[3] = 3")
  print("PASS: test_parse_array")
end

-- ---------------------------------------------------------------------------
-- parse: flat object
-- ---------------------------------------------------------------------------
local function test_parse_object()
  local obj = json.parse('{"name":"Alice","age":30}')
  test.expect_eq(type(obj), "table", "parse object: type is table")
  test.expect_eq(obj.name, "Alice", "parse object: name = 'Alice'")
  test.expect_eq(obj.age, 30, "parse object: age = 30")
  print("PASS: test_parse_object")
end

-- ---------------------------------------------------------------------------
-- parse: nested object
-- ---------------------------------------------------------------------------
local function test_parse_nested()
  local data = json.parse('{"user":{"id":1,"active":true},"tags":["a","b"]}')
  test.expect_eq(type(data), "table", "parse nested: type is table")
  test.expect_eq(type(data.user), "table", "parse nested: user is a table")
  test.expect_eq(data.user.id, 1, "parse nested: user.id = 1")
  test.expect(data.user.active == true, "parse nested: user.active = true")
  test.expect_eq(type(data.tags), "table", "parse nested: tags is a table")
  test.expect_eq(data.tags[1], "a", "parse nested: tags[1] = 'a'")
  test.expect_eq(data.tags[2], "b", "parse nested: tags[2] = 'b'")
  print("PASS: test_parse_nested")
end

-- ---------------------------------------------------------------------------
-- decode: alias for parse
-- ---------------------------------------------------------------------------
local function test_decode_alias()
  local v = json.decode("99")
  test.expect_eq(v, 99, "decode is an alias for parse")
  print("PASS: test_decode_alias")
end

-- ---------------------------------------------------------------------------
-- encode: array round-trip
-- ---------------------------------------------------------------------------
local function test_encode_array()
  local src = {10, 20, 30}
  local encoded = json.encode(src)
  test.expect_eq(type(encoded), "string", "encode array: result is string")

  -- Round-trip: decode the encoded string and verify values
  local decoded = json.parse(encoded)
  test.expect_eq(type(decoded), "table", "encode→decode array: is a table")
  test.expect_eq(#decoded, 3, "encode→decode array: length is 3")
  test.expect_eq(decoded[1], 10, "encode→decode array: [1] = 10")
  test.expect_eq(decoded[2], 20, "encode→decode array: [2] = 20")
  test.expect_eq(decoded[3], 30, "encode→decode array: [3] = 30")
  print("PASS: test_encode_array")
end

-- ---------------------------------------------------------------------------
-- encode: object round-trip
-- ---------------------------------------------------------------------------
local function test_encode_object()
  local src = { city = "London", population = 9000000 }
  local encoded = json.encode(src)
  test.expect_eq(type(encoded), "string", "encode object: result is string")

  local decoded = json.parse(encoded)
  test.expect_eq(type(decoded), "table", "encode→decode object: is a table")
  test.expect_eq(decoded.city, "London", "encode→decode object: city = 'London'")
  test.expect_eq(decoded.population, 9000000, "encode→decode object: population = 9000000")
  print("PASS: test_encode_object")
end

-- ---------------------------------------------------------------------------
-- encode: boolean values
-- ---------------------------------------------------------------------------
local function test_encode_boolean()
  local src = { active = true, deleted = false }
  local encoded = json.encode(src)
  local decoded = json.parse(encoded)
  test.expect(decoded.active == true,  "encode→decode: active = true")
  test.expect(decoded.deleted == false, "encode→decode: deleted = false")
  print("PASS: test_encode_boolean")
end

-- ---------------------------------------------------------------------------
-- encode: nested table
-- ---------------------------------------------------------------------------
local function test_encode_nested()
  local src = { meta = { version = 2 }, items = { "x", "y" } }
  local encoded = json.encode(src)
  local decoded = json.parse(encoded)
  test.expect_eq(type(decoded.meta), "table", "encode→decode nested: meta is table")
  test.expect_eq(decoded.meta.version, 2, "encode→decode nested: meta.version = 2")
  test.expect_eq(decoded.items[1], "x", "encode→decode nested: items[1] = 'x'")
  test.expect_eq(decoded.items[2], "y", "encode→decode nested: items[2] = 'y'")
  print("PASS: test_encode_nested")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_parse_number()
test_parse_string()
test_parse_boolean()
test_parse_null()
test_parse_array()
test_parse_object()
test_parse_nested()
test_decode_alias()
test_encode_array()
test_encode_object()
test_encode_boolean()
test_encode_nested()

print("All JSON parser tests passed.")
