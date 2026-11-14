local test = require "orca.test"
-- A Shader owns its uniforms: Shader.Uniforms declares `{ShaderName}.{Name}`
-- attached properties. Nothing is declared project-wide.
local fs = require "orca.filesystem"
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()

local material = fs.loadObject("Octopus/Materials/NeonTitle")
test.expect(material ~= nil, "the material loads")
test.expect_near(material["Gradient.Angle"], 0, 0.001, "the material sets a float uniform by its owner's name")
local shader = material.Shader
test.expect(shader ~= nil and shader.Name == "Gradient", "the material references the owning shader")
test.expect_near(shader["Gradient.Angle"], 0, 0.001, "the shader holds the declared default")

-- The uniform attribute may precede the Shader attribute that declares it.
local early = fs.loadObjectFromString [[
<Material Name="Early" Gradient.Angle="90" Shader="Octopus/Shaders/Gradient"/>
]]
test.expect(early ~= nil, "a material loads with the uniform written first")
test.expect_near(early["Gradient.Angle"], 90, 0.001, "an attribute waits for the owner a sibling attribute loads")
print("PASS: shader-owned uniforms")
