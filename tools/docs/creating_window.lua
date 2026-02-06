-- Import server
local server = require "orca.server"
-- Create window
local window = server.Window {
	width = 1024,
	height = 768
}
-- Load page
window:loadPage "StartupPage"
-- Start event loop
while true do
	for msg in server.peekMessage() do
		if msg:is "KeyDown" and msg.key == "q" then
			return "closed"
		elseif msg:is "WindowClosed" then
			return "closed"
		elseif msg:is "RequestReload" then
			window:refresh() -- handle hot reloading
		else
			server.dispatchMessage(msg)
		end
	end
end
