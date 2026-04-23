Name = "Example"
StartupScreen = "Example/Screens/Application"
WindowWidth = 1024
WindowHeight = 768
ScreenLibrary = { IsExternal = true }
ImageLibrary = { IsExternal = true }
PrefabLibrary = { IsExternal = true }
ThemeLibrary = {
	-- ThemeGroup {
	-- 	Name = "Main",
	-- 	SelectedTheme = "Default",
	-- 	Default = {
	{ Key = "panel-background", Value = "#1A1A28" },
	{ Key = "surface", Value = "#0B0B12" },
	{ Key = "surface-deep", Value = "#09090F" },
	{ Key = "card-background", Value = "#141420" },
	{ Key = "text-primary", Value = "#F1F5F9" },
	{ Key = "text-secondary", Value = "#94A3B8" },
	{ Key = "text-muted", Value = "#64748B" },
	{ Key = "text-dim", Value = "#4B5563" },
	{ Key = "accent", Value = "#8B5CF6" },
	{ Key = "accent-green", Value = "#10B981" },
	{ Key = "accent-blue", Value = "#3B82F6" },
	{ Key = "accent-amber", Value = "#F59E0B" },
	{ Key = "white", Value = "#FFFFFF" },
	{ Key = "icon-bg-purple", Value = "#1E1833" },
	{ Key = "icon-bg-green", Value = "#0D2020" },
	-- 	},
	-- },
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
EnginePlugins = {
	{ Name="orca.UIKit" },
}