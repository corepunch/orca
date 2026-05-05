Application = require "orca.core.application"

-- Navigate to a route, attaching nav_data for the destination screen.
-- nav_data is always set (cleared to nil when not provided) so stale data
-- from a previous navigation never leaks into the next screen.
navigate = (route, data) ->
	app = Application.current false
	if app
		app.nav_data = data
		app\navigate route

return { :navigate }
