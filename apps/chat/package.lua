Name = "Chat"
StartupViewController = "Chat/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "api", Path = "api" },
    { Name = "assets", Path = "assets" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
