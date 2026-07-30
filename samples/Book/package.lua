Name = "Book"
StartupScreen = "Book/Screens/Start"
WindowWidth = 1024
WindowHeight = 768
ProjectReferences = {
	{ Name = "zilscript", Path = "libs/zilscript/zilscript" },
	{ Name = "books", Path = "libs/zilscript/books" },
	{ Name = "infocom", Path = "libs/zilscript/infocom" },
}
ScreenLibrary = { IsExternal = true }
ImageLibrary = { IsExternal = true }
PrefabLibrary = { IsExternal = true }
FontLibrary = { Name = "Fonts", IsExternal = true }
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
