import StackView, TextBlock from require "orca.UIKit"

import Users, Transactions from require "model"
import navigate from require "Banking/views/helpers"

class Home extends require "orca.core.widget"
	title: "Overview"

	content: =>
		me  = Users\auth!
		sum = Transactions\findTotal me

		StackView class: "bg-background flex-col overflow-y-scroll h-full p-4 gap-4", =>
			-- Balance hero
			StackView class: "bg-surface rounded-3 p-5 gap-1", =>
				TextBlock class: "text-xs text-foreground-muted", "Total Balance"
				TextBlock class: "text-4xl font-bold text-foreground",
					Transactions\formatAmount { amount: sum }

			-- Recent transactions
			StackView class: "flex-col gap-2", =>
				TextBlock class: "text-base font-bold text-foreground", "Recent Transactions"

				for item in *Transactions\findAll me, 5
					txn = item
					StackView {
						class: "bg-surface rounded-3 p-3 flex-row items-center gap-3"
						LeftButtonUp: -> navigate "/transaction", { transaction: txn["$id"] }
					}, =>
						StackView class: "flex-col gap-1", =>
							TextBlock class: "text-base font-bold text-foreground",
								Users\getFullName item.beneficiary
							TextBlock class: "text-xs text-foreground-muted",
								(item["$createdAt"] or "")\sub 1, 10
						TextBlock class: "text-base text-right text-foreground",
							Transactions\formatAmount item
