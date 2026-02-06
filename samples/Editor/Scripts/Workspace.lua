local config = require "Editor.Scripts.config"
local fs = require "orca.filesystem"
local ui = require "orca.ui"
local core = require "orca.core"
local plugs = "-plugins=~/Developer/icui/plugins"
local server = "-server=true"
-- local proj = "/Users/icherna/Developer/icui/ICHMIMain"
local proj = "~/Developer/ui-framework/samples/Example"

local system = require "orca.system"

local Workspace = {
	undoStack = { idx = 1 },
  redoStack = { idx = 1 },
  cleanChange = 1,
}

Workspace.process = fs.pipe.popen(EXENAME, server, plugs, proj)

function ui.TerminalView:showDropDown(item, x, y, callback)
  local DropDown = require "Editor.Scripts.DropDown"
	local geom = require "orca.geometry"
	local dropdown = DropDown {
		doc = item,
    callback = callback,
		LayoutTransformTranslation = geom.Vector2.new(x, y),
	}
  self.modal = dropdown
	return dropdown
end

local function push_undo(undoStack, time, type, ...)
  undoStack[undoStack.idx] = { type = type, time = time, ... }
  undoStack[undoStack.idx - config.max_undos] = nil
  undoStack.idx = undoStack.idx + 1
end

local function pop_undo(self, undoStack, redoStack)
  -- pop command
  local cmd = undoStack[undoStack.idx - 1]
  if not cmd then return end
  undoStack.idx = undoStack.idx - 1
  local data = cmd[1].root
  print(data)
  if data:get'command' == 'edit' then
    -- local values = {}
    for n in data.children do
      self:setValue(data:get'source', n:get'name', n:get'value', redoStack)
      -- values[n:get'name'] = n:get'value'
    end
  end
  -- -- handle command
  -- if cmd.type == "insert" then
  --   local line, col, text = table.unpack(cmd)
  --   self:raw_insert(line, col, text, redoStack, cmd.time)

  -- elseif cmd.type == "remove" then
  --   local line1, col1, line2, col2 = table.unpack(cmd)
  --   self:raw_remove(line1, col1, line2, col2, redoStack, cmd.time)

  -- elseif cmd.type == "selection" then
  --   self.selection.a.line, self.selection.a.col = cmd[1], cmd[2]
  --   self.selection.b.line, self.selection.b.col = cmd[3], cmd[4]
  -- end

  -- if next undo command is within the merge timeout then treat as a single
  -- command and continue to execute it
  local next = undoStack[undoStack.idx - 1]
  if next and math.abs(cmd.time - next.time) < config.undo_merge_timeout then
    return pop_undo(self, undoStack, redoStack)
  end
end

function Workspace:setValue(path, name, value, undoStack)
  local resp = self.process:put("node/%s?%s=%s", path, name, value)
  -- print(resp)
  push_undo(undoStack or self.undoStack, system.getTime(), 'edit', resp)
end

function Workspace:undo()
  pop_undo(self, self.undoStack, self.redoStack)
end


function Workspace:redo()
  pop_undo(self, self.redoStack, self.undoStack)
end

core.addCommands(nil, {
  ["doc:save-project"] = function()
    Workspace.process:post('save-project')
  end,
})

core.addHotKeys {
  ["cmd+s"] = "doc:save-project",
}

return Workspace