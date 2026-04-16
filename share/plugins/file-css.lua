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

-- Keep in sync with STYLE_HOVER, STYLE_FOCUS, an so on
local css_pseudo_classes = {
	hover = 1,
	focus = 2,
	select = 4,
	dark = 8,
	disable = 16,
}

-- example CSS input:
-- .button, .link { color: red; font-size: 14px; }
-- .card { background-color: white; padding: 8px;
--   &:hover { background-color: grey; }
--   & .title { font-weight: bold; }
-- }
-- would parse to:
-- out = {
--   [".button"]      = { color="red", ["font-size"]="14px" },
--   [".link"]        = { color="red", ["font-size"]="14px" },  -- same table
--   [".card"]        = { ["background-color"]="white", padding="8px" },
--   [".card:hover"]  = { ["background-color"]="grey" },
--   [".card .title"] = { ["font-weight"]="bold" },
-- }

local function flatten_css(css)
  -- expand .parent { & .child { ... } } into .parent .child { ... }
  local changed = true
  while changed do
    changed = false
    css = css:gsub("([^{}-][^{]-)%s*{([^{}]-)&([^{}]-)(%b{})}", function(parent, before, sel_suffix, block)
      changed = true
      local inner = block:sub(2, -2)  -- strip { }
      return before .. "\n" .. parent:match("^%s*(.-)%s*$") .. sel_suffix:match("^%s*(.-)%s*$") .. " " .. "{" .. inner .. "}"
    end)
  end
  return css
end

local function css_parse(css)
  local result = {}

  css = css:gsub("/%*.-%*/", "")  -- strip comments
  css = flatten_css(css)

  for sel_raw, block in css:gmatch("([^{]+){([^}]*)}") do
    local props = {}

    for decl in block:gmatch("[^;]+") do
      local key, val = decl:match("^%s*([%-%w]+)%s*:%s*(.-)%s*$")
      if key and key ~= "" then
        props[key] = val
      end
    end

    for sel in (sel_raw .. ","):gmatch("%s*(.-)%s*,") do
      if sel ~= "" then
        result[sel] = props
      end
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
  -- local pseudo_mask = 0
  -- for p in pseudo:gmatch("([^:]+)") do pseudo_mask = pseudo_mask + (css_pseudo_classes[p] or 0) end
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
    local class_name, pseudo = parse_selector(selector)
    local rule = sheet + core.StyleRule { ClassName = class_name, PseudoClass = pseudo }
    for k, v in pairs(props) do
      if k:sub(1, 1) ~= "@" then  -- skip @-directives like @apply
        local prop_name = css_property_map[k:lower()]
        if prop_name then
          rule[prop_name] = v
        else
          io.stderr:write("Warning: Unsupported CSS property '" .. k .. "' in selector '" .. selector .. "'\n")
        end
      else
        io.stderr:write("Warning: Skipping unsupported CSS directive: " .. k .. "\n")
      end
    end
  end
  return sheet
end

local orca = require "orca"
orca.core.StyleSheet.Parse = function(css)
  return css_to_stylesheet(css_parse(css))
end

table.insert(package.searchers, function(path)
	local filesystem = require "orca.filesystem"
	local contents = filesystem.readTextFile(path..'.css')
	return contents and function()
    return orca.core.StyleSheet.Parse(contents)
  end or nil
end)
