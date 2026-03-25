Name = "Example"
StartupScreen = "Example/Screens/Application"
ScreenLibrary = { IsExternal = true }
ImageLibrary = { IsExternal = true }
PrefabLibrary = { IsExternal = true }
ThemeLibrary = {
	Main = {
		SelectedTheme = "Default",
		Default = {
			["panel-background"] = "#1A1A28",
			["surface"] = "#0B0B12",
			["surface-deep"] = "#09090F",
			["card-background"] = "#141420",
			["text-primary"] = "#F1F5F9",
			["text-secondary"] = "#94A3B8",
			["text-muted"] = "#64748B",
			["text-dim"] = "#4B5563",
			["accent"] = "#8B5CF6",
			["accent-green"] = "#10B981",
			["accent-blue"] = "#3B82F6",
			["accent-amber"] = "#F59E0B",
			["white"] = "#FFFFFF",
			["icon-bg-purple"] = "#1E1833",
			["icon-bg-green"] = "#0D2020",
		},
	},
}
PropertyTypeLibrary = {
	Title = { DataType="String", Category="Card" },
	Subtitle = { DataType="String", Category="Card" },
	Body = { DataType="String", Category="Card" },
	Icon = { DataType="String", Category="Card" },
	IconBackground = { DataType="Color", Category="Card" },
	Image = { DataType="Object", TypeString="Texture", Category="Card" },
	PrimaryColor = { DataType="Color", Category="Card" },
}
SystemMessageLibrary = {
	KeyDown = { Key="q", Command="return" },
	WindowClosed = { Command="return" },
	RequestReload = { Command="window:refresh()" },
}
