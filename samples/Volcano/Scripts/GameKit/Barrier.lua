local sk = require "orca.SpriteKit"
local cg = require "orca.geometry"
local sprites = require "Sprites"
local Barrier = sk.SKSpriteNode:extend()

function Barrier:new()
  Barrier.super.new(self)
  self.fixed = false
  self.will_be_fixed = false
end

function Barrier:onLeftMouseUp()
  if not self.will_be_fixed then
    self.scene:run_worker_to(self)
  end
  return true
end

function Barrier:do_work(worker)
  -- TODO: use orca animation system when available
  -- app.animations:append {
  --   duration = 5000,
  --   onComplete = function()
  --     if string.find(self.type, "bridge") == 1 then
  --       self.FreezeFrame = 3
  --     else
  --       self.Visible = false
  --     end
  --     self.fixed = true
  --     worker.has_load = true
  --     worker:run_to(self.scene.base)
  --   end
  -- }
  local barrier = cg.Vector2D(self.x, self.y)
  local direction = worker:get_direction(barrier)
  worker.is_busy = true
  worker.Animation = sprites[string.format("worker_srepair%s", direction)]
  if self.marker then self.marker.Visible = false end
end

return Barrier
