-- icons.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 11.07.2014
-- source: E:\icons.spt (256x128)

texinfo = createTextureInfoEx('progress')
frames =
{
  { dx=33.0, dy=6.0, u=0, v=0.046875, cu=0.2578125, cv=0.09375, w=66.0, h=12.0 },
  { dx=33.0, dy=6.0, u=0.5664063, v=0.90625, cu=0.2578125, cv=0.09375, w=66.0, h=12.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_head')
frames =
{
  { dx=16.0, dy=14.0, u=0.5664063, v=0, cu=0.125, cv=0.21875, w=32.0, h=28.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_star')
frames =
{
  { dx=12.0, dy=13.0, u=0.8242188, v=0.796875, cu=0.09375, cv=0.203125, w=24.0, h=26.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_man')
frames =
{
  { dx=19.0, dy=13.0, u=0.4140625, v=0.1796875, cu=0.1523438, cv=0.1953125, w=39.0, h=25.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_firefighter')
frames =
{
  { dx=17.0, dy=14.0, u=0.5664063, v=0.21875, cu=0.1367188, cv=0.2109375, w=35.0, h=27.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_marauder')
frames =
{
  { dx=16.0, dy=12.0, u=0.6953125, v=0.71875, cu=0.125, cv=0.1875, w=32.0, h=24.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_medic')
frames =
{
  { dx=16.0, dy=18.0, u=0.5664063, v=0.6484375, cu=0.1289063, cv=0.2578125, w=33.0, h=33.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_police')
frames =
{
  { dx=16.0, dy=14.0, u=0.5664063, v=0.4296875, cu=0.1328125, cv=0.21875, w=34.0, h=28.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_repair')
frames =
{
  { dx=19.0, dy=21.0, u=0.4140625, v=0.6875, cu=0.1523438, cv=0.3125, w=39.0, h=40.0 },
  { dx=19.0, dy=21.0, u=0.4140625, v=0.375, cu=0.1523438, cv=0.3125, w=39.0, h=40.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_upgrade')
frames =
{
  { dx=13.0, dy=17.0, u=0.6992188, v=0.453125, cu=0.09375, cv=0.265625, w=24.0, h=34.0 },
  { dx=13.0, dy=17.0, u=0.703125, v=0.1875, cu=0.09375, cv=0.265625, w=24.0, h=34.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('target')
frames =
{
  { dx=11.0, dy=11.0, u=0.6914063, v=0.015625, cu=0.0859375, cv=0.171875, w=22.0, h=22.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 0)

texinfo = createTextureInfoEx('icon_exclamation')
frames =
{
  { dx=51.0, dy=3.0, u=0, v=0.5703125, cu=0.2070313, cv=0.4296875, w=53.0, h=55.0 },
  { dx=51.0, dy=3.0, u=0, v=0.140625, cu=0.2070313, cv=0.4296875, w=53.0, h=55.0 },
  { dx=51.0, dy=3.0, u=0.2070313, v=0.5703125, cu=0.2070313, cv=0.4296875, w=53.0, h=55.0 },
  { dx=51.0, dy=3.0, u=0.2070313, v=0.140625, cu=0.2070313, cv=0.4296875, w=53.0, h=55.0 },
}
addTextureInfoExNodes(texinfo, 'icons', frames, 5)

texinfo = nil
frames = nil
bounds = nil
points = nil
