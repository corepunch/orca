local Class=require "Editor.Scripts.Class"
local Native=require "Editor.Scripts.Native"
local Menu=require "Editor.Scripts.Menu"
local Popups=require "Editor.Scripts.Popups"
local MenuBar=Class:extend {}
function MenuBar:init(menus,status)
	self.menus,self.status=menus,status
	self.view=Native.register(Native.load('Editor/Views/MenuBar'),self)
	local rows={}
	for _,menu in ipairs(menus) do rows[#rows+1]={Label=menu.title} end
	self.view:findChild('Menus',true).ItemsSource=Native.records(rows)
	Native.keys(self)
	self:invalidate()
end
function MenuBar:invalidate() self.view:findChild('Status',true).Text=self.status and self.status() or '' end
function MenuBar:checkResize() end
function MenuBar:focus() self.view:setFocus() end
function MenuBar:invoke(record,args,sender) if record then self:open(record.Index,sender) end return true end
function MenuBar:indexAt(x)
	local list=self.view:findChild('Menus',true)
	local child=list:getFirstChild()
	while child do
		if child.DataContext then
			local left=Native.origin(child)
			if x>=left and x<left+(child.ActualWidth or 0) then return child.DataContext.Index end
		end
		child=child:getNext()
	end
end
function MenuBar:open(index,sender)
	Popups.closeAll()
	local spec=self.menus[index]
	if not spec then return end
	self.openIndex=index
	local menu=Menu(type(spec.items)=='function' and spec.items() or spec.items,{
		onNavigate=function(dir) self:open((index-1+dir)%#self.menus+1) end,
	})
	menu.onClose=function() self.openIndex=nil end
	local x,y=Native.origin(sender or self.view)
	if not sender then x=x+(index-1)*64 end
	Popups.push(menu,x,y+28)
end
function MenuBar:accelerator(letter)
	for i,menu in ipairs(self.menus) do if menu.title:sub(1,1):lower()==letter then self:open(i) return true end end
	return false
end
return MenuBar
