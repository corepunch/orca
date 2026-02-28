local orca = require "orca"

local function safe_require(module, name)
	local ok, res = pcall(require, module)
	return ok and res[name] or nil
end

local function instantiate(node)
	local class =
		safe_require("orca.ui", node.name) or
		safe_require("orca.SceneKit", node.name) or
		safe_require("orca.SpriteKit", node.name)
	if class then
		print("Instantiating class "..node.name)
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
	local xml = require "orca.parsers.xml"
	local ok, res = pcall(xml.load, path..'.xml')
	return ok and res and function() return instantiate(res.root) end or nil;
end)