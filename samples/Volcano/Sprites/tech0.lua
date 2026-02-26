-- tech0.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 18.06.2014
-- source: E:\tech0.spt (256x128)

texinfo = createTextureInfoEx('generator0')
frames =
{
  { dx=32.0, dy=24.0, u=0.4609375, v=0.5859375, cu=0.21875, cv=0.4140625, w=56.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'tech0', frames, 0)

texinfo = createTextureInfoEx('generator1')
frames =
{
  { dx=32.0, dy=24.0, u=0, v=0.53125, cu=0.2304688, cv=0.46875, w=59.0, h=60.0 },
}
addTextureInfoExNodes(texinfo, 'tech0', frames, 0)

texinfo = createTextureInfoEx('generator2')
frames =
{
  { dx=27.0, dy=18.0, u=0.6796875, v=0.109375, cu=0.1796875, cv=0.4296875, w=46.0, h=55.0 },
}
addTextureInfoExNodes(texinfo, 'tech0', frames, 0)

texinfo = createTextureInfoEx('generator3')
frames =
{
  { dx=35.0, dy=24.0, u=0, v=0.0625, cu=0.2304688, cv=0.46875, w=59.0, h=60.0 },
}
addTextureInfoExNodes(texinfo, 'tech0', frames, 0)

texinfo = createTextureInfoEx('generator4')
frames =
{
  { dx=32.0, dy=24.0, u=0.4609375, v=0.171875, cu=0.21875, cv=0.4140625, w=56.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'tech0', frames, 0)

texinfo = createTextureInfoEx('generator5')
frames =
{
  { dx=35.0, dy=25.0, u=0.2304688, v=0.53125, cu=0.2304688, cv=0.46875, w=59.0, h=60.0 },
}
addTextureInfoExNodes(texinfo, 'tech0', frames, 0)

texinfo = createTextureInfoEx('generator6')
frames =
{
  { dx=27.0, dy=23.0, u=0.6796875, v=0.5390625, cu=0.1796875, cv=0.4609375, w=46.0, h=59.0 },
}
addTextureInfoExNodes(texinfo, 'tech0', frames, 0)

texinfo = createTextureInfoEx('generator')
frames =
{
  { dx=32.0, dy=25.0, u=0.2304688, v=0.0625, cu=0.2304688, cv=0.46875, w=59.0, h=60.0 },
}
addTextureInfoExNodes(texinfo, 'tech0', frames, 0)
bounds =
{
  { x=-13.0, y=38.0 },
  { x=28.0, y=16.0 },
  { x=28.0, y=-15.0 },
  { x=4.0, y=-27.0 },
  { x=-33.0, y=-15.0 },
  { x=-33.0, y=27.0 },
}
addBoundInfoExNodes(texinfo, bounds)

texinfo = nil
frames = nil
bounds = nil
points = nil
