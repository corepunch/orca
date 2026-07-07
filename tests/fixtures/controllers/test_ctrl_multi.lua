-- test_ctrl_multi.lua — controller with multiple event handlers
return {
  on_submit = function(self, args, sender)
    -- submit handler
  end,
  on_clear = function(self, args, sender)
    -- clear handler
  end,
}
