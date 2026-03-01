local orca = require "orca"

local special_attrs = {
	Name       = function(node, name, value) node:setName(value) end,
	id         = function(node, name, value) node:setName(value) end,
	class      = function(node, name, value) node:parseClassAttribute(value) end,
	Style      = function(node, name, value) node:parseClassAttribute(value:match("[^/]+$") or value) end,
	IsDisabled = function(node, name, value) end, -- intentionally ignored
}
special_attrs["Node.Style"] = special_attrs.Style

local function parse_argument(node, name, value)
	assert(orca.typeconverter, "Type converter plugin is required for XML support plugin")
	local special = special_attrs[name]
	if special then return special(node, name, value) end
	if value:sub(1, 1) == '{' then return node:bind(name, value) end
	local prop_type = node:findImplicitProperty(name)
	assert(prop_type, string.format("Unknown property: %s for node of type %s", name, node:getClassName()))
	local converter = orca.typeconverter[prop_type.DataType]
	assert(converter, string.format("No type converter for data type: %s (property %s)", prop_type.DataType, name))
	node[name] = converter(value, prop_type)
end

local function try_require_member(module, name)
	local ok, res = pcall(require, module)
	return ok and res[name] or nil
end

local function try_prefab_placeholder(element)
	local placeholders = {
		CustomNode              = require,
		LibraryPlaceholder      = require,
		LayerPrefabPlaceholder  = require,
		ObjectPrefabPlaceholder = require,
	}
	local placeholder = placeholders[element.tag]
	if placeholder then
		return placeholder(element:get "PlaceholderTemplate" or element:get "ClassName")
	else
		return nil
	end
end

local modules = {
	"orca.ui",
	"orca.SceneKit",
	"orca.SpriteKit",
	"orca.filesystem",
	"orca.renderer",
}

local function construct(element)
	local class = try_prefab_placeholder(element)
	for _, module in ipairs(modules) do
		class = class or try_require_member(module, element.tag)
	end

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