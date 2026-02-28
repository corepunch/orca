local sk = require "orca.SpriteKit"
local sprites = require "Scripts.Sprites"
local Loser = sk.SKSpriteNode:extend()

function Loser:new()
  Loser.super.new(self)
  self.Animation = sprites.man_idle0
  self.Animation2 = sprites.man_shadow
end

function Loser:onLeftMouseUp()
  self.scene:run_worker_to(self)
  return true
end

function Loser:do_work(worker)
end

return Loser
