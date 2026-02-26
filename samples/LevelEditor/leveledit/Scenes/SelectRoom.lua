local doc = require "Document"
local controls = import "Scenes/Controls"
local lv = require "level"

local selectedRoom = doc:getSelectedRoom()
local rooms = {}

function rooms:numberOfItems()
	return doc:numberOfRooms()
end

function rooms:cellForIndex(tableView, index)
	local cell = tableView:dequeCell()
	cell.roomName = "Room"..index
	return cell
end

function rooms:willDisplayCellAtIndex(tableView, _, index)
	if doc:roomAtIndex(index) == doc:getSelectedRoom() then
		tableView:selectRowAtIndex(index)
	end
end

function rooms:didSelectRowAtIndex(_, index)
	selectedRoom = doc:roomAtIndex(index)
end

return controls.PopUpWindow {
	width = 410,
	height = 410,
	title = "Select Room",
	roomsList = ui.TableView {
		background = renderer.Brush.color "#272829",
		prototypeCell = "Scenes/RoomCell.lua",
		dataSource = rooms,
		delegate = rooms,
		height = 240,
		horizontalAlignment = "stretch",
	},
	controls.PushButton {
		horizontalAlignment = "left",
		verticalAlignment = "bottom",
		text = "Add Room",
		onClick = action "onAddRoom",
	},
	onAddRoom = function (self)
		local room = doc.level:addRoom(0, lv.DEFAULT_CEILING)
		local index = doc:indexOfRoom(room)
		self.roomsList:insertRowAt(index)
		self.roomsList:selectRowAtIndex(index)
	end,
	getSelectedRoom = function (self)
		return selectedRoom
	end,
}