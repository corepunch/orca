local Application = require "orca.core.application"
local filesystem = require "orca.filesystem"

local function snapshot(draft)
    return { DisplayName = draft.DisplayName, Email = draft.Email, OptIn = draft.OptIn }
end

return Application:extend {
    layout = function(self, context) return context.slots.inner end,
    __init = function(self)
        Application.__init(self)
        self.form = assert(filesystem.loadObject("MvvmForms/Views/Form"))
        self.draft = assert(self.form:findChild("DraftForm", true).DataContext)
        self.committed = snapshot(self.draft)
    end,
    ["/"] = function(self) return self.form end,
    saveDraft = function(self)
        if not self.draft.DisplayName or not self.draft.DisplayName:match("%S") then
            self.draft.Status = "Enter a display name before saving."
            return false
        end
        if not self.draft.Email or not self.draft.Email:match("^[^%s@]+@[^%s@]+%.[^%s@]+$") then
            self.draft.Status = "Enter a valid email address before saving."
            return false
        end
        -- A production app passes this snapshot to its service here.
        self.committed = snapshot(self.draft)
        self.draft.Status = "Saved for this session."
        return true
    end,
    discardDraft = function(self)
        for key, value in pairs(self.committed) do self.draft[key] = value end
        self.draft.Status = "Unsaved changes discarded."
    end,
}
