-- Surf: the surf profile over the shared weather kit (../WeatherKit).
Name = "Surf"
StartupViewController = "Surf/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "WeatherKit", Path = "../WeatherKit" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#1BA1E2" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
