-- Binding expression editor: a modal dialog with a multi-line code editor,
-- syntax highlighting, bracket matching, live diagnostics, completion of
-- functions and {property} paths, and a binding mode selector.
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Workspace = require "Editor.Scripts.Workspace"
local Popups = require "Editor.Scripts.Popups"
local Menu = require "Editor.Scripts.Menu"
local Syntax = require "Editor.Scripts.Syntax"

local BindingEditor = Terminal:extend {}

local COLS, EDIT_ROWS, GUTTER = 84, 9, 5
local FIRST_EDIT_ROW = 3 -- buffer row of the first code line
local MODES = { "Expression", "OneWay", "TwoWay", "OneWayToSource", "OneTime" }
local MODE_HELP = {
	Expression = "Evaluate the expression whenever its inputs change",
	OneWay = "Copy a single {source} path into the property",
	TwoWay = "Keep the property and a single {source} path in sync",
	OneWayToSource = "Write the property back to a single {source} path",
	OneTime = "Evaluate once when the binding is created",
}
local ROW = '![8;0m'

local function functionTable()
	if BindingEditor.functions then return BindingEditor.functions end
	local map, list = {}, {}
	local root = Workspace.get('binding-functions')
	for f in (root and root:each() or function() end) do
		local name = f:get'name'
		map[name] = { name = name, signature = f:get'signature', summary = f:get'summary' }
		list[#list + 1] = map[name]
	end
	table.sort(list, function(a, b) return a.name < b.name end)
	BindingEditor.functions, BindingEditor.functionList = map, list
	return map
end

function BindingEditor:init(opts)
	self.opts = opts
	self.lines = {}
	for line in ((opts.expression ~= '' and opts.expression or '') .. '\n'):gmatch('([^\n]*)\n') do
		self.lines[#self.lines + 1] = line
	end
	if #self.lines == 0 then self.lines = { '' } end
	self.row = #self.lines
	self.col = #self.lines[self.row]
	self.mode = 'Expression'
	self.top = 1
	self.message = nil
	self.paths = { 'Screen.Width', 'Screen.Height', 'DataContext/' }
	for _, id in ipairs(opts.properties or {}) do self.paths[#self.paths + 1] = id end
	functionTable()
	local height = FIRST_EDIT_ROW + EDIT_ROWS + 9
	Terminal.init(self, {
		Width = COLS * config.char_width,
		Height = height * config.char_height,
		DropShadow = true,
	})
	self:invalidate()
end

function BindingEditor:text() return table.concat(self.lines, '\n') end

function BindingEditor:line() return self.lines[self.row] end

-- Diagnostics shared by drawing and Apply.
function BindingEditor:diagnose()
	local src = Syntax.flatten(self:text())
	local problems = {}
	if src:match('^%s*$') then return problems, true end
	local unbalanced = Syntax.unbalanced(src)
	if next(unbalanced) then problems[#problems + 1] = "Unbalanced brackets" end
	for _, name in ipairs(Syntax.unknownCalls(src, BindingEditor.functions)) do
		problems[#problems + 1] = "Unknown function " .. name .. "()"
	end
	if select(2, src:gsub('"', '')) % 2 == 1 then problems[#problems + 1] = "Unterminated string" end
	if self.mode ~= 'Expression' and self.mode ~= 'OneTime' and not src:match('^%s*{[^{}]+}%s*$') then
		problems[#problems + 1] = self.mode .. " needs a single {source} path"
	end
	return problems, false
end

-- Completion candidates for the word or {path} before the caret.
function BindingEditor:suggestions()
	if self.dismissed then return {} end
	local prefix, kind = Syntax.wordAt(self:line(), self.col + 1)
	local out = {}
	if kind == 'path' then
		for _, p in ipairs(self.paths) do
			if p:lower():sub(1, #prefix) == prefix:lower() and p ~= prefix then
				out[#out + 1] = { label = p, insert = p:sub(#prefix + 1), detail = 'property' }
			end
		end
	elseif prefix and #prefix >= 1 then
		for _, f in ipairs(BindingEditor.functionList) do
			if f.name:sub(1, #prefix) == prefix:upper() and f.name ~= prefix:upper() then
				out[#out + 1] = { label = f.name, insert = f.name:sub(#prefix + 1) .. '(', detail = f.summary, fn = f }
			end
		end
	end
	return out
end

function BindingEditor:draw()
	local o = self.opts
	local cols = COLS
	local title = string.format(' Binding  %s.%s ', Workspace.leaf(o.path), o.name)
	self:println(nil, '![15;1m', Terminal.fit(Terminal.escape(title), cols - 12), '![11;1m[Esc] Close ')
	local target = Terminal.fit(o.property .. ' (' .. (o.type or '?') .. ')', 36)
	local help = Terminal.fit(MODE_HELP[self.mode], cols - 8 - 36 - 6 - 19 - 1)
	self:println({ kind = 'mode' }, '![8;0m Target ![15;0m', target,
		'![8;0m Mode ![15;8m ', Terminal.fit(self.mode, 15), config.icons.dropdown, '![7;0m ', '![8;0m', help)
	self:println(nil, ROW, string.rep(config.icons.hline, cols))

	-- Code lines with highlighting and the bracket pair around the caret.
	local src = self:text()
	local bad = Syntax.unbalanced(src)
	if self.row < self.top then self.top = self.row end
	if self.row >= self.top + EDIT_ROWS then self.top = self.row - EDIT_ROWS + 1 end
	local offset = 0
	for r = 1, self.top - 1 do offset = offset + #self.lines[r] + 1 end
	local a, b = Syntax.matchBracket(self:line(), self.col + 1)
	for r = self.top, self.top + EDIT_ROWS - 1 do
		local line = self.lines[r]
		if line then
			local localBad = {}
			for k in pairs(bad) do
				local c = k - offset
				if c >= 1 and c <= #line then localBad[c] = true end
			end
			local hl = (r == self.row and b) and { [a] = true, [b] = true } or nil
			local number = r == self.row and '![15;0m' or '![8;0m'
			self:println({ kind = 'line', row = r }, number, string.format('%3d', r), ' ![8;0m', config.icons.vline,
				Syntax.highlight(line, { functions = BindingEditor.functions, highlight = hl, bad = localBad }))
			offset = offset + #line + 1
		else
			self:println({ kind = 'line', row = #self.lines }, '![8;0m    ', config.icons.vline)
		end
	end
	self:println(nil, ROW, string.rep(config.icons.hline, cols))

	-- Diagnostics and caret position.
	local problems, empty = self:diagnose()
	local status
	if self.message then status = '![12;0m x ' .. Terminal.escape(self.message)
	elseif empty then status = '![8;0m Empty expression: Apply removes the binding'
	elseif #problems > 0 then status = '![14;0m ! ' .. Terminal.escape(table.concat(problems, '; '))
	else status = '![10;0m ' .. config.icons.check .. ' Expression looks valid' end
	local pos = string.format('Ln %d, Col %d', self.row, self.col + 1)
	self:println(nil, Terminal.fit('', 0), status, '![8;0m', '   ', pos)

	-- Signature help for the call around the caret.
	local name, arg = Syntax.callAt(self:line(), self.col + 1)
	local f = name and BindingEditor.functions[name:upper()]
	if f then
		self:println(nil, '![14;0m ', f.signature, '![8;0m  argument ', arg + 1, '  ', Terminal.escape(f.summary))
	else
		self:println(nil, '![8;0m Ctrl+Space completes functions, { completes property paths, # starts a comment')
	end
	self:println(nil, ROW, string.rep(config.icons.hline, cols))

	-- Completion list, or the function palette.
	self.completions = self:suggestions()
	self.choice = math.max(1, math.min(self.choice or 1, #self.completions))
	if #self.completions > 0 then
		for i = 1, 3 do
			local c = self.completions[i + math.max(0, self.choice - 3)]
			if c then
				local selected = (i + math.max(0, self.choice - 3)) == self.choice
				self:println({ kind = 'complete', index = i + math.max(0, self.choice - 3) },
					selected and '![1;14m ' or '![14;0m ', Terminal.fit(c.label, 22), selected and '![1;14m' or '![8;0m',
					Terminal.fit(c.detail or '', cols - 25), ' ')
			else
				self:println(nil, '')
			end
		end
	else
		local row, rows = {}, {}
		local width = 0
		for _, fn in ipairs(BindingEditor.functionList) do
			if width + #fn.name + 1 > cols - 2 then
				rows[#rows + 1] = row
				row, width = {}, 0
			end
			row[#row + 1] = fn
			width = width + #fn.name + 1
		end
		rows[#rows + 1] = row
		for i = 1, 3 do
			local r = rows[i] or {}
			local parts = {}
			for _, fn in ipairs(r) do parts[#parts + 1] = fn.name end
			self:println({ kind = 'palette', fns = r }, '![14;0m ', table.concat(parts, ' '))
		end
	end
	self:println(nil, ROW, string.rep(config.icons.hline, cols))
	self.buttons = {
		{ label = ' Apply  Ctrl+Enter ', action = function() self:apply() end },
		{ label = ' Remove Binding ', action = function() self.lines = { '' } self:apply() end },
		{ label = ' Cancel  Esc ', action = function() Popups.close(self) end },
	}
	local parts, col = { ' ' }, 1
	for i, btn in ipairs(self.buttons) do
		btn.from, btn.to = col, col + #btn.label - 1
		parts[#parts + 1] = (i == 1 and '![15;2m' or '![15;8m') .. btn.label .. '![7;0m '
		col = col + #btn.label + 1
	end
	self:println({ kind = 'buttons' }, table.concat(parts))

	-- Place the console caret in the code area.
	self.view.Cursor = (FIRST_EDIT_ROW + self.row - self.top) * self.view.BufferWidth + GUTTER + self.col
end

------------------------------------------------------------------------------
-- Editing

function BindingEditor:edited()
	self.message = nil
	self.dismissed = false
	self.choice = 1
	self:invalidate()
end

function BindingEditor:insert(s)
	local line = self:line()
	self.lines[self.row] = line:sub(1, self.col) .. s .. line:sub(self.col + 1)
	self.col = self.col + #s
end

local CLOSERS = { ['('] = ')', ['{'] = '}', ['['] = ']', ['"'] = '"' }

function BindingEditor:onText(ch)
	local line = self:line()
	local nextChar = line:sub(self.col + 1, self.col + 1)
	if (ch == ')' or ch == '}' or ch == ']' or ch == '"') and nextChar == ch then
		self.col = self.col + 1 -- type over an auto-inserted closer
	elseif CLOSERS[ch] and (nextChar == '' or nextChar:match('[%s%),%]}]')) then
		self:insert(ch .. CLOSERS[ch])
		self.col = self.col - 1
	else
		self:insert(ch)
	end
	self:edited()
end

function BindingEditor:accept(c)
	if not c then return end
	self:insert(c.insert)
	if c.fn then
		local line = self:line()
		if line:sub(self.col + 1, self.col + 1) ~= ')' then
			self.lines[self.row] = line:sub(1, self.col) .. ')' .. line:sub(self.col + 1)
		end
	elseif self:line():sub(self.col + 1, self.col + 1) == '}' then
		self.col = self.col + 1
	end
	self:edited()
end

function BindingEditor:onKey(chord)
	local line = self:line()
	local completing = #(self.completions or {}) > 0
	if chord == 'escape' then
		if completing then self.dismissed = true self:invalidate() else Popups.close(self) end
	elseif chord == 'ctrl+enter' then self:apply()
	elseif chord == 'tab' then
		if completing then self:accept(self.completions[self.choice]) else self:insert('  ') self:edited() end
	elseif chord == 'ctrl+space' then self.dismissed = false self:invalidate()
	elseif chord == 'up' and completing then self.choice = math.max(1, self.choice - 1) self:invalidate()
	elseif chord == 'down' and completing then self.choice = math.min(#self.completions, self.choice + 1) self:invalidate()
	elseif chord == 'enter' then
		if completing then return self:accept(self.completions[self.choice]) or true end
		local indent = line:match('^%s*')
		self.lines[self.row] = line:sub(1, self.col)
		table.insert(self.lines, self.row + 1, indent .. line:sub(self.col + 1))
		self.row, self.col = self.row + 1, #indent
		self:edited()
	elseif chord == 'backspace' then
		if self.col > 0 then
			local before, after = line:sub(self.col, self.col), line:sub(self.col + 1, self.col + 1)
			local pair = CLOSERS[before] == after and 1 or 0
			self.lines[self.row] = line:sub(1, self.col - 1) .. line:sub(self.col + 1 + pair)
			self.col = self.col - 1
		elseif self.row > 1 then
			local prev = self.lines[self.row - 1]
			self.lines[self.row - 1] = prev .. line
			table.remove(self.lines, self.row)
			self.row, self.col = self.row - 1, #prev
		end
		self:edited()
	elseif chord == 'delete' then
		if self.col < #line then
			self.lines[self.row] = line:sub(1, self.col) .. line:sub(self.col + 2)
		elseif self.row < #self.lines then
			self.lines[self.row] = line .. table.remove(self.lines, self.row + 1)
		end
		self:edited()
	elseif chord == 'left' then
		if self.col > 0 then self.col = self.col - 1
		elseif self.row > 1 then self.row = self.row - 1; self.col = #self:line() end
		self.dismissed = true self:invalidate()
	elseif chord == 'right' then
		if self.col < #line then self.col = self.col + 1
		elseif self.row < #self.lines then self.row = self.row + 1; self.col = 0 end
		self.dismissed = true self:invalidate()
	elseif chord == 'up' then
		if self.row > 1 then self.row = self.row - 1; self.col = math.min(self.col, #self:line()) end
		self:invalidate()
	elseif chord == 'down' then
		if self.row < #self.lines then self.row = self.row + 1; self.col = math.min(self.col, #self:line()) end
		self:invalidate()
	elseif chord == 'home' then self.col = #line:match('^%s*') == self.col and 0 or #line:match('^%s*') self:invalidate()
	elseif chord == 'end' then self.col = #line self:invalidate()
	else
		return false
	end
	return true
end

function BindingEditor:onMouseDown(item, index, _, args)
	if not item then return end
	if item.kind == 'line' then
		self.row = math.min(item.row, #self.lines)
		self.col = math.min(#self:line(), math.max(0, self:columnAt(index, args.x) - GUTTER))
		self.dismissed = true
		self:invalidate()
	elseif item.kind == 'mode' then
		local options = {}
		for _, m in ipairs(MODES) do
			options[#options + 1] = { label = m, checked = m == self.mode, shortcut = '', action = function()
				self.mode = m
				self:invalidate()
			end }
		end
		local x, y = self:positionOf(index, 50, 1)
		Popups.push(Menu(options), x or 0, y or 0)
	elseif item.kind == 'complete' then
		self.choice = item.index
		self:accept(self.completions[item.index])
	elseif item.kind == 'palette' then
		local col, at = self:columnAt(index, args.x), 1
		for _, fn in ipairs(item.fns) do
			if col >= at and col < at + #fn.name then
				self:insert(fn.name .. '()')
				self.col = self.col - 1
				self:edited()
				return
			end
			at = at + #fn.name + 1
		end
	elseif item.kind == 'buttons' then
		local col = self:columnAt(index, args.x)
		for _, btn in ipairs(self.buttons) do
			if col >= btn.from and col <= btn.to then return btn.action() end
		end
	end
end

function BindingEditor:apply()
	local problems, empty = self:diagnose()
	if #problems > 0 then
		self.message = table.concat(problems, '; ')
		return self:invalidate()
	end
	local expression = empty and '' or Syntax.flatten(self:text())
	local ok, err = Workspace.setBinding(self.opts.path, self.opts.property, expression, self.mode)
	if ok then
		Popups.close(self)
	else
		self.message = err
		self:invalidate()
	end
end

function BindingEditor.open(opts)
	local editor = BindingEditor(opts)
	local sw, sh = Popups.size()
	local x = math.max(0, (sw - editor.view.Width) / 2)
	local y = math.max(16, (sh - editor.view.Height) / 3)
	return Popups.push(editor, x, y, true)
end

return BindingEditor
