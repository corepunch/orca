Name = "Example"
StartupScreen = "Example/Screens/Application"
WindowWidth = 1024
WindowHeight = 768
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
-- PropertyTypeLibrary = {
-- 	Title = { DataType="String", Category="Card" },
-- 	Subtitle = { DataType="String", Category="Card" },
-- 	Body = { DataType="String", Category="Card" },
-- 	Icon = { DataType="String", Category="Card" },
-- 	IconBackground = { DataType="Color", Category="Card" },
-- 	Image = { DataType="Object", TypeString="Texture", Category="Card" },
-- 	PrimaryColor = { DataType="Color", Category="Card" },
-- }
PropertyTypes = {
	{ Name="Title", DataType="String", Category="Card" },
	{ Name="Subtitle", DataType="String", Category="Card" },
	{ Name="Body", DataType="String", Category="Card" },
	{ Name="Icon", DataType="String", Category="Card" },
	{ Name="IconBackground", DataType="Color", Category="Card" },
	{ Name="Image", DataType="Object", TypeString="Texture", Category="Card" },
	{ Name="PrimaryColor", DataType="Color", Category="Card" },
}
-- SystemMessageLibrary = {
SystemMessages = {
	{ Name="KeyDown", Key="q", Command="return" },
	{ Name="WindowClosed", Command="return" },
	{ Name="RequestReload", Command="window:refresh()" },
}
