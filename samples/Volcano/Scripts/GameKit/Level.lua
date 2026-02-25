local LevelLoader = require "GameKit.LevelLoader"
local sprites = require "Sprites"

local Level = SKScene:extend()

function Level:new()
  Level.super.new(self)
  self.workers = {}
  self.barriers = {}
  self.losers = {}
end

function Level:find_barrier()
end

function Level:find_worker(object)
  for _, worker in ipairs(self.workers) do
    local path = worker:make_path_to(object)
    if not worker.is_busy and #path > 0 then
      return worker
    end
  end
end

function Level:run_worker_to(object)
  local worker = self:find_worker(object)
  if object.marker then return end
  if worker then
    worker:run_to(object)
  end
  object.marker = sk.SpriteNode { animation = sprites.target }
  object:addChild(object.marker)
  return true
end

function Level:awake()
  LevelLoader.load(self, "Levels/level000.lua")
end

function make_level(args)
  return system.Object.new_with_class(Level, "SKScene", args)
end

return make_level
