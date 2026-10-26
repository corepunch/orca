-- mapXX.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 13.07.2015
-- source: NONE (1024x768)

texinfo = createTextureInfoEx('map00')
frames =
{
  { dx=512.0, dy=384.0, u=0, v=0, cu=1, cv=1, w=1024.0, h=768.0 },
}
addTextureInfoExNodes(texinfo, 'map00', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
