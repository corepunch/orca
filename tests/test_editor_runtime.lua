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
assert(fs.makeDirectory(fixture .. '/Styles'))
write('Styles/base.css', 'TextBlock { color: #123456; }')
write('Styles/alternate.css', 'TextBlock { color: #654321; }')
write('Main.xml', [[<Screen Name="Main" Width="640" Height="480" ClearColor="#123456"
    SafeAreaLeft="1" SafeAreaTop="2" SafeAreaRight="3" SafeAreaBottom="4"
    StyleSheet="EditorFixture/Styles/base.css">
  <StackView Name="Root" Padding="8 12" ContentOffset="3 4">
    <TextBlock Name="Title" Text="Hello" FontSize="14"/>
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

    local screen_path = 'EditorFixture/Main'
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path))
    local safe = assert(find('PropertyName', 'Screen.SafeArea'))
    assert(safe.IsNumericCompound and safe.IsCompound and not safe.IsText and not safe.IsBindable)
    assert(not find('PropertyName', 'Screen.SafeAreaRight'), 'Collapsed groups do not emit duplicate leaf rows')
    local fields = assert(safe:findChild('Fields', false))
    local labels, leaves = {}, {}
    for field in fields.children do
        labels[#labels + 1], leaves[field.PropertyName] = field.ShortLabel, field
        assert(field.IsText and field.IsBindable and not field.ReadOnly, 'Compound leaves remain normal editable properties')
        assert(field.DraftValue == field.Value, 'Runtime initializes each compact numeric draft from its committed value')
    end
    assert(table.concat(labels, ',') == 'L,T,R,B', 'Safe area has explicit declaration order and compact edge labels')
    assert(leaves['Screen.SafeAreaLeft'].Value == '1' and leaves['Screen.SafeAreaRight'].Value == '3')
    leaves['Screen.SafeAreaLeft'].DraftValue = '91'
    assert(leaves['Screen.SafeAreaLeft'].Value == '1', 'Native draft editing leaves the committed runtime value intact until submit')
    assert(Workspace.setValue(screen_path, 'Screen.SafeAreaRight', '23'))
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path .. '?filter=SafeAreaRight'))
    assert(find('PropertyName', 'Screen.SafeArea').Expanded and find('PropertyName', 'Screen.SafeAreaRight').Value == '23')
    assert(find('PropertyName', 'Screen.SafeAreaLeft').Value == '1', 'Editing one edge preserves its siblings')
    assert(find('PropertyName', 'Screen.SafeAreaLeft').DraftValue == '1', 'An authoritative refresh resets the discarded native draft')
    assert(Workspace.undo())
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path))
    local resize = assert(find('PropertyName', 'Screen.ResizeMode'))
    assert(resize.IsEnum and not resize.IsText and resize:findChild('Options', false), 'Enums expose native choices')
    local color = assert(find('PropertyName', 'Screen.ClearColor'))
    assert(color.IsColor and not color.IsText and color.Value == '#123456')
    assert(color.DraftValue == color.Value, 'Color editors receive their own native draft value')
    local sheet = assert(find('PropertyName', 'StyleController.StyleSheet'))
    assert(sheet.IsReference and not sheet.ValueReadOnly and sheet.ReferenceType == 'StyleSheet')
    assert(sheet.ReferencePath == 'EditorFixture/Styles/base.css' and sheet.ReferenceLabel == 'base.css',
        'File resources retain their canonical loadable source path')
    assert(Workspace.configureSource(source, 'get/data-resources/' .. screen_path .. '?property=StyleController.StyleSheet&icons=Editor/Icons'))
    assert(find('Value', '') and find('Value', 'EditorFixture/Styles/base.css'))
    assert(find('Value', 'EditorFixture/Styles/base.css').Label == 'base.css', 'Picker labels match canonical resource chips')
    assert(find('Value', 'EditorFixture/Styles/alternate.css') and not find('Kind', 'Screen'),
        'Resource picker lists compatible declarations without unrelated object types')
    assert(Workspace.setValue(screen_path, 'StyleController.StyleSheet', 'EditorFixture/Styles/alternate.css'))
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path))
    assert(find('PropertyName', 'StyleController.StyleSheet').ReferencePath == 'EditorFixture/Styles/alternate.css')
    assert(Workspace.undo())
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path))
    assert(find('PropertyName', 'StyleController.StyleSheet').ReferencePath == 'EditorFixture/Styles/base.css',
        'Resource edits participate in undo using the previous canonical path')
    assert(Workspace.setValue(screen_path, 'StyleController.StyleSheet', ''))
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path))
    assert(find('PropertyName', 'StyleController.StyleSheet').ReferenceLabel == 'None (StyleSheet)')
    assert(Workspace.undo())
    assert(not Workspace.setValue(screen_path, 'StyleController.StyleSheet', 'EditorFixture/Main'),
        'Resource assignment rejects incompatible types')
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path))
    assert(find('PropertyName', 'StyleController.StyleSheet').ReferencePath == 'EditorFixture/Styles/base.css')
    assert(Workspace.configureSource(source, 'get/data-properties/EditorFixture/Main/Root'))
    local padding = assert(find('PropertyName', 'Node.Padding'))
    assert(padding.IsNumericCompound and not padding.IsText)
    local values = {}
    for field in padding:findChild('Fields', false).children do values[#values + 1] = field.ShortLabel .. '=' .. field.Value end
    assert(table.concat(values, ',') == 'L=8,T=12,R=8,B=12', 'Thickness fields follow documented XAML edge order')

    local offset = assert(find('PropertyName', 'Node2D.ContentOffset'))
    assert(offset.IsNumericCompound and offset.IsBindable and not offset.IsText, 'Atomic numeric structs use compact reflected component editors')
    local component_names = {}
    for field in offset:findChild('Fields', false).children do
        component_names[#component_names + 1] = field.PropertyName
        assert(field.IsText and not field.IsBindable and not field.ReadOnly,
            'Synthetic scalar component editors write values without claiming native binding properties')
        assert(field.DraftValue == field.Value, 'Reflected vector components initialize a separate native draft')
    end
    assert(component_names[1] == 'Node2D.ContentOffset.X' and component_names[2] == 'Node2D.ContentOffset.Y')
    local root_path = 'EditorFixture/Main/Root'
    assert(Workspace.setValue(root_path, component_names[1], '30'))
    assert(Workspace.configureSource(source, 'get/data-properties/' .. root_path .. '?expand=Node2D.ContentOffset'))
    assert(find('PropertyName', component_names[1]).Value == '30' and find('PropertyName', component_names[2]).Value == '4',
        'A reflected component edit preserves its sibling in the parent payload')
    assert(Workspace.undo())
    assert(Workspace.configureSource(source, 'get/data-properties/' .. root_path .. '?expand=Node2D.ContentOffset'))
    assert(find('PropertyName', component_names[1]).Value == '3' and find('PropertyName', component_names[2]).Value == '4')
    assert(Workspace.redo())
    assert(not Workspace.setValue(root_path, component_names[1], 'not-a-number'), 'Component editors reject invalid numeric input')
    assert(not Workspace.setValue(root_path, component_names[1], ' '), 'Empty component input does not silently become zero')

    local path = 'EditorFixture/Main/Root/Renamed'
    assert(Workspace.configureSource(source, 'get/data-properties/' .. path))
    local font = assert(find('PropertyName', 'TextRun.Font'))
    assert(font.IsSummary and not font.IsText and not font.IsNumericCompound)
    assert(not find('PropertyName', 'TextRun.FontSize'), 'Mixed font shorthand starts as a compact summary')
    assert(Workspace.configureSource(source, 'get/data-properties/' .. path .. '?expand=TextRun.Font'))
    assert(find('PropertyName', 'TextRun.FontSize').Value == '14' and find('PropertyName', 'TextRun.FontWeight').IsEnum)
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
    assert(saved:find('ContentOffset="30 4"', 1, true), 'Component edits save as the standard parent vector attribute')
    assert(Workspace.setValue(screen_path, 'StyleController.StyleSheet', ''))
    Workspace.save()
    file = assert(io.open(fixture .. '/Main.xml'))
    local cleared = file:read('*a')
    file:close()
    assert(not cleared:find('StyleSheet=', 1, true), 'Clearing a resource removes its source attribute rather than saving an unloadable empty reference')
    assert(Workspace.undo())
    Workspace.save()
    file = assert(io.open(fixture .. '/Main.xml'))
    local restored = file:read('*a')
    file:close()
    assert(restored:find('StyleSheet="EditorFixture/Styles/base.css"', 1, true), 'Resource reference undo remains valid across saves')

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
os.remove(fixture .. '/Styles/base.css')
os.remove(fixture .. '/Styles/alternate.css')
os.remove(fixture .. '/Styles')
os.remove(fixture)
assert(ok, err)
print('Editor runtime data-source and edit/save contract passed.')
