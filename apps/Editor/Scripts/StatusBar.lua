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
	local unsaved=0
	for _,doc in ipairs(Workspace.documents) do if Workspace.isDirty(doc.path) then unsaved=unsaved+1 end end
	if Workspace.dirty and unsaved<1 then unsaved=1 end
	state.Text=unsaved>1 and ('● '..unsaved..' unsaved documents') or unsaved==1 and (Workspace.dirty and '● Unsaved changes' or '● 1 unsaved document') or 'All changes saved'
	state.StateClasses=unsaved>0 and 'status-dirty' or ''
end
function StatusBar:checkResize() end
function StatusBar:focus() self.view:setFocus() end
return StatusBar
