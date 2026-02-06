orca = require "orca"
ui = require "orca.ui"

import Users from require "model"

class ContactDetails extends ui.Node2D
	new: (@params) => super!
	body: =>
		user = Users\find @params.user
		h6 ".text-neutral-50.align-middle-center", Users\getFullName user
