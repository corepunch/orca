Name="Weather"
StartupViewController="Weather/App"
StartupRoute="/"
WindowWidth=900
WindowHeight=600
ProjectReferences = {
  { Name="views", Path="views" }
}
SystemMessages = {
  { Message="KeyDown", Key="q", Command="return" },
  { Message="WindowClosed", Command="return" },
  { Message="RequestReload", Command="window:refresh()" }
}
