-- Controller for Play.xml.
-- The module is a factory function: called with the root node when ViewDidLoad
-- fires, returns a table of event callbacks to wire onto the node.
local function Play(self)
  local input      = self:findChild("CommandInput", true)
  local transcript = self:findChild("Transcript", true)

  local function appendMessage(msgType, text)
    -- TODO: mutate Transcript DataSource when the data mutation API is available
    _ = msgType
    _ = text
  end

  local function submitCommand()
    if not input then return end
    local text = input.Text
    if not text or text == "" then return end
    appendMessage("outgoing", "> " .. text)
    input.Text = ""
    appendMessage("incoming", "You can't do that yet.")
  end

  return {
    KeyDown = function(self, key, sender)
      if key.hotKey == "return" then
        submitCommand()
      end
    end,
  }
end

return Play
