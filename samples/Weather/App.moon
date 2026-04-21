ui = require "orca.UIKit"

class App extends require "orca.core2.application"
	layout: require "Weather/views/layout"

	@include_helpers {
		app_title: => "WeatherTabs"
	}

	"/": =>
		ui.StackView ".p-6.gap-4", ->
			ui.TextBlock Text: "Route Content", class: "text-2xl"
			ui.TextBlock Text: "This content is injected with @content_for 'inner'.", class: "text-base"

	"/about": =>
		ui.TextBlock Text: "About page from router", class: "text-xl"

	Awake: =>
		@navigate "/"
