renderer = require "orca.renderer"
Application = require "orca.core.application"

import Account from require "model"

-- Routes that bypass the authentication guard.
AUTH_ROUTES = { ["/sign-in"]: true, ["/sign-up"]: true }

class App extends Application
	layout: require "Banking/views/layout"
	views_prefix: "Banking/views/screens"

	@include_helpers {
		app_title: => "Banking"
		current_route: => @current_route or "/"
	}

	dispatch: (req) =>
		route = if type(req) == "table" then req.path or req.url or req.route else req

		-- Sign-out: clear session then redirect to sign-in.
		if route == "/sign-out"
			pcall Account.signout, Account
			route = "/sign-in"
			req   = "/sign-in"

		-- Protect every route that is not in AUTH_ROUTES.
		unless AUTH_ROUTES[route]
			ok = pcall Account.auth, Account
			unless ok
				route = "/sign-in"
				req   = "/sign-in"

		@current_route = route or "/"
		App.__parent.dispatch self, req

	navigate: (route) =>
		@activate_route route

	[Home:        "/"              ]: => render: true
	[SendMoney:   "/send-money"    ]: => render: true
	[Settings:    "/settings"      ]: => render: true
	[Tweets:      "/tweets"        ]: => render: true
	[NewTweet:    "/new-tweet"     ]: => render: true
	[Search:      "/search"        ]: => render: true
	[UserProfile: "/user"          ]: => render: true
	[Transaction: "/transaction"   ]: => render: true
	[Chat:        "/chat"          ]: => render: true
	[SignIn:      "/sign-in"       ]: => render: true
	[SignUp:      "/sign-up"       ]: => render: true
