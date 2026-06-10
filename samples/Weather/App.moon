renderer = require "orca.renderer"

class App extends require "orca.core.application"
	layout: require "Weather/views/layout"
	views_prefix: "Weather/views/screens"

	@include_helpers {
		app_title: => "WeatherTabs"
	}

	dispatch: (req) =>
		App.__parent.dispatch self, req

	navigate: (route) =>
		@activate_route route

	[Home: "/"]: => render: true
	[Forecast: "/forecast"]: => render: true
	[Alerts: "/alerts"]: => render: true
	[Saved: "/saved"]: => render: true
	[LocationSearch: "/location-search"]: => render: true
	[Settings: "/settings"]: => render: true
	[About: "/about"]: => render: true
