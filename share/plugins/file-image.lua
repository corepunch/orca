table.insert(package.searchers, function(path)
	local extensions = { "", ".png", ".jpg", ".jpeg", ".svg"}
	local filesystem = require "orca.filesystem"
	local cleanpath, query = path:match("^([^?]+)%??(.*)")
	local attrs = { Name = filesystem.getBaseName(cleanpath) }
	for pair in query:gmatch("[^&]+") do
		local k, v = pair:match("([^=]+)=?(.*)")
		if k and #k > 0 then
			attrs[k:gsub("^%l", string.upper)] = v
		end
	end
	for _, ext in ipairs(extensions) do
		local fullpath = cleanpath .. ext
		if filesystem.fileExists(fullpath) then
			attrs.Source = fullpath
			return function ()
				local renderer = require "orca.renderer"
				return renderer.Image(attrs)
			end
		end
	end
end)