local orca = require "orca"
local xml = require "orca.parsers.xml"

local shortcuts = {}

for k, v in pairs(orca.ui) do shortcuts[k] = v end
for k, v in pairs(orca.SceneKit) do shortcuts[k] = v end
for k, v in pairs(orca.SpriteKit) do shortcuts[k] = v end

local function instantiate(node)
	if shortcuts[node.name] then
		local class = shortcuts[node.name]
		-- return (node.attributes)
	else
		return error("Unknown node type: "..node.name)
	end
	-- local instance = obj(node.attributes)
	for sub in node.children do
		instantiate(sub)
	end
end

-- orca.register_loader("xml", load_xml)
table.insert(package.searchers, function(path)
	local ok, res = pcall(xml.load, path..'.xml')
	return ok and function() return instantiate(res.node) end or nil;
end)