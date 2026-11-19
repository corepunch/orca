-- Weather: one forecast per place, read by every activity that fits it
-- (outdoors, fishing, surf, sailing, hiking, garden, stars). See README.md.
Name = "Weather"
StartupViewController = "Weather/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#1BA1E2" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
