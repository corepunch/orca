local ok, ui = pcall(require, "orca.UIKit")
if not ok or not ui or not ui.Form then
  return
end

local function collect_inputs(node, out)
  local iter = node and node.children
  if type(iter) ~= "function" then
    return
  end

  for child in iter do
    if child then
      if child.className == "Input" then
        local name = child.Name
        if type(name) == "string" and name ~= "" then
          out[name] = child.Text
        end
      end
      collect_inputs(child, out)
    end
  end
end

function ui.Form:populateInputs()
  local out = {}
  collect_inputs(self, out)
  return out
end
