Name = "Editor"
StartupViewController = "Editor/App"
StartupRoute = "/"
WindowWidth = 1440
WindowHeight = 900
EnginePlugins = {
	{ Name = "orca.UIKit" },
}
-- Dark palette for the engine's default stylesheet (share/orca/UIKit/default.css).
-- Buttons, inputs, checkboxes and tabs need no editor rules beyond these keys.
ThemeLibrary = {
	{ Key = "ui-accent", Value = "#4a8cf7" },
	{ Key = "ui-accent-foreground", Value = "#ffffff" },
	{ Key = "ui-control", Value = "#383b42" },
	{ Key = "ui-control-hover", Value = "#434750" },
	{ Key = "ui-control-active", Value = "#2e3137" },
	{ Key = "ui-control-foreground", Value = "#dfe2e7" },
	{ Key = "ui-control-muted", Value = "#959ba6" },
	{ Key = "ui-input", Value = "#1d1f23" },
	{ Key = "ui-input-border", Value = "#3a3d44" },
	{ Key = "ui-input-border-hover", Value = "#4c5059" },
	{ Key = "ui-separator", Value = "#33363c" },
	{ Key = "ui-radius", Value = "4" },
	{ Key = "ui-radius-small", Value = "3" },
}
