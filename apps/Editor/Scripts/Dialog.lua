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
local function present(dialog)
	local width,height=Popups.size()
	return Popups.push(dialog,math.max(0,(width-dialog.view.Width)/2),math.max(36,(height-dialog.view.Height)/3),true)
end
function Dialog.show(title,lines) return present(Dialog(title,lines)) end

-- A question with up to three answers: Enter is primary, Escape cancels.
-- opts = { primary = {label, action}, secondary = {label, action}, cancel = fn }
local Confirm=Class:extend {}
function Confirm:init(title,message,opts)
	self.opts=opts
	self.view=Native.load('Editor/Views/ConfirmDialog')
	self.view:findChild('Title',true).Text=title
	self.view:findChild('Message',true).Text=message
	local function bind(name,answer)
		local button=self.view:findChild(name,true)
		button.Visible=answer~=nil
		if answer then
			if answer.label then button.Text=answer.label end
			button.Click=function() self:answer(answer) return true end
		end
	end
	bind('Primary',opts.primary)
	bind('Secondary',opts.secondary)
	bind('Cancel',{action=opts.cancel})
	Native.keys(self)
end
function Confirm:answer(answer)
	Popups.close(self)
	if answer and answer.action then answer.action() end
end
function Confirm:focus() self.view:setFocus() end
function Confirm:onKey(chord)
	if chord=='escape' then self:answer({action=self.opts.cancel}) return true end
	if chord=='enter' then self:answer(self.opts.primary) return true end
	return false
end
function Dialog.confirm(title,message,opts) return present(Confirm(title,message,opts)) end
return Dialog
