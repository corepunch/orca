ui = require "orca.ui"
routing = require "routing"

import Account from require "model"
import ErrorMessage from require "assets.components"

fields = {
	{ label: "Email", attribute: "email" }
	{ label: "Password", attribute: "password" }
}

class SignIn extends ui.Form
	apply: => "form"
	onSubmit: => routing.navigate "/" if Account\signin @populateInputs!
	body: =>
		handleSignin = (params) -> routing.navigate "/" if Account\signin params
		p ".card-title", "Log in with existing account"
		p ".card-description", "To use mobile banking now"
		for item in *fields 
			ui.Label ".label", For: item.attribute, item.label
			ui.Input "##{item.attribute}.mb-4.input"
				Name: item.attribute
				PlaceholderText: item.label
		ui.Button ".btn.btn-default.text-xl.w-full", Type: "Submit", "Sign in"
		ui.Button ".btn-link.mt-4", onClick: (=> routing.navigate "/sign-up"), "No account? Create one now"
		stack ".gap-2", ->
			links = {
				{ label: "Default signin", func: -> handleSignin email: "igor.chernakov@gmail.com", password: "qwerty1234" },
				{ label: "Test1", func: -> handleSignin email: "test1@gmail.com", password: "qwer1234" },
				{ label: "Test2", func: -> handleSignin email: "test2@gmail.com", password: "qwer1234" },
			}
			for link in *links
				ui.Button ".btn.btn-link.text-sm.mt-1", onClick: link.func, link.label
		ErrorMessage error: @error if @error

