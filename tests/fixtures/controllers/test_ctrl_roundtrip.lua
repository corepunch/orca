return {
    Submit = function(self) self.view.Text = "convention" end,
    CustomSubmit = function(self, args, sender)
        self.view.Text = args.Text .. ":" .. sender.Name
    end,
    CustomClear = function(self) self.view.Text = "cleared" end,
}
