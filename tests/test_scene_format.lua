-- Run unchanged in both SCENE_FORMAT=lisp and SCENE_FORMAT=xml builds.
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
