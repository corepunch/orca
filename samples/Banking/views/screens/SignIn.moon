import StackView, TextBlock, Input, Button from require "orca.UIKit"

import Account from require "model"
import navigate from require "Banking/views/helpers"

class SignIn extends require "orca.core.widget"
	title: "Sign In"

	content: =>
		@content_for "no_chrome", true

		email_input    = nil
		password_input = nil

		StackView class: "bg-background flex-col p-8 gap-4 h-full justify-center", =>
			TextBlock class: "text-3xl font-bold text-foreground", "Welcome back"
			TextBlock class: "text-sm text-foreground-muted", "Sign in to your account"

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
					params = { email: email_input.Text, password: password_input.Text }
					ok = pcall Account.signin, Account, params
					if ok then navigate "/"
			}, "Sign In"

			Button {
				class: "text-accent py-2"
				Click: -> navigate "/sign-up"
			}, "No account? Create one"
