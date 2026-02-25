local app = require "AppKit"
local sprites = require "Sprites"

-- local node1 = sk.SpriteNode { x = 10 }
-- local node2 = sk.SpriteNode { x = 20 }
-- local node3 = sk.SpriteNode { x = 20 }

-- node1:addChild(node2)
-- node1:addChild(node3)

-- node1.mouseUp = function(self)
--   print(self.name)
-- end

-- node1:mouseUp()
-- print(#node1.children)

function createTextureInfoEx(name)
  return name
end

function addTextureInfoExNodes(anim, sprite, frames, framerate)
  if not sprites[sprite] then
    sprites[sprite] = {}
  end
	local image = FS:Image(string.format("Sprites/%s.png", sprite))
  sprites[anim] = renderer.SpriteAnimation.new(image, frames, framerate)
end

function addBoundInfoExNodes(texinfo, bounds)
end

function addPointsInfoExNodes(texinfo, points)
end

FS:Script "Sprites/achieves.lua"
FS:Script "Sprites/albatross.lua"
FS:Script "Sprites/back_ex0.lua"
FS:Script "Sprites/back_ex1.lua"
FS:Script "Sprites/back_ex2.lua"
FS:Script "Sprites/barrier.lua"
FS:Script "Sprites/boat.lua"
FS:Script "Sprites/briefing.lua"
FS:Script "Sprites/build0.lua"
FS:Script "Sprites/build1.lua"
FS:Script "Sprites/butterfly.lua"
FS:Script "Sprites/cats.lua"
FS:Script "Sprites/cloud.lua"
FS:Script "Sprites/cop0.lua"
FS:Script "Sprites/decor0.lua"
FS:Script "Sprites/dogs.lua"
FS:Script "Sprites/explosion.lua"
FS:Script "Sprites/firefighter0.lua"
FS:Script "Sprites/firework.lua"
FS:Script "Sprites/helicopter.lua"
FS:Script "Sprites/icons.lua"
FS:Script "Sprites/items.lua"
FS:Script "Sprites/lava0.lua"
FS:Script "Sprites/lava1.lua"
FS:Script "Sprites/man00.lua"
FS:Script "Sprites/man01.lua"
FS:Script "Sprites/man10.lua"
FS:Script "Sprites/man11.lua"
-- FS:Script "Sprites/map00.lua"
-- FS:Script "Sprites/map01.lua"
-- FS:Script "Sprites/map02.lua"
-- FS:Script "Sprites/map20.lua"
-- FS:Script "Sprites/map21.lua"
-- FS:Script "Sprites/map22.lua"
-- FS:Script "Sprites/map_decor0.lua"
-- FS:Script "Sprites/map_destr0.lua"
FS:Script "Sprites/marauder0.lua"
FS:Script "Sprites/medic0.lua"
-- FS:Script "Sprites/menu00.lua"
-- FS:Script "Sprites/menu01.lua"
-- FS:Script "Sprites/menu02.lua"
FS:Script "Sprites/offroad.lua"
FS:Script "Sprites/plane.lua"
FS:Script "Sprites/raven.lua"
FS:Script "Sprites/tech0.lua"
FS:Script "Sprites/train.lua"
FS:Script "Sprites/tukan.lua"
FS:Script "Sprites/water0.lua"
FS:Script "Sprites/water1.lua"
FS:Script "Sprites/water2.lua"
FS:Script "Sprites/water3.lua"
FS:Script "Sprites/wman00.lua"
FS:Script "Sprites/wman01.lua"
FS:Script "Sprites/wman10.lua"
FS:Script "Sprites/wman11.lua"
FS:Script "Sprites/worker0.lua"

FS:Script "Sprites/highlight.lua"

app.ready = function()
	app.keepAlive = true
end

app.step = function ()
end
