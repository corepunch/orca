-- WPF-style XAML stylesheet loader for ORCA.
-- Registers a package.searcher so that `require "themes/dark"` loads
-- `themes/dark.xaml` as a style table suitable for `addStyleRule`.
--
-- Supported WPF constructs:
--   <Style x:Key="name">           → selector ".name"
--   <Style TargetType="Button">    → selector ".Button"
--   <Style BasedOn="base">         → @apply = "base" (inherits base rules)
--   <Setter Property="P" Value="V"/>   → property P = V
--   <Style.Triggers>
--     <Trigger Property="IsMouseOver"        Value="True">  :hover
--     <Trigger Property="IsFocused"          Value="True">  :focus
--     <Trigger Property="IsKeyboardFocused"  Value="True">  :focus
--     <Trigger Property="IsSelected"         Value="True">  :active
--     <Trigger Property="IsChecked"          Value="True">  :active
--     <Trigger Property="IsPressed"          Value="True">  :active
--   </Style.Triggers>
--
-- Usage:
--   local styles = require "themes/dark"   -- loads themes/dark.xaml
--   for k, v in pairs(styles) do
--       screen:addStyleRule(k, v)
--   end
--   -- Or register globally (applies to all screens):
--   for k, v in pairs(require "themes/dark") do orca.styles[k] = v end
--
-- Note: TargetType-based styles (e.g. <Style TargetType="Button">) register
-- rules under the type name as a class selector (e.g. ".Button"). To receive
-- them, either add class="Button" explicitly to elements, or use x:Key instead.

local trigger_to_pseudo = {
	IsMouseOver           = "hover",
	IsMouseDirectlyOver   = "hover",
	IsFocused             = "focus",
	IsKeyboardFocused     = "focus",
	IsKeyboardFocusWithin = "focus",
	IsSelected            = "active",
	IsChecked             = "active",
	IsPressed             = "active",
	IsActive              = "active",
}

-- Strip "{StaticResource key}" WPF resource reference syntax → "key".
local function strip_resource_ref(s)
	return s and (s:match("^{StaticResource%s+(.-)%}$") or s)
end

-- Read all <Setter Property="P" Value="V"/> children of `element` into `props`.
local function collect_setters(element, props)
	for setter in element:findall "Setter" do
		local p = setter:get "Property"
		local v = setter:get "Value"
		if p and v then props[p] = v end
	end
end

-- Parse a single <Style> element and merge its entries into `result`.
local function parse_style(style, result)
	-- x:Key or TargetType gives the selector name (namespace prefix stripped by parser)
	local selector = style:get "Key" or style:get "TargetType"
	if not selector then
		io.stderr:write("[wpf] Style has no x:Key or TargetType — skipped\n")
		return
	end

	-- Base rules: direct <Setter> children + optional BasedOn
	local base = {}
	local based_on = strip_resource_ref(style:get "BasedOn")
	if based_on then base["@apply"] = based_on end
	collect_setters(style, base)

	if next(base) then
		local k = "." .. selector
		result[k] = result[k] or {}
		for p, v in pairs(base) do result[k][p] = v end
	end

	-- Pseudo-state rules from <Style.Triggers>
	for tc in style:findall "Style.Triggers" do
		for trigger in tc:findall "Trigger" do
			local tp = trigger:get "Property"
			local tv = trigger:get "Value"
			if tp and tv == "True" then
				local pseudo = trigger_to_pseudo[tp]
				if pseudo then
					local tprops = {}
					collect_setters(trigger, tprops)
					if next(tprops) then
						local k = "." .. selector .. ":" .. pseudo
						result[k] = result[k] or {}
						for p, v in pairs(tprops) do result[k][p] = v end
					end
				else
					io.stderr:write(("[wpf] Unknown trigger property: %s\n"):format(tp))
				end
			end
		end
	end
end

-- Parse a WPF ResourceDictionary or Styles document into an ORCA style table.
-- The returned table maps selectors (e.g. ".button", ".button:hover") to
-- property tables suitable for `addStyleRule`.
local function wpf_parse(doc)
	local root = doc.root
	if root.tag ~= "ResourceDictionary" and root.tag ~= "Styles" then
		io.stderr:write(("[wpf] Root must be <ResourceDictionary> or <Styles>, got <%s>\n"):format(root.tag))
		return {}
	end
	local result = {}
	for style in root:findall "Style" do
		parse_style(style, result)
	end
	return result
end

-- Register a package.searcher for .xaml files.
-- `require "themes/dark"` will find and parse `themes/dark.xaml`.
table.insert(package.searchers, function(path)
	local xml     = require "orca.parsers.xml"
	local ok, doc = pcall(xml.load, path .. ".xaml")
	if not ok or not doc then return nil end
	return function() return wpf_parse(doc) end
end)
