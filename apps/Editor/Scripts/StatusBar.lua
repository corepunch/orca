local Class=require "Editor.Scripts.Class"
local Native=require "Editor.Scripts.Native"
local Workspace=require "Editor.Scripts.Workspace"
local StatusBar=Class:extend {}
function StatusBar:init()
	self.view=Native.load('Editor/Views/StatusBar')
	Native.keys(self)
	self:invalidate()
end
function StatusBar:invalidate()
	self.view:findChild('Selection',true).Text=Workspace.selection or Workspace.document or 'No document open'
	self.view:findChild('History',true).Text=string.format('Undo %d   Redo %d',#Workspace.undoStack,#Workspace.redoStack)
	self.view:findChild('State',true).Text=Workspace.dirty and 'Modified' or 'Saved'
end
function StatusBar:checkResize() end
function StatusBar:focus() self.view:setFocus() end
return StatusBar
