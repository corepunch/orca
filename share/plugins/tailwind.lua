local orca = require "orca"
local core = require "orca.core"
local filesystem = require "orca.filesystem"
local spacing = { 0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48, 56, 64 }
local tone={50,         100,        200,        300,        400,        500,        600,        700,        800,        900,        950  }
local color_families = {
	"slate", "gray", "zinc", "neutral", "stone", "red", "orange", "amber",
	"yellow", "lime", "green", "emerald", "teal", "cyan", "sky", "blue",
	"indigo", "violet", "purple", "fuchsia", "pink", "rose",
}
local semantic_colors = {
	"background", "foreground", "foreground-muted",
	"panel-background", "surface", "surface-deep", "surface-alt",
	"card", "card-foreground", "card-background",
	"popover", "popover-foreground",
	"primary", "primary-foreground",
	"secondary", "secondary-foreground",
	"muted", "muted-foreground",
	"accent", "accent-background", "accent-foreground", "accent-green",
	"accent-blue", "accent-amber", "accent-subtle",
	"destructive", "destructive-foreground",
	"border", "input", "ring", "header-background",
	"text-primary", "text-secondary", "text-muted", "text-dim",
	"control-background", "control-foreground", "control-border", "control-muted",
	"icon-bg-purple", "icon-bg-blue", "icon-bg-green", "white",
}
local rem=4
orca.theme = orca.theme or {}
orca.theme.colors = orca.theme.colors or {}
local theme = orca.theme

local function add_theme_color(name)
	if theme.colors[name] then return end
	local value = filesystem.getThemeValue("$"..name)
	if value then
		theme.colors[name] = value
		theme["$"..name] = value
	end
end

-- Helper: create a StyleRule from a selector string and a property table, then add to styles.
local function add_rule(classname, props)
	local rule = core.StyleRule { ClassName = classname }
	for key, value in pairs(props) do
		local ptype = rule:findExplicitProperty(key)
		if type(value) == "string" and ptype then
			rule[key] = orca.core.parseProperty(value, ptype)
		else
			rule[key] = value
		end
	end
	core.addGlobalStyleRule(rule)
end

for k1, v1 in pairs {m="Margin",p="Padding"} do
	for _, v3 in ipairs(spacing) do
		for k2, v2 in pairs {x="Horizontal",y="Vertical",t="Top",b="Bottom",l="Left",r="Right",} do
			local class = string.format("%s%s-%d", k1, k2, v3)
			local name = v1..v2
			local hack = {Horizontal=true, Vertical=true}
			if hack[v2] then
				name = v2..v1
			end
			add_rule(class, { ["Node."..name] = string.format("%d", v3*rem) })
		end
		add_rule(string.format("%s-%s", k1, v3), {
			["Node.Horizontal"..v1] = string.format("%d %d", v3*rem, v3*rem),
			["Node.Vertical"..v1] = string.format("%d %d", v3*rem, v3*rem),
		})
	end
end

for _, name in ipairs(semantic_colors) do
	add_theme_color(name)
end

for _, family in ipairs(color_families) do
	for _, step in ipairs(tone) do
		add_theme_color(string.format("%s-%s", family, step))
	end
end

for k1, v1 in pairs {
		placeholder="TextBlockConcept.Placeholder",
		text="Node2D.Foreground",
		bg="Node2D.Background",
		ring="Node2D.Ring",
		border="Node.Border",
	} do
	for k2, v2 in pairs(theme.colors) do
		local name = string.format("%s-%s", k1, k2)
		add_rule(name, { [v1.."Color"] = v2 })
	end
end

for k1, v1 in pairs {middle="Center",top="Top",bottom="Bottom"} do
	add_rule("align-"..k1, { ["Node.VerticalAlignment"] = v1 })
	for k2, v2 in pairs {center="Center",left="Left",right="Right"} do
		add_rule(string.format("align-%s-%s", k1, k2), {
			["Node.HorizontalAlignment"] = v2,
			["Node.VerticalAlignment"] = v1
		})
		add_rule("align-"..k2, { ["Node.HorizontalAlignment"] = v2 })
	end
end

for _, v1 in ipairs(spacing) do
	for k2, v2 in pairs {
			text="TextRun.FontSize",
			w="Node.Width",
			h="Node.Height"
		} do
		add_rule(string.format("%s-%s", k2, v1), { [v2] = v1*rem })
	end
	add_rule(string.format("rounded-%s", v1), { ["Node.BorderRadius"] = core.CornerRadius(v1*rem) })
	add_rule(string.format("gap-%s", v1), { ["StackView.Spacing"] = v1*rem, ["Grid.Spacing"] = v1*rem })
end

for _, v1 in ipairs(spacing) do
	for k2, v2 in pairs {
			["underline"]="TextRun.UnderlineWidth",
			["underline-offset"]="TextRun.UnderlineOffset",
			["ring"]="Node2D.RingWidth",
			["ring-offset"]="Node2D.RingOffset"
		} do
		add_rule(string.format("%s-%s", k2, v1), { [v2] = v1 })
	end
	add_rule(string.format("border-%s", v1), { ["Node.BorderWidth"] = core.Thickness(v1) })
end

for k, v in pairs {start="Start",["end"]="End",center="Center",baseline="Baseline",stretch="Stretch"} do
	add_rule("items-"..k, { ["StackView.AlignItems"] = v })
end

for k, v in pairs {left="Left",center="Center",right="Right"} do
	add_rule("text-"..k, { ["TextBlockConcept.TextHorizontalAlignment"] = v })
end

for k, v in pairs {top="Top",middle="Center",bottom="Bottom"} do
	add_rule("text-"..k, { ["TextBlockConcept.TextVerticalAlignment"] = v })
end

for k, v in pairs {
	["xs"] =   { 12,  16},
	["sm"] =   { 14,  20},
	["base"] = { 16,  24},
	["lg"] =   { 18,  28},
	["xl"] =   { 20,  28},
	["2xl"] =  { 24,  32},
	["3xl"] =  { 30,  36},
	["4xl"] =  { 36,  40},
	["5xl"] =  { 48,  48},
	["6xl"] =  { 60,  60},
	["7xl"] =  { 72,  72},
	["8xl"] =  { 96,  96},
	["9xl"] =  { 128, 128},
} do
	add_rule("text-"..k, { ["TextRun.FontSize"] = v[1], ["TextRun.LineHeight"] = v[2] })
end

for k, v in pairs {col="Vertical",row="Horizontal"} do
	add_rule("flex-"..k, { ["StackView.Direction"] = v })
end

for k1, v1 in pairs {
	auto   ="Auto",
	hidden ="Hidden",
	clip   ="Clip",
	visible="Visible",
	-- scroll ="Scroll"
} do
	for k2, v2 in pairs {x="X", y="Y"} do
		local class = string.format("overflow-%s-%s", k2, k1)
		add_rule(class, { ["Node2D.Overflow"..v2] = v1 })
	end	
end

add_rule("overflow-x-scroll", {
	["Node2D.OverflowX"] = "Scroll",
	["Node2D.ClipChildren"] = true,
})

add_rule("overflow-y-scroll", {
	["Node2D.OverflowY"] = "Scroll",
	["Node2D.ClipChildren"] = true,
})

for k, v in pairs {
	["start"]   = "Start",
	["end"]     = "End",
	["center"]  = "Center",
	["between"] = "SpaceBetween",
	["around"]  = "SpaceAround",
	["evenly"]  = "SpaceEvenly"
} do
	add_rule("justify-"..k, { ["StackView.JustifyContent"] = v })
end	

add_rule("text-clip", { ["TextBlockConcept.TextOverflow"] = "Clip" })
add_rule("text-ellipsis", { ["TextBlockConcept.TextOverflow"] = "Ellipsis" })
add_rule("text-wrap", { ["TextBlockConcept.TextWrapping"] = "WrapWithOverflow" })
add_rule("text-nowrap", { ["TextBlockConcept.TextWrapping"] = "NoWrap" })
add_rule("font-normal", { ["TextRun.FontWeight"] = "Normal" })
add_rule("font-bold", { ["TextRun.FontWeight"] = "Bold" })
add_rule("non-italic", { ["TextRun.FontStyle"] = "Normal" })
add_rule("italic", { ["TextRun.FontStyle"] = "Italic" })
add_rule("w-full", { ["Node.HorizontalAlignment"] = "Stretch" })
add_rule("h-full", { ["Node.VerticalAlignment"] = "Stretch" })
add_rule("rounded", { ["Node.BorderRadius"] = "8" })
add_rule("rounded-full", { ["Node.BorderRadius"] = core.CornerRadius(9999) })
add_rule("border", { ["Node.BorderWidth"] = "1" })
add_rule("underline", { ["TextRun.UnderlineWidth"] = "1" })
add_rule("no-underline", { ["TextRun.UnderlineWidth"] = "0" })

-- return style
