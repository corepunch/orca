local has_config, config = pcall(require, "config.tailwind")
local extends = { "colors" }
local spacing = { 0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48, 56, 64 }
local style = {}
local tone={50,         100,        200,        300,        400,        500,        600,        700,        800,        900,        950  }
local rem=4
local spreadsheet = {
	slate=  {"#f8fafc","#f1f5f9","#e2e8f0","#cbd5e1","#94a3b8","#64748b","#475569","#334155","#1e293b","#0f172a","#020617"},
	gray=   {"#f9fafb","#f3f4f6","#e5e7eb","#d1d5db","#9ca3af","#6b7280","#4b5563","#374151","#1f2937","#111827","#030712"},
	zinc=   {"#fafafa","#f4f4f5","#e4e4e7","#d4d4d8","#a1a1aa","#71717a","#52525b","#3f3f46","#27272a","#18181b","#09090b"},
	neutral={"#fafafa","#f5f5f5","#e5e5e5","#d4d4d4","#a3a3a3","#737373","#525252","#404040","#262626","#171717","#0a0a0a"},
	stone=  {"#fafaf9","#f5f5f4","#e7e5e4","#d6d3d1","#a8a29e","#78716c","#57534e","#44403c","#292524","#1c1917","#0c0a09"},
	red=    {"#fef2f2","#fee2e2","#fecaca","#fca5a5","#f87171","#ef4444","#dc2626","#b91c1c","#991b1b","#7f1d1d","#450a0a"},
	orange= {"#fff7ed","#ffedd5","#fed7aa","#fdba74","#fb923c","#f97316","#ea580c","#c2410c","#9a3412","#7c2d12","#431407"},
	amber=  {"#fffbeb","#fef3c7","#fde68a","#fcd34d","#fbbf24","#f59e0b","#d97706","#b45309","#92400e","#78350f","#451a03"},
	yellow= {"#fefce8","#fef9c3","#fef08a","#fde047","#facc15","#eab308","#ca8a04","#a16207","#854d0e","#713f12","#422006"},
	lime=   {"#f7fee7","#ecfccb","#d9f99d","#bef264","#a3e635","#84cc16","#65a30d","#4d7c0f","#3f6212","#365314","#1a2e05"},
	green=  {"#f0fdf4","#dcfce7","#bbf7d0","#86efac","#4ade80","#22c55e","#16a34a","#15803d","#166534","#14532d","#052e16"},
	emerald={"#ecfdf5","#d1fae5","#a7f3d0","#6ee7b7","#34d399","#10b981","#059669","#047857","#065f46","#064e3b","#022c22"},
	teal=   {"#f0fdfa","#ccfbf1","#99f6e4","#5eead4","#2dd4bf","#14b8a6","#0d9488","#0f766e","#115e59","#134e4a","#042f2e"},
	cyan=   {"#ecfeff","#cffafe","#a5f3fc","#67e8f9","#22d3ee","#06b6d4","#0891b2","#0e7490","#155e75","#164e63","#083344"},
	sky=    {"#f0f9ff","#e0f2fe","#bae6fd","#7dd3fc","#38bdf8","#0ea5e9","#0284c7","#0369a1","#075985","#0c4a6e","#082f49"},
	blue=   {"#eff6ff","#dbeafe","#bfdbfe","#93c5fd","#60a5fa","#3b82f6","#2563eb","#1d4ed8","#1e40af","#1e3a8a","#172554"},
	indigo= {"#eef2ff","#e0e7ff","#c7d2fe","#a5b4fc","#818cf8","#6366f1","#4f46e5","#4338ca","#3730a3","#312e81","#1e1b4b"},
	violet= {"#f5f3ff","#ede9fe","#ddd6fe","#c4b5fd","#a78bfa","#8b5cf6","#7c3aed","#6d28d9","#5b21b6","#4c1d95","#2e1065"},
	purple= {"#faf5ff","#f3e8ff","#e9d5ff","#d8b4fe","#c084fc","#a855f7","#9333ea","#7e22ce","#6b21a8","#581c87","#3b0764"},
	fuchsia={"#fdf4ff","#fae8ff","#f5d0fe","#f0abfc","#e879f9","#d946ef","#c026d3","#a21caf","#86198f","#701a75","#4a044e"},
	pink=   {"#fdf2f8","#fce7f3","#fbcfe8","#f9a8d4","#f472b6","#ec4899","#db2777","#be185d","#9d174d","#831843","#500724"},
	rose=   {"#fff1f2","#ffe4e6","#fecdd3","#fda4af","#fb7185","#f43f5e","#e11d48","#be123c","#9f1239","#881337","#4c0519"},
}
for k1, v1 in pairs {m="Margin",p="Padding"} do
	for _, v3 in ipairs(spacing) do
		for k2, v2 in pairs {x="Horizontal",y="Vertical",t="Top",b="Bottom",l="Left",r="Right",} do
			local class = string.format(".%s%s-%d", k1, k2, v3)
			local name = v1..v2
			local hack = {Horizontal=true, Vertical=true}
			if hack[v2] then
				name = v2..v1
			end
			style[class] = { [name] = string.format("%d", v3*rem) }
		end
		style[string.format(".%s-%s", k1, v3)] = {
			["Horizontal"..v1] = string.format("%d", v3*rem),
			["Vertical"..v1] = string.format("%d", v3*rem),
		}
	end
end

local theme = {
	colors = {}
}

for k, v in pairs(spreadsheet) do
	for i, color in ipairs(v) do
		theme.colors[string.format("%s-%s", k, tone[i])] = color
	end
end

if has_config and config.extend then
	for _, name in ipairs(extends) do
		for k, v in pairs(config.extend[name] or {}) do
			theme[name][k] = v
		end
	end
end

for k1, v1 in pairs {bg="Background",text="Foreground",border="Border",ring="Ring",placeholder="Placeholder"} do
	for k2, v2 in pairs(theme.colors) do
		local name = string.format(".%s-%s", k1, k2)
		style[name] = { [v1.."Color"] = v2 }
	end
end

for k1, v1 in pairs {middle="Center",top="Top",bottom="Bottom"} do
	style[".align-"..k1] = { VerticalAlignment = v1 }
	for k2, v2 in pairs {center="Center",left="Left",right="Right"} do
		style[string.format(".align-%s-%s", k1, k2)] = {
			HorizontalAlignment = v2,
			VerticalAlignment = v1
		}
		style[".align-"..k2] = { HorizontalAlignment = v2 }
	end
end

for _, v1 in ipairs(spacing) do
	for k2, v2 in pairs {rounded="BorderRadius",text="FontSize",gap="Spacing",w="Width",h="Height"} do
		style[string.format(".%s-%s", k2, v1)] = { [v2] = v1*rem }
	end
end

for _, v1 in ipairs(spacing) do
	for k2, v2 in pairs {border="BorderWidth", underline="UnderlineWidth", ["underline-offset"]="UnderlineOffset", ring="RingWidth", ["ring-offset"]="RingOffset"} do
		style[string.format(".%s-%s", k2, v1)] = { [v2] = v1 }
	end
end

for k, v in pairs {start="Start",["end"]="End",center="Center",baseline="Baseline",stretch="Stretch"} do
	style[".items-"..k] = { AlignItems = v }
end

for k, v in pairs {left="Left",center="Center",right="Right"} do
	style[".text-"..k] = { TextHorizontalAlignment = v }
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
	style[".text-"..k] = { FontSize = v[1], LineHeight = v[2] }
end

for k, v in pairs {col="Vertical",row="Horizontal"} do
	style[".flex-"..k] = { Direction = v }
end

for k1, v1 in pairs {
	auto   ="Auto",
	hidden ="Hidden",
	clip   ="Clip",
	visible="Visible",
	-- scroll ="Scroll"
} do
	for k2, v2 in pairs {x="X", y="Y"} do
		local class = string.format(".overflow-%s-%s", k2, k1)
		style[class] = { ["Overflow"..v2] = v1 }
	end	
end

style[".overflow-x-scroll"] = {
	OverflowX = "Scroll",
	ClipChildren = true,
}

style[".overflow-y-scroll"] = {
	OverflowY = "Scroll",
	ClipChildren = true,
}

for k, v in pairs {
	["start"]   = "Start",
	["end"]     = "End",
	["center"]  = "Center",
	["between"] = "SpaceBetween",
	["around"]  = "SpaceAround",
	["evenly"]  = "SpaceEvenly"
} do
	style[".justify-"..k] = { JustifyContent = v }
end	


style[".font-normal"] = { FontWeight = "Normal" }
style[".font-bold"] = { FontWeight = "Bold" }
style[".non-italic"] = { FontStyle = "Normal" }
style[".italic"] = { FontStyle = "Italic" }
style[".w-full"] = { HorizontalAlignment = "Stretch" }
style[".h-full"] = { VerticalAlignment = "Stretch" }
style[".rounded"] = { BorderRadius = "8" }
style[".border"] = { BorderWidth = "1" }
style[".underline"] = { UnderlineWidth = "1" }
style[".no-underline"] = { UnderlineWidth = "0" }

return style
