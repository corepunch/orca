-- Small XML reader for project-server replies. Produces plain Lua tables so
-- nodes stay valid for as long as panels reference them.
local Xml = {}

local Node = {}
Node.__index = Node

function Node:get(name) return self.attrs[name] end

-- Iterate child elements: for child in node:each() do ... end
function Node:each()
	local i = 0
	return function()
		i = i + 1
		return self.nodes[i]
	end
end

function Node:first() return self.nodes[1] end

local ENTITIES = { amp = '&', lt = '<', gt = '>', quot = '"', apos = "'" }

local function decode(s)
	return (s:gsub('&(#?)([xX]?)(%w+);', function(hash, hex, v)
		if hash == '#' then
			local code = tonumber(v, hex ~= '' and 16 or 10)
			return code and utf8.char(code) or ''
		end
		return ENTITIES[v] or ('&' .. v .. ';')
	end))
end

local function element(tag, attrstr, parent)
	local attrs = {}
	for k, _, v in attrstr:gmatch('([%w_:%.%-]+)%s*=%s*(["\'])(.-)%2') do
		attrs[k] = decode(v)
	end
	return setmetatable({ tag = tag, attrs = attrs, nodes = {}, text = nil, parent = parent }, Node)
end

-- Parse a document and return its root element (or nil, error).
function Xml.parse(src)
	local pos = src:find('<response', 1, true) or src:find('<[%a_]') or 1
	local root, current
	local len = #src
	while pos <= len do
		local open = src:find('<', pos, true)
		local text = src:sub(pos, (open or len + 1) - 1)
		if current and text:find('%S') then current.text = (current.text or '') .. decode(text) end
		if not open then break end
		if src:sub(open, open + 3) == '<!--' then
			pos = (select(2, src:find('-->', open, true)) or len) + 1
		elseif src:sub(open, open + 1) == '<?' then
			pos = (select(2, src:find('?>', open, true)) or len) + 1
		elseif src:sub(open, open + 1) == '</' then
			local close = src:find('>', open, true) or len
			current = current and current.parent
			pos = close + 1
		else
			local close = src:find('>', open, true)
			if not close then return root, 'unterminated tag' end
			local body = src:sub(open + 1, close - 1)
			local selfClosing = body:sub(-1) == '/'
			if selfClosing then body = body:sub(1, -2) end
			local tag, attrstr = body:match('^([^%s/>]+)(.*)$')
			local node = element(tag or '?', attrstr or '', current)
			if current then current.nodes[#current.nodes + 1] = node else root = node end
			if not selfClosing then current = node end
			pos = close + 1
		end
	end
	if not root then return nil, 'empty reply' end
	return root
end

return Xml
