-- build1.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 07.05.2015
-- source: E:\build1.spt (512x512)

texinfo = createTextureInfoEx('railstation0_br')
frames =
{
  { dx=103.0, dy=46.0, u=0, v=0.7636719, cu=0.3710938, cv=0.2363281, w=190.0, h=121.0 },
  { dx=103.0, dy=46.0, u=0.3710938, v=0.7675781, cu=0.3710938, cv=0.2324219, w=190.0, h=119.0 },
  { dx=103.0, dy=47.0, u=0, v=0.5292969, cu=0.3710938, cv=0.234375, w=190.0, h=120.0 },
}
addTextureInfoExNodes(texinfo, 'build1', frames, 0)
bounds =
{
  { x=-74.0, y=-14.0 },
  { x=-74.0, y=39.0 },
  { x=-2.0, y=76.0 },
  { x=77.0, y=31.0 },
  { x=77.0, y=-6.0 },
  { x=1.0, y=-49.0 },
}
addBoundInfoExNodes(texinfo, bounds)
points =
{
  { r=255, g=0, b=0, x=40.0, y=-24.0 },
}
addPointsInfoExNodes(texinfo, points)

texinfo = createTextureInfoEx('railstation0')
frames =
{
  { dx=103.0, dy=47.0, u=0, v=0.2949219, cu=0.359375, cv=0.234375, w=184.0, h=120.0 },
}
addTextureInfoExNodes(texinfo, 'build1', frames, 0)
bounds =
{
  { x=-74.0, y=-14.0 },
  { x=-74.0, y=39.0 },
  { x=-2.0, y=76.0 },
  { x=77.0, y=31.0 },
  { x=77.0, y=-6.0 },
  { x=1.0, y=-49.0 },
}
addBoundInfoExNodes(texinfo, bounds)

texinfo = createTextureInfoEx('finish_base0')
frames =
{
  { dx=34.0, dy=9.0, u=0.2636719, v=0.1835938, cu=0.1152344, cv=0.1113281, w=59.0, h=57.0 },
}
addTextureInfoExNodes(texinfo, 'build1', frames, 0)

texinfo = createTextureInfoEx('delivery_place0')
frames =
{
  { dx=55.0, dy=51.0, u=0.3710938, v=0.4238281, cu=0.2167969, cv=0.1445313, w=111.0, h=74.0 },
}
addTextureInfoExNodes(texinfo, 'build1', frames, 0)
bounds =
{
  { x=5.0, y=-37.0 },
  { x=-27.0, y=-34.0 },
  { x=-49.0, y=-12.0 },
  { x=-42.0, y=16.0 },
  { x=3.0, y=29.0 },
  { x=49.0, y=8.0 },
  { x=58.0, y=-47.0 },
  { x=24.0, y=-51.0 },
}
addBoundInfoExNodes(texinfo, bounds)
points =
{
  { r=255, g=0, b=0, x=54.0, y=-20.0 },
}
addPointsInfoExNodes(texinfo, points)

texinfo = createTextureInfoEx('delivery_place_hl')
frames =
{
  { dx=39.0, dy=20.0, u=0.359375, v=0.3359375, cu=0.1484375, cv=0.08789063, w=76.0, h=45.0 },
  { dx=33.0, dy=25.0, u=0.2636719, v=0.09375, cu=0.1464844, cv=0.08984375, w=75.0, h=46.0 },
  { dx=39.0, dy=19.0, u=0, v=0.0078125, cu=0.1582031, cv=0.078125, w=81.0, h=40.0 },
  { dx=14.0, dy=23.0, u=0.6347656, v=0.6738281, cu=0.06054688, cv=0.09375, w=31.0, h=48.0 },
}
addTextureInfoExNodes(texinfo, 'build1', frames, 1)

texinfo = createTextureInfoEx('garage0')
frames =
{
  { dx=67.0, dy=51.0, u=0.3710938, v=0.5683594, cu=0.2636719, cv=0.1992188, w=135.0, h=102.0 },
}
addTextureInfoExNodes(texinfo, 'build1', frames, 0)

texinfo = createTextureInfoEx('garage0_br')
frames =
{
  { dx=67.0, dy=56.0, u=0, v=0.0859375, cu=0.2636719, cv=0.2089844, w=135.0, h=107.0 },
  { dx=67.0, dy=56.0, u=0.7421875, v=0.7910156, cu=0.2578125, cv=0.2089844, w=132.0, h=107.0 },
  { dx=67.0, dy=55.0, u=0.7421875, v=0.5839844, cu=0.2578125, cv=0.2070313, w=132.0, h=106.0 },
}
addTextureInfoExNodes(texinfo, 'build1', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
