orca = require "orca"
ui = require "orca.ui"
routing = require "routing"

import Users, Transactions from require "model"

class TransactionsView extends ui.StackView
	apply: => "flex-col w-full gap-2"
	body: =>
		me = Users\auth!
		for item in *Transactions\findAll Users\auth!, @limit
			handleClick = -> routing.navigate "/transaction/#{item["$id"]}", true
			stack class: "flex-col bg-background hover:bg-muted w-full p-2 gap-1 rounded", onLeftMouseUp: handleClick, ->
				user = item.beneficiary
				-- user = item.sender if me['$id'] == user['$id'] 
				div ".h-5", ->
					p ".text-xl.font-bold", Users\getFullName user 
					p ".text-xl.align-right", Transactions\formatAmount item
				div ".h-5", ->
					p ".text-sm.text-muted-foreground", item.iban
					p ".align-right.text-sm.text-muted-foreground", item["$createdAt"]\sub 1, 10
