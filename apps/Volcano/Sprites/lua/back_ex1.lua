-- back_ex1.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 27.06.2015
-- source: C:\Users\atish_000\Desktop\Проекты\rt5\src\tools\rt5\back_ex1.spt (1024x1024)

texinfo = createTextureInfoEx('407_tunnel')
frames =
{
  { dx=348.0, dy=245.0, u=0, v=0.1132813, cu=0.6796875, cv=0.4794922, w=696.0, h=491.0 },
}
addTextureInfoExNodes(texinfo, 'back_ex1', frames, 0)

texinfo = createTextureInfoEx('209_tunnel')
frames =
{
  { dx=399.0, dy=208.0, u=0, v=0.5927734, cu=0.7783203, cv=0.4072266, w=797.0, h=417.0 },
}
addTextureInfoExNodes(texinfo, 'back_ex1', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
