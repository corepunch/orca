local orca = require "orca"

local function split_string(values)
	local vals = {}
	for v in values:gmatch("%S+") do table.insert(vals, v) end
	return vals
end

local function set_property_value(node, explicit, value)
	local type = node:findImplicitProperty(explicit) or node:findExplicitProperty(explicit)
	assert(type, string.format("Unknown property: %s for node of type %s", explicit, node.className))
	local converter = orca.typeconverter[type.DataType]
	assert(converter, string.format("No type converter for data type: %s (property %s)", type.DataType, explicit))
	node[explicit] = converter(value, type)
end

local function parse_edges(node, edges, values)
	-- set_edge_property_value(node, edges.TypeString, "Top", values:match("Top=([^%s]+)"))
	set_property_value(node, string.format(edges.TypeString, "Top"), values[1])
	set_property_value(node, string.format(edges.TypeString, "Right"), values[2] or values[1])
	set_property_value(node, string.format(edges.TypeString, "Bottom"), values[3] or values[1])
	set_property_value(node, string.format(edges.TypeString, "Left"), values[4] or values[2] or values[1])
end

local function parse_argument(node, name, value)
	assert(orca.typeconverter, "Type converter plugin is required for XML support plugin")
	if name == "Name" or name == "id" then node:setName(value) return end
	local type = node:findImplicitProperty(name) or node:findExplicitProperty(name)
	assert(type, string.format("Unknown property: %s for node of type %s", name, node.className))
	if type.DataType == "edges" then
		return parse_edges(node, type, split_string(value))
	end
	local converter = orca.typeconverter[type.DataType]
	assert(converter, string.format("No type converter for data type: %s (property %s)", type.DataType, name))
	node[name] = converter(value, type)
end

local function try_require_memeber(module, name)
	local ok, res = pcall(require, module)
	return ok and res[name] or nil
end

local function try_prefab_placeholder(element)
	local placeholders = {
		CustomNode = require,
		LibraryPlaceholder = require,
		LayerPrefabPlaceholder = require,
		ObjectPrefabPlaceholder = require,
	}
	local placeholder = placeholders[element.tag]
	if placeholder then
		return placeholder(element:get "PlaceholderTemplate" or element:get "ClassName")
	else
		return nil
	end
end

local function construct(element)
	local class =
		try_prefab_placeholder(element) or
		try_require_memeber("orca.ui", element.tag) or
		try_require_memeber("orca.SceneKit", element.tag) or
		try_require_memeber("orca.SpriteKit", element.tag) or
		try_require_memeber("orca.filesystem", element.tag) or
		try_require_memeber("orca.renderer", element.tag)

	if not class then
		error("Unknown element type: "..element.tag)
	end

	local node = class()
	for k, v in element.attributes do
		parse_argument(node, k, v)
	end
	for sub in element.children do
		node:addChild(construct(sub))
	end
	return node
end

table.insert(package.searchers, function(path)
	local xml = require "orca.parsers.xml"
	local ok, doc = pcall(xml.load, path..'.xml')
	return ok and doc and function()
		return function()
			return construct(doc.root)
		end
	end or nil
end)