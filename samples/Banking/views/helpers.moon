Application = require "orca.core.application"

-- Navigate to a route, optionally attaching nav_data for the destination screen.
navigate = (route, data) ->
	app = Application.current false
	if app
		if data != nil then app.nav_data = data
		app\navigate route

return { :navigate }
