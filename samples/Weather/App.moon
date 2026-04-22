import renderer from require "orca.renderer"

class App extends require "orca.core.application"
	layout: require "Weather/views/layout"

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

		@controller = ctrl
		@screen = ctrl.view

		Application = require "orca.core.application"
		Application.screen = @screen
		Application.controller = @controller

		@screen\post "Window.Paint", renderer.getSize!

	"/": =>
		require("Weather/views/screens/Home")!

	"/forecast": =>
		require("Weather/views/screens/Forecast")!

	"/alerts": =>
		require("Weather/views/screens/Alerts")!

	"/saved": =>
		require("Weather/views/screens/Saved")!

	"/about": =>
		require("Weather/views/screens/About")!
