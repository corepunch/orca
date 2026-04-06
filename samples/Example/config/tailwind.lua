local LightTheme = {
	["background"] = "#FFFFFF",
	["foreground"] = "#0B0F1A",
	["card"] = "#FFFFFF",
	["card-foreground"] = "#0B0F1A",
	["primary"] = "#345EC7",
	["primary-foreground"] = "#FFFFFF",
	["secondary"] = "#E3E8F0",
	["secondary-foreground"] = "#0B0F1A",
	["muted"] = "#E3E8F0",
	["muted-foreground"] = "#6B7280",
	["destructive"] = "#D63939",
	["destructive-foreground"] = "#FFFFFF",
	["border"] = "#E3E8F0",
	["input"] = "#E3E8F0",
	["ring"] = "#4782F5",

	["primary-500"] = "#877EFF",
	["primary-600"] = "#5D5FEF",
	["secondary-500"] = "#FFB620",

	["off-white"] = "#D0DFFF",
	["red"] = "#FF5A5A",
	["dark-1"] = "#000000",
	["dark-2"] = "#141417",
	["dark-3"] = "#25252B",
	["dark-4"] = "#3A3A42",
	["light-1"] = "#FFFFFF",
	["light-2"] = "#E0E0E6",
	["light-3"] = "#B0B0D0",
	["light-4"] = "#9494BB",

	["neutral-1"] = "#FFFFFF",
	["neutral-2"] = "#F3F4F6",
	["neutral-3"] = "#E5E7EB",
	["neutral-4"] = "#D1D5DB",
	["neutral-5"] = "#9CA3AF",
	["neutral-6"] = "#6B7280",
	["neutral-7"] = "#374151",
	["neutral-8"] = "#1F2937",
	["neutral-9"] = "#111827",

	["button"] = "#77AADD",
	["button-hover"] = "#88BBFF",
}

local DarkTheme = {
	["background"] = "#050813",
	["foreground"] = "#F8FAFC",
	["card"] = "#050813",
	["card-foreground"] = "#F8FAFC",
	["primary"] = "#4782F5",
	["primary-foreground"] = "#0B0F1A",
	["secondary"] = "#1C263A",
	["secondary-foreground"] = "#F8FAFC",
	["muted"] = "#1C263A",
	["muted-foreground"] = "#92A1C3",
	["destructive"] = "#5A1313",
	["destructive-foreground"] = "#F8FAFC",
	["border"] = "#1C263A",
	["input"] = "#1C263A",
	["ring"] = "#345EC7",

	["primary-500"] = "#877EFF",
	["primary-600"] = "#5D5FEF",
	["secondary-500"] = "#FFB620",

	["off-white"] = "#D0DFFF",
	["red"] = "#FF5A5A",
	["dark-1"] = "#000000",
	["dark-2"] = "#141417",
	["dark-3"] = "#25252B",
	["dark-4"] = "#3A3A42",
	["light-1"] = "#FFFFFF",
	["light-2"] = "#E0E0E6",
	["light-3"] = "#B0B0D0",
	["light-4"] = "#9494BB",

	["neutral-9"] = "#FFFFFF",
	["neutral-8"] = "#F3F4F6",
	["neutral-7"] = "#E5E7EB",
	["neutral-6"] = "#D1D5DB",
	["neutral-5"] = "#9CA3AF",
	["neutral-4"] = "#6B7280",
	["neutral-3"] = "#374151",
	["neutral-2"] = "#1F2937",
	["neutral-1"] = "#111827",

	["button"] = "#6699CC",
	["button-hover"] = "#77AADD",
}

-- orca = require "orca",
-- system = require "orca.system",
-- system.set_theme "dark",

-- class Theme
-- 	extend:
-- 		colors: if system.get_theme() then DarkTheme else LightTheme

_ENV.extend = {
	colors = DarkTheme,
}
