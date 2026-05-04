import StackView, TextBlock, Input, Button from require "orca.UIKit"
Application = require "orca.core.application"

import Users, Messages from require "model"

navigate = (route) ->
	app = Application.current false
	app\navigate route if app

class NewTweet extends require "orca.core.widget"
	title: "New Tweet"

	content: =>
		body_input = nil

		StackView class: "bg-background flex-col p-4 gap-3 h-full", =>
			TextBlock class: "text-2xl font-bold text-foreground", "New Tweet"

			body_input = Input
				class: "bg-surface px-4 py-3 rounded text-foreground"
				PlaceholderText: "What's on your mind?"
				Multiline: true
				Height: 120
				Name: "body"

			StackView class: "flex-row gap-2", =>
				Button {
					class: "bg-accent text-accent-foreground px-4 py-2 rounded font-bold"
					Click: ->
						ok = pcall Messages.create, Messages, { chat: nil, body: body_input.Text }
						if ok then navigate "/tweets"
				}, "Post"

				Button {
					class: "bg-surface text-foreground px-4 py-2 rounded"
					Click: -> navigate "/tweets"
				}, "Cancel"
