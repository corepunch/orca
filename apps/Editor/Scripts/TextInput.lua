local Class=require "Editor.Scripts.Class"
local Native=require "Editor.Scripts.Native"
local Popups=require "Editor.Scripts.Popups"
local TextInput=Class:extend {}
function TextInput:init(opts)
	self.view=Native.load('Editor/Views/TextInput')
	self.input=self.view:findChild('Value',true)
	self.input.Text=tostring(opts.text or '')
	self.input.Cursor=#self.input.Text
	self.input.PlaceholderText=opts.placeholder or ''
	self.onAccept,self.onCancel,self.onInput=opts.onAccept,opts.onCancel,opts.onInput
	self.selectAll=opts.selectAll and self.input.Text~=''
	self.input.onTextChanged=function() if self.onInput then self.onInput(self.input.Text) end end
	self.input.TextInput=function(_,args)
		if self.selectAll then self.input.Text='' self.input.Cursor=0 self.selectAll=false end
		self.input:send('Node.TextInput',{text=args.text,character=args.character,modifiers=args.modifiers})
		return true
	end
	self.input.Submit=function(_,args) return self:accept(args.Text) end
	self.input.KeyDown=function(_,args)
		local chord=Native.chord(args)
		if chord=='escape' then Popups.close(self) return true end
		if self.selectAll and (chord=='backspace' or chord=='delete') then
			self.input.Text='' self.input.Cursor=0 self.selectAll=false
			return true
		end
		if chord=='left' or chord=='right' or chord=='home' or chord=='end' then self.selectAll=false end
		return self.input:send('Node.KeyDown',{keyCode=args.keyCode,text=args.text,modifiers=args.modifiers}) or false
	end
end
function TextInput:focus() self.input:setFocus() end
function TextInput:accept(text)
	local accept=self.onAccept
	self.onCancel=nil
	Popups.close(self)
	if accept then accept(text or self.input.Text) end
	return true
end
function TextInput:onClose() if self.onCancel then self.onCancel(self.input.Text) end end
function TextInput.show(x,y,opts) return Popups.push(TextInput(opts),x,y) end
return TextInput
