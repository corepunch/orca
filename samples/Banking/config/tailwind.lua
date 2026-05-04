local system = require "orca.system"
system.set_theme(true)  -- force dark theme

local DarkTheme = {
	["background"]             = "#050813",
	["foreground"]             = "#F8FAFC",
	["card"]                   = "#050813",
	["card-foreground"]        = "#F8FAFC",
	["popover"]                = "#050813",
	["popover-foreground"]     = "#F8FAFC",
	["primary"]                = "#4782F5",
	["primary-foreground"]     = "#0B0F1A",
	["secondary"]              = "#1C263A",
	["secondary-foreground"]   = "#F8FAFC",
	["muted"]                  = "#1C263A",
	["muted-foreground"]       = "#92A1C3",
	["accent"]                 = "#1C263A",
	["accent-foreground"]      = "#F8FAFC",
	["destructive"]            = "#5A1313",
	["destructive-foreground"] = "#F8FAFC",
	["border"]                 = "#1C263A",
	["input"]                  = "#1C263A",
	["ring"]                   = "#345EC7",
	["header-bg"]              = "#0B0F1A",
	["surface"]                = "#1C263A",
	["foreground-muted"]       = "#92A1C3",
	["primary-500"]            = "#877EFF",
	["primary-600"]            = "#5D5FEF",
	["secondary-500"]          = "#FFB620",
	["off-white"]              = "#D0DFFF",
	["red"]                    = "#FF5A5A",
	["dark-1"]                 = "#000000",
	["dark-2"]                 = "#141417",
	["dark-3"]                 = "#25252B",
	["dark-4"]                 = "#3A3A42",
	["light-1"]                = "#FFFFFF",
	["light-2"]                = "#E0E0E6",
	["light-3"]                = "#B0B0D0",
	["light-4"]                = "#9494BB",
}

return {
	extend = {
		colors = DarkTheme,
	},
}
