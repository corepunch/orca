local core = require "orca.core"
local fs = require "orca.filesystem"
local ui = require "orca.UIKit"
local ItemsView = {}
local prototypes = {}

-- Register each presentation record once. Service tables stay outside the UI;
-- typed DataObjects provide observable properties for native bindings.
function ItemsView.record(name, columns)
    if prototypes[name] then return prototypes[name] end
    local schema = { '<Schema><Entity Name="' .. name .. '">' }
    for field, kind in pairs(columns) do
        schema[#schema + 1] = '<Column Name="' .. field .. '" Type="' .. kind .. '"/>'
    end
    schema[#schema + 1] = '</Entity></Schema>'
    local root = assert(fs.loadXmlDataStringWithSchema('<Items><' .. name .. '/></Items>', table.concat(schema)))
    prototypes[name] = assert(root:getFirstChild())
    return prototypes[name]
end

function ItemsView.new(options)
    local node = options.node or ui.StackView { Spacing = 0 }
    node:addComponentByName("ItemsControl")
    node.DataContext = core.DataObject {}
    node:attachPropertyProgram("ItemsSource", "{DataContext}", "OneWay", true)
    node.ItemTemplate = assert(options.template)
    core.runAllPrograms(node)
    return setmetatable({ node = node, options = options, entries = {}, byKey = {} }, { __index = ItemsView })
end

-- Reconcile service keys to stable DataObjects. Native ItemsControl owns row
-- creation, removal and ordering; XML is the sole owner of each row's layout.
function ItemsView:refresh(values)
    local options, ordered, byKey = self.options, {}, {}
    -- Validate keys before touching the retained collection.
    for i, value in ipairs(values) do
        local key = assert(options.key(value, i), "ItemsView requires stable keys")
        assert(not byKey[key], "Duplicate ItemsView key: " .. tostring(key))
        byKey[key] = true
        ordered[i] = { key = key, value = value }
    end
    byKey = {}
    for i, item in ipairs(ordered) do
        local entry = self.byKey[item.key]
        if not entry then entry = { key = item.key, data = options.record:instantiate() } end
        entry.value = item.value
        for field, content in pairs(options.fields(item.value, i)) do entry.data[field] = content end
        ordered[i], byKey[item.key] = entry, entry
    end
    local source = assert(self.node.ItemsSource, "ItemsView requires a bound ItemsSource")
    source:beginUpdate()
    local ok, err = xpcall(function()
        for key, entry in pairs(self.byKey) do
            if not byKey[key] then
                if options.remove then options.remove(entry) end
                entry.data:removeFromParent()
            end
        end
        for _, entry in ipairs(ordered) do
            if entry.data:getParent() ~= source then source:addChild(entry.data) end
        end
        local before
        for i = #ordered, 1, -1 do
            ordered[i].data:moveBefore(before)
            before = ordered[i].data
        end
    end, debug.traceback)
    source:endUpdate()
    if not ok then error(err, 0) end
    self.node:send("ItemsControl.Refresh")
    local row = self.node:getFirstChild()
    for _, entry in ipairs(ordered) do
        while row and not (row.DataContext == entry.data) do row = row:getNext() end
        assert(row, "ItemsControl did not create a view for its source record")
        local created = entry.node ~= row
        entry.node = row
        if options.update then options.update(entry, entry.value, created) end
        row = row:getNext()
    end
    self.entries, self.byKey = ordered, byKey
    core.advanceFrame()
    core.runAllPrograms(self.node)
    return ordered
end

return ItemsView
