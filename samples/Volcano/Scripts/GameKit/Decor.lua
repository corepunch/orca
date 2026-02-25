local app = require "AppKit"
local sprites = require "Sprites"
local Decor = SKNode:extend()

function Decor:new()
  Decor.super.new(self)
  -- self.fixed = false
  -- self.will_be_fixed = false
end

-- function Decor:mouseReleased()
--   if not self.will_be_fixed then
--     self.scene:run_worker_to(self)
--   end
--   return true
-- end

-- function Decor:do_work(worker)
--   app.animations:append {
--     duration = 5000,
--     onComplete = function()
--       if string.find(self.type, "bridge") == 1 then
--         self.freezeFrame = 3
--       else
--         self.visible = false
--       end
--       self.fixed = true
--       worker.has_load = true
--       worker:run_to(self.scene.base)
--     end
--   }
--   local Decor = cg.Vector2.new(self.x, self.y)
--   local direction = worker:get_direction(Decor)
--   worker.is_busy = true
--   worker.animation = sprites[string.format("worker_srepair%s", direction)]
--   if self.marker then self.marker.visible = false end
-- end

function make_Decor(args)
  return system.Object.new_with_class(Decor, "SKSpriteNode", args)
end

return make_Decor
