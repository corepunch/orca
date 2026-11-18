-- Stargazing: the stargazing profile over the shared weather kit (../WeatherKit).
Name = "Stargazing"
StartupViewController = "Stargazing/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "WeatherKit", Path = "../WeatherKit" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#AA00FF" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
