-- Run with tools/benchmark_editor_selection.py; this is a benchmark, not a test suite.
local orca = require 'orca'
orca.init()
local fs, core = require 'orca.filesystem', require 'orca.core'
require('orca.UIKit')
require('orca.renderer').init(1440, 900, true)
fs.init('apps/Editor')
require('orca.core.startup').load_plugins()
local Workspace = require 'Editor.Scripts.Workspace'
local fixture = assert(os.getenv('ORCA_BENCH_FIXTURE'), 'Run through tools/benchmark_editor_selection.py')
local function write(name, data) local f=assert(io.open(fixture..'/'..name,'w')); f:write(data); f:close() end
write('package.lua', [[Name='BenchFixture'
StartupScreen='BenchFixture/Main'
WindowWidth=640
WindowHeight=480
EnginePlugins={{Name='orca.UIKit'}}
ScreenLibrary={IsExternal=true}
]])
write('Main.xml', [[<Screen Name="Main" Width="640" Height="480"><StackView Name="Root"><TextBlock Name="Title" Text="Hello"/><Button Name="Action" Text="Run"/></StackView></Screen>]])

local Hierarchy, Inspector = require 'Editor.Scripts.Hierarchy', require 'Editor.Scripts.Inspector'
local hierarchy, inspector = Hierarchy(), Inspector(function() end)
local screen = assert(fs.loadObjectFromDisk('Editor/Screens/Editor'))
screen.Width, screen.Height, screen.ResizeMode = 1440, 900, 'NoResize'
local conn = assert(screen:findChild('Runtime', true))
Workspace.start(fixture, conn)
local Panel = require 'Editor.Scripts.Panel'
screen:findChild('HierarchySlot', true):addChild(Panel.wrap(hierarchy))
screen:findChild('InspectorSlot', true):addChild(Panel.wrap(inspector))
local function settle()
 core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
 core.runAllPrograms(screen); core.flushQueue(); screen:UpdateLayout(1440,900)
end
Workspace.open('BenchFixture/Main')
settle()
hierarchy:expandAll(true)
settle()
local paths = {'BenchFixture/Main/Root/Title','BenchFixture/Main/Root/Action'}
local mode = os.getenv('ORCA_BENCH_MODE') or 'preview'
assert(mode == 'preview' or mode == 'tree', 'Expected preview or tree benchmark mode')
local preview, positions
if mode == 'preview' then
 local Preview = require 'Editor.Scripts.Preview'
 local image, rects = Workspace.snapshot(640, 480)
 assert(image and rects)
 preview = { scale=1, canvas={x=0,y=0}, rects=rects, origin=function() return 0,0 end, hitTest=Preview.hitTest }
 positions = {}
 for _, r in ipairs(rects) do
  for index, path in ipairs(paths) do
   if r.path == path then
    for y=math.floor(r.y),math.ceil(r.y+r.h)-1 do
     for x=math.floor(r.x),math.ceil(r.x+r.w)-1 do
      local hit=preview:hitTest{x=x+0.5,y=y+0.5}
      if hit and hit.path==path then positions[index]={x=x+0.5,y=y+0.5}; break end
     end
     if positions[index] then break end
    end
   end
  end
 end
 assert(positions[1] and positions[2], 'both elements need distinct hittable runtime snapshot regions')
 print('preview hit positions',positions[1].x,positions[1].y,positions[2].x,positions[2].y)
 preview.pick=Preview.pick
end
for i=1, tonumber(os.getenv('ORCA_BENCH_ITERATIONS')) or 7 do
 local path=paths[(i-1)%2+1]
 io.write('BEGIN ',i,'\n'); io.flush()
 local cpuStart = os.clock()
 if mode == 'preview' then preview:pick(positions[(i-1)%2+1])
 else hierarchy.list:send('TreeView.SelectItem', {Value=path}) end
 settle()
 if path == paths[1] then
  local record
  for row in inspector.source.Items.children do
   if row.PropertyName == 'TextRun.Text' then record = row break end
  end
  assert(record, 'Text property missing')
  local view = assert(inspector.view:findChild('Property' .. record:getName(), true), 'Text row missing')
  local input = assert(view:findChild('PropertyText', true), 'Text editor missing')
  assert(input.Text == 'Hello', 'Text editor has not received its value')
 end
 assert(Workspace.selection==path and inspector.path==path and not inspector.source.HasError, tostring(Workspace.selection)..' '..tostring(inspector.path)..' '..tostring(inspector.source.ErrorMessage))
 io.write('CPU ',i,' ',(os.clock()-cpuStart)*1000,'\n'); io.flush()
 io.write('END ',i,'\n'); io.flush()
end
conn:close()
if Workspace.lastSnapshot then os.remove(Workspace.lastSnapshot) end
