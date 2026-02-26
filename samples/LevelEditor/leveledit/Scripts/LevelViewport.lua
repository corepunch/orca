local lv = require "level"
local doc = require "Document"
local common = require "AppKit.common"
local Viewport = require "UIKit.Viewport"

local LevelViewport = Viewport:extend()

local EditorMaterial = renderer.Material {
	shader = FS:Shader "Shaders/Editor",
	ambientColor = cg.Color.new "#404040",
	u_image = doc.texture,
}

local BackFaceMaterial = renderer.Material {
	shader = FS:Shader "Shaders/BackFace",
	cullmode = "front",
	blendMode = "premultiplied_alpha",
	u_faceOpacity = 0.33,
	u_diffuse = cg.Color.new "#ffffff",
	u_image = doc.texture,
}

local function getRoomMatrix(room)
	local mat = cg.Matrix4.identity()
	local loc = room:getLocation()
	local cen = doc:getSelectedRoom():getRoomCenter()
	local x = (loc.x - cen.x - 0.5) * lv.BLOCK_SIZE
	local y = -cen.z * lv.BLOCK_SIZE / 4
	local z = (loc.y - cen.y - 0.5) * lv.BLOCK_SIZE
	mat:translate(x, y, z)
	return mat
end


local function addRoomModel(room, material)
	if room == doc:getSelectedRoom() then
		local s = doc:selectedArea()
		material.u_selection = cg.Vector4.new(s.min.x, s.min.y, s.max.x, s.max.y)
	else
		material.u_selection = cg.Vector4.new(-1, -1, -1, -1)
	end
	material.u_lightmap = room:getLightmap()
	renderer.addModel {
		matrix = getRoomMatrix(room),
		mesh = room:getModel(),
		material = material,
	}
	local numlines = room:getNumLines()
	for i = 0, numlines do
		local a, b = room:getLine(i)
		renderer.addLine {
			matrix = getRoomMatrix(room),
			color = cg.Color.new "#00ff00",
			a = a,
			b = b
		}
	end
end

local function addRoomLights(room)
	for object in room:getObjects() do
		if object.type == "PointLight" then
			local origin = cg.Vector3.new(object.x , object.y, object.z)
			renderer.addLight {
				origin = getRoomMatrix(room) * (origin * lv.CLICK_SIZE),
				color = cg.Color.new "#ffffff",
				intensity = 0.75,
				radiusIn = 100,
				radiusOut = 200,
			}
		end
	end
end

local function drawRoomLights(room)
	for object in room:getObjects() do
		if object.type == "PointLight" then
			renderer.drawLines(getRoomMatrix(room), cg.Color.new "#ffffff", {
				cg.Vector3.new(object.x, object.y, object.z) * lv.CLICK_SIZE,
				cg.Vector3.new(object.x, 0, object.z) * lv.CLICK_SIZE,
			})
		end
	end
end

local function isRoomVisible(room)
	return doc.state.drawDoors or room == doc:getSelectedRoom()
end

function LevelViewport:viewportMouseLine(event)
	local mat = self:getContentsMatrix() * getRoomMatrix(doc:getSelectedRoom())
	local mouse = event.locationInWindow
	local loc = self.matrix:inversed() * cg.Vector3.new(mouse.x, mouse.y, 0)
	return common.mouseLine(mat, self.frame, loc)
end

function LevelViewport:raycastBlock(event)
	return doc:getSelectedRoom():raycastBlock(self:viewportMouseLine(event))
end

function LevelViewport:raycastObjectPostion(event)
	return doc:getSelectedRoom():raycastObjectPostion(self:viewportMouseLine(event))
end

function LevelViewport:drawWorld(material, clearColor)
	for room in doc.level:getRooms() do
		if isRoomVisible(room) then
			addRoomModel(room, material)
			addRoomLights(room)
		end
	end

	renderer.drawWorld {
		clearColor = clearColor,
		view = self.camera:getViewMatrix(),
		projection = self.camera:getProjectionMatrix(self.frame),
		noShadows = doc.state.viewmode ~= "lighting",
		noLighting = doc.state.viewmode ~= "lighting",
	}

	for room in doc.level:getRooms() do
		if isRoomVisible(room) then
			drawRoomLights(room)
		end
	end
end

function LevelViewport:draw()
	local mat = EditorMaterial
	mat.u_useTextures = doc.state.viewmode ~= "geometry" and 1 or 0
	mat.u_useLighting = doc.state.viewmode == "lighting" and 1 or 0
	mat.u_selectedFace = doc.state.viewmode == "faceedit" and doc.selectedFace or -1
	mat.u_selectedDirection = doc.selectedDirection or 0
	mat.ambientColor = doc.state.viewmode ~= "lighting" and cg.Color.new "#ffffff" or cg.Color.new "#000000"
	-- mat.ambientColor = cg.Color.new "#ffffff"
	self:drawWorld(mat, cg.Color.new "#202122")
	-- self:drawWorld(BackFaceMaterial)
end

function LevelViewport:mouseWheel(event)
	local d = 1.0 - event.delta.y * 0.01
	local p = self.camera.position
	local v = math.max(math.min(p.z * d, 800), 10)
	-- local r = sender.cameraTarget.rotation
	-- sender.cameraTarget.rotation = cg.Vector3.new(r.x - d.y, r.y - d.x, 0)
	self.camera.position = cg.Vector3.new(0, 0, v)
	return true
end

return LevelViewport