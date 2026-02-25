local app = require "AppKit"
local sprites = require "Sprites"
local find_path = require "GameKit.pathfinding"
local Worker = SKNode:extend()

function Worker:new()
  Worker.super.new(self)
  self.is_boat = false
  self.is_busy = false
  self.has_load = false
  self.animation2 = sprites.worker_shadow
	self.animation = sprites.worker_wait0
end

function Worker:get_direction(point)
	local PI = 3.14159
	local angle = math.atan2(self.y - point.y, self.x - point.x) + PI
  if self.is_boat then
    return math.floor(((12 * angle + 1.5) / PI) % 24)
  else
    return math.floor(((4 * angle + 1.5) / PI) % 8)
  end
end

function Worker:run_progress(on_arrived)
  local point = cg.Vector2.new(self.path[1].x, self.path[1].y)
	local direction = self:get_direction(point)
  local distance = (point - cg.Vector2.new(self.x, self.y)).length
  local fmt
  if not self.is_boat then
    fmt = self.has_load and "worker_bag%d" or "worker_run%d"
  else
    fmt = "boat%d"
  end
  self.animation = sprites[string.format(fmt, direction)]
  self.is_busy = true
  app.animations:append {
    ipo = "sine",
    easing = "out",
    duration = distance * 5,
    animations = {
      { self, "layoutTransform", cg.Transform2.new { translation = point } },
    },
    onComplete = function()
      if #self.path > 0 then
        self:run_progress(on_arrived)
      else
        self.has_load = false
        self.is_busy = false
        if not self.is_boat then
          self.animation =  sprites.worker_wait0
        end
        if on_arrived then on_arrived(self) end
      end
    end
  }
  table.remove(self.path, 1)
end

function Worker:make_path_to(target)
  return find_path(self, target, self.scene.traffic)
end

function Worker:run_to(target, _path)
  local path = _path or self:make_path_to(target)
  local callback = function(worker) target:do_work(worker) end
  if #path == 0 then return end
  self.path = path
  self.visible = true
  target.will_be_fixed = true
	self:run_progress(target.do_work and callback)
end

return function(args)
  return system.Object.new_with_class(Worker, "SKSpriteNode", args)
end
