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
ScreenLibrary={IsExternal=true}
ThemeLibrary={{Key='fixture-accent',Value='#123456'}}
]])
write('Widgets.xml', [[<Library>
  <StackView Name="Card"><TextBlock Name="Caption" Text="Card"/></StackView>
  <TextBlock Name="Badge" Text="New"/>
</Library>]])
assert(fs.makeDirectory(fixture .. '/Styles'))
write('Styles/base.css', 'TextBlock { color: #123456; }')
write('Styles/alternate.css', 'TextBlock { color: #654321; }')
local deep_path = ''
for i = 1, 6 do
    deep_path = deep_path .. (i == 1 and '' or '/') .. 'Level' .. i
    assert(fs.makeDirectory(fixture .. '/' .. deep_path))
end
write(deep_path .. '/leaf.lua', 'return {}')
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
local function find(field, value, items)
    for row in (items or source.Items).children do
        if row[field] == value then return row end
        local children = row:findChild('Children', false)
        local descendant = children and find(field, value, children)
        if descendant then return descendant end
    end
end
local function count(items)
    local n = 0
    for _ in (items or source.Items).children do n = n + 1 end
    return n
end
local ok, err = xpcall(function()
    Workspace.start(fixture, connection)
    local arguments = connection.Arguments
    assert(#arguments == 2 and arguments[1].Value == '-server=true' and arguments[2].Value == fixture,
        'The editor starts its project runtime in offscreen server mode with literal project argv')
    Workspace.open('EditorFixture/Main')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main'))
    local root = source.Items
    local screen_record = assert(find('Path', 'EditorFixture/Main'))
    local screen_children = assert(screen_record:findChild('Children', false))
    local branch = assert(find('Path', 'EditorFixture/Main/Root'))
    local branch_children = assert(branch:findChild('Children', false))
    local title = assert(find('Path', 'EditorFixture/Main/Root/Title'))
    assert(count() == 1 and count(screen_children) == 1 and count(branch_children) == 2,
        'The source exposes only roots; each record owns its direct child collection')
    assert(title.ClassName == 'TextBlock' and title.parent == branch_children and branch.parent == screen_children)
    assert(screen_record.HasChildren and branch.HasChildren and not title.HasChildren,
        'Child membership, rather than flattened depth, describes the hierarchy')
    local leaf_children = assert(title:findChild('Children', false))
    assert(count(leaf_children) == 0, 'Leaf records own an empty Children collection rather than omitting the binding target')
    local id = title.Id
    Workspace.rename(title.Path, 'Renamed')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main'))
    assert(source.Items == root and find('Id', id) == title, 'Rename retains native record identity')
    assert(find('Path', 'EditorFixture/Main') == screen_record and find('Path', 'EditorFixture/Main/Root') == branch
        and branch:findChild('Children', false) == branch_children,
        'Refreshing nested records preserves parents and their child collections')
    assert(title.Path == 'EditorFixture/Main/Root/Renamed' and title.Label == 'Renamed')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main?filter=Renamed'))
    assert(find('Id', id) == title and count() == 1 and count(screen_children) == 1 and count(branch_children) == 1,
        'Filtering retains the matching record beneath its ancestor chain')
    assert(not find('Path', 'EditorFixture/Main/Root/Action'), 'Filtering excludes unrelated sibling branches')
    assert(title.parent == branch_children and branch.parent == screen_children,
        'Filtering never reparents a match into the root collection')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main'))
    assert(find('Id', id) == title and count(branch_children) == 2,
        'Clearing a filter restores siblings while retaining the matched record')

    assert(title:findChild('Children', false) == leaf_children,
        'Leaf child collections retain their identity through rename and filtering')
    local branch_path = 'EditorFixture/Main/Root/Added'
    assert(Workspace.create('EditorFixture/Main/Root', 'StackView', 'Added'))
    assert(Workspace.create(branch_path, 'TextBlock', 'Leaf'))
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main'))
    local added = assert(find('Path', branch_path))
    local added_children = assert(added:findChild('Children', false))
    local added_leaf = assert(find('Path', branch_path .. '/Leaf'))
    assert(added.parent == branch_children and added_leaf.parent == added_children
        and added.HasChildren and not added_leaf.HasChildren and count(added_children) == 1,
        'Creating a nested runtime branch populates only its owning child collections')
    assert(find('Id', id) == title and find('Path', 'EditorFixture/Main') == screen_record,
        'Structural insertion retains unrelated native records')
    assert(Workspace.remove(branch_path))
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main'))
    assert(not find('Path', branch_path) and not find('Path', branch_path .. '/Leaf')
        and count(branch_children) == 2 and find('Id', id) == title,
        'Removing a runtime branch removes its subtree without recreating retained siblings')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main?filter=__no_tree_match__'))
    assert(source.Items == root and count() == 0 and not source.HasError,
        'An unmatched scene filter is a successful empty collection')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main?filter=TextBlock'))
    local class_match = assert(find('ClassName', 'TextBlock'))
    assert(count() == 1 and class_match.parent.parent.Label == 'Root'
        and class_match.parent.parent.parent.parent.Label == 'Main',
        'Class-name matches restore their full ancestor chain after an empty result')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Main'))

    local screen_path = 'EditorFixture/Main'
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path))
    assert(Workspace.configureSource(source, 'get/data-properties/EditorFixture/Main/Root'))
    local shared_padding = assert(find('PropertyName', 'Node.Padding'))
    local left = assert(shared_padding:findChild('Fields', false)):getFirstChild()
    assert(left.Value == '8')
    assert(Workspace.configureSource(source, 'get/data-properties/EditorFixture/Main/Root/Renamed'))
    assert(find('PropertyName', 'Node.Padding') == shared_padding and left.Value == '0',
        'Common property records survive selection and receive the new value')
    assert(Workspace.configureSource(source, 'get/data-properties/EditorFixture/Main/Root'))
    assert(find('PropertyName', 'Node.Padding') == shared_padding and left.Value == '8',
        'Returning to the first object refreshes retained property values')
    assert(Workspace.configureSource(source, 'get/data-properties/' .. screen_path))
    local safe = assert(find('PropertyName', 'Screen.SafeArea'))
    assert(safe.IsNumericCompound and safe.IsCompound and not safe.IsText and not safe.IsBindable
        and safe.ValueTemplate == 'Editor/Templates/PropertyValueCompound')
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
    assert(resize.IsEnum and not resize.IsText and resize:findChild('Options', false)
        and resize.ValueTemplate == 'Editor/Templates/PropertyValueEnum', 'Enums expose native choices')
    local color = assert(find('PropertyName', 'Screen.ClearColor'))
    assert(color.IsColor and not color.IsText and color.Value == '#123456'
        and color.ValueTemplate == 'Editor/Templates/PropertyValueColor')
    assert(color.DraftValue == color.Value, 'Color editors receive their own native draft value')
    local sheet = assert(find('PropertyName', 'StyleController.StyleSheet'))
    assert(sheet.IsReference and not sheet.ValueReadOnly and sheet.ReferenceType == 'StyleSheet'
        and sheet.ValueTemplate == 'Editor/Templates/PropertyValueReference')
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
    assert(font.IsSummary and not font.IsText and not font.IsNumericCompound
        and font.ValueTemplate == 'Editor/Templates/PropertyValueSummary')
    assert(not find('PropertyName', 'TextRun.FontSize'), 'Mixed font shorthand starts as a compact summary')
    assert(Workspace.configureSource(source, 'get/data-properties/' .. path .. '?expand=TextRun.Font'))
    assert(find('PropertyName', 'TextRun.FontSize').Value == '14' and find('PropertyName', 'TextRun.FontWeight').IsEnum)
    local text = assert(find('PropertyName', 'TextRun.Text'))
    assert(text.Value == 'Hello' and text.IsProperty and text.IsText and not text.ReadOnly and not text.ValueReadOnly
        and text.ValueTemplate == 'Editor/Templates/PropertyValueText',
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
    local snapshot, _, artboardWidth, artboardHeight = Workspace.snapshot(400, 300)
    assert(snapshot, 'Runtime renders the editor preview')
    assert(artboardWidth == 640 and artboardHeight == 480, 'Preview reports the screen design size it rendered')
    local image = assert(io.open(snapshot, 'rb'))
    local header = assert(image:read(24))
    assert(header:sub(1, 8) == '\137PNG\13\10\26\10' and header:sub(13, 16) == 'IHDR',
        'The offscreen runtime produces a native PNG preview frame')
    local width, height = string.unpack('>I4I4', header, 17)
    assert(width == 640 and height == 480, 'Offscreen Preview renders the artboard at its design size')
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
    local project_record = source.Items:getFirstChild()
    local document = assert(find('SourcePath', 'EditorFixture/Main'), 'Project source exposes openable documents')
    assert(count() == 1 and document.parent == project_record:findChild('Children', false),
        'Project files are owned by their project root rather than flattened beside it')
    local deep_file = assert(find('Path', 'EditorFixture/' .. deep_path .. '/leaf.lua'),
        'Project hierarchy includes folders deeper than the old three-level cap')
    local ancestor = deep_file
    for i = 6, 1, -1 do
        ancestor = ancestor.parent.parent
        assert(ancestor.Label == 'Level' .. i, 'Each deep folder owns its direct descendants')
    end
    assert(ancestor.parent.parent == project_record, 'Deep file ancestry reaches the project root')
    local project_items = source.Items
    local deep_folder = deep_file.parent.parent
    local deep_children = deep_file.parent
    assert(deep_file:findChild('Children', false) and count(deep_file:findChild('Children', false)) == 0,
        'Project leaf records also provide an empty child collection')
    write(deep_path .. '/данные & test.lua', 'return {}')
    assert(Workspace.configureSource(source, 'get/data-project'))
    local added_file = assert(find('Path', 'EditorFixture/' .. deep_path .. '/данные & test.lua'))
    assert(added_file.Label == 'данные & test.lua' and added_file.parent == deep_children,
        'Native XML transport preserves Unicode and escaped characters in nested file paths')
    assert(find('Path', deep_folder.Path) == deep_folder and find('Path', deep_file.Path) == deep_file
        and source.Items:getFirstChild() == project_record,
        'Adding a project file retains its folder, existing siblings and project root')
    os.remove(fixture .. '/' .. deep_path .. '/данные & test.lua')
    assert(Workspace.configureSource(source, 'get/data-project'))
    assert(not find('Path', 'EditorFixture/' .. deep_path .. '/данные & test.lua')
        and deep_folder:findChild('Children', false) == deep_children and find('Path', deep_file.Path) == deep_file,
        'Removing a project file updates only its direct native collection')
    assert(Workspace.configureSource(source, 'get/data-project?filter=base.css'))
    local stylesheet = assert(find('Label', 'base.css'))
    assert(count() == 1 and stylesheet.parent.parent.Label == 'Styles'
        and stylesheet.parent.parent.parent.parent == source.Items:getFirstChild(),
        'Project filtering preserves the owning folder and project ancestor')
    assert(Workspace.configureSource(source, 'get/data-project?filter=__no_tree_match__'))
    assert(source.Items == project_items and count() == 0 and not source.HasError,
        'An unmatched project filter yields a successful empty root collection')
    assert(Workspace.configureSource(source, 'get/data-project'))
    local last_project = source.Items:getFirstChild()
    assert(not Workspace.configureSource(source, 'get/data-scene/does-not-exist'))
    assert(source.HasError and source.ErrorMessage ~= '')
    assert(source.Items == project_items and source.Items:getFirstChild() == last_project,
        'A runtime error preserves the last successful hierarchical source')
    assert(Workspace.configureSource(source, 'get/data-project') and not source.HasError)

    -- Library view: resources grouped by type rather than by folder.
    local libraries = fs.XmlDataSource { Schema='Editor/Data/Editor.Schema.xml', AutoLoad=false }
    local function library(id)
        for row in libraries.Items.children do if row.Path == '@' .. id then return row end end
    end
    local function entries(row)
        local list = {}
        for item in row:findChild('Children', false).children do list[#list + 1] = item end
        return list
    end
    assert(Workspace.configureSource(libraries, 'get/data-libraries?icons=Editor/Icons'), libraries.ErrorMessage)
    local screens = assert(library('screens'), 'Screens library is listed')
    assert(screens.IsLibrary and screens.Declared and screens.Detail == '1' and screens.Icon == 'Editor/Icons/screen.svg',
        'A library record carries its item count, declaration and icon')
    local main = entries(screens)[1]
    assert(main.Label == 'Main' and main.SourcePath == 'EditorFixture/Main' and main.Kind == 'document',
        'Screens are classified by their root element and open as documents')
    local prefabs = assert(library('prefabs'), 'Library files contribute their named children')
    local names = {}
    for _, item in ipairs(entries(prefabs)) do names[item.Label] = item end
    assert(names.Card and names.Badge and names.Card.Detail == 'Widgets'
        and names.Card.SourcePath == 'EditorFixture/Widgets/Card',
        'Each named child of a <Library> file is a library item that opens in place')
    local styles = assert(library('styles'))
    assert(#entries(styles) == 2 and entries(styles)[1].Detail == 'Styles', 'Style sheets are listed with their folder')
    assert(library('scripts') and #entries(library('scripts')) == 1, 'Lua sources form the Scripts library')
    local themes = assert(library('themes'))
    assert(entries(themes)[1].Label == 'fixture-accent' and entries(themes)[1].Detail == '#123456',
        'ThemeLibrary values are listed with their values')
    assert(not library('materials'), 'Undeclared empty libraries are omitted')
    assert(Workspace.configureSource(libraries, 'get/data-libraries?filter=alternate'))
    assert(library('styles') and #entries(library('styles')) == 1 and not library('screens'),
        'Filtering keeps only libraries with matching items')

    -- Tabs: each document keeps its own history; closing can discard edits.
    Workspace.reset()
    Workspace.open('EditorFixture/Main')
    Workspace.open('EditorFixture/Widgets/Card')
    assert(#Workspace.documents == 2 and Workspace.document == 'EditorFixture/Widgets/Card')
    assert(Workspace.configureSource(source, 'get/data-scene/EditorFixture/Widgets/Card'), source.ErrorMessage)
    assert(find('Path', 'EditorFixture/Widgets/Card/Caption'), 'A library item opens as a document')
    assert(Workspace.activate('EditorFixture/Main'))
    assert(Workspace.setValue('EditorFixture/Main/Root/Renamed', 'TextRun.Text', 'Unsaved'))
    assert(Workspace.dirty and #Workspace.undoStack == 1 and not Workspace.isDirty('EditorFixture/Widgets/Card'),
        'Edits mark only their own document as unsaved')
    Workspace.activate('EditorFixture/Widgets/Card')
    assert(not Workspace.dirty and #Workspace.undoStack == 0, 'Each tab has its own undo history')
    local closed, reason = Workspace.close('EditorFixture/Main')
    assert(not closed and reason == 'dirty' and Workspace.findDocument('EditorFixture/Main'),
        'An unsaved tab is not closed without a decision')
    assert(Workspace.close('EditorFixture/Main', true) and not Workspace.findDocument('EditorFixture/Main'))
    assert(Workspace.configureSource(source, 'get/data-properties/EditorFixture/Main/Root/Renamed'))
    local text = assert(find('PropertyName', 'TextRun.Text'))
    assert(text.Value == 'Saved', 'Discarding reloads the document from its source file: ' .. tostring(text.Value))
    Workspace.open('EditorFixture/Main')
    local saved = Workspace.save('EditorFixture/Main')
    assert(saved, 'Discarded edits are no longer pending for the document')
    core.flushQueue()
end, debug.traceback)
connection:close()
os.remove(fixture .. '/Main.xml')
os.remove(fixture .. '/Widgets.xml')
os.remove(fixture .. '/package.lua')
os.remove(fixture .. '/Styles/base.css')
os.remove(fixture .. '/Styles/alternate.css')
os.remove(fixture .. '/Styles')
os.remove(fixture .. '/' .. deep_path .. '/leaf.lua')
os.remove(fixture .. '/' .. deep_path .. '/данные & test.lua')
for _ = 1, 6 do
    os.remove(fixture .. '/' .. deep_path)
    deep_path = deep_path:match('^(.*)/[^/]+$') or ''
end
os.remove(fixture)
assert(ok, err)
print('Editor runtime data-source and edit/save contract passed.')
