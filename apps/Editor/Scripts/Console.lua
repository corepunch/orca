local Class=require "Editor.Scripts.Class"
local Native=require "Editor.Scripts.Native"
local Workspace=require "Editor.Scripts.Workspace"
local Console=Class:extend {title='Console'}
function Console:init()
	self.entries,self.counts={},{info=0,ok=0,warning=0,error=0}
	self.view=Native.load('Editor/Views/Console')
	Native.keys(self)
	Workspace.on('log',function(level,text) self:add(level,text) end)
	self:invalidate()
end
function Console:add(level,text)
	table.insert(self.entries,1,{Label=text,Detail=level,State='log-'..level,Time=os.date('%H:%M:%S')})
	if #self.entries>500 then table.remove(self.entries) end
	self.counts[level]=(self.counts[level] or 0)+1
	self:invalidate()
end
function Console:invalidate()
	self.view:findChild('Messages',true).ItemsSource=Native.records(self.entries)
	self.view:findChild('Empty',true).Visible=#self.entries==0
	self.subtitle=string.format('%d errors · %d warnings',self.counts.error,self.counts.warning)
	if self.header then self.header:invalidate() end
end
function Console:tools()
	return {{label='Clear',action=function()
		self.entries,self.counts={},{info=0,ok=0,warning=0,error=0}
		self:invalidate()
	end}}
end
function Console:focus() self.view:setFocus() end
function Console:checkResize() end
return Console
