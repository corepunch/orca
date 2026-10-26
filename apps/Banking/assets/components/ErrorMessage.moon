ui = require "orca.ui"

class ErrorMessage extends ui.TextBlock
	apply: => "mt-16 text-red text-sm"

	new: (...) =>
		super...
		if type(@error) == 'string'
			@addChild @error
		else
			@addChild @error\json!.message
