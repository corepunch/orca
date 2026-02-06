ui = require "orca.ui"
routing = require "routing"

import Users, Messages from require "model"

class NewTweet extends ui.Form
	apply: => "gap-2 px-2"
	title: "New Tweet"
	body: =>
		input ".input"
			Name: "body"
			Height: 100
			Multiline: true
			PlaceholderText: "What's on your mind?"
		stack ".gap-2", ->
			button ".btn.btn-default.w-full", Type: "Submit", "Submit"
			button ".btn.btn-secondary.w-full", onClick: @onBack, "Cancel"

	onBack: => routing.navigate "/tweets"

	onSubmit: => 
		tbl = @populateInputs!
		Messages\create chat: nil, body: tbl.body
		routing.navigate "/tweets"

