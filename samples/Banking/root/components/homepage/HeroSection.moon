ui = require "orca.ui"

import Users, Transactions from require "model"

class HeroSection extends ui.StackView
	apply: => "flex-col items-start px-2"
	body: =>
		sum = Transactions\findTotal Users\auth!
		-- h0 ".text-foreground", "Balance"
		p ".text-muted-foreground", "Jan 2022"
		h2 ".text-foreground.font-bold", Transactions\formatAmount amount: sum
		-- h3 ".text-foreground", "Total Balance"
		-- h0 ".text-foreground", Transactions\formatAmount amount: sum
