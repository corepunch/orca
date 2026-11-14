Name = "Chat"
StartupViewController = "Chat/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
ProjectReferences = {
    { Name = "api", Path = "api" },
    { Name = "assets", Path = "assets" },
    { Name = "Octopus", Path = "../octopus" },
}
EnginePlugins = { { Name = "orca.UIKit" } }
SystemMessages = {
    { Message = "WindowClosed", Command = "return" },
}
-- Appwrite returns sign-in links (OAuth and emailed links) to this scheme. Keep it
-- in step with config.lua's projectId; packaging adds it to the bundle's Info.plist.
URLSchemes = { "appwrite-callback-69f1c06800389dc6a1a0" }
-- End-to-end encryption (Monocypher) is not exempt from export compliance.
NonExemptEncryption = true
