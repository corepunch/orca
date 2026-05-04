import StackView, TextBlock from require "orca.UIKit"
Application = require "orca.core.application"

navigate = (route) ->
	app = Application.current false
	app\navigate route if app

class Settings extends require "orca.core.widget"
	title: "Settings"

	content: =>
		StackView class: "bg-background flex-col p-4 gap-4 h-full", =>
			TextBlock class: "text-2xl font-bold text-foreground", "Settings"

			StackView class: "bg-surface rounded-3 px-4 py-3 gap-2", =>
				TextBlock class: "text-base font-bold text-foreground", "Account"
				StackView {
					class: "bg-destructive rounded px-4 py-3"
					LeftButtonUp: -> navigate "/sign-out"
				}, =>
					TextBlock class: "text-destructive-foreground text-base font-bold", "Sign Out"
