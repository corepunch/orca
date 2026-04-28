local test = require "orca.test"
-- Headless tests for the orca.geometry module.
-- Covers Vector2D, Vector3D, Color component access and arithmetic operations.
-- No renderer or display required.
--
-- Run with: $(TARGET) -test=tests/test_geometry.lua

local g = require "orca.geometry"

-- ---------------------------------------------------------------------------
-- Vector2D: component access
-- ---------------------------------------------------------------------------
local function test_vec2_components()
  local v = g.Vector2D { X = 3.0, Y = 4.0 }
  test.expect_near(v.X, 3.0, 0.001, "vec2.X round-trip")
  test.expect_near(v.Y, 4.0, 0.001, "vec2.Y round-trip")
  print("PASS: test_vec2_components")
end

-- ---------------------------------------------------------------------------
-- Vector2D: addition
-- ---------------------------------------------------------------------------
local function test_vec2_add()
  local a = g.Vector2D { X = 1.0, Y = 2.0 }
  local b = g.Vector2D { X = 3.0, Y = 4.0 }
  local c = a + b
  test.expect_near(c.X, 4.0, 0.001, "vec2 add X")
  test.expect_near(c.Y, 6.0, 0.001, "vec2 add Y")
  print("PASS: test_vec2_add")
end

-- ---------------------------------------------------------------------------
-- Vector2D: subtraction
-- ---------------------------------------------------------------------------
local function test_vec2_sub()
  local a = g.Vector2D { X = 5.0, Y = 7.0 }
  local b = g.Vector2D { X = 2.0, Y = 3.0 }
  local c = a - b
  test.expect_near(c.X, 3.0, 0.001, "vec2 sub X")
  test.expect_near(c.Y, 4.0, 0.001, "vec2 sub Y")
  print("PASS: test_vec2_sub")
end

-- ---------------------------------------------------------------------------
-- Vector2D: scalar scale
-- ---------------------------------------------------------------------------
local function test_vec2_scale()
  local v = g.Vector2D { X = 2.0, Y = 3.0 }
  local s = v:scale(2.0)
  test.expect_near(s.X, 4.0, 0.001, "vec2 scale X")
  test.expect_near(s.Y, 6.0, 0.001, "vec2 scale Y")
  print("PASS: test_vec2_scale")
end

-- ---------------------------------------------------------------------------
-- Vector2D: dot product
-- ---------------------------------------------------------------------------
local function test_vec2_dot()
  local a = g.Vector2D { X = 1.0, Y = 0.0 }
  local b = g.Vector2D { X = 0.0, Y = 1.0 }
  test.expect_near(a:dot(b), 0.0, 0.001, "vec2 dot of perpendicular vectors is 0")

  local c = g.Vector2D { X = 2.0, Y = 3.0 }
  local d = g.Vector2D { X = 4.0, Y = 5.0 }
  test.expect_near(c:dot(d), 23.0, 0.001, "vec2 dot (2,3)·(4,5) = 23")
  print("PASS: test_vec2_dot")
end

-- ---------------------------------------------------------------------------
-- Vector2D: length (3-4-5 right triangle)
-- ---------------------------------------------------------------------------
local function test_vec2_len()
  local v = g.Vector2D { X = 3.0, Y = 4.0 }
  test.expect_near(v:len(), 5.0, 0.001, "vec2 len of (3,4) = 5")
  print("PASS: test_vec2_len")
end

-- ---------------------------------------------------------------------------
-- Vector2D: normalize → unit length (in-place; no return value)
-- ---------------------------------------------------------------------------
local function test_vec2_normalize()
  local v = g.Vector2D { X = 3.0, Y = 4.0 }
  v:normalize()
  test.expect_near(v:len(), 1.0, 0.001, "vec2 after normalize has unit length")
  print("PASS: test_vec2_normalize")
end

-- ---------------------------------------------------------------------------
-- Vector2D: lerp midpoint
-- ---------------------------------------------------------------------------
local function test_vec2_lerp()
  local a   = g.Vector2D { X = 0.0,  Y = 0.0  }
  local b   = g.Vector2D { X = 10.0, Y = 20.0 }
  local mid = a:lerp(b, 0.5)
  test.expect_near(mid.X, 5.0,  0.001, "vec2 lerp 0.5 X")
  test.expect_near(mid.Y, 10.0, 0.001, "vec2 lerp 0.5 Y")

  -- t=0 → a, t=1 → b
  local at0 = a:lerp(b, 0.0)
  local at1 = a:lerp(b, 1.0)
  test.expect_near(at0.X, 0.0,  0.001, "vec2 lerp t=0 X → a.X")
  test.expect_near(at1.X, 10.0, 0.001, "vec2 lerp t=1 X → b.X")
  print("PASS: test_vec2_lerp")
end

-- ---------------------------------------------------------------------------
-- Vector2D: unary negation
-- ---------------------------------------------------------------------------
local function test_vec2_unm()
  local v = g.Vector2D { X = 2.0, Y = -3.0 }
  local n = -v
  test.expect_near(n.X, -2.0, 0.001, "vec2 unm X")
  test.expect_near(n.Y,  3.0, 0.001, "vec2 unm Y")
  print("PASS: test_vec2_unm")
end

-- ---------------------------------------------------------------------------
-- Vector3D: component access
-- ---------------------------------------------------------------------------
local function test_vec3_components()
  local v = g.Vector3D { X = 1.0, Y = 2.0, Z = 3.0 }
  test.expect_near(v.X, 1.0, 0.001, "vec3.X round-trip")
  test.expect_near(v.Y, 2.0, 0.001, "vec3.Y round-trip")
  test.expect_near(v.Z, 3.0, 0.001, "vec3.Z round-trip")
  print("PASS: test_vec3_components")
end

-- ---------------------------------------------------------------------------
-- Vector3D: addition
-- ---------------------------------------------------------------------------
local function test_vec3_add()
  local a = g.Vector3D { X = 1.0, Y = 2.0, Z = 3.0 }
  local b = g.Vector3D { X = 4.0, Y = 5.0, Z = 6.0 }
  local c = a + b
  test.expect_near(c.X, 5.0, 0.001, "vec3 add X")
  test.expect_near(c.Y, 7.0, 0.001, "vec3 add Y")
  test.expect_near(c.Z, 9.0, 0.001, "vec3 add Z")
  print("PASS: test_vec3_add")
end

-- ---------------------------------------------------------------------------
-- Vector3D: subtraction
-- ---------------------------------------------------------------------------
local function test_vec3_sub()
  local a = g.Vector3D { X = 5.0, Y = 7.0, Z = 9.0 }
  local b = g.Vector3D { X = 1.0, Y = 2.0, Z = 3.0 }
  local c = a - b
  test.expect_near(c.X, 4.0, 0.001, "vec3 sub X")
  test.expect_near(c.Y, 5.0, 0.001, "vec3 sub Y")
  test.expect_near(c.Z, 6.0, 0.001, "vec3 sub Z")
  print("PASS: test_vec3_sub")
end

-- ---------------------------------------------------------------------------
-- Vector3D: dot product
-- ---------------------------------------------------------------------------
local function test_vec3_dot()
  local x = g.Vector3D { X = 1.0, Y = 0.0, Z = 0.0 }
  local y = g.Vector3D { X = 0.0, Y = 1.0, Z = 0.0 }
  test.expect_near(x:dot(y), 0.0, 0.001, "vec3 dot of perpendicular axes is 0")

  local a = g.Vector3D { X = 1.0, Y = 2.0, Z = 3.0 }
  local b = g.Vector3D { X = 4.0, Y = 5.0, Z = 6.0 }
  test.expect_near(a:dot(b), 32.0, 0.001, "vec3 dot (1,2,3)·(4,5,6) = 32")
  print("PASS: test_vec3_dot")
end

-- ---------------------------------------------------------------------------
-- Vector3D: cross product (X × Y = Z)
-- ---------------------------------------------------------------------------
local function test_vec3_cross()
  local x  = g.Vector3D { X = 1.0, Y = 0.0, Z = 0.0 }
  local y  = g.Vector3D { X = 0.0, Y = 1.0, Z = 0.0 }
  local z  = x:cross(y)
  test.expect_near(z.X, 0.0, 0.001, "cross X×Y → Z: result.X = 0")
  test.expect_near(z.Y, 0.0, 0.001, "cross X×Y → Z: result.Y = 0")
  test.expect_near(z.Z, 1.0, 0.001, "cross X×Y → Z: result.Z = 1")

  -- Cross product is anti-commutative: Y × X = -Z
  local neg = y:cross(x)
  test.expect_near(neg.Z, -1.0, 0.001, "cross Y×X → -Z: result.Z = -1")
  print("PASS: test_vec3_cross")
end

-- ---------------------------------------------------------------------------
-- Vector3D: length
-- ---------------------------------------------------------------------------
local function test_vec3_len()
  local unit = g.Vector3D { X = 1.0, Y = 0.0, Z = 0.0 }
  test.expect_near(unit:len(), 1.0, 0.001, "unit vec3 length = 1")

  -- (2, 2, 1): sqrt(4+4+1) = sqrt(9) = 3
  local v = g.Vector3D { X = 2.0, Y = 2.0, Z = 1.0 }
  test.expect_near(v:len(), 3.0, 0.001, "vec3 (2,2,1) length = 3")
  print("PASS: test_vec3_len")
end

-- ---------------------------------------------------------------------------
-- Vector3D: normalize → unit length (in-place; no return value)
-- ---------------------------------------------------------------------------
local function test_vec3_normalize()
  local v = g.Vector3D { X = 0.0, Y = 3.0, Z = 4.0 }
  v:normalize()
  test.expect_near(v:len(), 1.0, 0.001, "vec3 after normalize has unit length")
  print("PASS: test_vec3_normalize")
end

-- ---------------------------------------------------------------------------
-- Vector3D: lerp midpoint
-- ---------------------------------------------------------------------------
local function test_vec3_lerp()
  local a   = g.Vector3D { X = 0.0,  Y = 0.0,  Z = 0.0  }
  local b   = g.Vector3D { X = 10.0, Y = 20.0, Z = 30.0 }
  local mid = a:lerp(b, 0.5)
  test.expect_near(mid.X, 5.0,  0.001, "vec3 lerp 0.5 X")
  test.expect_near(mid.Y, 10.0, 0.001, "vec3 lerp 0.5 Y")
  test.expect_near(mid.Z, 15.0, 0.001, "vec3 lerp 0.5 Z")
  print("PASS: test_vec3_lerp")
end

-- ---------------------------------------------------------------------------
-- Color: component access
-- ---------------------------------------------------------------------------
local function test_color_components()
  local c = g.Color { R = 1.0, G = 0.5, B = 0.25, A = 0.75 }
  test.expect_near(c.R, 1.0,  0.001, "color.R round-trip")
  test.expect_near(c.G, 0.5,  0.001, "color.G round-trip")
  test.expect_near(c.B, 0.25, 0.001, "color.B round-trip")
  test.expect_near(c.A, 0.75, 0.001, "color.A round-trip")
  print("PASS: test_color_components")
end

-- ---------------------------------------------------------------------------
-- Color: lerp midpoint between black and white
-- ---------------------------------------------------------------------------
local function test_color_lerp()
  local black = g.Color { R = 0.0, G = 0.0, B = 0.0, A = 1.0 }
  local white = g.Color { R = 1.0, G = 1.0, B = 1.0, A = 1.0 }
  local grey  = black:lerp(white, 0.5)
  test.expect_near(grey.R, 0.5, 0.001, "color lerp R at t=0.5")
  test.expect_near(grey.G, 0.5, 0.001, "color lerp G at t=0.5")
  test.expect_near(grey.B, 0.5, 0.001, "color lerp B at t=0.5")

  -- t=0 → black, t=1 → white
  local at0 = black:lerp(white, 0.0)
  local at1 = black:lerp(white, 1.0)
  test.expect_near(at0.R, 0.0, 0.001, "color lerp t=0 → black.R = 0")
  test.expect_near(at1.R, 1.0, 0.001, "color lerp t=1 → white.R = 1")
  print("PASS: test_color_lerp")
end

-- ---------------------------------------------------------------------------
-- Color: parse hex string (#RRGGBBAA)
-- ---------------------------------------------------------------------------
local function test_color_parse()
  -- Red: #ff0000ff
  local red = g.Color.parse("#ff0000ff")
  test.expect_near(red.R, 1.0, 0.01, "parsed #ff0000ff: R = 1.0")
  test.expect_near(red.G, 0.0, 0.01, "parsed #ff0000ff: G = 0.0")
  test.expect_near(red.B, 0.0, 0.01, "parsed #ff0000ff: B = 0.0")
  test.expect_near(red.A, 1.0, 0.01, "parsed #ff0000ff: A = 1.0")
  print("PASS: test_color_parse")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_vec2_components()
test_vec2_add()
test_vec2_sub()
test_vec2_scale()
test_vec2_dot()
test_vec2_len()
test_vec2_normalize()
test_vec2_lerp()
test_vec2_unm()
test_vec3_components()
test_vec3_add()
test_vec3_sub()
test_vec3_dot()
test_vec3_cross()
test_vec3_len()
test_vec3_normalize()
test_vec3_lerp()
test_color_components()
test_color_lerp()
test_color_parse()

print("All geometry tests passed.")
