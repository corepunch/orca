local lv = require "level"
local Document = {}

function Document:numberOfRooms()
	local count = 0
	for _ in self.level:getRooms() do
		count = count + 1
	end
  return count
end

function Document:roomAtIndex(index)
	if index < 1 then return nil end
	for room in self.level:getRooms() do
		if index == 1 then
			return room
		else
			index = index - 1
		end
	end
end

function Document:indexOfRoom(room)
	local index = 1
	for _room in self.level:getRooms() do
		if _room == room then
			return index
		else
			index = index + 1
		end
	end
end

function Document:selectedArea()
	return self.selectedRoom:getSelection()
end

function Document:selectRoom(room)
	Document.previousRoom = Document.selectedRoom
	Document.selectedRoom = room
end

function Document:getSelectedRoom()
	return Document.selectedRoom
end

function Document:getSelectedUV()
	return {
		{ u = self.curtex.u1, v = self.curtex.v1 },
		{ u = self.curtex.u2, v = self.curtex.v1 },
		{ u = self.curtex.u2, v = self.curtex.v2 },
		{ u = self.curtex.u1, v = self.curtex.v2 },
	}
end

Document.level = lv.Level.load("/Users/igor/Desktop/mylevel.lev")
Document.selectedRoom = Document:roomAtIndex(1)
Document.texture = FS:Image "Tilesets/city.png"
Document.curtex = { u1 = 0, v1 = 0, u2 = lv.NUM_CLICKS, v2 = lv.NUM_CLICKS }

Document.state = {
	drawDoors = false
}

return Document