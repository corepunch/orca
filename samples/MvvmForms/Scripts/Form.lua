local Application = require "orca.core.application"

return {
    Save = function(self) Application.app:saveDraft() end,
    Cancel = function(self) Application.app:discardDraft() end,
}
