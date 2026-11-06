local Native = require "Editor.Scripts.Native"
local Panel = {}
function Panel.wrap(body, bodyView)
	local view = Native.load('Editor/Views/Panel')
	local header = {view=view:findChild('Header',true),panel=body}
	local tools = view:findChild('Tools',true)
	function header:invalidate()
		view:findChild('Title',true).Text=body.title or ''
		view:findChild('Subtitle',true).Text=body.subtitle or ''
		self.entries=body.tools and body:tools() or {}
		local rows={}
		for _,tool in ipairs(self.entries) do rows[#rows+1]={Label=tool.label} end
		tools.ItemsSource=Native.records(rows)
		self.view.StateClasses=body.active and 'panel-active' or ''
	end
	function header:invoke(record)
		local tool=record and self.entries[record.Index]
		if tool and tool.action then tool.action() end
		return true
	end
	function header:origin() return Native.origin(self.view) end
	function header:focus() if body.focus then body:focus() end end
	function header:checkResize() end
	Native.register(view,header)
	body.header,body.container=header,view
	view:addChild(bodyView or body.view)
	header:invalidate()
	return view
end
return Panel
