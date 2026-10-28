return {
  MyInput_Submit = function(self, args, sender)
    self.calls = (self.calls or 0) + 1
    _G.__controllerProbe = { controller = self, view = self.view, sender = sender,
      text = args.Text, calls = self.calls, version = "wired" }
    sender.Text = "wired:" .. args.Text
  end,
}
