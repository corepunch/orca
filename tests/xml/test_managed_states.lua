local test = require "orca.test"
local core = require "orca.core"
require "orca.UIKit"
local fs = require "orca.filesystem"

local root = assert(fs.loadObjectFromString([[
<Node2D Components="StateManagerController" ActiveState="Closed" Width="40">
  <StateManagerController.StateManager>
    <StateManager>
      <StateGroup ControllerProperty="ActiveState">
        <StateTransition Duration="0"/>
        <State Value="Closed" Node.Width="10">
          <State Path="Content" Node.Visible="false"/>
        </State>
        <State Value="Open" Node.Width="110">
          <State Path="Content" Node.Visible="true"/>
        </State>
      </StateGroup>
    </StateManager>
  </StateManagerController.StateManager>
  <Node2D Name="Content"/>
</Node2D>
]]))
root:send("Object.Start")
core.flushQueue()
test.expect_eq(root.Width, 10, "XML components initialize managed state overrides")
test.expect(not root:findChild("Content", false).Visible, "Initial XML state targets nested content")
root.ActiveState = "Open"
core.flushQueue()
test.expect_eq(root.Width, 110, "Normal queued controller changes apply XML states")
test.expect(root:findChild("Content", false).Visible, "XML transition enters nested content")
root.StateManager = nil
core.flushQueue()
test.expect_eq(root.Width, 40, "Removing an XML manager restores the base value")
print("PASS: declarative components and queued managed XML states")
