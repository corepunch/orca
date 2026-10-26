Name = "MvvmForms"
StartupViewController = "MvvmForms/App"
StartupRoute = "/"
WindowWidth = 390
WindowHeight = 844
EnginePlugins = { { Name = "orca.UIKit" } }
ThemeLibrary = {
    { Key = "background", Value = "#ffffff" },
    { Key = "foreground", Value = "#17212b" },
    { Key = "muted", Value = "#526170" },
    { Key = "accent", Value = "#136d99" },
}
DataSourceLibrary = {
    Name = "Models",
    require "orca.filesystem.XmlDataSource" {
        Name = "Profile", Source = "MvvmForms/Models/Profile", Schema = "MvvmForms/Models/Profile.schema",
    },
}
