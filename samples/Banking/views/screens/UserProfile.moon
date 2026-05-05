import StackView, TextBlock, ImageView from require "orca.UIKit"
Application = require "orca.core.application"

import Users from require "model"
import navigate from require "Banking/views/helpers"

class UserProfile extends require "orca.core.widget"
	title: "Contact"

	content: =>
		app  = Application.current false
		data = app and app.nav_data
		user = data and data.user and Users\find data.user

		StackView class: "bg-background flex-col h-full p-4 gap-4", =>
			-- Back button
			StackView {
				class: "flex-row items-center gap-2"
				LeftButtonUp: -> navigate "/send-money"
			}, =>
				ImageView
					class: "text-foreground-muted"
					Source: "assets/icons/back.svg?width=24&type=mask"
				TextBlock class: "text-foreground-muted", "Back"

			if user
				StackView class: "flex-col items-center gap-3 py-6", =>
					TextBlock class: "text-2xl font-bold text-foreground",
						Users\getFullName user
					TextBlock class: "text-sm text-foreground-muted",
						"@" .. (user["$id"] or "")
			else
				TextBlock class: "text-foreground-muted align-middle-center", "User not found"
