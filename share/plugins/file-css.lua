-- Mapping from CSS property names to ORCA component property names.
-- Keys are lowercase CSS names; values are fully-qualified ORCA names
-- (Class.Property) to disambiguate across component namespaces.
-- Only properties in this map are accepted from CSS sources.
local css_property_map = {
  -- Visual / background
  ["background-color"]  = "Node2D.BackgroundColor",
  ["color"]             = "Node2D.ForegroundColor",
  ["opacity"]           = "Node.Opacity",
  -- Box model
  ["width"]             = "Node.Width",
  ["height"]            = "Node.Height",
  ["min-width"]         = "Node.MinWidth",
  ["min-height"]        = "Node.MinHeight",
  ["margin"]            = "Node.Margin",
  ["margin-top"]        = "Node.MarginTop",
  ["margin-right"]      = "Node.MarginRight",
  ["margin-bottom"]     = "Node.MarginBottom",
  ["margin-left"]       = "Node.MarginLeft",
  ["padding"]           = "Node.Padding",
  ["padding-top"]       = "Node.PaddingTop",
  ["padding-right"]     = "Node.PaddingRight",
  ["padding-bottom"]    = "Node.PaddingBottom",
  ["padding-left"]      = "Node.PaddingLeft",
  -- Border
  ["border"]            = "Node.Border",
  ["border-color"]      = "Node.BorderColor",
  ["border-width"]      = "Node.BorderWidth",
  -- Typography (TextRun / TextBlockConcept)
  ["font-size"]         = "TextRun.FontSize",
  ["font-family"]       = "TextRun.FontFamily",
  ["line-height"]       = "TextRun.LineHeight",
  ["letter-spacing"]    = "TextRun.LetterSpacing",
  ["word-wrap"]         = "TextBlockConcept.WordWrap",
  ["text-overflow"]     = "TextBlockConcept.TextOverflow",
  -- Visibility
  ["visibility"]        = "Node.Visible",
}

local function skip(s, i) return s:match("^%s*()", i) end

local function parse_props(s, i, cls, out)
  i = s:match("^%s*{()", i) or (error("expected {") or i)
  while i <= #s do
    i = skip(s, i)
    local c = s:sub(i,i)
    if c == "" or c == "}" then break end
    i = s:match("^%-*()", i)  -- strip leading dashes
    c = s:sub(i,i)
    if c == "&" then
      -- nested rule: &.foo { ... }
      local sel, j = s:match("^&([^{]+)()", i)
      local props = {}
      i = parse_props(s, j-1, cls..sel:match("^%s*(.-)%s*$"), props)
      for k,v in pairs(props) do out[k] = v end
    elseif c == "@" then
      local key, j = s:match("^(%S+)()", i)
      local val = s:match("^%s*([^;{}]*)", j):match("^%s*(.-)%s*$")
      out[key] = val
      i = s:match("[;{}]()", j) or (#s+1)
    else
      local rawkey, j = s:match("^([^:{};]*)()", i)
      local key = css_property_map[rawkey:match("^%s*(.-)%s*$")]
      i = s:match("^:?()", j)
      i = skip(s, i)
      local val = s:match("^([^;{}]*)", i):match("^(.-)%s*$")
      if key then out[key] = val end  -- skip CSS properties not in the map
      i = i + #s:match("^([^;{}]*)", i)
    end
    i = s:match("^%s*;?%s*()", i)
  end
  return (s:match("^}()", i) or i)
end

local function css_parse(css)
  local result, i = {}, 1
  while i <= #css do
    i = skip(css, i)
    if i > #css then break end
    local sel, j = css:match("^([^{]+)()", i)
    if not sel then break end
    local props = {}
    i = parse_props(css, j-1, sel:match("^%s*(.-)%s*$"), props)
    for s in (sel..","):gmatch("([^,]+),") do
      result[s:match("^%s*(.-)%s*$")] = props
    end
  end
  return result
end

-- Split a CSS selector into (className, pseudoClass).
-- ".button:hover:focus" → "button", "hover:focus"
-- "card" → "card", ""
local function parse_selector(selector)
  local base = selector:match("^%.(.+)$") or selector
  local className = base:match("^([^:/]+)") or base
  local pseudo    = base:match(":(.+)$") or ""
  return className, pseudo
end

-- Build and return a core.StyleSheet from a parsed CSS table.
-- Each top-level selector becomes a StyleRule child whose ClassName and
-- PseudoClass are extracted from the selector string, and whose property
-- overrides are attached as C properties for zero-cost binary copy on apply.
local function css_to_stylesheet(parsed)
  local core = require "orca.core"
  local sheet = core.StyleSheet()
  for selector, props in pairs(parsed) do
    local className, pseudo = parse_selector(selector)
    local rule = core.StyleRule()
    rule.ClassName  = className
    rule.PseudoClass = pseudo
    for propName, propVal in pairs(props) do
      if propName:sub(1, 1) ~= "@" then  -- skip @-directives like @apply
        rule[propName] = propVal
      end
    end
    sheet:addChild(rule)
  end
  return sheet
end

table.insert(package.searchers, function(path)
	local filesystem = require "orca.filesystem"
	local contents = filesystem.readTextFile(path..'.css')
	return contents and function() return css_to_stylesheet(css_parse(contents)) end or nil
end)
