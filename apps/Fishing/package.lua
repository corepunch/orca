-- Fishing: the fishing profile over the shared weather kit (../WeatherKit).
Name = "Fishing"
StartupViewController = "Fishing/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "WeatherKit", Path = "../WeatherKit" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#60A917" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
