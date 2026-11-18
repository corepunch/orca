-- Sailing: the sailing profile over the shared weather kit (../WeatherKit).
Name = "Sailing"
StartupViewController = "Sailing/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "WeatherKit", Path = "../WeatherKit" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#6A00FF" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
