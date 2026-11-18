-- Garden: the garden profile over the shared weather kit (../WeatherKit).
Name = "Garden"
StartupViewController = "Garden/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "WeatherKit", Path = "../WeatherKit" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#008A00" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
