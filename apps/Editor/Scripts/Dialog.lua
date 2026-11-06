local Class=require "Editor.Scripts.Class"
local Native=require "Editor.Scripts.Native"
local Popups=require "Editor.Scripts.Popups"
local Dialog=Class:extend {}
function Dialog:init(title,lines)
	self.view=Native.load('Editor/Views/Dialog')
	self.view:findChild('Title',true).Text=title
	local rows={}
	for _,line in ipairs(lines) do rows[#rows+1]={Label=line[2] and line[1] or '',Detail=line[2] or line[1] or ' '} end
	self.view:findChild('Lines',true).ItemsSource=Native.records(rows)
	self.view:findChild('Close',true).Click=function() Popups.close(self) return true end
	Native.keys(self)
end
function Dialog:focus() self.view:setFocus() end
function Dialog:onKey(chord)
	if chord=='escape' or chord=='enter' then Popups.close(self) return true end
	return false
end
function Dialog.show(title,lines)
	local dialog=Dialog(title,lines)
	local width,height=Popups.size()
	return Popups.push(dialog,math.max(0,(width-dialog.view.Width)/2),math.max(36,(height-dialog.view.Height)/3),true)
end
return Dialog
