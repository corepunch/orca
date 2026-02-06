orca = require "orca"
behaviour = require "orca.behaviour"

return {
	Application: require "routing.Application"
	Page: require "routing.Page"

	navigate: (url, store) ->	orca.router\navigate url, store if orca.router
	push: (url) => orca.router\navigate url, true if orca.router
	go_back: -> orca.router\go_back!
	has_history: -> #orca.router.history > 1
	get_location: -> orca.router.history[1]
}