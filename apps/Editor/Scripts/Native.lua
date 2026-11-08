-- Small adapters shared by the editor's XML views. Remote records never pass
-- through this module: their XmlDataSource is bound directly to ItemsSource.
local fs = require "orca.filesystem"
require "orca.UIKit"
local Native = { owners = setmetatable({}, {__mode = 'v'}) }

function Native.load(path)
	return assert(fs.loadObjectFromDisk(path), "Could not load " .. path)
end

function Native.register(view, owner)
	Native.owners[view:getUniqueID()] = owner
	return view
end

function Native.owner(view)
	while view do
		local owner = Native.owners[view:getUniqueID()]
		if owner then return owner end
		view = view.parent
	end
end

function Native.context(view)
	while view do
		if view.DataContext then return view.DataContext end
		view = view.parent
	end
end

function Native.origin(view)
	local x, y = 0, 0
	while view do
		x, y = x + (view.ActualX or 0), y + (view.ActualY or 0)
		view = view.parent
	end
	return x, y
end

local KEYS = { [8]='backspace', [9]='tab', [13]='enter', [27]='escape',
	[32]='space', [127]='backspace', [128]='up', [129]='down', [130]='left',
	[131]='right', [148]='delete', [149]='pagedown', [150]='pageup', [151]='home', [152]='end' }
for i = 1, 12 do KEYS[134+i] = 'f' .. i end
function Native.chord(args)
	local code = args.keyCode or 0
	local key = KEYS[code] or (code > 32 and code < 127 and string.char(code):lower())
	if not key then return end
	local m = args.modifiers or 0
	return ((m & ((1<<17)|(1<<19))) ~= 0 and 'ctrl+' or '')
		.. ((m & (1<<18)) ~= 0 and 'alt+' or '')
		.. ((m & (1<<16)) ~= 0 and 'shift+' or '') .. key
end

function Native.keys(panel, view)
	view = view or panel.view
	view.LeftButtonDown = function()
		if Native.onActivate then Native.onActivate(panel) end
		return false
	end
	view.KeyDown = function(_, args)
		local chord = Native.chord(args)
		if chord and panel.onKey and panel:onKey(chord, args) then return true end
		return chord and panel.hotkey and panel.hotkey(chord) or false
	end
end

function Native.escape(value)
	return tostring(value or ''):gsub('&','&amp;'):gsub('<','&lt;'):gsub('>','&gt;')
		:gsub('"','&quot;'):gsub("'",'&apos;'):gsub('\n','&#10;'):gsub('\r','&#13;')
end

-- Local chrome (menus, toolbars and logs) uses the same native data parser.
-- This constructs records, never controls; ItemsControl owns every row view.
function Native.records(rows)
	local schema = [[<Schema><Entity Name="EditorChromeRow">
		<Column Name="Index" Type="int"/><Column Name="Label" Type="string"/>
		<Column Name="Detail" Type="string"/><Column Name="Mark" Type="string"/>
		<Column Name="Tail" Type="string"/><Column Name="Disabled" Type="bool"/>
		<Column Name="Separator" Type="bool"/><Column Name="IsItem" Type="bool"/>
		<Column Name="State" Type="string"/><Column Name="Time" Type="string"/>
		<Column Name="Icon" Type="string"/><Column Name="Monogram" Type="string"/>
	</Entity></Schema>]]
	local xml = {'<Items>'}
	for i, row in ipairs(rows) do
		xml[#xml+1] = '<EditorChromeRow name="Row' .. i .. '" Index="' .. i .. '"'
		for _, key in ipairs {'Label','Detail','Mark','Tail','Disabled','Separator','IsItem','State','Time','Icon','Monogram'} do
			if row[key] ~= nil then xml[#xml+1] = ' ' .. key .. '="' .. Native.escape(row[key]) .. '"' end
		end
		xml[#xml+1] = '/>'
	end
	xml[#xml+1] = '</Items>'
	local records = assert(fs.loadXmlDataStringWithSchema(table.concat(xml), schema),
		'Invalid editor chrome records')
	return records
end

return Native
