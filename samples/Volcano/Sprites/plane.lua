-- plane.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 05.07.2014
-- source: E:\plane.spt (256x256)

texinfo = createTextureInfoEx('plane')
frames =
{
  { dx=69.0, dy=18.0, u=0.5, v=0.6484375, cu=0.4179688, cv=0.3515625, w=107.0, h=90.0 },
  { dx=40.0, dy=21.0, u=0.5, v=0.4023438, cu=0.421875, cv=0.2460938, w=108.0, h=63.0 },
  { dx=70.0, dy=44.0, u=0, v=0.5898438, cu=0.5, cv=0.4101563, w=128.0, h=105.0 },
  { dx=61.0, dy=35.0, u=0, v=0.01171875, cu=0.5039063, cv=0.2109375, w=129.0, h=54.0 },
}
addTextureInfoExNodes(texinfo, 'plane', frames, 0)

texinfo = createTextureInfoEx('plane_shadow')
frames =
{
  { dx=81.0, dy=14.0, u=0, v=0.2226563, cu=0.4335938, cv=0.3671875, w=111.0, h=94.0 },
}
addTextureInfoExNodes(texinfo, 'plane', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
