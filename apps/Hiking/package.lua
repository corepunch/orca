-- Hiking: the hiking profile over the shared weather kit (../WeatherKit).
Name = "Hiking"
StartupViewController = "Hiking/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "WeatherKit", Path = "../WeatherKit" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#825A2C" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
