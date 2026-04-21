import StackView, TextBlock from require "orca.UIKit"

class App extends require "orca.core2.application"
	layout: require "Weather/views/layout"

	@include_helpers {
		app_title: => "WeatherTabs"
	}

	"/": =>
		StackView class: "p-6 gap-4", ->
			TextBlock class: "text-2xl", "Route Content"
			TextBlock class: "text-base", "This content is injected with @content_for 'inner'."

	"/about": =>
		TextBlock class: "text-xl", "About page from router"
