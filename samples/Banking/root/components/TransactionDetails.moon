orca = require "orca"
ui = require "orca.ui"

import Users, Transactions from require "model"

class TransactionDetails extends ui.Node2D
	new: (@params) => super!
	body: =>
		data = Transactions\find @params.transaction
		stack ".flex-col.align-middle-center.items-center", ->
			p ".text-lg.text-light-4", "You sent"
			h3 ".text-secondary-500", Transactions\formatAmount data 
			h6 "to "..Users\getFullName data.beneficiary
