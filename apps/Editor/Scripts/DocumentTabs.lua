-- Tab strip of open documents. Workspace owns the documents and their state;
-- this view only presents them and turns clicks into Workspace commands.
local Class = require "Editor.Scripts.Class"
local Native = require "Editor.Scripts.Native"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
local DocumentTabs = Class:extend {}

function DocumentTabs:init(confirmClose)
	self.confirmClose = confirmClose
	self.view = Native.register(Native.load('Editor/Views/DocumentTabs'), self)
	self.tabs = self.view:findChild('Tabs', true)
	self.hint = self.view:findChild('Hint', true)
	Native.keys(self)
	Workspace.on('tabs', function() self:invalidate() end)
	self:invalidate()
end

function DocumentTabs:invalidate()
	local rows = {}
	for _, doc in ipairs(Workspace.documents) do
		local active = doc.path == Workspace.document
		local dirty = Workspace.isDirty(doc.path)
		rows[#rows + 1] = { Label = Workspace.leaf(doc.path) .. (dirty and ' *' or ''), Detail = doc.path,
			State = active and 'tab-active' or '' }
	end
	self.tabs.ItemsSource = Native.records(rows)
	self.hint.Visible = #rows == 0
end

local function document(record)
	local doc = record and Workspace.documents[record.Index]
	return doc and doc.path
end

function DocumentTabs:invoke(record)
	local path = document(record)
	if path then Workspace.activate(path) end
	return true
end

-- Closes a tab; unsaved documents ask first (see App:closeDocument).
function DocumentTabs:close(record)
	local path = document(record)
	if path then self.confirmClose(path) end
	return true
end

function DocumentTabs:context(record, args)
	local path = document(record)
	if not path then return false end
	local others = #Workspace.documents > 1
	Menu.show({
		{ label = 'Close', shortcut = 'Ctrl+W', action = function() self.confirmClose(path) end },
		{ label = 'Close Others', disabled = not others, action = function()
			for _, doc in ipairs { table.unpack(Workspace.documents) } do
				if doc.path ~= path then self.confirmClose(doc.path) end
			end
		end },
		{ separator = true },
		{ label = 'Save', shortcut = 'Ctrl+S', disabled = not Workspace.isDirty(path), action = function() Workspace.save(path) end },
		{ label = 'Copy Path', action = function()
			Workspace.clipboard = path
			Workspace.log('info', "Copied %s", path)
		end },
	}, args.x, args.y)
	return true
end

function DocumentTabs:checkResize() end
function DocumentTabs:focus() self.view:setFocus() end
return DocumentTabs
