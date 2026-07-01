Name = "Adventure"
StartupScreen = "Adventure/Screens/Adventures"
WindowWidth = 400
WindowHeight = 800
ScreenLibrary = { IsExternal = true }
ImageLibrary = { IsExternal = true }
PrefabLibrary = { IsExternal = true }
ThemeLibrary = {
	{ Key = "background", Value = "#FFFFFF" },
	{ Key = "foreground", Value = "#0B0F1A" },
	{ Key = "accent", Value = "#345EC7" },
	{ Key = "accent-foreground", Value = "#FFFFFF" },
	{ Key = "accent-background", Value = "#C5CDDC" },
	{ Key = "muted-foreground", Value = "#6B7280" },
	{ Key = "border", Value = "#E3E8F0" },
	{ Key = "card-background", Value = "#F8F9FA" },
}
SystemMessages = {
	{ Name = "KeyDown", Key = "q", Command = "return" },
	{ Name = "WindowClosed", Command = "return" },
	{ Name = "RequestReload", Command = "window:refresh()" },
}
EnginePlugins = {
	{ Name = "orca.UIKit" },
}
