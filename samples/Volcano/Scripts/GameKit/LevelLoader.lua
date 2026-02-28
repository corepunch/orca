local sk = require "orca.SpriteKit"
local fs = require "orca.filesystem"
local cg = require "orca.geometry"
local rn = require "orca.renderer"
-- local sprites = require "Scripts.Sprites"
local Worker = require "GameKit.Worker"
local Barrier = require "GameKit.Barrier"
local Decor = require "GameKit.Decor"
local Loser = require "GameKit.Loser"

local map = {
  [125] = "firefighter_idle0",
  [104] = "worker_wait0",
}

local function sprite(path)
  return "Volcano/Sprite Animations/"..path
end

local factory = {
  worker = function(level, x, y)
    local worker = Worker { Position = cg.Vector2D(x, y) }
    table.insert(level.workers, worker)
    return worker
  end,
  barrier = function(level, x, y, type, time)
    local animation
    if type == "tree00" or type == "stone00" then
      animation = sprite("bar_"..type)
    else
      animation = sprite(type)
    end
    local barrier = Barrier { Position = cg.Vector2D(x, y), Animation = animation, time = time, type = type }
    table.insert(level.barriers, barrier)
    return barrier
  end,
  base = function(level, x, y)
    local base = sk.SKSpriteNode {
      Animation = sprite("base0"),
      Position = cg.Vector2D(x, y),
      do_work = function(self, worker)
        worker.Visible = false
      end
    }
    level.base = base
    return base
  end,
  delivery_place = function(level, x, y)
    return sk.SKSpriteNode { Animation = sprite("delivery_place0"), Position = cg.Vector2D(x, y) }
  end,
  empty = function(level, x, y, objtype)

    if objtype == 113 then
      local worker = Worker { Position = cg.Vector2D(x, y), is_boat = true, Animation = sprite("boat0") }
      table.insert(level.workers, worker)
      return worker
    end

    if objtype == 104 then
      local worker = Worker { Position = cg.Vector2D(x, y) }
      table.insert(level.workers, worker)
      return worker
    end

    if objtype == 109 then
      local loser = Loser { Position = cg.Vector2D(x, y) }
      table.insert(level.losers, loser)
      return loser
    end

    local type = map[objtype]
    if type then
      local gameobj = sk.SKSpriteNode {
        Animation = sprite(type),
        Position = cg.Vector2D(x, y),
        onLeftMouseUp = function(self)
          level:run_worker_to(self)
        end
      }
      return gameobj
    end
    return sk.SKNode { Position = cg.Vector2D(x, y) }
  end,
  decor = function(level, x, y, anim, type)
    if type == 112 then
      return Decor { Animation = sprite(anim.."_br"), Position = cg.Vector2D(x, y) }
    end
    if anim == "burger" or anim == "sawmill" then
      return sk.SKSpriteNode { Animation = sprite(anim.."0"), Position = cg.Vector2D(x, y) }
    end
    if sprite(anim) then
      return sk.SKSpriteNode { Animation = sprite(anim), Position = cg.Vector2D(x, y) }
    else
      return sk.SKNode { Position = cg.Vector2D(x, y) }
    end
  end,
  background = function(level, name)
    -- return sk.SKSpriteNode { Image = rn.Image { Source = "Images/"..name..".jpg" } }
    return sk.SKSpriteNode { Image = "Volcano/Images/"..name..".jpg" }
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
      level:addChild(sk.SKSpriteNode { Position = cg.Vector2D(x, y), Opacity = 0.5 })
    end
    level.traffic = traffic
  end
}

local level_loader = {}

function level_loader.load(level, file)
  local objects = require(file)
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
