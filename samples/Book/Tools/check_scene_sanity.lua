-- Run from samples/Book: lua Tools/check_scene_sanity.lua Rooms/workshop.blks
package.path = "Scripts/?.lua;" .. package.path

local Projection = require "SceneProjection"
local scenePath = arg[1]
if not scenePath then
    io.stderr:write("usage: lua Tools/check_scene_sanity.lua SCENE.blks\n")
    os.exit(2)
end

local scene, loadError = Projection.load(scenePath)
if not scene then
    io.stderr:write(scenePath .. ": " .. tostring(loadError) .. "\n")
    os.exit(2)
end

local bounds = Projection.semanticBounds(scene)
local intersections = Projection.intersections(scene)
for _, hit in ipairs(intersections) do
    io.stderr:write(string.format(
        "%s: %s intersects %s by %.2f x %.2f x %.2f cm\n",
        scenePath, hit.first, hit.second,
        hit.overlap[1] * 100, hit.overlap[2] * 100, hit.overlap[3] * 100))
end

if #intersections > 0 then os.exit(1) end
print(string.format("Scene sanity: %s (%d semantic assemblies, no intersections)",
    scenePath, #bounds))
