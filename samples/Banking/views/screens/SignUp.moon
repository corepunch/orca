import StackView, TextBlock, Input, Button from require "orca.UIKit"
Application = require "orca.core.application"

import Account, Users from require "model"

navigate = (route) ->
	app = Application.current false
	app\navigate route if app

class SignUp extends require "orca.core.widget"
	title: "Sign Up"

	content: =>
		@content_for "no_chrome", true

		name_input     = nil
		user_id_input  = nil
		email_input    = nil
		password_input = nil

		StackView class: "bg-background flex-col p-8 gap-4 h-full justify-center", =>
			TextBlock class: "text-3xl font-bold text-foreground", "Create an account"
			TextBlock class: "text-sm text-foreground-muted", "Enter your details below"

			name_input = Input
				class: "bg-surface px-4 py-3 rounded text-foreground"
				PlaceholderText: "Full name"
				Name: "name"

			user_id_input = Input
				class: "bg-surface px-4 py-3 rounded text-foreground"
				PlaceholderText: "Username"
				Name: "userId"

			email_input = Input
				class: "bg-surface px-4 py-3 rounded text-foreground"
				PlaceholderText: "Email"
				Name: "email"

			password_input = Input
				class: "bg-surface px-4 py-3 rounded text-foreground"
				PlaceholderText: "Password"
				Name: "password"

			Button {
				class: "bg-accent text-accent-foreground px-4 py-3 rounded font-bold"
				Click: ->
					params = {
						name:     name_input.Text
						userId:   user_id_input.Text
						email:    email_input.Text
						password: password_input.Text
					}
					ok = pcall Account.signup, Account, params
					if ok
						pcall Account.signin, Account, params
						pcall Users.create, Users, params.userId, { name: params.name }
						navigate "/"
			}, "Sign Up"

			Button {
				class: "text-accent py-2"
				Click: -> navigate "/sign-in"
			}, "Already have an account? Sign in"
