-- Syntax-colored code input inside a native UIKit binding dialog.
local Class=require "Editor.Scripts.Class"
local Native=require "Editor.Scripts.Native"
local Workspace=require "Editor.Scripts.Workspace"
local Popups=require "Editor.Scripts.Popups"
local Menu=require "Editor.Scripts.Menu"
local Syntax=require "Editor.Scripts.Syntax"
local CodeInput=require "Editor.Scripts.CodeInput"
local BindingEditor=Class:extend {}
local MODES={'Expression','OneWay','TwoWay','OneWayToSource','OneTime'}
local CLOSERS={['(']=')',['{']='}',['[']=']',['"']='"'}
function BindingEditor:init(opts)
	self.opts,self.mode,self.message=opts,opts.mode or 'Expression',nil
	self.paths={'Screen.Width','Screen.Height','DataContext/'}
	for _,id in ipairs(opts.properties or {}) do self.paths[#self.paths+1]=id end
	self.view=Native.register(Native.load('Editor/Views/BindingEditor'),self)
	self.input=CodeInput(self.view:findChild('Code',true))
	self.palette=self.view:findChild('Palette',true)
	self.completionView=self.view:findChild('Completions',true)
	self.source=self.view:findChild('Functions',true)
	self.input.Text=opts.expression or ''
	self.input.Cursor=#self.input.Text
	self.view:findChild('Title',true).Text='Binding · '..Workspace.leaf(opts.path)..'.'..opts.name
	self.view:findChild('Target',true).Text=opts.property..' ('..(opts.type or '?')..')'
	self.view:findChild('Apply',true).Click=function() self:apply() return true end
	self.view:findChild('Remove',true).Click=function() self.input.Text='' self:apply() return true end
	self.view:findChild('Cancel',true).Click=function() Popups.close(self) return true end
	self.view:findChild('Mode',true).Click=function() self:chooseMode() return true end
	self.input.KeyDown=function(_,args) return self:onKey(Native.chord(args),args) end
	self.input.TextInput=function(_,args)
		local text=args.text
		if not text or text=='' or text:byte()<32 then return false end
		if ((args.modifiers or 0)&((1<<17)|(1<<19)))~=0 then return false end
		self:onText(text)
		return true
	end
	self.input.CaretChanged=function()
		self.dismissed=true
		self:invalidate()
	end
	Native.keys(self,self.view)
	self:catalog()
	self:invalidate()
end
function BindingEditor:catalog()
	self.functions,self.functionList={},{}
	local record=self.source.Items and self.source.Items:getFirstChild()
	while record do
		self.functions[record.FunctionName]=record
		self.functionList[#self.functionList+1]=record
		record=record:getNext()
	end
	-- Retain the public cache reset used by Reload Project.
	BindingEditor.functions,BindingEditor.functionList=self.functions,self.functionList
	self:invalidate()
end
function BindingEditor:focus() self.input:setFocus() end
function BindingEditor:checkResize() self.input:checkResize() end
function BindingEditor:text() return self.input.Text or '' end
function BindingEditor:caret() return math.min(#self:text(),math.max(0,self.input.Cursor or 0)) end
function BindingEditor:line()
	local src,cursor=self:text(),self:caret()
	local start=1
	for pos in src:sub(1,cursor):gmatch('()\n') do start=pos+1 end
	local finish=src:find('\n',cursor+1,true) or #src+1
	return src:sub(start,finish-1),cursor-start+1,start
end
function BindingEditor:insert(text)
	local src,cursor=self:text(),self:caret()
	self.input.Text=src:sub(1,cursor)..text..src:sub(cursor+1)
	self.input.Cursor=cursor+#text
end
function BindingEditor:diagnose()
	local src=Syntax.flatten(self:text())
	local problems={}
	if src:match('^%s*$') then return problems,true end
	if next(Syntax.unbalanced(src)) then problems[#problems+1]='Unbalanced brackets' end
	for _,name in ipairs(Syntax.unknownCalls(src,self.functions)) do problems[#problems+1]='Unknown function '..name..'()' end
	for _,token in ipairs(Syntax.tokenize(src)) do
		if token.kind=='error' and (token.text:sub(1,1)=='"' or token.text:sub(1,1)=="'") then
			problems[#problems+1]='Unterminated string'
		end
	end
	if self.mode~='Expression' and self.mode~='OneTime' and not src:match('^%s*{[^{}]+}%s*$') then
		problems[#problems+1]=self.mode..' needs a single {source} path'
	end
	return problems,false
end
function BindingEditor:suggestions()
	if self.dismissed then return {} end
	local line,col=self:line()
	local prefix,kind=Syntax.wordAt(line,col+1)
	local out={}
	if kind=='path' then
		for _,path in ipairs(self.paths) do
			if path:lower():sub(1,#prefix)==prefix:lower() and path~=prefix then
				out[#out+1]={label=path,insert=path:sub(#prefix+1),detail='property'}
			end
		end
	elseif prefix and #prefix>=1 then
		for _,fn in ipairs(self.functionList) do
			local name=fn.FunctionName
			if name:sub(1,#prefix)==prefix:upper() and name~=prefix:upper() then
				out[#out+1]={label=name,insert=name:sub(#prefix+1)..'(',detail=fn.Summary,fn=fn}
			end
		end
	end
	return out
end
function BindingEditor:invalidate()
	self.input.functions=self.functions or {}
	self.input:render()
	self.view:findChild('Mode',true).Text=self.mode
	local problems,empty=self:diagnose()
	local line,col=self:line()
	local rows=1
	for _ in self:text():sub(1,self:caret()):gmatch('\n') do rows=rows+1 end
	local status=self.message or (empty and 'Empty expression: Apply removes the binding')
		or (#problems>0 and table.concat(problems,'; ')) or 'Expression looks valid'
	local a,b=Syntax.matchBracket(line,col+1)
	if b then status=status..string.format(' · Brackets %d ↔ %d',a,b) end
	self.view:findChild('Diagnostics',true).Text=status..string.format(' · Ln %d, Col %d',rows,col+1)
	local name,arg=Syntax.callAt(line,col+1)
	local fn=name and self.functions[name:upper()]
	self.view:findChild('Signature',true).Text=fn and (fn.Signature..' · argument '..(arg+1)..' · '..fn.Summary)
		or 'Use {property paths}, function calls and # comments. Enter continues with the current indentation.'
	self.completions=self:suggestions()
	self.choice=math.max(1,math.min(self.choice or 1,#self.completions))
	local showing=#self.completions>0
	self.palette.Visible=not showing
	self.completionView.Visible=showing
	if showing then
		local rows={}
		for i,candidate in ipairs(self.completions) do rows[#rows+1]={Label=candidate.label,Detail=candidate.detail,State=i==self.choice and 'completion-selected' or ''} end
		self.completionView.ItemsSource=Native.records(rows)
	end
end
function BindingEditor:edited()
	self.message,self.dismissed,self.choice=nil,false,1
	self:invalidate()
end
function BindingEditor:invoke(record)
	if record.FunctionName then
		self:insert(record.FunctionName..'()')
		self.input.Cursor=self.input.Cursor-1
		self:edited()
	elseif record.Index then self:accept(self.completions[record.Index]) end
	self:focus()
	return true
end
function BindingEditor:accept(candidate)
	if not candidate then return end
	self:insert(candidate.insert)
	local src,cursor=self:text(),self:caret()
	if candidate.fn and src:sub(cursor+1,cursor+1)~=')' then
		self:insert(')')
		self.input.Cursor=cursor
	elseif not candidate.fn and src:sub(cursor+1,cursor+1)=='}' then self.input.Cursor=cursor+1 end
	self:edited()
end
function BindingEditor:onText(text)
	local src,cursor=self:text(),self:caret()
	local nextChar=src:sub(cursor+1,cursor+1)
	if (text==')' or text=='}' or text==']' or text=='"') and nextChar==text then
		self.input.Cursor=cursor+1
	elseif CLOSERS[text] and (nextChar=='' or nextChar:match('[%s%),%]}]')) then
		self:insert(text..CLOSERS[text])
		self.input.Cursor=self.input.Cursor-1
	else self:insert(text) end
	self:edited()
end
function BindingEditor:chooseMode()
	local items={}
	for _,mode in ipairs(MODES) do
		items[#items+1]={label=mode,checked=mode==self.mode,action=function() self.mode=mode self:invalidate() end}
	end
	local button=self.view:findChild('Mode',true)
	local x,y=Native.origin(button)
	Menu.show(items,x,y+(button.ActualHeight or 32))
end
function BindingEditor:onKey(chord,args)
	if not chord then return false end
	local src,cursor=self:text(),self:caret()
	local line,col,start=self:line()
	local completing=#(self.completions or {})>0
	if chord=='escape' then
		if completing then self.dismissed=true self:invalidate() else Popups.close(self) end
	elseif chord=='ctrl+enter' then self:apply()
	elseif chord=='tab' then
		if completing then self:accept(self.completions[self.choice]) else self:insert('  ') self:edited() end
	elseif chord=='ctrl+space' then self.dismissed=false self:invalidate()
	elseif chord=='up' and completing then self.choice=math.max(1,self.choice-1) self:invalidate()
	elseif chord=='down' and completing then self.choice=math.min(#self.completions,self.choice+1) self:invalidate()
	elseif chord=='enter' then
		if completing then self:accept(self.completions[self.choice]) else self:insert('\n'..line:match('^%s*')) self:edited() end
	elseif chord=='backspace' and cursor>0 then
		local first=cursor
		while first>1 and (src:byte(first)&0xc0)==0x80 do first=first-1 end
		local before,after=src:sub(first,cursor),src:sub(cursor+1,cursor+1)
		local pair=CLOSERS[before]==after and 1 or 0
		self.input.Text=src:sub(1,first-1)..src:sub(cursor+1+pair)
		self.input.Cursor=first-1
		self:edited()
	elseif chord=='delete' then
		local after=cursor+2
		while after<=#src and (src:byte(after)&0xc0)==0x80 do after=after+1 end
		self.input.Text=src:sub(1,cursor)..src:sub(after)
		self:edited()
	elseif chord=='left' then
		local target=math.max(0,cursor-1)
		while target>0 and (src:byte(target+1)&0xc0)==0x80 do target=target-1 end
		self.input.Cursor=target self.dismissed=true self:invalidate()
	elseif chord=='right' then
		local target=math.min(#src,cursor+1)
		while target<#src and (src:byte(target+1)&0xc0)==0x80 do target=target+1 end
		self.input.Cursor=target self.dismissed=true self:invalidate()
	elseif chord=='home' then self.input.Cursor=start-1 self.dismissed=true self:invalidate()
	elseif chord=='end' then self.input.Cursor=start-1+#line self.dismissed=true self:invalidate()
	elseif chord=='up' or chord=='down' then
		local target
		if chord=='up' then
			if start<=1 then return true end
			local before=src:sub(1,start-2)
			local prev=1
			for pos in before:gmatch('()\n') do prev=pos+1 end
			target=prev-1+math.min(col,#before-prev+1)
		else
			local nextStart=src:find('\n',cursor+1,true)
			if not nextStart then return true end
			local finish=src:find('\n',nextStart+1,true) or #src+1
			target=nextStart+math.min(col,finish-nextStart-1)
		end
		self.input.Cursor=target
		self.dismissed=true self:invalidate()
	else return false end
	return true
end
function BindingEditor:apply()
	local problems,empty=self:diagnose()
	if #problems>0 then self.message=table.concat(problems,'; ') self:invalidate() return false end
	local ok,err=Workspace.setBinding(self.opts.path,self.opts.property,empty and '' or Syntax.flatten(self:text()),self.mode)
	if ok then Popups.close(self) else self.message=err self:invalidate() end
	return ok
end
function BindingEditor.open(opts)
	local editor=BindingEditor(opts)
	local width,height=Popups.size()
	Popups.push(editor,math.max(0,(width-editor.view.Width)/2),math.max(36,(height-editor.view.Height)/3),true)
	if Workspace.connection then
		Workspace.configureSource(editor.source,'get/data-functions')
		editor:catalog()
		editor:invalidate()
	end
	return editor
end
return BindingEditor
