local app = require "AppKit"
local command = require "AppKit.command"

local function doc()
  return require "Document"
end

local commands = {
	["app:save-file"] = function ()
		doc().level:save("/Users/igor/Desktop/mylevel.lev")
	end,
}

command.add(doc, commands)
