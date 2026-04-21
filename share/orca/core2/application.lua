local Object = require "orca.core2.object"
local Router = require "orca.core2.router"
local UIKit = require "orca.UIKit"

local Application = Object:extend {
	layout = {
		content = function(element)
			local screen = UIKit.Screen()
			screen:addChild(element)
			return screen
		end
	}
}

function Application:__init()
	Object.__init(self)
	self.router = Router(self)
end

function Application:match(name, url, func)
	return self.router:add(name, url, func)
end

function Application:dispatch(req)
	return {
		view = self.layout.content(self.router:dispatch(req))
	}
end

-- function Application:html(func)
-- 	return {
-- 		view = UIKit.Screen(func)
-- 	}
-- end

return Application