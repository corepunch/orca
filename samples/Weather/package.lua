Name="Weather"
StartupViewController="Weather/App"
StartupRoute="/"
WindowWidth=400
WindowHeight=800
ProjectReferences = {
  { Name="views", Path="views" },
  { Name="assets", Path="assets" }
}
SystemMessages = {
  { Message="KeyDown", Key="q", Command="return" },
  { Message="WindowClosed", Command="return" },
  { Message="RequestReload", Command="window:refresh()" }
}
