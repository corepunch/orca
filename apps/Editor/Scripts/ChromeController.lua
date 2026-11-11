local Native = require "Editor.Scripts.Native"
local function forward(method)
	return function(self, args, sender)
		local owner = Native.owner(sender)
		if owner and owner[method] then return owner[method](owner, Native.context(sender), args, sender) end
	end
end
return {
	Invoke = forward('invoke'),
	Hover = forward('hover'),
	Close = forward('close'),
	Context = forward('context'),
}
