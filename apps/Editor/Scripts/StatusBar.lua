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
	-- Undo depth is noise here; the bar keeps to the selection and save state.
	local history=self.view:findChild('History',true)
	history.Text,history.Visible='',false
	local state=self.view:findChild('State',true)
	state.Text=Workspace.dirty and '● Unsaved changes' or 'All changes saved'
	state.StateClasses=Workspace.dirty and 'status-dirty' or ''
end
function StatusBar:checkResize() end
function StatusBar:focus() self.view:setFocus() end
return StatusBar
