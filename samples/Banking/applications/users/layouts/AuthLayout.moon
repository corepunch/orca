ui = require "orca.ui"
constants = require "assets.constants"

import Page from require "routing"

class AuthLayout extends Page
	body: =>
		grid Rows: "48px auto", ->
			h6 ".bg-dark-2.text-secondary-500.align-middle-center.p-2.rounded", "Auth"
			@content!
