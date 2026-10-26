local app = require "AppKit"
local command = require "AppKit.command"
local lv = require "level"
local portal = require "doc.portal"

local function doc()
  return require "Document"
end

local commands = {
  ["doc:select-room"] = function()
		app.rootView:showDialog {
			filename = "Scenes/SelectRoom.lua",
			onSubmit = function(dialog)
				doc():selectRoom(dialog:getSelectedRoom())
				app.rootView:refreshView()
			end,
		}
  end,
	["doc:crop-room"] = function ()
		doc():getSelectedRoom():crop(doc():selectedArea())
		app.rootView:refreshView()
	end,
	["doc:new-room-up"] = function ()
		app.rootView:showDialog {
			filename = "Scenes/NewRoomHeight.lua",
			onSubmit = function(d)
				portal.newRoomUp(doc(), doc():getSelectedRoom(), d:getRoomCeiling())
				app.rootView:refreshView()
			end,
		}
	end,
	["doc:new-room-down"] = function ()
		app.rootView:showDialog {
			filename = "Scenes/NewRoomHeight.lua",
			onSubmit = function(d)
				portal.newRoomDown(doc(), doc():getSelectedRoom(), d:getRoomCeiling())
				app.rootView:refreshView()
			end,
		}
	end,
	["doc:new-tunnel-room"] = function ()
		app.rootView:showDialog {
			filename = "Scenes/NewTunnelRoom.lua",
			onSubmit = function(d)
				portal.newTunnelRoom(doc(), doc():getSelectedRoom(), d:getRoomCeiling(), d:getRoomDepth())
				app.rootView:refreshView()
			end,
		}
	end,
	["doc:set-wall"] = function ()
		doc():getSelectedRoom():setType(doc():selectedArea(), "wall")
		app.rootView:refreshView()
	end,
	["doc:set-floor"] = function ()
		doc():getSelectedRoom():setType(doc():selectedArea(), "floor")
		app.rootView:refreshView()
	end,
	["doc:add-portal"] = function ()
		portal.addPortal(doc(), doc():getSelectedRoom(), doc():selectedArea())
		app.rootView:refreshView()
	end,
	["doc:select-previous-room"] = function ()
		local prev = doc().previousRoom
		if prev then
			doc():selectRoom(prev)
			app.rootView:refreshView()
		end
	end,
	["doc:texture-walls"] = function ()
		local room = doc():getSelectedRoom()
		local uv = doc():getSelectedUV()
		for n = 0, 19 do
			for k, val in pairs(uv) do
				room:setFaceTexCoord(lv.Region2int8.new(0,0,19,19), n, val.u, val.v, k)
			end
		end
		app.rootView:refreshView()
	end,
	["doc:texture-floor"] = function ()
		local room = doc():getSelectedRoom()
		local uv = doc():getSelectedUV()
		for k, val in pairs(uv) do
			room:setFaceTexCoord(lv.Region2int8.new(0,0,19,19), 20, val.u, val.v, k)
		end
		app.rootView:refreshView()
	end,
	["doc:texture-ceiling"] = function ()
		local room = doc():getSelectedRoom()
		local uv = doc():getSelectedUV()
		for k, val in pairs(uv) do
			room:setFaceTexCoord(lv.Region2int8.new(0,0,19,19), 21, val.u, val.v, k)
		end
		app.rootView:refreshView()
	end,
	["doc:toggle-portals"] = function ()
		doc().state.drawDoors = not doc().state.drawDoors
	end
}

local viewmodes = {
	"overview",
	"geometry",
	"faceedit",
	"lighting"
}

local heights = {
	["qa"] = "floor",
	["ws"] = "ceiling",
	["ed"] = "ed",
	["rf"] = "rf",
}

local direction = {
	["raise1"] = 1,
	["raise4"] = 4,
	["lower1"] = -1,
	["lower4"] = -4,
}

for _, viewmode in pairs(viewmodes) do
	commands["doc:set-mode-"..viewmode] = function ()
		doc().state.viewmode = viewmode
		app.rootView:setViewMode(viewmode)
		app.rootView:refreshView()
	end
end

for hkey, hval in pairs(heights) do
	for dkey, dval in pairs(direction) do
		commands["doc:"..dkey.."-"..hkey] = function ()
			doc():getSelectedRoom():changeHeight(doc():selectedArea(), hval, dval, false, doc().selectedDirection)
			app.rootView:refreshView()
		end
		commands["doc:"..dkey.."-"..hkey.."-smooth"] = function ()
			doc():getSelectedRoom():changeHeight(doc():selectedArea(), hval, dval, true, doc().selectedDirection)
			app.rootView:refreshView()
		end
	end
end

command.add(doc, commands)
