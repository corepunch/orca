local orca = require "orca"

local print = function(...)
	io.stderr:write(table.concat({...}, "\t").."\n")
end

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

local function construct_property(node, property, element)
	if property.IsArray then

	elseif element.text then
		xpcall(parse_argument, print, node, property.Name, element.text)
	end
end

local specials = {
	script = function(node, element)
		local script, err = load(element.text, string.format("@%s.script", node.Name), "t", node)
		if not script then error(err) return end
		local ok, result = pcall(script)
		if not ok then error(result) end
		return result
	end,
	Animation = function(node, element)
		local clipname = element:get "Clip"
		assert(clipname, "Animation element requires a Clip attribute")
		local clip = require(clipname)
		assert(clip, "Could not load animation clip: "..clipname)
		node:addAnimation(clip)
	end,
	EventListener = function(node, element)
		print("Adding event listener for event: "..element:get "Event")
	end,
	Entry = function(node, element)
		node:addAlias(element:get("id"), element.text)
	end,
	ResourceDictionaryItem = function(node, element)
		for entry in element.children do
			node:addResource(entry:get("id"), entry.text)
		end
	end,
	StyleSheet = function(node, element)
		local xml = require "orca.parsers.xml"
		local doc = xml.load(element.text)
		assert(doc.root.tag == "Styles", "StyleSheet content must be a Styles element")
		for style in doc.root:findall "Style" do
			assert(style:get "Key", "Style element requires a Key attribute")
			for key, value in style.attributes do
				if key ~= "Key" and key ~= "StyleTargetType" and key ~= "StyleType" then
					node:addStyleClass(style:get "Key", key, value)
				end
			end
		end
	end,
	AnimationPlayer = function () end,
	ValueTicker = function () end,
}

local function construct_node(element)
	local class =
		try_prefab_placeholder(element) or
		try_require_memeber("orca.ui", element.tag) or
		try_require_memeber("orca.SceneKit", element.tag) or
		try_require_memeber("orca.SpriteKit", element.tag) or
		try_require_memeber("orca.filesystem", element.tag) or
		try_require_memeber("orca.renderer", element.tag)
	if not class then error("Unknown element type: "..element.tag) end
	local node = class()
	for k, v in element.attributes do xpcall(parse_argument, print, node, k, v) end
	for sub in element.children do
		local property = node:findExplicitProperty(sub.tag)
		if sub:get "IsDisabled" == "true" then
			-- skip disabled elements, but still check if they are valid properties to catch typos
		elseif property then
			xpcall(construct_property, print, node, property, sub)
		elseif specials[sub.tag] then
			xpcall(specials[sub.tag], print, node, sub)
		else
			xpcall(node.addChild, print, node, construct_node(sub))
		end
	end
	return node
end

_G["doxmlfile"] = function (path)
	local xml = require "orca.parsers.xml"
	local file = io.open(path, "r")
	assert(file, "Failed to open XML file: "..path)
	local doc = xml.parse(file:read("*a"))
	file:close()
	return construct_node(doc.root)
end

table.insert(package.searchers, function(path)
	local xml = require "orca.parsers.xml"
	local ok, doc = pcall(xml.load, path..'.xml')
	return ok and doc and function()
		return function()
			return construct_node(doc.root)
		end
	end or nil
end)