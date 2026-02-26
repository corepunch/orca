local sk = require "orca.SpriteKit"
local fs = require "orca.filesystem"
local cg = require "orca.geometry"
local sprites = require "Sprites"
local Worker = require "GameKit.Worker"
local Barrier = require "GameKit.Barrier"
local Decor = require "GameKit.Decor"
local Loser = require "GameKit.Loser"

local map = {
  [125] = "firefighter_idle0",
  [104] = "worker_wait0",
}

local factory = {
  worker = function(level, x, y)
    local worker = Worker { x = x, y = y }
    table.insert(level.workers, worker)
    return worker
  end,
  barrier = function(level, x, y, type, time)
    local animation
    if type == "tree00" or type == "stone00" then
      animation = sprites["bar_"..type]
    else
      animation = sprites[type]
    end
    local barrier = Barrier { x = x, y = y, Animation = animation, time = time, type = type }
    table.insert(level.barriers, barrier)
    return barrier
  end,
  base = function(level, x, y)
    local base = sk.SKSpriteNode {
      Animation = sprites.base0,
      x = x,
      y = y,
      do_work = function(self, worker)
        worker.Visible = false
      end
    }
    level.base = base
    return base
  end,
  delivery_place = function(level, x, y)
    return sk.SKSpriteNode { Animation = sprites.delivery_place0, x = x, y = y }
  end,
  empty = function(level, x, y, objtype)

    if objtype == 113 then
      local worker = Worker { x = x, y = y, is_boat = true, Animation = sprites.boat0 }
      table.insert(level.workers, worker)
      return worker
    end

    if objtype == 104 then
      local worker = Worker { x = x, y = y }
      table.insert(level.workers, worker)
      return worker
    end

    if objtype == 109 then
      local loser = Loser { x = x, y = y }
      table.insert(level.losers, loser)
      return loser
    end

    local type = map[objtype]
    if type then
      local gameobj = sk.SKSpriteNode {
        Animation = sprites[type],
        x = x,
        y = y,
        onLeftMouseUp = function(self)
          level:run_worker_to(self)
        end
      }
      return gameobj
    end
    return sk.SKNode { x = x, y = y }
  end,
  decor = function(level, x, y, anim, type)
    if type == 112 then
      return Decor { Animation = sprites[anim.."_br"], x = x, y = y }
    end
    if anim == "burger" or anim == "sawmill" then
      return sk.SKSpriteNode { Animation = sprites[anim.."0"], x = x, y = y }
    end
    if sprites[anim] then
      return sk.SKSpriteNode { Animation = sprites[anim], x = x, y = y }
    else
      return sk.SKNode { x = x, y = y }
    end
  end,
  background = function(level, name)
    return sk.SKSpriteNode { Image = fs:Image("Images/"..name..".jpg") }
  end,
  resource = function(level, x, y, anim)
  end,
  traffic = function(level, points)
    local index = 1
    local traffic = {}
    while index <= #points do
      local x = points[index]
      local y = points[index + 1]
      traffic[#traffic + 1] = cg.Vector2D(x, y)
      index = index + 2
      level:addChild(sk.SKSpriteNode { x = x, y = y, Opacity = 0.5 })
    end
    level.traffic = traffic
  end
}

local level_loader = {}

function level_loader.load(level, file)
  local objects = fs:Script(file)
  for _, object in ipairs(objects) do
    local construct = factory[object[1]]
    object[1] = level
    if construct then
      local node = construct(table.unpack(object))
      if node then
        level:addChild(node)
      end
    end
  end
end

return level_loader
