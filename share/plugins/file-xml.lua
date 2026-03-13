local orca = require "orca"

local print = function(...)
	for i = 1, select("#", ...) do io.stderr:write(tostring(select(i, ...)).." ") end
	io.stderr:write("\n")
end

local simple_convertes = {
	number = tonumber,
	string = tostring,
	boolean = function(value)
		if value == "true" then return true end
		if value == "false" then return false end
		return tonumber(value) or error(string.format("Cannot convert '%s' to boolean", value))
	end,
	userdata = function(value, original)
	  local mt_name = tostring(original):match("^([^:]+):")
		assert(mt_name, string.format("Cannot determine userdata type from value: %s", tostring(original)))
		local mt = orca.find_metatable(mt_name)
		assert(mt, string.format("No metatable found for userdata type: %s", mt_name))
		if mt.fromstring then return mt.fromstring(value) end
		error(string.format("Cannot convert '%s' to userdata of type %s", value, mt_name))
	end,
}

local Property = {}

function Property.parse(node, name, value)
	assert(orca.typeconverter, "Type converter plugin is required for XML support plugin")
	if name == "ClassName" or name == "PlaceholderTemplate" then return end -- handled separately when constructing the node
	if name == "Name" or name == "id" then node:setName(value) return end
	local type = node:findImplicitProperty(name) or node:findExplicitProperty(name)
	assert(type, string.format("Unknown property: %s for node of type %s", name, node.className))
	local converter = orca.typeconverter[type.DataType]
	assert(converter, string.format("No type converter for data type: %s (property %s)", type.DataType, name))
	node[name] = converter(value, type)
end

function Property.parse_item(element, item, typename)
	for k, v in element.attributes do
		local converter = simple_convertes[type(item[k])]
		assert(converter, string.format("No simple converter for array item %s property %s", typename, k))
		item[k] = converter(v, item[k])
	end
	return item
end

function Property.parse_binding(node, name, element)
	assert(element.text, string.format("Binding element for property %s must have binding expression as text content", name))
	local Enabled = element:get "Enabled" or "true"
	local Attribute = element:get "Attribute" or "WholeProperty"
	local Mode = element:get "Mode" or "Expression"
	node:attachPropertyProgram(name, Attribute, element.text or "", Mode, Enabled ~= "false")
end

function Property.construct(node, property, element)
	if property.IsArray then
		local typename = property.TypeString
		local array, mt = {}, orca.find_metatable(typename)
		assert(mt, string.format("No metatable found for array item type: %s", property.TypeString))
		assert(mt.new, string.format("Array item type %s does not support new() method", property.TypeString))
		for sub in element:findall(property.TypeString) do
			table.insert(array, Property.parse_item(sub, mt.new(), typename))
		end
		node[property.Name] = array
	elseif element:get "Value" then
		xpcall(Property.parse, print, node, property.Name, element:get "Value")
	elseif element.text then
		xpcall(Property.parse_binding, print, node, property.Name, element)
		-- xpcall(Property.parse, print, node, property.Name, element.text)
	end
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
	for k, v in element.attributes do xpcall(Property.parse, print, node, k, v) end
	if element.text then
		node:setTextContent(element.text)
		return node
	end
	for sub in element.children do
		if sub:get "IsDisabled" == "true" then
			-- skip disabled elements, but still check if they are valid properties to catch typos
		elseif node:findExplicitProperty(sub.tag) then
			xpcall(Property.construct, print, node, node:findExplicitProperty(sub.tag), sub)
		elseif specials[sub.tag] then
			xpcall(specials[sub.tag], print, node, sub)
		else
			xpcall(node.addChild, print, node, construct_node(sub))
		end
	end
	node:sendMessage2("Start")
	return node
end

_G["doxmlstring"] = function (string)
	local xml = require "orca.parsers.xml"
	return construct_node(xml.parse(string).root)
end

_G["doxmlfile"] = function (path)
	local xml = require "orca.parsers.xml"
	local file = io.open(path, "r")
	assert(file, "Failed to open XML file: "..path)
	local doc = xml.parse(file:read("*a"))
	assert(doc, "Failed to parse XML file: "..path)
	file:close()
	return construct_node(doc.root)
end

table.insert(package.searchers, function(path)
	local xml = require "orca.parsers.xml"
	local ok, doc = pcall(xml.load, path..'.xml')
	return ok and doc and function()
		return function()
			local filesystem = require "orca.filesystem"
			local node = construct_node(doc.root)
			node:setSourceFile(path)
			filesystem.registerObject(node, path)
			return node
		end
	end or nil
end)