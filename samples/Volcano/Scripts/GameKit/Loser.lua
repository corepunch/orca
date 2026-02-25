local app = require "AppKit"
local sprites = require "Sprites"
local Loser = SKNode:extend()

function Loser:new()
  Loser.super.new(self)
	self.animation = sprites.man_idle0
	self.animation2 = sprites.man_shadow
end

function Loser:mouseReleased()
  self.scene:run_worker_to(self)
	return true
end

function Loser:do_work(worker)
  -- app.animations:append {
  --   duration = 5000,
  --   onComplete = function()
  --     if string.find(self.type, "bridge") == 1 then
  --       self.freezeFrame = 3
  --     else
  --       self.visible = false
  --     end
  --     self.fixed = true
  --     worker.has_load = true
  --     worker:run_to(self.scene.base)
  --   end
  -- }
  -- local Loser = cg.Vector2.new(self.x, self.y)
  -- local direction = worker:get_direction(Loser)
  -- worker.is_busy = true
  -- worker.animation = sprites[string.format("worker_srepair%s", direction)]
  -- if self.marker then self.marker.visible = false end
end

function make_Loser(args)
  return system.Object.new_with_class(Loser, "SKSpriteNode", args)
end

return make_Loser
