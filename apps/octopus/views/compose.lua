-- The write tab: Pages/Write.xml over the Compose scope (Data/Compose.xml),
-- which keeps one draft per kind of post for the reader's session.
local filesystem = require "orca.filesystem"
local C = require "Octopus/views/components"
local OctopusProvider = require "Octopus/providers/octopus"
local Compose = {}

function Compose.scope(self)
    if self.composeScope and not self.composeScope.disposed then return self.composeScope end
    local scope = self:octopusScope("Octopus/Data/Compose.xml")
    self.composeScope = scope
    scope:expose("Composer", scope.store:put("OctopusComposer", { id = "composer", kind = "text" }))
    OctopusProvider.selectKind(scope, "text")
    return scope
end

function Compose.page(self)
    local scope = Compose.scope(self)
    local page = assert(filesystem.loadObject("Octopus/Pages/Write"), "Missing Octopus/Pages/Write")
    scope:attach(page)
    self.status, self.statusBar = page:findChild("WriteStatus", true), nil
    local pivot = C.page("", page)
    -- The header names the kind being drafted ("new quote").
    local item = pivot:getFirstChild()
    scope:present(item, "Composer")
    item:attachPropertyProgram("PivotItem.Header", "{DataContext/Heading}", "OneWay", true)
    return self:tab("Write", pivot)
end

return Compose
