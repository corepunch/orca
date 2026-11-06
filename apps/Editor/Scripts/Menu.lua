-- Native menu records and an XML row template; UIKit owns layout and hit tests.
local Class=require "Editor.Scripts.Class"
local Native=require "Editor.Scripts.Native"
local Popups=require "Editor.Scripts.Popups"
local Menu=Class:extend {}
local function selectable(item) return item and not item.separator and not item.disabled end
function Menu:init(items,opts)
	opts=opts or {}
	self.entries,self.parentMenu,self.onNavigate=items,opts.parent,opts.onNavigate
	self.view=Native.register(Native.load('Editor/Views/Menu'),self)
	self.selected=1
	local rows,height={},8
	for i,item in ipairs(items) do
		rows[#rows+1]={Label=item.label,Detail=item.shortcut,Mark=item.checked and '✓' or '',
			Tail=item.items and '›' or '',Disabled=item.disabled or false,
			Separator=item.separator or false,IsItem=not item.separator,State=item.disabled and 'menu-disabled' or ''}
		height=height+(item.separator and 11 or 32)
	end
	self.view.Height=math.min(600,height)
	self.view.ItemsSource=Native.records(rows)
	Native.keys(self)
	for i,item in ipairs(items) do if selectable(item) then self:select(i) break end end
end
function Menu:focus() self.view:setFocus() end
function Menu:invalidate() end
function Menu:checkResize() end
function Menu:select(index)
	if self.selectedRecord then self.selectedRecord.State='' end
	self.selected=index
	local record=self.view.ItemsSource and self.view.ItemsSource:getFirstChild()
	while record and record.Index~=index do record=record:getNext() end
	self.selectedRecord=record
	if record then record.State='menu-selected' end
end
function Menu:invoke(record) if record then self:activate(record.Index) end return true end
function Menu:hover(record,args,sender)
	if not record or not selectable(self.entries[record.Index]) then return false end
	self:select(record.Index)
	if self.child and Popups.isOpen(self.child) and self.child.source~=self.entries[record.Index] then Popups.close(self.child) end
	if self.entries[record.Index].items then self:openSubmenu(record.Index,sender) end
	return true
end
function Menu:move(dir)
	local n,index=#self.entries,self.selected
	for _=1,n do
		index=(index-1+dir)%n+1
		if selectable(self.entries[index]) then self:select(index) return end
	end
end
function Menu:openSubmenu(index,sender)
	local item=self.entries[index]
	if not selectable(item) or not item.items then return false end
	if self.child and Popups.isOpen(self.child) then
		if self.child.source==item then return true end
		Popups.close(self.child)
	end
	local x,y=Native.origin(sender or self.view)
	if not sender then y=y+(index-1)*32 end
	self.child=Menu(item.items,{parent=self,onNavigate=self.onNavigate})
	self.child.source=item
	Popups.push(self.child,x+(self.view.ActualWidth or 320)-4,y)
	return true
end
function Menu:activate(index)
	local item=self.entries[index]
	if not selectable(item) then return end
	if item.items then return self:openSubmenu(index) end
	Popups.closeAll()
	if item.action then item.action(item) end
end
function Menu:onKey(chord)
	if chord=='up' then self:move(-1)
	elseif chord=='down' then self:move(1)
	elseif chord=='enter' or chord=='space' then self:activate(self.selected)
	elseif chord=='right' then
		if not self:openSubmenu(self.selected) and self.onNavigate then self.onNavigate(1) end
	elseif chord=='left' then
		if self.parentMenu then Popups.close(self) elseif self.onNavigate then self.onNavigate(-1) end
	elseif chord=='escape' then if self.parentMenu then Popups.close(self) else Popups.closeAll() end
	elseif #chord==1 then
		local n=#self.entries
		for offset=1,n do
			local i=(self.selected-1+offset)%n+1
			if selectable(self.entries[i]) and (self.entries[i].label or ''):sub(1,1):lower()==chord then self:select(i) break end
		end
	else return false end
	return true
end
function Menu.show(items,x,y,opts) return Popups.push(Menu(items,opts),x,y) end
return Menu
