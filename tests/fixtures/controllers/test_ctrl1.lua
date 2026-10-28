local function submit(self, args, sender)
  self.calls = (self.calls or 0) + 1
  _G.__controllerProbe = { controller = self, view = self.view, sender = sender,
    text = args.Text, calls = self.calls, version = 1 }
  sender.Text = "one:" .. args.Text
end
return { on_submit = submit, Submit = submit }
