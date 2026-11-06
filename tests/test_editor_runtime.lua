-- Local editor/runtime contract: actual native pipe + XML source + commands.
-- Kept separate from the headless suite because it starts a rendered project.
local orca = require "orca"
orca.init()
local fs, core = require "orca.filesystem", require "orca.core"
require("orca.renderer").init(800, 600, true)
fs.init("apps/Editor")
require("orca.core.startup").load_plugins()
local Workspace = require "Editor.Scripts.Workspace"
local fixture = os.tmpname()
os.remove(fixture)
assert(fs.makeDirectory(fixture))
local function write(path, text)
    local file = assert(io.open(fixture .. '/' .. path, 'w'))
    file:write(text)
    file:close()
end
write('package.lua', [[Name='EditorFixture'
StartupScreen='EditorFixture/Main'
WindowWidth=640
WindowHeight=480
EnginePlugins={{Name='orca.UIKit'}}
]])
write('Main.xml', [[<Screen Name="Main" Width="640" Height="480" ClearColor="#123456">
  <StackView Name="Root">
    <TextBlock Name="Title" Text="Hello"/>
    <Button Name="Action" Text="Run"/>
  </StackView>
</Screen>]])

local connection = fs.ProcessConnection {}
local source = fs.XmlDataSource { Schema='Editor/Data/Editor.Schema.xml', AutoLoad=false }
local function find(field, value)
    for row in source.Items.children do if row[field] == value then return row end end
end
local function count()
    local n = 0
    for _ in source.Items.children do n = n + 1 end
    return n
end
local ok, err = xpcall(function()
    Workspace.start(fixture, connection)
    Workspace.open('EditorFixture/Main')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main'))
    local root = source.Items
    local title = assert(find('Path', 'EditorFixture/Main/Root/Title'))
    assert(title.ClassName == 'TextBlock' and title.Indent == 32)
    local id = title.Id
    Workspace.rename(title.Path, 'Renamed')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main'))
    assert(source.Items == root and find('Id', id) == title, 'Rename retains native record identity')
    assert(title.Path == 'EditorFixture/Main/Root/Renamed' and title.Label == 'Renamed')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main?collapse=EditorFixture/Main/Root'))
    assert(not find('Id', id), 'Collapsed hierarchy excludes descendant records')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main?filter=Renamed'))
    assert(find('Id', id) and count() == 1, 'Runtime supplies filtered rows')

    local path = 'EditorFixture/Main/Root/Renamed'
    assert(Workspace.configureSource(source, 'get/data-properties/' .. path))
    local text = assert(find('PropertyName', 'TextRun.Text'))
    assert(text.Value == 'Hello' and text.IsProperty and text.IsText and not text.ReadOnly and not text.ValueReadOnly,
        ('Text record: value=%s kind=%s text=%s readonly=%s'):format(tostring(text.Value), tostring(text.Kind), tostring(text.IsText), tostring(text.ReadOnly)))
    assert(Workspace.setValue(path, 'TextRun.Text', 'Changed & escaped'))
    assert(Workspace.configureSource(source, 'get/data-properties/' .. path))
    assert(find('PropertyName', 'TextRun.Text').Value == 'Changed & escaped')
    assert(Workspace.undo())
    assert(Workspace.configureSource(source, 'get/data-properties/' .. path))
    assert(find('PropertyName', 'TextRun.Text').Value == 'Hello')
    assert(Workspace.redo())
    assert(Workspace.setBinding(path, 'TextRun.Text', '"Bound"', 'Expression'))
    assert(Workspace.configureSource(source, 'get/data-properties/' .. path))
    assert(find('PropertyName', 'TextRun.Text').HasBinding)
    assert(Workspace.setBinding(path, 'TextRun.Text', '', 'Expression'))
    assert(Workspace.setValue(path, 'TextRun.Text', 'Saved'))
    local snapshot = assert(Workspace.snapshot(400, 300), 'Runtime renders the editor preview')
    local image = assert(io.open(snapshot, 'rb'))
    assert(image:read(8) == '\137PNG\13\10\26\10', 'Preview is a native PNG frame')
    image:close()
    os.remove(snapshot)
    Workspace.lastSnapshot = nil
    Workspace.save()
    local file = assert(io.open(fixture .. '/Main.xml'))
    local saved = file:read('*a')
    file:close()
    assert(saved:find('Name="Renamed"', 1, true) and saved:find('Saved', 1, true))

    assert(Workspace.configureSource(source, 'get/data-project'), source.ErrorMessage)
    assert(find('SourcePath', 'EditorFixture/Main'), 'Project source exposes openable documents')
    assert(not Workspace.configureSource(source, 'get/data-scene/does-not-exist'))
    assert(source.HasError and source.ErrorMessage ~= '')
    assert(Workspace.configureSource(source, 'get/data-project') and not source.HasError)
    core.flushQueue()
end, debug.traceback)
connection:close()
os.remove(fixture .. '/Main.xml')
os.remove(fixture .. '/package.lua')
os.remove(fixture)
assert(ok, err)
print('Editor runtime data-source and edit/save contract passed.')
