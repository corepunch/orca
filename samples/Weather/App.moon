ui = require "orca.UIKit"

class App extends require "orca.core2.application"
	layout: require "Weather/views/layout"

	@include_helpers {
		app_title: => "WeatherTabs"
	}

	"/": =>
		ui.StackView ".p-6.gap-4", ->
			ui.TextBlock class: "text-2xl", "Route Content"
			ui.TextBlock class: "text-base", "This content is injected with @content_for 'inner'."

	"/about": =>
		ui.TextBlock class: "text-xl", "About page from router"

	Awake: =>
		@navigate "/"
