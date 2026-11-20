local Application = require "orca.core.application"
local fs = require "orca.filesystem"
local Library = require "encarta/Scripts/Library"

return Application:extend {
    quitOnQ = false,
    layout = function(_, context) return context.slots.inner end,
    __init = function(self, openAudio, passportPath)
        Application.__init(self)
        self.library = assert(fs.loadObject("encarta/Screens/Library"))
        local home = assert(fs.loadObject("encarta/Prefabs/Study"))
        home:addChild(assert(fs.loadObject("encarta/Prefabs/RoomPicker")))
        self.library:findChild("LibraryRoot"):addChild(home)
        self.libraryController = Library.mount(self.library, openAudio, passportPath)
    end,
    run = function(self)
        local result = Application.run(self)
        self.libraryController.readAloud:stop()
        return result
    end,
    ["/"] = function(self) return self.library end,
    ["/study"] = function(self)
        self.libraryController:showStudy()
        self.libraryController.study:selectRoom("Original")
        return self.library
    end,
}
