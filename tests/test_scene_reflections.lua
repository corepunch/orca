local core = require "orca.core"
require "orca.UIKit"
local fs = require "orca.filesystem"
local scene = require "orca.SceneKit"

local model = scene.Model3D {}
assert(model.ScreenSpaceReflectionEnabled == false)
model.ScreenSpaceReflectionEnabled = true
assert(model.ScreenSpaceReflectionEnabled == true)

local loaded = fs.loadObjectFromXmlString [[
  <Scene Name="ReflectionScene">
    <Model3D Name="Ordinary"/>
    <Model3D Name="Deck" ScreenSpaceReflectionEnabled="true"/>
  </Scene>
]]
assert(loaded:findChild("Ordinary").ScreenSpaceReflectionEnabled == false)
assert(loaded:findChild("Deck").ScreenSpaceReflectionEnabled == true)
loaded:findChild("Deck").ScreenSpaceReflectionEnabled = false
assert(loaded:findChild("Deck").ScreenSpaceReflectionEnabled == false)
core.flushQueue()
print("PASS: reflection model opt-in and XML loading")
