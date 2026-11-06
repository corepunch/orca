-- Tokenizer and highlighter for ORCA binding expressions such as
--   IF(STEP(640, {Screen.Width}), Vector2(40, 40), "narrow")
-- Pure Lua so it can be tested without the engine.
local Syntax = {}

Syntax.colors = {
	text = '![7;0m',
	space = '![7;0m',
	number = '![11;0m',
	string = '![10;0m',
	comment = '![8;0m',
	path = '![9;0m',
	brace = '![13;0m',
	func = '![14;0m',
	unknown = '![12;0m',
	keyword = '![13;0m',
	operator = '![15;0m',
	paren = '![7;0m',
	match = '![15;1m',
	error = '![15;4m',
}

local KEYWORDS = { ['true'] = true, ['false'] = true, ['null'] = true }

-- Split one expression into tokens { kind, text, start } (1-based columns).
function Syntax.tokenize(src)
	local tokens, i, n = {}, 1, #src
	local function push(kind, j) tokens[#tokens + 1] = { kind = kind, text = src:sub(i, j), start = i } i = j + 1 end
	while i <= n do
		local c = src:sub(i, i)
		if c:match('%s') then
			push('space', (src:find('[^%s]', i) or n + 1) - 1)
		elseif c == '#' then
			push('comment', (src:find('\n', i, true) or n + 1) - 1)
		elseif c == '"' or c == "'" then
			local j = i + 1
			while j <= n and src:sub(j, j) ~= c do
				if src:sub(j, j) == '\\' then j = j + 1 end
				j = j + 1
			end
			push(j <= n and 'string' or 'error', math.min(j, n))
		elseif c == '{' then
			local j = src:find('}', i + 1, true)
			if j then
				tokens[#tokens + 1] = { kind = 'brace', text = '{', start = i }
				tokens[#tokens + 1] = { kind = 'path', text = src:sub(i + 1, j - 1), start = i + 1 }
				tokens[#tokens + 1] = { kind = 'brace', text = '}', start = j }
				i = j + 1
			else
				push('error', n)
			end
		elseif c:match('%d') or (c == '.' and src:sub(i + 1, i + 1):match('%d')) then
			local j = select(2, src:find('^[%d%.]*', i))
			j = select(2, src:find('^[eE][%+%-]?%d+', j + 1)) or j
			j = select(2, src:find('^%a+', j + 1)) or j -- units such as px
			push('number', j)
		elseif c:match('[%a_]') then
			local j = select(2, src:find('^[%w_]*', i))
			local word = src:sub(i, j)
			local after = src:find('^%s*%(', j + 1)
			if after then push('call', j)
			elseif KEYWORDS[word:lower()] then push('keyword', j)
			else push('text', j) end
		elseif c == '(' or c == ')' or c == '[' or c == ']' then
			push('paren', i)
		elseif c == '}' then
			push('error', i)
		else
			push('operator', i)
		end
	end
	return tokens
end

-- Matching bracket for the one at (or just before) a 1-based column.
function Syntax.matchBracket(src, col)
	local pairs_ = { ['('] = ')', ['['] = ']' }
	local closing = { [')'] = '(', [']'] = '[' }
	local function scan(at)
		local c = src:sub(at, at)
		if pairs_[c] then
			local depth = 0
			for k = at, #src do
				local d = src:sub(k, k)
				if d == c then depth = depth + 1 elseif d == pairs_[c] then depth = depth - 1 end
				if depth == 0 then return at, k end
			end
			return at, nil
		elseif closing[c] then
			local depth = 0
			for k = at, 1, -1 do
				local d = src:sub(k, k)
				if d == c then depth = depth + 1 elseif d == closing[c] then depth = depth - 1 end
				if depth == 0 then return at, k end
			end
			return at, nil
		end
	end
	local a, b = scan(col)
	if not a then a, b = scan(col - 1) end
	return a, b
end

-- The binding VM reads a newline as a statement separator and '#' as a line
-- comment, so multi-line editor text is sent as one line without comments.
function Syntax.flatten(src)
	local parts = {}
	for line in (src .. '\n'):gmatch('([^\n]*)\n') do
		local code, quote = {}, nil
		for k = 1, #line do
			local c = line:sub(k, k)
			if quote then
				if c == quote then quote = nil end
			elseif c == '"' or c == "'" then quote = c
			elseif c == '#' then break end
			code[#code + 1] = c
		end
		local text = table.concat(code):match('^%s*(.-)%s*$')
		if text ~= '' then parts[#parts + 1] = text end
	end
	return table.concat(parts, ' ')
end

-- Unbalanced brackets across the whole expression, as a set of columns.
function Syntax.unbalanced(src)
	local stack, bad = {}, {}
	local inString
	for k = 1, #src do
		local c = src:sub(k, k)
		if inString then
			if c == inString then inString = nil end
		elseif c == '"' or c == "'" then inString = c
		elseif c == '(' or c == '[' then stack[#stack + 1] = k
		elseif c == ')' or c == ']' then
			local open = stack[#stack]
			local want = c == ')' and '(' or '['
			if open and src:sub(open, open) == want then stack[#stack] = nil else bad[k] = true end
		end
	end
	for _, k in ipairs(stack) do bad[k] = true end
	return bad
end

-- Function call names in the expression that the binding VM does not know.
function Syntax.unknownCalls(src, functions)
	local unknown = {}
	for _, t in ipairs(Syntax.tokenize(src)) do
		if t.kind == 'call' and not functions[t.text:upper()] then unknown[#unknown + 1] = t.text end
	end
	return unknown
end

-- Render one line as console escapes. opts.functions marks known calls,
-- opts.highlight is a set of columns to emphasize (bracket match),
-- opts.bad marks unbalanced brackets.
function Syntax.highlight(src, opts)
	opts = opts or {}
	local C = Syntax.colors
	local out = {}
	for _, t in ipairs(Syntax.tokenize(src)) do
		local kind = t.kind
		if kind == 'call' then
			kind = (not opts.functions or opts.functions[t.text:upper()]) and 'func' or 'unknown'
		end
		if kind == 'paren' and opts.bad and opts.bad[t.start] then kind = 'error'
		elseif kind == 'paren' and opts.highlight and opts.highlight[t.start] then kind = 'match' end
		local text = t.text:gsub('!%[', '\xad[')
		out[#out + 1] = (C[kind] or C.text) .. text
	end
	return table.concat(out) .. C.text
end

-- The innermost call surrounding a column: name and argument index.
function Syntax.callAt(src, col)
	local depth, arg = 0, 0
	for k = math.min(col - 1, #src), 1, -1 do
		local c = src:sub(k, k)
		if c == ')' then depth = depth + 1
		elseif c == '(' then
			if depth == 0 then
				local name = src:sub(1, k - 1):match('([%a_][%w_]*)%s*$')
				return name, arg
			end
			depth = depth - 1
		elseif c == ',' and depth == 0 then arg = arg + 1 end
	end
	return nil
end

-- The identifier or {path prefix being typed before a column.
function Syntax.wordAt(src, col)
	local before = src:sub(1, col - 1)
	local path = before:match('{([^{}]*)$')
	if path then return path, 'path' end
	local word = before:match('([%a_][%w_]*)$')
	return word, 'word'
end

return Syntax
