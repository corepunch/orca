local controls = import "Scenes/Controls"

local function CommandButton(args)
	local command = require "AppKit.command"
	local pushButton = controls[args.type or "PushButton"](args)
	pushButton.onClick = function(_, event)
		if event.option and args.alt then
			command.perform(args.alt)
		else
			command.perform(args.command)
		end
	end
	return pushButton
end

return CommandButton