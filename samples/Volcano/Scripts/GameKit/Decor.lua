local sk = require "orca.SpriteKit"
local sprites = require "Sprites"
local Decor = sk.SKSpriteNode:extend()

function Decor:new()
  Decor.super.new(self)
end

return Decor
