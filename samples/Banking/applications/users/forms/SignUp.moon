ui = require "orca.ui"
routing = require "routing"

import Account, Users from require "model"
import ErrorMessage from require "assets.components"

fields = {
	{ label: "Name", attribute: "name" }
	{ label: "Username", attribute: "userId" }
	{ label: "Email", attribute: "email" }
	{ label: "Password", attribute: "password" }
}

class SignUp extends ui.Form
	apply: => "form"

	body: =>
		p ".card-title", "Create a new account"
		p ".card-description", "To use mobile banking enter you details"
		for item in *fields 
			ui.Label ".label", for: item.attribute, item.label
			ui.Input "##{item.attribute}.mb-4.input"
				Name: item.attribute
				PlaceholderText: item.label
		ui.Button ".btn.btn-default.text-xl.w-full", Type: "Submit", "Sign up"
		ErrorMessage error: @error if @error

	onSubmit: =>
		parms = @populateInputs!
		Account\signup params
		Account\signin params
		Users\create params.userId, name: params.name
		routing.navigate "/"

