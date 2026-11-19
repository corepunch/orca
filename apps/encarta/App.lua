local Application = require "orca.core.application"
local fs = require "orca.filesystem"
local Library = require "encarta/Scripts/Library"

return Application:extend {
    quitOnQ = false,
    layout = function(_, context) return context.slots.inner end,
    __init = function(self, openAudio)
        Application.__init(self)
        self.library = assert(fs.loadObject("encarta/Screens/Library"))
        self.libraryController = Library.mount(self.library, openAudio)
    end,
    run = function(self)
        local result = Application.run(self)
        self.libraryController.readAloud:stop()
        return result
    end,
    ["/"] = function(self) return self.library end,
    -- The 3D room planned as the home screen. Preview: orca apps/encarta -url=/study
    ["/study"] = function(self)
        self.study = self.study or assert(fs.loadObject("encarta/Screens/Study"))
        return self.study
    end,
}
