-- Run unchanged in the default XML build and with -DORCA_USE_LISP.
local test = require 'orca.test'
local core = require 'orca.core'
local filesystem = require 'orca.filesystem'
local system = require 'orca.system'
require 'orca.UIKit'

local has_lisp = filesystem.loadObjectFromLispString ~= nil
local has_xml = filesystem.loadObjectFromXmlString ~= nil
test.expect(has_lisp ~= has_xml, 'exactly one scene string loader is exported')
test.expect_eq(filesystem.loadObjectFromLisp ~= nil, has_lisp, 'Lisp file and string API selection agrees')
test.expect_eq(filesystem.loadObjectFromXml ~= nil, has_xml, 'XML file and string API selection agrees')

local extension = has_lisp and 'lisp' or 'xml'
local disabled = has_lisp and 'xml' or 'lisp'
local expected = os.getenv('ORCA_EXPECT_SCENE_FORMAT')
if expected then
  test.expect_eq(extension, expected, 'loaded binary uses the requested scene format')
end
local load_string = has_lisp and filesystem.loadObjectFromLispString or filesystem.loadObjectFromXmlString
local load_file = has_lisp and filesystem.loadObjectFromLisp or filesystem.loadObjectFromXml
local serialize = has_lisp and filesystem.serializeObjectToLispString or filesystem.serializeObjectToXmlString
local fixture = 'tests/fixtures/scene_format/Scene'
local source = filesystem.readTextFile(fixture .. '.' .. extension)
test.expect(source ~= nil and source ~= '', 'selected fixture exists')
test.expect(serialize ~= nil, 'selected serializer is available')

-- DataContext paths address schema fields rather than Object's metadata names.
local model = filesystem.loadXmlDataStringWithSchema(
  '<SceneFormatDraft Title="draft"/>',
  '<Schema><Entity Name="SceneFormatDraft"><Column Name="Title" Type="string"/></Entity></Schema>')
test.expect_eq(model.Title, 'draft', 'XML data schemas remain available in either scene build')
model = nil

local function evaluate(root)
  core.flushQueue()
  core.advanceFrame()
  core.runAllPrograms(root)
end

local function pump(root)
  while true do
    local message = system.peekMessage()
    if not message then break end
    system.dispatchMessage(root, message)
  end
  core.flushQueue()
end

local function check_scene(root)
  test.expect(root ~= nil, 'selected scene loads')
  test.expect_eq(root.Name, 'FormatScene', 'scene root is constructed')
  test.expect(root.DataContext ~= nil, 'node-valued property is assigned')
  test.expect_eq(root.DataContext.Name, 'Context', 'nested object keeps its fields')
  test.expect_eq(root:findChild('Label').Text, 'A & B <tag> "quoted"\nПривет', 'text escaping survives loading')
  root.Width = 100
  evaluate(root)
  local edit = root:findChild('Edit')
  test.expect_eq(edit.Text, 'draft', 'two-way binding initializes from the model')
  edit.Text = 'Edited'
  core.flushQueue()
  test.expect_eq(root.DataContext.Title, 'Edited', 'two-way binding writes edits to the model')
  root.DataContext.Title = 'draft'
  evaluate(root)
  test.expect_eq(root:findChild('Computed').Width, 100, 'property binding reads the parent')
  test.expect_eq(root:findChild('Computed').Height, 40, 'expression selects its first branch')
  root.Width = 50
  evaluate(root)
  test.expect_eq(root:findChild('Computed').Width, 50, 'property binding reacts to changes')
  test.expect_eq(root:findChild('Computed').Height, 20, 'expression recomputes after changes')
  local target = root:findChild('Target')
  target.Visible = true
  root:findChild('Hide'):send('Node.LeftButtonUp')
  pump(root)
  test.expect(not target.Visible, 'message action executes against its relative target')
end

-- Direct file, generic file and extensionless paths must select the same reader.
check_scene(load_string(source))
check_scene(load_file(fixture .. '.' .. extension))
check_scene(filesystem.loadObject(fixture .. '.' .. extension))
check_scene(filesystem.loadObject(fixture))
test.expect(filesystem.loadObject(fixture .. '.' .. disabled) == nil,
  'generic loader rejects an existing file of the disabled scene format')
print('PASS: exclusive ' .. extension .. ' API and file registration, bindings, objects and actions')

local function roundtrip()
  local original = load_string(source)
  local encoded = serialize(original)
  test.expect(type(encoded) == 'string' and #encoded > 0, 'selected serializer returns a scene')
  test.expect(encoded:match(has_lisp and '^%s*%(' or '^%s*<') ~= nil, 'serializer uses the selected syntax')
  local restored = load_string(encoded)
  check_scene(restored)
  local instance = original:instantiate()
  test.expect(instance ~= original, 'instantiate creates a separate object')
  check_scene(instance)
  instance.Width = 25
  evaluate(instance)
  test.expect_eq(original.Width, 100, 'instance updates do not change the source tree')
end

-- Warm class/property registration before counting native objects.
roundtrip()
core.flushQueue()
collectgarbage(); collectgarbage()
local baseline = core.objectCount()
for _ = 1, 5 do
  roundtrip()
  core.flushQueue()
  collectgarbage(); collectgarbage()
end
test.expect_eq(core.objectCount(), baseline, 'loaded, roundtripped and instantiated trees release their owned objects')
print('PASS: ' .. extension .. ' serialization, instantiation and lifetime')

-- XML remains a data format even when its scene reader is not compiled in.
local data = filesystem.loadXmlDataStringWithSchema('<Node2D Width="12"/>', '')
test.expect_eq(data:getClassName(), 'DataObject', 'XML data parser cannot construct scene classes')
local legacy_data_loader = filesystem.loadObjectFromXmlStringWithSchema
test.expect(type(legacy_data_loader) == 'function', 'legacy XML data API remains available in both builds')
local legacy_data = legacy_data_loader('<Node2D Width="12"/>', '')
test.expect_eq(legacy_data:getClassName(), 'DataObject', 'legacy XML data API cannot construct scene classes')
local legacy_record = legacy_data_loader('<SceneFormatLegacyRow Title="kept"/>',
  '<Schema><Entity Name="SceneFormatLegacyRow"><Column Name="Title" Type="string"/></Entity></Schema>')
test.expect_eq(legacy_record.Title, 'kept', 'legacy XML data API preserves typed schema fields')
legacy_data, legacy_record = nil, nil
core.flushQueue()
collectgarbage(); collectgarbage()
local data_baseline = core.objectCount()
for _ = 1, 5 do
  local row = legacy_data_loader('<DataObject Name="temporary"/>', '')
  row = nil
  core.flushQueue()
  collectgarbage(); collectgarbage()
end
test.expect_eq(core.objectCount(), data_baseline, 'legacy XML data loader transfers its owning reference to Lua')

-- The package uses one extensionless startup path in both builds. Exercise the
-- real screens, CSS, data-backed templates and popup rather than a copied UI.
local function check_example_startup()
  local project = filesystem.init('apps/Example')
  test.expect(project ~= nil, 'Example package initializes in the selected build')
  test.expect_eq(project.StartupScreen, 'Example/Screens/Application', 'Example startup is independent of scene format')
  local root = filesystem.loadObject(project.StartupScreen)
  test.expect(root ~= nil, 'generic startup loader opens the selected Example scene')
  test.expect(root:getSourceFile():match('%.' .. extension .. '$') ~= nil,
    'extensionless Example startup selects the compiled scene format')
  test.expect(root.StyleSheet ~= nil, 'Example stylesheet loads')
  test.expect_near(root.ClearColor.R, 0x0B / 255, 0.001, 'Example surface theme resolves red')
  test.expect_near(root.ClearColor.B, 0x12 / 255, 0.001, 'Example surface theme resolves blue')

  root.ResizeMode = 'NoResize'
  root.Height = 600
  local hero = root:findChild('Hero', true)
  local body = root:findChild('Body', true)
  test.expect(hero ~= nil and body ~= nil, 'Example startup constructs the main layout')
  for _, size in ipairs {
    { width = 800, columns = '1fr 1fr', padding = 40 },
    { width = 500, columns = '1fr', padding = 8 },
  } do
    root.Width = size.width
    evaluate(root)
    root:UpdateLayout(root.Width, root.Height)
    test.expect_eq(hero.Columns, size.columns, 'Example columns react to width in ' .. extension .. ' mode')
    test.expect_near(body.PaddingLeft, size.padding, 0.001, 'Example padding reacts to width in ' .. extension .. ' mode')
  end

  local source = project.DataSourceLibrary:findChild('ApplicationData', false)
  local signal = source and source:findChild('SignalXml', true)
  local metric = source and source:findChild('MetricUsers', true)
  test.expect(signal ~= nil and signal.Title ~= nil and signal.Title ~= '', 'Example signal datasource contains a title')
  test.expect(metric ~= nil and metric.Title ~= nil and metric.Title ~= '', 'Example metric datasource contains a value')
  local signal_title = root:findChild('SignalCardTitle', true)
  local metric_value = root:findChild('MetricUsersValue', true)
  test.expect(signal_title ~= nil and metric_value ~= nil, 'Example instantiates signal and metric templates')
  test.expect_eq(signal_title.Text, signal.Title, 'Example signal template binds the shared data')
  test.expect_eq(metric_value.Text, metric.Title, 'Example metric template binds the shared data')
  local original_title = signal.Title
  signal.Title = 'Updated by scene-format test'
  evaluate(root)
  test.expect_eq(signal_title.Text, signal.Title, 'Example template reacts to datasource edits')
  signal.Title = original_title

  local tabs = root:findChild('OrcaTabs', true)
  root:findChild('LuaTab', true):send('Node.LeftButtonUp')
  pump(root)
  test.expect_eq(tabs.SelectedValue, 'lua', 'Example tabs still switch')
  root:findChild('CtaButtonPrimary', true):send('Node.LeftButtonUp')
  pump(root)
  local popup = root:getNext()
  test.expect(popup ~= nil, 'Example CTA loads its popup in the selected format')
  test.expect_eq(popup:getClassName(), 'Popup', 'Example modal uses the Popup class')
  local close = popup:findChild('GetStartedPopupClose', true)
  test.expect(close ~= nil, 'Example popup contains its close action')
  close:send('Node.LeftButtonUp')
  pump(root)
  test.expect_eq(root:getNext(), nil, 'Example popup close action detaches the modal')
  root:clear()
  filesystem.unloadProject(project)
  print('PASS: Example ' .. extension .. ' startup, theme, data, responsive layout and popup')
end

check_example_startup()

local function check_mvvm_form()
  local project = filesystem.init('apps/MvvmForms')
  test.expect(project ~= nil, 'MvvmForms package initializes')
  local form = filesystem.loadObject('MvvmForms/Views/Form')
  test.expect(form ~= nil, 'generic loader opens the actual MvvmForms form')
  test.expect(form:getSourceFile():match('%.' .. extension .. '$') ~= nil, 'MvvmForms selects the compiled format')
  evaluate(form)
  local name = form:findChild('DisplayName', true)
  local preview = form:findChild('Preview', true)
  local draft = form:findChild('DraftForm', true).DataContext
  test.expect(name ~= nil and preview ~= nil and draft ~= nil, 'MvvmForms constructs its fields and typed draft')
  test.expect_eq(name.Text, 'Sam Rivera', 'MvvmForms initializes from its real XML datasource')
  test.expect_eq(preview.Text, name.Text, 'MvvmForms preview reflects the initial draft')
  name.Text = 'Scene format draft'
  evaluate(form)
  test.expect_eq(draft.DisplayName, 'Scene format draft', 'MvvmForms two-way binding updates the draft')
  test.expect_eq(preview.Text, 'Scene format draft', 'MvvmForms preview reacts to the edited draft')
  form:clear()
  filesystem.unloadProject(project)
  print('PASS: MvvmForms ' .. extension .. ' loading and two-way bindings')
end

check_mvvm_form()

if has_xml then
  for _, invalid in ipairs {
    '<Node2D><Bogus/></Node2D>',
    '<Node2D Width="{Binding }"/>',
    '<Node2D LeftButtonUp="{HideAction Path=}"/>',
  } do
    test.expect(load_string(invalid) == nil, 'malformed XML scene must not return a partial tree: ' .. invalid)
  end

  -- Local fixtures prove external entities and DTD defaults stay unresolved.
  -- No system files or network endpoints are involved in this check.
  local directory = (os.getenv('PWD') or '.') .. '/tests/fixtures/scene_format/'
  local sentinel = 'ORCA_SCENE_EXTERNAL_SENTINEL'
  local external = directory .. 'External.txt'
  local dtd = directory .. 'External.dtd'
  local handle = assert(io.open(external, 'r'))
  test.expect(handle:read('*a'):find(sentinel, 1, true) ~= nil, 'external entity fixture is present')
  handle:close()
  for _, document in ipairs {
    '<!DOCTYPE TextBlock [<!ENTITY payload SYSTEM "' .. external .. '">]><TextBlock>&payload;</TextBlock>',
    '<!DOCTYPE TextBlock SYSTEM "' .. dtd .. '"><TextBlock/>',
    '<!DOCTYPE TextBlock [<!ENTITY % ext SYSTEM "' .. dtd .. '">%ext;]><TextBlock/>',
    '<!DOCTYPE TextBlock [<!ENTITY payload SYSTEM "' .. external .. '">]><TextBlock Text="&payload;"/>',
  } do
    local node = load_string(document)
    test.expect(node == nil or node.Text == nil or node.Text:find(sentinel, 1, true) == nil,
      'XML scene reader must not resolve external entities or DTD default attributes')
  end
  local escaped = load_string('<TextBlock Text="&lt;hello&gt; &amp; &quot;world&quot;"/>')
  test.expect_eq(escaped.Text, '<hello> & "world"', 'built-in XML entities still decode')
  print('PASS: XML malformed-input rejection and external entity isolation')

  -- Exercise an existing app asset unchanged, including its {Binding ...} markup.
  local project = filesystem.init('apps/Adventure')
  test.expect(project ~= nil, 'legacy Adventure package initializes')
  local template = filesystem.loadObject('Adventure/Prefabs/TabItem.xml')
  test.expect(template ~= nil, 'generic XML loader reads the existing Adventure prefab')
  local host = load_string('<Node2D DataContextSource="Adventure/DataSources/Tabs:TabAdventures"/>')
  test.expect(host.DataContext ~= nil, 'legacy datasource reference resolves')
  template.DataContext = host.DataContext
  evaluate(template)
  test.expect_eq(template:findChild('TabItemLabel').Text, 'Adventures', 'old app binding shorthand still evaluates')
  host.DataContext.Label = 'Renamed'
  evaluate(template)
  test.expect_eq(template:findChild('TabItemLabel').Text, 'Renamed', 'old app binding remains reactive')
  template:clear()
  host:clear()
  filesystem.unloadProject(project)
  print('PASS: unchanged Adventure XML prefab and legacy binding syntax')
end

print('All ' .. extension .. ' scene-format tests passed.')
