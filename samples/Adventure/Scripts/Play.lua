local core = require "orca.core"
local ui = require "orca.UIKit"

local Play = {
  HandleSubmit = function(self, args, sender)
    local text = args.Text
    if not text or text == "" then return end

    -- Find the Transcript ListBox from the page view
    local listbox = self.view:FindChild("Transcript", true)
    if not listbox then return end

    -- Get the ItemsSource (Entries DataObject) from the ListBox
    local entries = listbox.ItemsSource
    if not entries then return end

    -- Create a new Message DataObject with the submitted text
    local msg = core.DataObject { Name = "NewMsg" }
    msg.Text = text
    msg.Type = "outgoing"

    -- Add the new entry to the Transcript DataSource
    entries:addChild(msg)

    -- Create a new item matching the Message template structure
    -- (StackView + TextBlock with binding to DataContext/Text)
    local item = ui.StackView { Name = "Message", class = "message", Direction = "Vertical" }
    local tb = ui.TextBlock { Name = "MessageText", class = "message-text" }
    tb:attachPropertyProgram("TextRun.Text", "{DataContext/Text}", "OneWay", true)
    item:addChild(tb)
    item.DataContext = msg
    listbox:addChild(item)
  end,
}

return Play
