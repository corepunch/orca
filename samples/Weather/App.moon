renderer = require "orca.renderer"

class App extends require "orca.core.application"
	layout: require "Weather/views/layout"
	views_prefix: "Weather/views/screens"

	@include_helpers {
		app_title: => "WeatherTabs"
		current_route: => @current_route or "/"
	}

	dispatch: (req) =>
		route = if type(req) == "table" then req.path or req.url or req.route else req
		@current_route = route or @current_route or "/"
		App.__parent.dispatch self, req

	navigate: (route) =>
		ctrl = @dispatch route
		return unless ctrl and ctrl.view
		@activate_controller ctrl

	[Home: "/"]: => render: true
	[Forecast: "/forecast"]: => render: true
	[Alerts: "/alerts"]: => render: true
	[Saved: "/saved"]: => render: true
	[About: "/about"]: => render: true
