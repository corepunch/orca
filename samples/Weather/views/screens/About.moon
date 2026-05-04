import StackView, TextBlock from require "orca.UIKit"

class AboutScreen extends require "orca.core.widget"
	title: "About"

	content: =>
		StackView class: "bg-background p-5 gap-3", =>
			TextBlock class: "text-2xl font-bold text-foreground", "About"
			StackView class: "bg-surface rounded-3 px-4 py-3 gap-2", =>
				TextBlock class: "text-base font-bold text-accent", "Weather Demo"
				TextBlock class: "text-sm text-foreground-muted",
					"Route-driven app with bottom navigation, live weather data, and a 7-day forecast."
			StackView class: "bg-surface rounded-3 px-4 py-3 gap-2", =>
				TextBlock class: "text-base font-bold text-accent", "Data Source"
				TextBlock class: "text-sm text-foreground-muted",
					"Powered by Open-Meteo — free weather API, no key required."
