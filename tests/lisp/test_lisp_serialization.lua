local test = require 'orca.test'
local core = require 'orca.core'
local fs = require 'orca.filesystem'
require 'orca.UIKit'

local function update(root)
  core.flushQueue()
  core.advanceFrame()
  core.runAllPrograms(root)
end

local root = fs.loadObjectFromLispString([[
  (Node2D Name="Root" Width=31
    LeftButtonUp=(HideAction Path="Text")
    (Node2D Name="ContextHolder" DataContext=(DataObject Name="Record"))
    (TextBlock Name="Text" Width=(bind "../Width") "Text & <tags> \"quoted\"\nnext"))
]])
test.expect(root ~= nil, 'source loads')
update(root)
test.expect_eq(root:findChild('Text').Width, 31, 'source binding evaluates')
local encoded = fs.serializeObjectToLispString(root)
test.expect(encoded and encoded:sub(1, 1) == '(', 'serializer returns Lisp')
local decoded = fs.loadObjectFromLispString(encoded)
local instantiated = root:instantiate()
test.expect(decoded ~= nil, 'roundtrip returns an object')
test.expect(instantiated ~= nil, 'instantiate returns an object')
for _, copy in ipairs { decoded, instantiated } do
  update(copy)
  test.expect_eq(copy:findChild('ContextHolder').DataContext.Name, 'Record', 'object-valued property survives')
  test.expect_eq(copy:findChild('Text'):getTextContent(), 'Text & <tags> "quoted"\nnext', 'body text survives')
  copy.Width = 49
  update(copy)
  test.expect_eq(copy:findChild('Text').Width, 49, 'body text does not erase live binding')
  test.expect(copy:findChild('Text').Visible, 'cloned action target starts visible')
  copy:send('Node.LeftButtonUp', {})
  update(copy)
  test.expect_eq(copy:findChild('Text').Visible, false, 'cloned inline action still dispatches')
end
print('PASS: Lisp serialization preserves object values, body text, and bindings')
