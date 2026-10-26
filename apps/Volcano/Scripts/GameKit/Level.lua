local sk = require "orca.SpriteKit"
local LevelLoader = require "GameKit.LevelLoader"
local sprites = require "Scripts.Sprites"

local Level = sk.SKScene:extend {
  __init = function(self, ...)
    self.__class.__parent.__init(self, ...)
    self.workers = {}
    self.barriers = {}
    self.losers = {}
  end,

  find_barrier = function(self)
  end,

  find_worker = function(self, object)
    for _, worker in ipairs(self.workers) do
      local path = worker:make_path_to(object)
      if not worker.is_busy and #path > 0 then
        return worker
      end
    end
  end,

  run_worker_to = function(self, object)
    local worker = self:find_worker(object)
    if object.marker then return end
    if worker then
      worker:run_to(object)
    end
    object.marker = sk.SKSpriteNode { Animation = sprites.target }
    object:addChild(object.marker)
    return true
  end,

  onAwake = function(self)
    print("Level awake")
    LevelLoader.load(self, "Volcano/Levels/level000")
  end

}

return Level
