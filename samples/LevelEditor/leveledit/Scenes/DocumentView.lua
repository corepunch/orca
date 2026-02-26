local app = require "AppKit"
local common = require "AppKit.common"
local controls = import "Scenes/Controls"
local LevelViewport = require "LevelViewport"
local CommandButton = require "CommandButton"
local doc = require "Document"
local lv = require "level"

local upsideDown = cg.Rect.new(0, 1, 1, -1)

local function findRoomAt(x, y)
	for room in doc.level:getRooms() do
		local loc = room:getLocation()
		local block = room:getBlockAt(x - loc.x, y - loc.y)
		if block and block.type == "floor" then
			return room
		end
	end
end

local function eventToImageSpace(sender, event, size)
	local x, y = sender:pointToLocalSpace(event)
	local f = sender.frame
	return size * x / f.width, size * y / f.height
end

local function getRoot(self)
	local view = self
	while view.parent do
		view = view.parent
	end
	return view
end

return ui.HSplitView {
	awake = function(self)
		app.rootView = getRoot(self)
		for room in doc.level:getRooms() do
			room:buildGeometry()
		end
	end,
	width = 1280,
	height = 720,
	splitPositions = { 0.2, 0.8, 1.0 },
	background = renderer.Brush.color "#3D3F41",
	dataModel = doc.state,
	ui.VSplitView {
		splitPosition = 0.7,
		ui.VStackView {
			verticalAlignment = "top",
			spacing = 5,
			ui.ImageView {
				name = "miniMap",
				filter = "point",
				horizontalAlignment = "stretch",
				image = doc:getSelectedRoom():getRoomView(),
				uv = upsideDown,
				updateBindings = function(self)
					self.height = self.frame.width
				end,
				didStartDrag = action "miniMapDidStartDrag",
				draggingUpdated = action "miniMapDraggingUpdated",
				mousePressed = action "miniMapMousePressed",
			},
			ui.VGridView {
				spacing = 5,
				horizontalPadding = cg.Vector2.new(5, 5),
				verticalPadding = cg.Vector2.new(5, 5),
				columns = 2,
				CommandButton { text = "Select Room", command = "doc:select-room" },
				CommandButton { text = "Crop Room", command = "doc:crop-room" },
				CommandButton { text = "New Room Up", command = "doc:new-room-up" },
				CommandButton { text = "New Room Down", command = "doc:new-room-down" },
				CommandButton { text = "New Tunnel Room", command = "doc:new-tunnel-room" },
				CommandButton { text = "Floor+", command = "doc:raise1-qa", alt = "doc:raise1-qa-smooth" },
				CommandButton { text = "Floor-", command = "doc:lower1-qa", alt = "doc:lower1-qa-smooth" },
				CommandButton { text = "Ceiling+", command = "doc:raise1-ws", alt = "doc:raise1-ws-smooth" },
				CommandButton { text = "Ceiling-", command = "doc:lower1-ws", alt = "doc:lower1-ws-smooth" },
				CommandButton { text = "Door", command = "doc:add-portal" },
				CommandButton { text = "Wall", command = "doc:set-wall" },
				CommandButton { text = "Floor", command = "doc:set-floor" },
				CommandButton { text = "Save Level", command = "app:save-file" },
				CommandButton { text = "Texture Walls", command = "doc:texture-walls" },
				CommandButton { text = "Texture Floor", command = "doc:texture-floor" },
				CommandButton { text = "Texture Ceiling", command = "doc:texture-ceiling" },
			},
		},
		ui.VStackView {
			spacing = 5,
			horizontalPadding = cg.Vector2.new(5, 5),
			verticalPadding = cg.Vector2.new(5, 5),
			ui.Label {
				background = renderer.Brush.color "#202020",
				height = 20,
				horizontalAlignment = "stretch",
				textHorizontalAlignment = "left",
				text = "Lighting",
				horizontalPadding = cg.Vector2.new(5, 5),
			},
			ui.VGridView {
				columns = 4,
				controls.PushButton {
					text = "Point",
					didStartDrag = function (self)
						app.dragSession = {
							view = self,
							item = "PointLight"
						}
					end
				},
				controls.PushButton { text = "Sun", },
			},
		},
	},
	ui.VSplitView {
		splitPosition = 0.9,
		tabView = ui.TabView {
			mapView = ui.View {
				mapImageView = ui.ImageView {
					image = doc.level:getWorldView(doc:getSelectedRoom()),
					filter = "point",
					uv = upsideDown,
					getIntrinsicWidth = function(self)
						if not self.parent.frame.width then return nil end
						if not self.parent.frame.height then return nil end
						return math.min(self.parent.frame.width, self.parent.frame.height)
					end,
					updateBindings = function(self)
						self.height = self.actualWidth
					end,
					didStartDrag = action "worldMapDidStartDrag",
					draggingUpdated = action "worldMapDraggingUpdated",
					mouseReleased = action "worldMapMouseReleased",
				},
			},
			viewport = LevelViewport {
				cameraTarget = scn.Node {
					rotation = cg.Vector3.new(-30, 0, 0),
					camera = scn.Camera {
						position = cg.Vector3.new(0, 0, 200),
						fov = 70,
					},
				},
				camera = ref "cameraTarget.camera",
				didStartDrag = action "viewportDidStartDrag",
				draggingUpdated = action "viewportDraggingUpdated",
				performDragOperation = action "viewportPerformDragOperation",
				mousePressed = action "viewportMousePressed",
			},
			awake = function (self)
				self.activeTab = self.viewport
			end
		},
		controlPanel = ui.VGridView {
			columns = 4,
			ui.VStackView {
				name = "viewModeGroup",
				horizontalAlignment = "left",
				verticalAlignment = "top",
				spacing = 5,
				valueSource = "viewmode",
				CommandButton {
					horizontalAlignment = "left",
					type = "RadioButton",
					text = "2D View",
					command = "doc:set-mode-overview",
					modelValue = "overview",
				},
				CommandButton {
					horizontalAlignment = "left",
					type = "RadioButton",
					text = "Geometry",
					command = "doc:set-mode-geometry",
					modelValue = "geometry",
				},
				CommandButton {
					horizontalAlignment = "left",
					type = "RadioButton",
					text = "Face Edit",
					command = "doc:set-mode-faceedit",
					modelValue = "faceedit",
				},
				CommandButton {
					horizontalAlignment = "left",
					type = "RadioButton",
					text = "Lighting",
					command = "doc:set-mode-lighting",
					modelValue = "lighting",
				},
			},
			CommandButton {
				type = "Checkbox",
				horizontalAlignment = "left",
				text = "Draw Doors",
				command = "doc:toggle-portals",
			},
		},
	},
	FS:Prefab {
		verticalAlignment = "top",
		source = "Scenes/TextureView.lua",
	},
	refreshView = function (self, ignoreModel)
		local mapImageView = self:findChild("mapImageView")
		local miniMap = self:findChild("miniMap")
		if not ignoreModel then
			doc:getSelectedRoom():buildGeometry()
		end
		miniMap.image = doc:getSelectedRoom():getRoomView()
		mapImageView.image = doc.level:getWorldView(doc:getSelectedRoom())
		app.redraw = true
		local viewModeGroup = self:findChild("viewModeGroup")
		for _, child in ipairs(viewModeGroup.children) do
			if child.modelValue then
				child.value = self.dataModel[viewModeGroup.valueSource] == child.modelValue
			end
		end
	end,
	setViewMode = function (self, mode)
		local tabView = self:findChild("tabView")
		local viewport = tabView:findChild("viewport")
		local mapView = tabView:findChild("mapView")
		tabView.activeTab = mode == "overview" and mapView or viewport
		doc.selectedFace = nil
		doc:getSelectedRoom():setSelection(255,255,255,255)
	end,
	worldMapDidStartDrag = function (self, sender, event)
		local x, y = eventToImageSpace(sender, event, lv.WORLD_SIZE)
		local room = findRoomAt(x, y)
		if room then
			local loc = room:getLocation()
			app.dragSession = {
				item = sender,
				room = room,
				x = x - loc.x,
				y = y - loc.y
			}
			self:refreshView(true)
		end
	end,
	worldMapDraggingUpdated = function (self, sender, event)
		if app.dragSession.item ~= sender then return end
		local x, y = eventToImageSpace(sender, event, lv.WORLD_SIZE)
		local loc = lv.Vector2int8.new(x - app.dragSession.x, y - app.dragSession.y)
		app.dragSession.room:setLocation(loc)
		self:refreshView(true)
		return true
	end,
	worldMapMouseReleased = function (self, sender, event)
		local x, y = eventToImageSpace(sender, event, lv.WORLD_SIZE)
		local room = findRoomAt(x, y)
		if room then
			doc:selectRoom(room)
			self:refreshView()
		end
	end,
	selectBlocks = function (self, sender, event)
		local block = sender:raycastBlock(event)
		if block then
			doc:getSelectedRoom():setSelection(app.dragSession.x, app.dragSession.y, block.x, block.y)
		end
		if doc.state.viewmode == "geometry" then
			self:refreshView(true)
		end
	end,
	rotateView = function (self, sender, event)
		local r = sender.cameraTarget.rotation
		local d = event.delta * 0.3
		local n = cg.Vector3.new(r.x - d.y, r.y - d.x, 0)
		sender.cameraTarget.rotation = n
	end,
	viewportDidStartDrag = function (_, sender, event)
		if event.modkeys.option then
			app.dragSession = { mode = "rotateView", item = sender }
		elseif event.button == "right" then
			local block = sender:raycastBlock(event)
			if block then
				app.dragSession = { mode = "selectBlocks", item = sender, x = block.x, y = block.y }
			end
		end
	end,
	viewportDraggingUpdated = function (self, sender, event)
		if app.dragSession.item == "PointLight" then
			local loc = sender:raycastObjectPostion(event)
			local origin = lv.Vector3int8.new(loc.x, 8, loc.y)
			if app.dragSession.object == nil then
				local object = doc:getSelectedRoom():addObject("PointLight", origin)
				app.dragSession.object = object
			else
				app.dragSession.object.x = origin.x
				app.dragSession.object.z = origin.z
			end
		end
		if app.dragSession.item ~= sender then return end
		self[app.dragSession.mode](self, sender, event)
		return true
	end,
	viewportPerformDragOperation = function (self, sender, event)
		if doc.state.viewmode == "faceedit" and doc.selectedFace then
			local sel = doc:selectedArea()
			local uv = doc:getSelectedUV()
			local face = doc.selectedFace
			for k, val in pairs(uv) do
				doc:getSelectedRoom():setFaceTexCoord(sel, face, val.u, val.v, k)
			end
			doc:getSelectedRoom():setSelection(255,255,255,255)
			self:refreshView()
		end
	end,
	viewportMousePressed = function (self, sender, event)
		local block, face = sender:raycastBlock(event)
		if event.modkeys.option or not block then
			return true
		elseif doc.state.viewmode == "faceedit" then
			local uv = doc:getSelectedUV()
			doc.selectedFace = face
			for k, val in pairs(uv) do
				doc:getSelectedRoom():setFaceTexCoord(block.location, face, val.u, val.v, k)
			end
			self:refreshView()
		elseif event.button ~= "right" then
			if doc:selectedArea():containsPoint(block.x, block.y) then
				doc.selectedDirection = (doc.selectedDirection or 0) + 1
			end
		else
			doc:getSelectedRoom():setSelection(block.x, block.y, block.x, block.y)
			doc.selectedDirection = 0
			self:refreshView(true)
		end
		return true
	end,
	miniMapDidStartDrag = function (self, sender, event)
		local x, y = eventToImageSpace(sender, event, lv.ROOM_SIZE)
		app.dragSession = { item = sender, x = x, y = y }
	end,
	miniMapDraggingUpdated = function (self, sender, event)
		if app.dragSession.item ~= sender then return end
		local x, y = eventToImageSpace(sender, event, lv.ROOM_SIZE)
		doc:getSelectedRoom():setSelection(app.dragSession.x, app.dragSession.y, x, y)
		self:refreshView(true)
	end,
	miniMapMousePressed = function (self, sender, event)
		local x, y = eventToImageSpace(sender, event, lv.ROOM_SIZE)
		doc:getSelectedRoom():setSelection(x, y, x, y)
		self:refreshView(true)
	end,
	selectRoomClicked = function (self)
		getRoot(self):showDialog {
			filename = "Scenes/SelectRoom.lua",
			onSubmit = function(d)
				doc:selectRoom(d:getSelectedRoom())
				self:refreshView()
			end,
		}
	end,
}