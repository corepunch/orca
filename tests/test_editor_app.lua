-- Standalone Lua tests for the Editor app's binding syntax highlighter and
-- project-server reply reader. Run with: make test-editor-app
package.path = "apps/?.lua;" .. package.path

local Syntax = require "Editor.Scripts.Syntax"
local Xml = require "Editor.Scripts.Xml"

local failures = 0
local function check(cond, msg)
	if not cond then
		failures = failures + 1
		io.stderr:write("FAIL: " .. msg .. "\n")
	end
end

-- Tokenizer
local src = 'IF(STEP(640, {Screen.Width}), Vector2(40, 40), "1fr 1fr")'
local kinds = {}
for _, t in ipairs(Syntax.tokenize(src)) do kinds[#kinds + 1] = t.kind .. ':' .. t.text end
local joined = table.concat(kinds, ' ')
check(joined:find('call:IF', 1, true), "IF is a call")
check(joined:find('path:Screen.Width', 1, true), "{Screen.Width} is a path")
check(joined:find('number:640', 1, true), "640 is a number")
check(joined:find('string:"1fr 1fr"', 1, true), "quoted text is a string")

-- Brackets
local a, b = Syntax.matchBracket(src, 3)
check(a == 3 and b == #src, "outer parentheses match")
check(next(Syntax.unbalanced(src)) == nil, "balanced expression has no errors")
check(Syntax.unbalanced('IF((1, 2)')[3], "unclosed bracket is reported")
check(next(Syntax.unbalanced('"(" ')) == nil, "brackets inside strings are ignored")

-- Function checks are case-insensitive, like the binding VM
local known = { IF = true, STEP = true, VECTOR2 = true }
check(#Syntax.unknownCalls(src, known) == 0, "known functions pass")
check(Syntax.unknownCalls('FOO(1)', known)[1] == 'FOO', "unknown function is reported")

-- Completion context
check(Syntax.callAt(src, 14) == 'STEP', "caret inside STEP's arguments")
check(select(2, Syntax.callAt(src, 14)) == 1, "second argument of STEP")
local word, kind = Syntax.wordAt('MIX(0, {Scr', 12)
check(word == 'Scr' and kind == 'path', "partial {path} before caret")
word, kind = Syntax.wordAt('MIX(0, ST', 10)
check(word == 'ST' and kind == 'word', "partial function name before caret")

-- Highlighting keeps the text and never emits a literal color escape
local line = Syntax.highlight('A(1)![2;3m', { functions = { A = true } })
check(not line:find('!%[2;3m'), "user text cannot inject color escapes")

-- Multi-line editor text becomes one VM line without comments
check(Syntax.flatten('IF(STEP(640, {Screen.Width}),  # wide\n   48,\n   16)') ==
	'IF(STEP(640, {Screen.Width}), 48, 16)', "lines joined and comments dropped")
check(Syntax.flatten('STRING("#1")') == 'STRING("#1")', "'#' inside a string is kept")
check(Syntax.tokenize('1 # note')[3].kind == 'comment', "'#' starts a comment")

-- Reply reader
local root = Xml.parse('X11 noise\n<?xml version="1.0"?>\n<response status="success" source="A/B">'
	.. '<Float name="Opacity" value="0.5" data-program="MIX(0,&#10;1)"/><group name="g"><Bool name="v" value="true"/></group>'
	.. '</response>')
check(root and root.tag == 'response', "reply root parsed after stdout noise")
check(root:get'source' == 'A/B', "attribute read")
local first = root:first()
check(first:get'data-program' == 'MIX(0,\n1)', "character references decoded")
local count = 0
for _ in root:each() do count = count + 1 end
check(count == 2, "two child elements")
local err = Xml.parse('<response status="error">Can&apos;t find object</response>')
check(err.text == "Can't find object", "error text decoded")

if failures > 0 then
	io.stderr:write(failures .. " editor app test(s) failed\n")
	os.exit(1)
end
print("All editor app tests passed.")
