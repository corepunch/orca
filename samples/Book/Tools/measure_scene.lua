-- Run from samples/Book: lua Tools/measure_scene.lua Rooms/workshop.blks SWEEP-BROOM [TARGET]
package.path = "Scripts/?.lua;" .. package.path

local Projection = require "SceneProjection"
local scenePath, objectName, targetName = arg[1], arg[2], arg[3]
if targetName == "" then targetName = nil end
if not (scenePath and objectName) then
    io.stderr:write("usage: lua Tools/measure_scene.lua SCENE.blks OBJECT [TARGET]\n")
    os.exit(2)
end

local scene, loadError = Projection.load(scenePath)
if not scene then
    io.stderr:write(scenePath .. ": " .. tostring(loadError) .. "\n")
    os.exit(2)
end

local object, objectError = Projection.objectBounds(scene, objectName)
if not object then
    io.stderr:write(scenePath .. ": " .. objectError .. "\n")
    os.exit(2)
end

local function centimeters(value) return value * 100 end
local function printDistance(target)
    local distance, gaps = Projection.distance(object, target)
    print(string.format("%-20s %7.2f cm  (dx=%7.2f dy=%7.2f dz=%7.2f)",
        target.name, centimeters(distance), centimeters(gaps[1]),
        centimeters(gaps[2]), centimeters(gaps[3])))
end

print(string.format("%s in %s", objectName, scenePath))
print(string.format("bounds: X %.2f..%.2f cm, Y %.2f..%.2f cm, Z %.2f..%.2f cm",
    centimeters(object.min[1]), centimeters(object.max[1]),
    centimeters(object.min[2]), centimeters(object.max[2]),
    centimeters(object.min[3]), centimeters(object.max[3])))

local walls = Projection.wallClearances(scene, object)
print("room clearances:")
for _, name in ipairs({"west", "east", "south", "north", "floor", "ceiling"}) do
    print(string.format("  %-8s %7.2f cm", name, centimeters(walls[name])))
end

print("semantic object distances:")
if targetName then
    local target, targetError = Projection.objectBounds(scene, targetName)
    if not target then
        io.stderr:write(scenePath .. ": " .. targetError .. "\n")
        os.exit(2)
    end
    printDistance(target)
else
    for _, target in ipairs(Projection.semanticBounds(scene)) do
        if target.name ~= objectName then printDistance(target) end
    end
end
