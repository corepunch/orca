local Native = require "Editor.Scripts.Native"
return {
	Invoke = function(self, args, sender)
		local owner = Native.owner(sender)
		if owner and owner.invoke then return owner:invoke(Native.context(sender), args, sender) end
	end,
	Hover = function(self, args, sender)
		local owner = Native.owner(sender)
		if owner and owner.hover then return owner:hover(Native.context(sender), args, sender) end
	end,
}
