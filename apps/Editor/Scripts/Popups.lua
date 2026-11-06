-- Overlay layer for menus, completion lists and dialogs. While anything is
-- open the overlay covers the window, so a click outside closes the stack.
local Popups = { stack = {} }

function Popups.attach(overlay, onOutsideClick)
	Popups.overlay = overlay
	Popups.onOutsideClick = onOutsideClick
end

function Popups.size()
	local o = Popups.overlay
	return o.ActualWidth or 0, o.ActualHeight or 0
end

-- Show a panel (anything with a .view) at screen coordinates, keeping it
-- inside the window.
function Popups.push(panel, x, y, modal)
	local view = panel.view
	-- A click inside a native control must not bubble into the outside-click
	-- dismiss layer while the control waits for its release event.
	view.LeftButtonDown = function() return true end
	view.RightButtonDown = function() return true end
	local sw, sh = Popups.size()
	local w, h = view.Width or 0, view.Height or 0
	if sw > 0 and x + w > sw then x = math.max(0, sw - w) end
	if sh > 0 and y + h > sh then y = math.max(0, sh - h) end
	view.HorizontalAlignment = "Left"
	view.VerticalAlignment = "Top"
	view.x, view.y = math.floor(x), math.floor(y)
	panel.modal = modal
	Popups.overlay:addChild(view)
	Popups.stack[#Popups.stack + 1] = panel
	Popups.overlay.Visible = true
	if panel.focus then panel:focus() end
	return panel
end

function Popups.top() return Popups.stack[#Popups.stack] end

function Popups.isOpen(panel)
	for _, p in ipairs(Popups.stack) do if p == panel then return true end end
	return false
end

-- Close a panel and everything opened above it.
function Popups.close(panel)
	for i = #Popups.stack, 1, -1 do
		local p = Popups.stack[i]
		table.remove(Popups.stack, i)
		p.view:removeFromParent()
		if p.onClose then p:onClose() end
		if p == panel then break end
	end
	Popups.overlay.Visible = #Popups.stack > 0
	local top = Popups.top()
	if top and top.focus then top:focus()
	elseif Popups.restoreFocus then Popups.restoreFocus() end
end

function Popups.closeAll()
	if #Popups.stack > 0 then Popups.close(Popups.stack[1]) end
end

-- The overlay received a click that no popup handled.
function Popups.outside(args)
	local top = Popups.top()
	if top and top.modal then return true end
	Popups.closeAll()
	if Popups.onOutsideClick then Popups.onOutsideClick(args) end
	return true
end

return Popups
