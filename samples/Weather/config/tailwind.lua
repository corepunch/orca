local system = require "orca.system"

local LightTheme = {
	["background"]        = "#f8fafc",  -- slate-50:  page background
	["surface"]           = "#ffffff",  -- white:     card / panel background
	["surface-alt"]       = "#f1f5f9",  -- slate-100: elevated surface (inputs, radio buttons)
	["foreground"]        = "#1e293b",  -- slate-800: primary text
	["foreground-muted"]  = "#64748b",  -- slate-500: secondary / label text
	["accent"]            = "#0284c7",  -- sky-600:   interactive accent
	["accent-foreground"] = "#ffffff",  -- white:     text on accent backgrounds
	["accent-subtle"]     = "#e0f2fe",  -- sky-100:   subtle text on selected rows
	["header-bg"]         = "#075985",  -- sky-800:   header / bottom-nav background
}

local DarkTheme = {
	["background"]        = "#0f172a",  -- slate-900: page background
	["surface"]           = "#1e293b",  -- slate-800: card / panel background
	["surface-alt"]       = "#334155",  -- slate-700: elevated surface (inputs, radio buttons)
	["foreground"]        = "#f1f5f9",  -- slate-100: primary text
	["foreground-muted"]  = "#94a3b8",  -- slate-400: secondary / label text
	["accent"]            = "#38bdf8",  -- sky-400:   interactive accent
	["accent-foreground"] = "#ffffff",  -- white:     text on accent backgrounds
	["accent-subtle"]     = "#e0f2fe",  -- sky-100:   subtle text on selected rows
	["header-bg"]         = "#1e293b",  -- slate-800: header / bottom-nav background
}

extend = {
	colors = system.get_theme() and DarkTheme or LightTheme,
}
