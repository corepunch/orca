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
	if name == "ClassName" or name == "PlaceholderTemplate" then return end -- handled separately when constructing the node
	if name == "Name" or name == "id" then node:setName(value) return end
	if name == "class" then node.class = value return end
	local type = node:findImplicitProperty(name) or node:findExplicitProperty(name)
	assert(type, string.format("Unknown property: %s for node of type %s", name, node.className))
	node[name] = orca.core.parseProperty(orca.theme[value] or value, type)
end

function Property.parse_item(element, item, typename)
	for key, value in element.attributes do
		local converter = simple_convertes[type(item[key])]
		assert(converter, string.format("No simple converter for array item %s property %s", typename, key))
		item[key] = converter(orca.theme[value] or value, item[key])
	end
	return item
end

function Property.parse_binding(node, name, element)
	assert(element.text, string.format("Binding element for property %s must have binding expression as text content", name))
	local Enabled = element:get "Enabled" or "true"
	local Attribute = element:get "Attribute" or "WholeProperty"
	local Mode = element:get "Mode" or "Expression"
	node:attachPropertyProgram(name, element.text, Attribute, Mode, Enabled ~= "false")
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
	EventListener = function(node, element)
		print("Adding event listener for event: "..element:get "Event")
	end,
	StyleSheet = function(node, element)
		if not element.text or element.text == "" then
			io.stderr:write("[xml] <StyleSheet> element has no file path\n")
			return
		end
		local xml = require "orca.parsers.xml"
		local ok, doc = pcall(xml.load, element.text)
		if not ok or not doc then
			io.stderr:write("[xml] Failed to load StyleSheet: " .. element.text .. "\n")
			return
		end
		local root = doc.root
		if root.tag ~= "Styles" and root.tag ~= "ResourceDictionary" then
			io.stderr:write(("[xml] StyleSheet root must be <Styles> or <ResourceDictionary>, got <%s>\n"):format(root.tag))
			return
		end

		local trigger_pseudo = {
			IsMouseOver="hover", IsMouseDirectlyOver="hover",
			IsFocused="focus", IsKeyboardFocused="focus", IsKeyboardFocusWithin="focus",
			IsSelected="active", IsChecked="active", IsPressed="active",
		}

		for style in root:findall "Style" do
			local selector = style:get "Key" or style:get "TargetType"
			if not selector then goto continue end

			-- Detect WPF format: has <Setter> children
			local has_setters = false
			for _ in style:findall "Setter" do has_setters = true; break end

			if has_setters then
				-- WPF format: <Setter Property="..." Value="..."/>
				local base = {}
				local based_on = style:get "BasedOn"
				if based_on then
					base["@apply"] = based_on:match("{StaticResource%s+(.-)%}") or based_on
				end
				for setter in style:findall "Setter" do
					local p, v = setter:get "Property", setter:get "Value"
					if p and v then base[p] = v end
				end
				if next(base) then node:addStyleRule("." .. selector, base) end

				-- <Style.Triggers> → pseudo-state rules
				for tc in style:findall "Style.Triggers" do
					for trigger in tc:findall "Trigger" do
						local tp, tv = trigger:get "Property", trigger:get "Value"
						if tp and tv == "True" and trigger_pseudo[tp] then
							local tprops = {}
							for setter in trigger:findall "Setter" do
								local p, v = setter:get "Property", setter:get "Value"
								if p and v then tprops[p] = v end
							end
							if next(tprops) then
								node:addStyleRule("." .. selector .. ":" .. trigger_pseudo[tp], tprops)
							end
						end
					end
				end
			else
				-- Legacy format: properties as XML attributes on <Style>
				local props = {}
				for attr_key, attr_value in style.attributes do
					if attr_key ~= "Key" and attr_key ~= "TargetType" and attr_key ~= "BasedOn"
					   and attr_key ~= "StyleTargetType" and attr_key ~= "StyleType" then
						props[attr_key] = attr_value
					end
				end
				if next(props) then node:addStyleRule("." .. selector, props) end
			end
			::continue::
		end
	end,
	ValueTicker = function () end,
}

-- Handler for attach-only component child elements (e.g. <AnimationPlayer Playing="true"/>).
-- Attaches the named component to the parent node, then parses its attributes as properties.
local function attach_only_component(node, element)
	node:addComponentByName(element.tag)
	for k, v in element.attributes do
		xpcall(Property.parse, print, node, k, v)
	end
end

-- AnimationCurve is a regular child object of AnimationClip (not attach-only).
-- Create it as a child node and parse its attributes as properties.
local function animation_curve_child(node, element)
	local curve = orca.AnimationCurve()
	for k, v in element.attributes do
		xpcall(Property.parse, print, curve, k, v)
	end
	-- Parse child property elements (e.g. Keyframes array)
	for sub in element.children do
		if curve:findExplicitProperty(sub.tag) then
			xpcall(Property.construct, print, curve, curve:findExplicitProperty(sub.tag), sub)
		end
	end
	node:addChild(curve)
	curve:send("Object.Start")
end

specials.AnimationPlayer = attach_only_component
specials.AnimationCurve  = animation_curve_child

-- StateManagerController is an attach-only component.  Attributes are set as
-- properties on the host node.  An optional inline <StateManager> child element
-- is constructed as a StateManager object and assigned to the StateManager property.
specials.StateManagerController = function(node, element)
	node:addComponentByName("StateManagerController")
	for k, v in element.attributes do
		xpcall(Property.parse, print, node, k, v)
	end
	for sub in element.children do
		if sub.tag == "StateManager" then
			local sm = construct_node(sub)
			node.StateManager = sm
		end
	end
end

local function construct_node(element)
	local class =
		try_prefab_placeholder(element) or
		try_require_memeber("orca.core", element.tag) or
		try_require_memeber("orca.UIKit", element.tag) or
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
	node:send("Object.Start")
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
		local node = construct_node(doc.root)
		node:setSourceFile(path)
		node.instantiate = function()
			local instance = construct_node(doc.root)
			instance:setSourceFile(path)
			return instance
		end
		return node
	end or nil
end)
