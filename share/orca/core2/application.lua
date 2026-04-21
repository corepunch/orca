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

function Application:new_render_context(req)
	return {
		req = req,
		content = {}
	}
end

function Application:match(name, url, func)
	return self.router:add(name, url, func)
end

function Application:dispatch(req)
	local ctx = self:new_render_context(req)
	local body = self.router:dispatch(req)
	ctx.content.inner = body

	local layout_def = self.layout
	local view = body

	if type(layout_def) == "table" and layout_def.__class then
		local layout = layout_def()
		layout:set_render_context(ctx)
		layout:include_helper(self)
		if type(layout.content) == "function" then
			view = layout:content()
		end
	elseif type(layout_def) == "table" and type(layout_def.content) == "function" then
		view = layout_def.content(ctx.content.inner, ctx, self)
	elseif type(layout_def) == "function" then
		view = layout_def(ctx.content.inner, ctx, self)
	elseif layout_def == nil and body ~= nil then
		local screen = UIKit.Screen()
		screen:addChild(body)
		view = screen
	end

	return {
		view = view,
		context = ctx
	}
end

-- function Application:html(func)
-- 	return {
-- 		view = UIKit.Screen(func)
-- 	}
-- end

return Application