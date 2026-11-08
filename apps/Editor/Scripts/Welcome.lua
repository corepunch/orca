-- Project launch chrome. Native ItemsControl owns the project and folder rows.
local Native = require 'Editor.Scripts.Native'
local Projects = require 'Editor.Scripts.Projects'
local Welcome = {}
Welcome.__index = Welcome

function Welcome.new(options)
	local self = setmetatable({ options = options, mode = 'all', query = '',
		initialPath = Projects.validate(os.getenv('ORCA_EDITOR_PROJECT')) }, Welcome)
	self.view = Native.register(Native.load('Editor/Views/Welcome'), self)
	self.list = self:node('Projects')
	self.search = self:node('ProjectSearch')
	self:refresh()
	return self
end

function Welcome:node(name) return assert(self.view:findChild(name, true), name) end

function Welcome:error(message, browser)
	local node = self:node(browser and 'FolderError' or 'WelcomeError')
	node.Text, node.Visible = message or '', message ~= nil
end

function Welcome:refresh()
	self.catalog = Projects.list()
	self:filter()
end

function Welcome:filter()
	local previous = self:selected()
	local previousPath = previous and previous.path or self.initialPath
	local query, rows = self.query:lower(), {}
	self.filtered = {}
	for _, project in ipairs(self.catalog) do
		local matches = (project.title .. ' ' .. project.path):lower():find(query, 1, true)
		if matches and (self.mode == 'all' or project.kind == 'recent') then
			self.filtered[#self.filtered + 1] = project
			rows[#rows + 1] = { Label = project.title, Detail = Projects.displayPath(project.path),
				Monogram = (project.title:match('%w') or '?'):upper(),
				Tail = project.kind == 'sample' and 'Example' or project.kind == 'configured' and 'Configured' or 'Recent' }
		end
	end
	self.list.ItemsSource = Native.records(rows)
	self.list:send('ItemsControl.Refresh')
	local selected = #rows > 0 and 1 or nil
	for i, project in ipairs(self.filtered) do if project.path == previousPath then selected = i break end end
	self.list.SelectedValue = selected and 'Row' .. selected or ''
	self:selection()
	self:node('NoProjects').Visible = #rows == 0
	self:node('EmptyTitle').Text = self.query ~= '' and 'No matching projects' or 'A fresh start'
	self:node('EmptyDetail').Text = self.query ~= '' and 'Try another name or open a project folder.'
		or 'Projects you open will appear here. Choose All projects to explore the examples, or open your own.'
	self:node('AllProjects').StateClasses = self.mode == 'all' and 'welcome-tab-selected' or ''
	self:node('RecentProjects').StateClasses = self.mode == 'recent' and 'welcome-tab-selected' or ''
end

function Welcome:selected() return self.filtered and self.filtered[tonumber((self.list.SelectedValue or ''):match('^Row(%d+)$'))] end

function Welcome:selection()
	local project = self:selected()
	self:node('SelectedTitle').Text = project and project.title or 'Choose a project'
	self:node('SelectedPath').Text = project and Projects.displayPath(project.path) or 'Select a project above to continue.'
	local button = self:node('OpenSelected')
	button.IgnoreHitTest = not project
	button.StateClasses = not project and 'welcome-primary-unavailable' or ''
end

function Welcome:launch(path, browser)
	local valid, err = Projects.validate(path)
	if not valid then self:error(err, browser) return false end
	self:error(nil, browser)
	local ok, openError = self.options.open(valid)
	if ok == false then self:error(openError or 'This project could not be opened.', browser) return false end
	-- A history failure never prevents opening a valid project.
	local remembered, historyError = Projects.remember(valid)
	if not remembered and self.options.warning then self.options.warning(historyError) end
	return true
end

function Welcome:openSelected()
	local project = self:selected()
	if project then return self:launch(project.path) end
	self:error('Select a project, or choose Open Folder… to browse for one.')
	return false
end

-- The folder browser serves both opening and creating: creating picks the
-- parent folder and adds a name field.
local BROWSER = {
	open = { 'OPEN A PROJECT', 'Find your next project', 'Choose a folder containing an ORCA package.lua file.' },
	create = { 'NEW PROJECT', 'Create a project', 'Choose where to create it, then give it a name.' },
}

function Welcome:browse(creating)
	self.creating = creating == true
	local text = BROWSER[self.creating and 'create' or 'open']
	self:node('BrowserEyebrow').Text, self:node('BrowserTitle').Text, self:node('BrowserDetail').Text = text[1], text[2], text[3]
	self:node('ProjectName').Visible = self.creating
	self:node('ProjectChooser').Visible = false
	self:node('FolderBrowser').Visible = true
	local project = self:selected()
	local start = project and project.path
	if self.creating and start then start = start .. '/..' end
	self:folder(self.folderPath or start or (DATADIR or 'apps/Editor') .. '/..')
	if self.creating then self:node('ProjectName').Text = '' self:node('ProjectName'):setFocus()
	else self:node('FolderPath'):setFocus() end
end

function Welcome:folder(path)
	-- package.lua is accepted both in the path field and in the launch action.
	path = path:gsub('/package%.lua/?$', '')
	local folders, normalized = Projects.directories(path)
	if not folders then self:error(normalized, true) return false end
	self.folders, self.folderPath = folders, normalized
	self:node('FolderPath').Text = normalized
	local rows = {}
	for _, folder in ipairs(folders) do
		rows[#rows + 1] = { Label = folder.title, Tail = folder.kind == 'project' and 'ORCA PROJECT' or '' }
	end
	self:node('Folders').ItemsSource = Native.records(rows)
	self:node('Folders'):send('ItemsControl.Refresh')
	self:node('NoFolders').Visible = #rows == 0
	local project = Projects.validate(normalized)
	self:node('OpenFolder').Text = self.creating and 'Create project'
		or project and 'Open ' .. (normalized:match('([^/]+)$') or normalized) or 'Open this project'
	self:error(nil, true)
	return true
end

function Welcome:create()
	local path, err = Projects.create(self.folderPath or self:node('FolderPath').Text or '', self:node('ProjectName').Text)
	if not path then self:error(err, true) return false end
	return self:launch(path, true)
end

function Welcome:cancelBrowse()
	self:node('FolderBrowser').Visible = false
	self:node('ProjectChooser').Visible = true
	self.list:setFocus()
end

function Welcome:show(canReturn)
	self:cancelBrowse()
	self:node('ReturnToEditor').Visible = canReturn == true
	self:error(nil)
	self:refresh()
	self.view.Visible = true
end

function Welcome:update()
	local query = self.search.Text or ''
	if query ~= self.query then self.query = query self:filter() end
	local width = self.view.ActualWidth or 0
	local compact = width > 0 and width < 880
	if compact ~= self.compact then
		self.compact = compact
		self:node('Artwork').Visible = not compact
		self.view.Columns = compact and '1fr' or '416px 1fr'
	end
end

local function action(method)
	return function(_, args, sender)
		local owner = Native.owner(sender)
		if owner then return owner[method](owner, args, sender) end
	end
end
Welcome.Browse = function(_, _, sender)
	local self = Native.owner(sender)
	if self then return self:browse(false) end
end
Welcome.New = function(_, _, sender)
	local self = Native.owner(sender)
	if self then return self:browse(true) end
end
Welcome.CancelBrowse = action('cancelBrowse')
Welcome.Select = action('selection')
Welcome.Open = action('openSelected')
Welcome.All = function(_, _, sender)
	local self = Native.owner(sender)
	self.mode = 'all'
	self:filter()
end
Welcome.Recent = function(_, _, sender)
	local self = Native.owner(sender)
	self.mode = 'recent'
	self:filter()
end
Welcome.OpenRow = function(_, _, sender)
	local self, record = Native.owner(sender), Native.context(sender)
	local project = record and self.filtered[record.Index]
	if project then return self:launch(project.path) end
end
Welcome.GoFolder = function(_, _, sender)
	local self = Native.owner(sender)
	return self:folder(self:node('FolderPath').Text or '')
end
Welcome.ParentFolder = function(_, _, sender)
	local self = Native.owner(sender)
	return self:folder((self.folderPath or (DATADIR or 'apps/Editor')) .. '/..')
end
Welcome.EnterFolder = function(_, _, sender)
	local self, record = Native.owner(sender), Native.context(sender)
	local folder = record and self.folders[record.Index]
	if folder then return self:folder(folder.path) end
end
Welcome.OpenFolder = function(_, _, sender)
	local self = Native.owner(sender)
	if self.creating then return self:create() end
	return self:launch(self:node('FolderPath').Text or '', true)
end
Welcome.Return = function(_, _, sender)
	local self = Native.owner(sender)
	if self.options.cancel then self.options.cancel() end
end
Welcome.Key = function(_, args, sender)
	local self = Native.owner(sender)
	if not self then return false end
	local chord = Native.chord(args)
	if chord == 'ctrl+o' then self:browse(false) return true end
	if chord == 'ctrl+n' then self:browse(true) return true end
	if chord == 'escape' then
		if self:node('FolderBrowser').Visible then self:cancelBrowse()
		elseif self.options.cancel then self.options.cancel() end
		return true
	end
	if self:node('FolderBrowser').Visible then return false end
	if chord == 'enter' then self:openSelected() return true end
	if (chord == 'up' or chord == 'down') and sender ~= self.search then
		local index = tonumber((self.list.SelectedValue or ''):match('^Row(%d+)$')) or 0
		index = math.max(1, math.min(#self.filtered, index + (chord == 'up' and -1 or 1)))
		self.list:send('ListBox.SelectItem', {Value = 'Row' .. index})
		self:selection()
		return true
	end
	return false
end
return Welcome
