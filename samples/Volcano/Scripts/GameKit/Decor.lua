local sk = require "orca.SpriteKit"
local sprites = require "Scripts.Sprites"
local Decor = sk.SKSpriteNode:extend()

function Decor:new()
  Decor.super.new(self)
end

return Decor
