-- Weather: one forecast per place, read by every activity that fits it
-- (outdoors, fishing, surf, sailing, hiking, garden, stars). See README.md.
Name = "Weather"
StartupViewController = "Weather/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
-- One-activity builds: make iphone APP=weather EDITION=surf installs
-- "Surfers' Weather" as org.orca.weather.surf (see activities.lua).
Editions = {
    { Key = "fishing", Name = "Fishers' Weather" },
    { Key = "surf", Name = "Surfers' Weather" },
    { Key = "sailing", Name = "Sailors' Weather" },
    { Key = "hiking", Name = "Hikers' Weather" },
    { Key = "garden", Name = "Gardeners' Weather" },
    { Key = "stargazing", Name = "Stargazers' Weather" },
}
-- Native features the bundle declares (tools/ios/bundle.py): "use my
-- location" adds the when-in-use usage string, nothing more.
Capabilities = { "location" }
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "accent", Value = "#1BA1E2" },
}
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
