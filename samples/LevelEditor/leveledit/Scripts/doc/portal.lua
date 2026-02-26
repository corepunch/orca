local lv = require "level"
local portal = {}

local function makeBounds(width, depth)
	local x = (lv.ROOM_SIZE - width) / 2
	local y = (lv.ROOM_SIZE - depth) / 2
	return lv.Region2int8.new(x, y, x + width, y + depth)
end

local offset = {
	["floor"] = lv.Vector2int8.new(0, 0),
	["ceiling"] = lv.Vector2int8.new(0, 0),
	["wall_negative_x"] = lv.Vector2int8.new( 1, 0),
	["wall_positive_x"] = lv.Vector2int8.new(-1, 0),
	["wall_negative_y"] = lv.Vector2int8.new( 0, 1),
	["wall_positive_y"] = lv.Vector2int8.new( 0,-1),
}

local getNewRoomRegion = {
	["wall_negative_x"] = function(bounds, location, region, depth)
		local newbounds = makeBounds(depth, region.height)
		local x = location.x + bounds.min.x - newbounds.max.x
		local y = location.y + region.min.y - newbounds.min.y
		return lv.Vector2int8.new(x, y), newbounds
	end,
	["wall_positive_x"] = function(bounds, location, region, depth)
		local newbounds = makeBounds(depth, region.height)
		local x = location.x + bounds.max.x - newbounds.min.x
		local y = location.y + region.min.y - newbounds.min.y
		return lv.Vector2int8.new(x, y), newbounds
	end,
	["wall_negative_y"] = function(bounds, location, region, depth)
		local newbounds = makeBounds(region.width, depth)
		local x = location.x + region.min.x - newbounds.min.x
		local y = location.y + bounds.min.y - newbounds.max.y
		return lv.Vector2int8.new(x, y), newbounds
	end,
	["wall_positive_y"] = function(bounds, location, region, depth)
		local newbounds = makeBounds(region.width, depth)
		local x = location.x + region.min.x - newbounds.min.x
		local y = location.y + bounds.max.y - newbounds.min.y
		return lv.Vector2int8.new(x, y), newbounds
	end,
}

local function findDirection(room, region)
	local bounds = room:getBounds()
	if region.width == 0 then
		if region.min.x == bounds.min.x then return "wall_negative_x", "wall_positive_x" end
		if region.max.x == bounds.max.x then return "wall_positive_x", "wall_negative_x" end
	end
	if region.height == 0 then
		if region.min.y == bounds.min.y then return "wall_negative_y", "wall_positive_y" end
		if region.max.y == bounds.max.y then return "wall_positive_y", "wall_negative_y" end
	end
	if not room:hasOuterWalls(region) then return "floor", "floor" end
end

local function findPossiblePortal(doc, room1, region1)
	local floor1, ceiling1 = room1:getVerticalBounds(region1)
	local dir1, dir2 = findDirection(room1, region1)
	if not dir1 then return end
	for room2 in doc.level:getRooms() do
		local diff = room1:getLocation() - room2:getLocation()
		local region2 = region1 + diff + offset[dir1]
		local floor2, ceiling2 = room2:getVerticalBounds(region2)
		if room2 ~= room1 and floor1 <= ceiling2 and ceiling1 >= floor2 then
			if findDirection(room2, region2) == dir2 then
				if dir2 == "floor" then
					if ceiling2 == floor1 then return room2, region2, "floor", "ceiling" end
					if floor2 == ceiling1 then return room2, region2, "ceiling", "floor" end
				else
					return room2, region2, dir1, dir2
				end
			end
		end
	end
end

function portal.addPortal(doc, room1, region1)
	local room2, region2, portal1, portal2 = findPossiblePortal(doc, room1, region1)
	if room2 then
		room1:addPortal(region1, portal1, room2)
		room2:addPortal(region2, portal2, room1)
		room1:buildGeometry()
		room2:buildGeometry()
	else
		system.Console.log("Can't add portal")
	end
end

function portal.newRoomDown(doc, room1, height)
	local region = room1:getSelection()
	local floor, _ = room1:getVerticalBounds(region)
	local room2 = doc.level:addRoom(floor - height, height)
	room2:setLocation(room1:getLocation())
	room2:crop(region)
	room1:setSelection(255, 255, 255, 255)
	portal.addPortal(doc, room1, region)
	return room2
end

function portal.newRoomUp(doc, room1, height)
	local region = room1:getSelection()
	local _, ceiling = room1:getVerticalBounds(region)
	local room2 = doc.level:addRoom(ceiling, height)
	room2:setLocation(room1:getLocation())
	room2:crop(region)
	room1:setSelection(255, 255, 255, 255)
	portal.addPortal(doc, room1, region)
	return room2
end

function portal.newTunnelRoom(doc, room1, height, depth)
	local region = room1:getSelection()
	local floor, _ = room1:getVerticalBounds(region)
	local dir = findDirection(room1, region)
	if dir and dir ~= "floor" then
		local bounds1 = room1:getBounds()
		local location1 = room1:getLocation()
		local func = getNewRoomRegion[dir]
		local location2, bounds2 = func(bounds1, location1, region, depth)
		local room2 = doc.level:addRoom(floor, height)
		room2:crop(bounds2)
		room2:setLocation(location2)
		portal.addPortal(doc, room1, region)
	end
end

return portal