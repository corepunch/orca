-- local orca = require "orca"

local function try_require_memeber(module, name)
	local ok, res = pcall(require, module)
	return ok and res[name] or nil
end

local function construct(node)
	local class =
		try_require_memeber("orca.ui", node.tag) or
		try_require_memeber("orca.SceneKit", node.tag) or
		try_require_memeber("orca.SpriteKit", node.tag) or
		try_require_memeber("orca.filesystem", node.tag) or
		try_require_memeber("orca.renderer", node.tag)

	if not class then
		return error("Unknown node type: "..node.tag)
	end

	local instance = class()
	for k, v in node.attributes do
		instance[k] = v
	end

	for sub in node.children do
		instance:addChild(construct(sub))
	end

	return instance
end

table.insert(package.searchers, function(path)
	local xml = require "orca.parsers.xml"
	local ok, doc = pcall(xml.load, path..'.xml')
	return ok and doc and function()
		return function()
			return construct(doc.root)
		end
	end or nil
end)