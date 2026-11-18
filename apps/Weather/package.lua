-- Weather: the everyday profile over the shared weather kit (../WeatherKit).
-- Fishing, Surf and the other activity apps are siblings of this one.
Name = "Weather"
StartupViewController = "Weather/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "WeatherKit", Path = "../WeatherKit" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#0050EF" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
