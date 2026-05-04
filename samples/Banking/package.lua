Name = "Banking"
StartupViewController = "Banking/App"
StartupRoute = "/"
WindowWidth = 375
WindowHeight = 812
ProjectReferences = {
	{ Name = "views",    Path = "views"       },
	{ Name = "assets",   Path = "assets"      },
	{ Name = "model",    Path = "model"       },
	{ Name = "appwrite", Path = "lib/appwrite" },
	{ Name = "config",   Path = "config"      },
	{ Name = "tailwind", Path = "tailwind"    },
}
SystemMessages = {
	{ Message = "KeyDown",      Key = "q", Command = "return"             },
	{ Message = "WindowClosed",            Command = "return"             },
	{ Message = "RequestReload",           Command = "window:refresh()"   },
}
