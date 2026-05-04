import StackView, TextBlock, ImageView from require "orca.UIKit"
Application = require "orca.core.application"

import Users, Transactions from require "model"

navigate = (route) ->
	app = Application.current false
	app\navigate route if app

class Transaction extends require "orca.core.widget"
	title: "Transaction"

	content: =>
		app  = Application.current false
		data = app and app.nav_data
		txn  = data and data.transaction and Transactions\find data.transaction

		StackView class: "bg-background flex-col h-full p-4 gap-4", =>
			-- Back button
			StackView {
				class: "flex-row items-center gap-2"
				LeftButtonUp: -> navigate "/"
			}, =>
				ImageView
					class: "text-foreground-muted"
					Source: "assets/icons/back.svg?width=24&type=mask"
				TextBlock class: "text-foreground-muted", "Back"

			if txn
				StackView class: "flex-col items-center gap-2 py-6", =>
					TextBlock class: "text-sm text-foreground-muted", "You sent"
					TextBlock class: "text-4xl font-bold text-primary",
						Transactions\formatAmount txn
					TextBlock class: "text-base text-foreground", "to " .. Users\getFullName txn.beneficiary
					TextBlock class: "text-sm text-foreground-muted",
						(txn["$createdAt"] or "")\sub 1, 10
			else
				TextBlock class: "text-foreground-muted align-middle-center", "Transaction not found"
