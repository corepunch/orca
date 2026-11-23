Name = "Octopus"
StartupViewController = "Octopus/Scripts/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "AppwriteAuth", Path = "../shared/AppwriteAuth" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
