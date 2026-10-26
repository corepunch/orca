-- back_ex0.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 12.08.2015
-- source: C:\Users\Archer\Desktop\Проекты\rt5\src\tools\rt5\back_ex0.spt (1024x1024)

texinfo = createTextureInfoEx('airport')
frames =
{
  { dx=90.0, dy=55.0, u=0.3291016, v=0.2255859, cu=0.1796875, cv=0.1171875, w=184.0, h=120.0 },
}
addTextureInfoExNodes(texinfo, 'back_ex0', frames, 0)

texinfo = createTextureInfoEx('307_tunnel')
frames =
{
  { dx=167.0, dy=149.0, u=0, v=0.06445313, cu=0.3291016, cv=0.2783203, w=337.0, h=285.0 },
}
addTextureInfoExNodes(texinfo, 'back_ex0', frames, 0)

texinfo = createTextureInfoEx('103_tunnel')
frames =
{
  { dx=206.0, dy=195.0, u=0, v=0.3427734, cu=0.4023438, cv=0.3691406, w=412.0, h=378.0 },
}
addTextureInfoExNodes(texinfo, 'back_ex0', frames, 0)

texinfo = createTextureInfoEx('206_tunnel0')
frames =
{
  { dx=229.0, dy=147.0, u=0, v=0.7119141, cu=0.4472656, cv=0.2880859, w=458.0, h=295.0 },
}
addTextureInfoExNodes(texinfo, 'back_ex0', frames, 0)

texinfo = createTextureInfoEx('206_tunnel1')
frames =
{
  { dx=223.0, dy=226.0, u=0.4472656, v=0.5585938, cu=0.4365234, cv=0.4414063, w=447.0, h=452.0 },
}
addTextureInfoExNodes(texinfo, 'back_ex0', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
