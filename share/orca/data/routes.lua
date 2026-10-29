-- A route registry: each route names its page, its data scope and where its
-- parameters come from, so a standard screen needs no Lua of its own.
--
--   <Routes>
--     <Route Path="/details" Name="DetailsPage" Page="Chat/Pages/Details"
--            Scope="Chat/Data/Details.xml" HeaderFrom="Room.Title" Back="/thread">
--       <Param Name="RoomId" From="room.id"/>
--       <Present Node="DetailsBody" Record="Room"/>
--     </Route>
--   </Routes>
--
-- Header is a fixed page name; HeaderFrom="Record.Field" follows an exposed
-- record. A route's parameters are read from the app (`From` is a dotted path on the
-- app table) when it opens. The scope's queries take them through
-- <Param Route="RoomId"/> and its records through <Expose Key="RoomId"/>.
-- The app keeps navigation and page chrome; it calls `open` for a path and
-- disposes the returned scope when the page goes away.
local Xml = require "orca.data.xml"
local Routes = {}
Routes.__index = Routes

local function fail(resource, node, message)
    error(("%s:%d: %s"):format(resource, node.line or 0, message), 0)
end

function Routes.parse(text, resource)
    local root = Xml.parse(text, resource)
    if root.tag ~= "Routes" then fail(resource, root, "the root element must be <Routes>") end
    local self = setmetatable({ resource = resource, list = {}, byPath = {} }, Routes)
    for _, node in ipairs(root.children) do
        if node.tag ~= "Route" then fail(resource, node, "unknown element <" .. node.tag .. ">") end
        local a = node.attrs
        for _, required in ipairs { "Path", "Page", "Scope" } do
            if not a[required] then fail(resource, node, "<Route> needs " .. required) end
        end
        if self.byPath[a.Path] then fail(resource, node, "route " .. a.Path .. " is declared twice") end
        local route = { path = a.Path, name = a.Name, page = a.Page, scope = a.Scope, header = a.Header,
            headerFrom = a.HeaderFrom, status = a.Status,
            back = a.Back, description = a.Description, params = {}, present = {}, line = node.line }
        for _, child in ipairs(node.children) do
            if child.tag == "Param" then
                if not child.attrs.Name then fail(resource, child, "<Param> needs Name") end
                route.params[#route.params + 1] = { name = child.attrs.Name, from = child.attrs.From or child.attrs.Name }
            elseif child.tag == "Present" then
                if not child.attrs.Node or not child.attrs.Record then
                    fail(resource, child, "<Present> needs Node and Record")
                end
                route.present[#route.present + 1] = { node = child.attrs.Node, record = child.attrs.Record }
            else
                fail(resource, child, "unknown element <" .. child.tag .. "> in a route")
            end
        end
        self.list[#self.list + 1] = route
        self.byPath[route.path] = route
    end
    return self
end

local cache = {}
function Routes.load(path)
    if not cache[path] then cache[path] = Routes.parse(Xml.read(path), path) end
    return cache[path]
end

local function read(source, path)
    local value = source
    for key in path:gmatch("[^%.]+") do
        if type(value) ~= "table" then return nil end
        value = value[key]
    end
    return value
end

-- The route's parameters, read from `source` (normally the app).
function Routes:params(path, source)
    local route = assert(self.byPath[path], "Unknown route " .. tostring(path))
    local params = {}
    for _, param in ipairs(route.params) do params[param.name] = read(source, param.from) end
    return params
end

-- Opens `path`: builds its scope (options are data.scope options: provider,
-- store, host, valid), opens it with the route's parameters, loads the page
-- and attaches it. Returns the page, the scope and the route.
function Routes:open(path, source, options)
    local data = require "orca.data"
    local filesystem = require "orca.filesystem"
    local route = assert(self.byPath[path], "Unknown route " .. tostring(path))
    local scope = data.scope(route.scope, options)
    local params = self:params(path, source)
    scope:open(params)
    local page = filesystem.loadObject(route.page)
    if not page then
        scope:dispose()
        error(("%s:%d: route %s cannot load page %s"):format(self.resource, route.line, path, route.page), 0)
    end
    -- Present records before the page's bindings first run.
    for _, present in ipairs(route.present) do
        local node = page.Name == present.node and page or page:findChild(present.node, true)
        if not node then
            scope:dispose()
            error(("%s:%d: route %s has no node %s to present %s"):format(self.resource, route.line, path,
                present.node, present.record), 0)
        end
        scope:present(node, present.record)
    end
    scope:attach(page)
    return page, scope, route
end

return Routes
