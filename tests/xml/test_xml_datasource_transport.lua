-- Requiring filesystem first must establish its native Object dependency.
local fs = require 'orca.filesystem'
local orca = require 'orca'
orca.init()
local core = require 'orca.core'
local ui = require 'orca.UIKit'

-- The only intermediate representation is native DataObject records. No Lua
-- record tables, provider functions or per-row construction are involved.
local schemaPath = (os.getenv('TMPDIR') or '/tmp') .. '/orca-native-source-schema.xml'
local schemaFile = assert(io.open(schemaPath, 'w'))
schemaFile:write([[<Schema><Entity Name="TransportRow" Key="Id">
  <Column Name="Id" Type="string"/>
  <Column Name="Title" Type="string"/>
  <Column Name="Count" Type="int"/>
  <Column Name="Enabled" Type="bool"/>
</Entity></Schema>]])
schemaFile:close()

local source = fs.XmlDataSource { Schema = schemaPath, AutoLoad = false }
source.Content = [[<response status="success"><TransportRow name="a" Id="a" Title="First" Count="3" Enabled="true"/>
<TransportRow name="b" Id="b" Title="Second" Count="5" Enabled="false"/></response>]]
assert(source:reload() and source.Items and not source.HasError)
local items = source.Items
local a, b = items:getFirstChild(), items:getFirstChild():getNext()
assert(a.Id == 'a' and a.Title == 'First' and a.Count == 3 and a.Enabled == true)
assert(b.Id == 'b' and b.Count == 5 and b.Enabled == false)
local list = ui.ListBox { ItemsSource = source.Items }
local firstView = list:getFirstChild()
assert(firstView and firstView.DataContext == a)

source.Content = [[<response status="success"><TransportRow name="b" Id="b" Title="Changed" Count="7" Enabled="true"/>
<TransportRow name="a" Id="a" Title="Retained" Count="9" Enabled="false"/>
<TransportRow name="c" Id="c" Title="Third" Count="1" Enabled="true"/></response>]]
assert(source:reload())
assert(source.Items == items and items:getFirstChild() == b and b:getNext() == a,
    'Refresh retains record identity and applies ordering')
assert(a.Title == 'Retained' and b.Title == 'Changed' and a.Count == 9)
assert(list:getFirstChild():getNext() == firstView, 'Refresh retains generated controls')

local c = a:getNext()
source.Content = [[<response status="success"><TransportRow name="c" Id="c" Title="Third"/>
<TransportRow name="b" Id="b" Title="Changed"/><TransportRow name="a" Id="a" Title="Retained"/></response>]]
assert(source:reload() and items:getFirstChild() == c and c:getNext() == b and b:getNext() == a,
    'Reverse ordering retains every record')
assert(source:reload() and items:getFirstChild() == c and c:getNext() == b and b:getNext() == a,
    'An unchanged ordering remains stable on repeated refresh')

source.Content = '<response status="success"><TransportRow name="a" Id="a"/></response>'
assert(source:reload() and source.Items:getFirstChild() == a)
assert((a.Title == nil or a.Title == '') and a.Count == 0 and a.Enabled == false,
    'A complete refresh resets omitted typed fields to their defaults')

source.Content = '<response status="error">Permission &amp; validation failed</response>'
assert(not source:reload() and source.HasError and source.ErrorMessage == 'Permission & validation failed')
assert(source.Items == items and items:getFirstChild() == a, 'Service errors retain successful rows')
source.Content = '<response><broken>'
assert(not source:reload() and source.HasError and source.Items == items)
source.Content = '<response status="success"/>'
source:send('XmlDataSource.Reload')
assert(not source.HasError and items:getFirstChild() == nil and list:getFirstChild() == nil)
print('PASS: native XML datasource typed records, retained rows, ordering and failure state')

local connection = fs.ProcessConnection { Executable = '/bin/cat', Arguments = { { Value = '-' } } }
local reply = assert(connection:request('<response status="success">literal $(no-shell)</response>'))
assert(reply:find('literal $(no-shell)', 1, true), 'Process requests are literal data')
source.Content = ''
source.Connection = connection
source.Source = '<response status="success"><TransportRow name="remote" Id="remote" Title="From process" Count="4" Enabled="true"/></response>'
assert(source:reload() and source.Items:getFirstChild().Title == 'From process')
connection:close()
assert(source:reload(), 'An explicitly closed connection reopens lazily')
assert(connection:request('one\ntwo') == nil and connection.ErrorMessage:find('single line', 1, true))
connection:close()
local missing = fs.ProcessConnection { Executable = '/does/not/exist', Timeout = 100 }
assert(missing:request('get/rows') == nil and missing.ErrorMessage ~= '')
missing:close()
local timeout = fs.ProcessConnection { Executable = '/bin/cat', Timeout = 20 }
assert(timeout:request('no response marker') == nil and timeout.ErrorMessage ~= '')
timeout:close()
print('PASS: shared native process transport, lazy start, close, invalid requests and deadlines')

local screen = assert(fs.loadObjectFromXmlString(([[<Screen ResizeMode="NoResize" Width="320" Height="240">
  <ProcessConnection Name="Runtime" Executable="/bin/cat"/>
  <XmlDataSource Name="Rows" ConnectionPath="Runtime" AutoLoad="true" Schema="%s"
    Source="&lt;response status=&quot;success&quot;&gt;&lt;TransportRow name=&quot;answer&quot; Count=&quot;42&quot;/&gt;&lt;/response&gt;"/>
  <ListBox Name="List" ItemsSource="{Binding ../Rows/Items}"/>
</Screen>]]):format(schemaPath)))
local declared = assert(screen:findChild('Rows'))
core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
assert(declared.ConnectionPath == 'Runtime', 'XML declares the shared connection path')
assert(declared.Items and not declared.HasError, declared.ErrorMessage)
core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
local declaredList = assert(screen:findChild('List'))
assert(declaredList.ItemsSource == declared.Items and declaredList:getFirstChild().DataContext.Count == 42,
    'An XML-declared connection and data source feed the list directly')
screen:findChild('Runtime'):close()
os.remove(schemaPath)
print('PASS: declarative XML connection to native datasource to list')

-- A filesystem module can finalize before an independently retained native
-- record. Its class and property descriptors must remain valid afterward.
local lifetimeSchema = '<Schema><Entity Name="TransportLifetimeRow"><Column Name="Label" Type="string"/></Entity></Schema>'
local retained = assert(fs.loadXmlDataStringWithSchema('<TransportLifetimeRow Label="retained"/>', lifetimeSchema))
getmetatable(fs).__gc(fs)
assert(retained.Label == 'retained', 'Module finalization preserves retained record descriptors')
retained.Label = 'still writable'
assert(retained.Label == 'still writable')
local later = assert(fs.loadXmlDataStringWithSchema('<TransportLifetimeRow Label="reused"/>', lifetimeSchema))
assert(later.Label == 'reused', 'A later parser reuses the process-lifetime schema registration')
print('PASS: schema descriptors survive module finalization while native records remain alive')
