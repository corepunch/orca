-- A small reader for data declarations and schema metadata. It accepts
-- elements, attributes, comments, CDATA and the five predefined entities;
-- scene XML still goes through the native loader.
local Xml = {}

local entities = { amp = "&", lt = "<", gt = ">", quot = '"', apos = "'" }

local function decode(text)
    return (text:gsub("&(#?x?)(%w+);", function(kind, name)
        if kind == "#" then return utf8.char(tonumber(name)) end
        if kind == "#x" then return utf8.char(tonumber(name, 16)) end
        return entities[name] or ("&" .. name .. ";")
    end))
end

local function fail(label, text, at, message)
    local line = select(2, text:sub(1, at):gsub("\n", "")) + 1
    error(("%s:%d: %s"):format(label or "xml", line, message), 0)
end

-- Returns the root element: { tag, attrs, children, text, line }.
function Xml.parse(text, label)
    local root, stack, at = nil, {}, 1
    local function lineAt(pos) return select(2, text:sub(1, pos):gsub("\n", "")) + 1 end
    while at <= #text do
        local lt = text:find("<", at, true)
        if not lt then
            if text:sub(at):match("%S") then fail(label, text, at, "text outside the root element") end
            break
        end
        local content = text:sub(at, lt - 1)
        if content:match("%S") and #stack > 0 then
            local top = stack[#stack]
            top.text = (top.text or "") .. decode(content)
        end
        if text:sub(lt, lt + 3) == "<!--" then
            local close = text:find("-->", lt + 4, true) or fail(label, text, lt, "unterminated comment")
            at = close + 3
        elseif text:sub(lt, lt + 8) == "<![CDATA[" then
            local close = text:find("]]>", lt + 9, true) or fail(label, text, lt, "unterminated CDATA")
            if #stack > 0 then stack[#stack].text = (stack[#stack].text or "") .. text:sub(lt + 9, close - 1) end
            at = close + 3
        elseif text:sub(lt, lt + 1) == "<?" or text:sub(lt, lt + 1) == "<!" then
            at = (text:find(">", lt, true) or fail(label, text, lt, "unterminated declaration")) + 1
        elseif text:sub(lt, lt + 1) == "</" then
            local tag, close = text:match("^</%s*([%w_%.:%-]+)%s*>()", lt)
            if not tag then fail(label, text, lt, "malformed closing tag") end
            local top = table.remove(stack)
            if not top or top.tag ~= tag then fail(label, text, lt, "unexpected </" .. tag .. ">") end
            at = close
        else
            local tag, rest = text:match("^<([%w_%.:%-]+)()", lt)
            if not tag then fail(label, text, lt, "malformed tag") end
            local node = { tag = tag, attrs = {}, order = {}, children = {}, line = lineAt(lt) }
            local pos = rest
            while true do
                local space, name, quote, value, after = text:match("^(%s*)([%w_%.:%-]+)%s*=%s*(['\"])(.-)%3()", pos)
                if not name then break end
                if space == "" then fail(label, text, pos, "attributes need separating spaces") end
                node.attrs[name] = decode(value)
                node.order[#node.order + 1] = name
                pos = after
            end
            local selfClosing, after = text:match("^%s*(/?)>()", pos)
            if not after then fail(label, text, pos, "malformed attribute list in <" .. tag .. ">") end
            if #stack > 0 then
                local parent = stack[#stack]
                parent.children[#parent.children + 1] = node
            elseif root then
                fail(label, text, lt, "a document has one root element")
            else
                root = node
            end
            if selfClosing == "" then stack[#stack + 1] = node end
            at = after
        end
    end
    if #stack > 0 then fail(label, text, #text, "unclosed <" .. stack[#stack].tag .. ">") end
    if not root then fail(label, text, 1, "empty document") end
    return root
end

function Xml.escape(value)
    return (tostring(value):gsub("[&<>\"']", function(c)
        return ({ ["&"] = "&amp;", ["<"] = "&lt;", [">"] = "&gt;", ['"'] = "&quot;", ["'"] = "&apos;" })[c]
    end))
end

-- Reads a project resource ("App/Data/Feed.xml") through the ORCA filesystem,
-- or a plain path when running outside the engine.
function Xml.read(path)
    local ok, fs = pcall(require, "orca.filesystem")
    local text = ok and fs.readTextFile and fs.readTextFile(path)
    if not text then
        local file = io.open(path, "rb")
        if file then text = file:read("a"); file:close() end
    end
    if not text then error("Could not read data resource '" .. tostring(path) .. "'", 0) end
    return text
end

function Xml.load(path)
    return Xml.parse(Xml.read(path), path)
end

return Xml
