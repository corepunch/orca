local sk = require "orca.SpriteKit"
local fs = require "orca.filesystem"
local sprites = require "Scripts.Sprites"

function createTextureInfoEx(name)
  return name
end

function addTextureInfoExNodes(anim, sprite, frames, framerate)
  if not sprites[sprite] then
    sprites[sprite] = {}
  end
  local image = fs:Image(string.format("Sprites/%s.png", sprite))
  sprites[anim] = sk.createAnimation(image, frames, framerate)
end

function addBoundInfoExNodes(texinfo, bounds)
end

function addPointsInfoExNodes(texinfo, points)
end

fs:Script "Sprites/achieves.lua"
fs:Script "Sprites/albatross.lua"
fs:Script "Sprites/back_ex0.lua"
fs:Script "Sprites/back_ex1.lua"
fs:Script "Sprites/back_ex2.lua"
fs:Script "Sprites/barrier.lua"
fs:Script "Sprites/boat.lua"
fs:Script "Sprites/briefing.lua"
fs:Script "Sprites/build0.lua"
fs:Script "Sprites/build1.lua"
fs:Script "Sprites/butterfly.lua"
fs:Script "Sprites/cats.lua"
fs:Script "Sprites/cloud.lua"
fs:Script "Sprites/cop0.lua"
fs:Script "Sprites/decor0.lua"
fs:Script "Sprites/dogs.lua"
fs:Script "Sprites/explosion.lua"
fs:Script "Sprites/firefighter0.lua"
fs:Script "Sprites/firework.lua"
fs:Script "Sprites/helicopter.lua"
fs:Script "Sprites/icons.lua"
fs:Script "Sprites/items.lua"
fs:Script "Sprites/lava0.lua"
fs:Script "Sprites/lava1.lua"
fs:Script "Sprites/man00.lua"
fs:Script "Sprites/man01.lua"
fs:Script "Sprites/man10.lua"
fs:Script "Sprites/man11.lua"
-- fs:Script "Sprites/map00.lua"
-- fs:Script "Sprites/map01.lua"
-- fs:Script "Sprites/map02.lua"
-- fs:Script "Sprites/map20.lua"
-- fs:Script "Sprites/map21.lua"
-- fs:Script "Sprites/map22.lua"
-- fs:Script "Sprites/map_decor0.lua"
-- fs:Script "Sprites/map_destr0.lua"
fs:Script "Sprites/marauder0.lua"
fs:Script "Sprites/medic0.lua"
-- fs:Script "Sprites/menu00.lua"
-- fs:Script "Sprites/menu01.lua"
-- fs:Script "Sprites/menu02.lua"
fs:Script "Sprites/offroad.lua"
fs:Script "Sprites/plane.lua"
fs:Script "Sprites/raven.lua"
fs:Script "Sprites/tech0.lua"
fs:Script "Sprites/train.lua"
fs:Script "Sprites/tukan.lua"
fs:Script "Sprites/water0.lua"
fs:Script "Sprites/water1.lua"
fs:Script "Sprites/water2.lua"
fs:Script "Sprites/water3.lua"
fs:Script "Sprites/wman00.lua"
fs:Script "Sprites/wman01.lua"
fs:Script "Sprites/wman10.lua"
fs:Script "Sprites/wman11.lua"
fs:Script "Sprites/worker0.lua"

fs:Script "Sprites/highlight.lua"
