-- XML chrome uses UIKit controls and native records, including template events.
local orca = require "orca"
orca.init()
local fs, core, ui = require "orca.filesystem", require "orca.core", require "orca.UIKit"
require("orca.renderer").init(1440, 900, true)
fs.init('apps/Editor')
require('orca.core.startup').load_plugins()
local Native = require 'Editor.Scripts.Native'
local Workspace = require 'Editor.Scripts.Workspace'
local Popups = require 'Editor.Scripts.Popups'
local Menu = require 'Editor.Scripts.Menu'
local Panel = require 'Editor.Scripts.Panel'
local MenuBar = require 'Editor.Scripts.MenuBar'
local Console = require 'Editor.Scripts.Console'
local TextInput = require 'Editor.Scripts.TextInput'
local Dialog = require 'Editor.Scripts.Dialog'
local BindingEditor = require 'Editor.Scripts.BindingEditor'
local screen = ui.Screen {Width=1440,Height=900,ResizeMode='NoResize'}
local overlay = ui.Node2D {Visible=false}
screen:addChild(overlay)
Popups.attach(overlay)
local function settle()
 core.flushQueue()
 core.advanceFrame()
 core.runAllPrograms(screen)
 core.flushQueue()
 screen:UpdateLayout(1440,900)
 local popup=Popups.top()
 if popup and popup.checkResize then popup:checkResize() end
end
local function press(node) node:send('Node.LeftButtonUp'); settle() end
local function mouseClick(node)
 local x,y=Native.origin(node)
 orca.system.dispatchMessage{target=screen,message='LeftButtonDown',x=x+math.min(12,node.ActualWidth/2),y=y+node.ActualHeight/2}
 orca.system.dispatchMessage{target=screen,message='LeftButtonUp',x=x+math.min(12,node.ActualWidth/2),y=y+node.ActualHeight/2}
 settle()
end
local pressed=0
local menu=Menu.show({{label='Save',action=function() pressed=pressed+1 end},{separator=true},{label='Disabled',disabled=true}},20,30)
settle()
local row=assert(menu.view:getFirstChild())
local button=row:findChild('Button',true)
-- Resolve the button by class-independent child traversal.
button=row:getFirstChild():getNext()
assert(button and button:getClassName()=='Button', 'Menu content uses a native button with a grid')
local mouseX,mouseY=Native.origin(button)
orca.system.dispatchMessage{target=screen,message='LeftButtonDown',x=mouseX+12,y=mouseY+16}
orca.system.dispatchMessage{target=screen,message='LeftButtonUp',x=mouseX+12,y=mouseY+16}
settle()
assert(pressed==1 and #Popups.stack==0,'Template Click invokes the owning native menu action')
local lastAction=0
local manyItems={}
for i=1,7 do manyItems[#manyItems+1]={label='Item '..i,action=function() end} end
manyItems[#manyItems+1]={separator=true}
manyItems[#manyItems+1]={label='Quit',action=function() lastAction=lastAction+1 end}
local longMenu=Menu.show(manyItems,20,30)
settle()
local lastRow=longMenu.view:getFirstChild()
while lastRow and (not lastRow.DataContext or lastRow.DataContext.Index~=9) do lastRow=lastRow:getNext() end
assert(lastRow,'Last menu item is present')
mouseClick(lastRow:getFirstChild():getNext())
assert(lastAction==1 and #Popups.stack==0,'Screen-routed pointer clicks activate the last menu row after separators')
local bar=MenuBar({{title='File',items=function() return {{label='Save',action=function() end}} end}},function() return 'ORCA Editor' end)
screen:addChild(bar.view)
settle()
assert(bar.view.Height==28 and bar:indexAt(20)==1,'Menu titles use native dimensions and hit bounds')
local console=Console()
screen:addChild(Panel.wrap(console))
overlay:moveBefore(nil)
Workspace.log('error','A useful error with <literal> & text')
settle()
local messages=console.view:findChild('Messages',true)
assert(messages.ItemsSource:getFirstChild().Label=='A useful error with <literal> & text','Local log records preserve literal data')
assert(messages:getFirstChild(),'Native ItemsStack materializes log rows')
console:tools()[1].action()
assert(console.view:findChild('Empty',true).Visible,'Clear restores the empty state')
local accepted
local input=TextInput.show(50,50,{text='Before',selectAll=true,onAccept=function(value) accepted=value end})
input.input:post('Node.TextInput',{text='Z',character=90})
settle()
assert(input.input.Text=='Z','First typing replaces the selected rename value')
input.input.Text='After'
input.input:post('Node.KeyDown',{keyCode=13})
settle()
assert(accepted=='After','Inline native input submits its real value')
local dialog=Dialog.show('Keyboard shortcuts',{{'Ctrl+S','Save document'}})
settle()
assert(dialog.view:findChild('Lines',true):getFirstChild(),'Dialog content uses native rows')
press(dialog.view:findChild('Close',true))
assert(#Popups.stack==0,'Close button dismisses dialog')
Workspace.connection=nil
local editor=BindingEditor.open{path='Example/Screen/Title',name='Text',property='TextRun.Text',type='String',expression=''}
local functions={'<response status="success"><EditorFunction name="IF" FunctionName="IF" Signature="IF(condition, a, b)" Summary="Choose a value"/>'}
for i=1,80 do functions[#functions+1]='<EditorFunction name="FN'..i..'" FunctionName="FN'..i..'" Signature="FN'..i..'(value)" Summary="Test function"/>' end
functions[#functions+1]='</response>'
editor.source.Content=table.concat(functions)
editor.source:reload()
editor:catalog()
settle()
assert(editor.palette:getFirstChild(), 'Function palette binds the native runtime data source directly')
local choices=editor.view:findChild('Choices',true)
local _,choiceY=Native.origin(choices)
local _,applyY=Native.origin(editor.view:findChild('Apply',true))
assert(choices.ActualHeight==112 and choices.ScrollHeight>112,'Large function catalogs stay in a fixed-height scrolling viewport')
assert(applyY>=choiceY+112,'The function viewport cannot cover the binding dialog footer')
local functionButton=editor.palette:getFirstChild()
press(functionButton)
assert(editor:text()=='IF()' and editor:caret()==3, 'Function palette inserts a call through its native template event')
assert(editor.input.view:getClassName()=='ConsoleView','Only the code surface uses the syntax-colored character control')
assert(table.concat(editor.input.rendered):find('![14;0m',1,true),'Code rendering preserves function syntax colors')
assert(table.concat(editor.input.rendered):find('![15;1m',1,true),'Matching brackets are highlighted at the code caret')
editor.input.Text='' editor.input.Cursor=0
editor:edited()
editor.input:post('Node.TextInput',{text='(',character=40})
settle()
assert(editor:text()=='()' and editor:caret()==1,'Code input preserves automatic bracket pairs')
editor.input:post('Node.TextInput',{text=')',character=41})
settle()
assert(editor:text()=='()' and editor:caret()==2,'Typing a closer advances over its pair')
editor.input.Text='' editor.input.Cursor=0
editor:edited()
editor.input:setFocus()
orca.system.dispatchMessage{target=screen,message='KeyDown',key=32,text='space'}
settle()
assert(editor:text()=='','Code input waits for translated native text after physical keys')
orca.system.dispatchMessage{target=screen,message='Char',key=32,text=' '}
settle()
assert(editor:text()==' ','A native space is inserted once rather than the key name')
editor.input.Text='' editor.input.Cursor=0
editor:edited()
orca.system.dispatchMessage{target=screen,message='KeyDown',key=91,text='[',modflags=1}
settle()
assert(editor:text()=='','A shifted physical key does not insert unshifted text')
orca.system.dispatchMessage{target=screen,message='Char',key=123,text='{',modflags=1}
settle()
assert(editor:text()=='{}' and editor:caret()==1,'Native shifted braces preserve automatic pairs without a stray bracket')
editor.input.Text='' editor.input.Cursor=0
editor:edited()
orca.system.dispatchMessage{target=screen,message='KeyDown',key=105,text='i',modflags=1}
orca.system.dispatchMessage{target=screen,message='Char',key=73,text='I',modflags=1}
settle()
assert(editor:text()=='I','Native translated text preserves uppercase letters')
editor.input.Text='' editor.input.Cursor=0
editor:edited()
orca.system.dispatchMessage{target=screen,message='Char',key=123,text='{',modflags=4}
settle()
assert(editor:text()=='{}' and editor:caret()==1,'Option or Alt translated braces are accepted without a layout mapping')


editor.input.Text='first\nsecond' editor.input.Cursor=0
editor:edited()
settle()
local codeX,codeY=Native.origin(editor.input.view)
editor.input:post('Node.LeftButtonDown',{x=codeX+7*8,y=codeY+16+4})
settle()
assert(editor:caret()==8,'Pointer coordinates select the correct code row and column')
editor.input.Text='UNKNOWN(' editor.input.Cursor=8
editor:edited()
assert(table.concat(editor.input.rendered):find('![12;0m',1,true),'Unknown function calls retain syntax error colors')
editor.input.Text='FN' editor.input.Cursor=2
editor:edited()
settle()
assert(editor.completionView.Visible and choices.ActualHeight==112 and editor.completionView.ActualHeight==112 and editor.completionView.ScrollHeight>112,'Many completion rows remain in the clipped scrolling viewport')
local _,completionY=Native.origin(editor.completionView)
assert(completionY+choices.ActualHeight<=applyY,'Completion viewport cannot cover Apply or Cancel')
editor.input.Text='{DataContext/Title}'
editor.mode='TwoWay'
assert(#editor:diagnose()==0,'Writable binding modes accept a single source path')
editor.input.Text='1 + 2'
assert(#editor:diagnose()==1,'Writable binding modes reject computed expressions')
local applied
Workspace.setBinding=function(path,property,expression,mode) applied={path,property,expression,mode}; return true end
editor.mode='Expression'
editor.input.Text='1 + 2\n# comment'
mouseClick(editor.view:findChild('Apply',true))
assert(applied[3]=='1 + 2' and #Popups.stack==0,'Native Apply validates and commits the flattened expression')
local cancelEditor=BindingEditor.open{path='Example/Screen/Title',name='Text',property='TextRun.Text',type='String',expression=''}
cancelEditor.source.Content=table.concat(functions)
cancelEditor.source:reload() cancelEditor:catalog()
settle()
mouseClick(cancelEditor.view:findChild('Cancel',true))
assert(#Popups.stack==0,'Screen-routed Cancel remains clickable beneath a large palette')
print('PASS: native editor chrome, template events, popup inputs, and binding controls')
