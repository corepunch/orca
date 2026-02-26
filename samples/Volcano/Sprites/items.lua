-- items.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 02.06.2015
-- source: E:\items.spt (256x256)

texinfo = createTextureInfoEx('food_big')
frames =
{
  { dx=18.0, dy=15.0, u=0.7695313, v=0.6171875, cu=0.140625, cv=0.1757813, w=36.0, h=45.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('food')
frames =
{
  { dx=18.0, dy=12.0, u=0.4648438, v=0.06640625, cu=0.1367188, cv=0.1328125, w=35.0, h=34.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('wood_big')
frames =
{
  { dx=26.0, dy=20.0, u=0.2929688, v=0.2109375, cu=0.203125, cv=0.1679688, w=52.0, h=43.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)
bounds =
{
  { x=-22.0, y=6.0 },
  { x=13.0, y=24.0 },
  { x=27.0, y=8.0 },
  { x=27.0, y=-6.0 },
  { x=2.0, y=-20.0 },
  { x=-21.0, y=-8.0 },
}
addBoundInfoExNodes(texinfo, bounds)

texinfo = createTextureInfoEx('wood')
frames =
{
  { dx=22.0, dy=16.0, u=0, v=0.02734375, cu=0.1640625, cv=0.1367188, w=42.0, h=35.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)
bounds =
{
  { x=-17.0, y=6.0 },
  { x=8.0, y=19.0 },
  { x=19.0, y=11.0 },
  { x=19.0, y=-1.0 },
  { x=-5.0, y=-14.0 },
  { x=-17.0, y=-9.0 },
}
addBoundInfoExNodes(texinfo, bounds)

texinfo = createTextureInfoEx('gas')
frames =
{
  { dx=16.0, dy=11.0, u=0.8789063, v=0.8671875, cu=0.1171875, cv=0.1328125, w=30.0, h=34.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('gas_big')
frames =
{
  { dx=19.0, dy=13.0, u=0.1640625, v=0.015625, cu=0.1289063, cv=0.1484375, w=33.0, h=38.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('money_big')
frames =
{
  { dx=23.0, dy=20.0, u=0.2929688, v=0.04296875, cu=0.171875, cv=0.1679688, w=44.0, h=43.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)
bounds =
{
  { x=-18.0, y=10.0 },
  { x=7.0, y=22.0 },
  { x=19.0, y=16.0 },
  { x=20.0, y=-5.0 },
  { x=-3.0, y=-18.0 },
  { x=-18.0, y=-10.0 },
}
addBoundInfoExNodes(texinfo, bounds)

texinfo = createTextureInfoEx('money')
frames =
{
  { dx=22.0, dy=15.0, u=0.5859375, v=0.2304688, cu=0.1640625, cv=0.1210938, w=42.0, h=31.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)
bounds =
{
  { x=-17.0, y=4.0 },
  { x=7.0, y=16.0 },
  { x=18.0, y=9.0 },
  { x=18.0, y=0.0 },
  { x=-6.0, y=-13.0 },
  { x=-19.0, y=-7.0 },
}
addBoundInfoExNodes(texinfo, bounds)

texinfo = createTextureInfoEx('item_spark')
frames =
{
  { dx=8.0, dy=10.0, u=0.8789063, v=0.796875, cu=0.07421875, cv=0.0703125, w=19.0, h=18.0 },
  { dx=10.0, dy=13.0, u=0.4960938, v=0.2851563, cu=0.08984375, cv=0.09375, w=23.0, h=24.0 },
  { dx=13.0, dy=17.0, u=0.6015625, v=0.1054688, cu=0.1132813, cv=0.125, w=29.0, h=32.0 },
  { dx=13.0, dy=16.0, u=0.78125, v=0.4179688, cu=0.1132813, cv=0.1210938, w=29.0, h=31.0 },
  { dx=12.0, dy=16.0, u=0.78125, v=0.3007813, cu=0.1054688, cv=0.1171875, w=27.0, h=30.0 },
  { dx=10.0, dy=12.0, u=0.4960938, v=0.1992188, cu=0.08984375, cv=0.0859375, w=23.0, h=22.0 },
  { dx=9.0, dy=11.0, u=0.7695313, v=0.5390625, cu=0.08203125, cv=0.078125, w=21.0, h=20.0 },
  { dx=7.0, dy=9.0, u=0.8515625, v=0.5546875, cu=0.06640625, cv=0.0625, w=17.0, h=16.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 10)

texinfo = createTextureInfoEx('dynamite')
frames =
{
  { dx=25.0, dy=20.0, u=0.5859375, v=0.3515625, cu=0.1953125, cv=0.1679688, w=50.0, h=43.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 30)

texinfo = createTextureInfoEx('bonus_fast_boat')
frames =
{
  { dx=37.0, dy=27.0, u=0.2929688, v=0.7929688, cu=0.2929688, cv=0.2070313, w=75.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('bonus_fast_helicopter')
frames =
{
  { dx=37.0, dy=27.0, u=0, v=0.7890625, cu=0.2929688, cv=0.2109375, w=75.0, h=54.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('bonus_fast_people')
frames =
{
  { dx=37.0, dy=27.0, u=0, v=0.3710938, cu=0.2929688, cv=0.2070313, w=75.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('bonus_fast_money')
frames =
{
  { dx=37.0, dy=27.0, u=0.2929688, v=0.5859375, cu=0.2929688, cv=0.2070313, w=75.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('bonus_fast_food')
frames =
{
  { dx=37.0, dy=27.0, u=0.5859375, v=0.7929688, cu=0.2929688, cv=0.2070313, w=75.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('bonus_fast_wood')
frames =
{
  { dx=37.0, dy=27.0, u=0, v=0.1640625, cu=0.2929688, cv=0.2070313, w=75.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('bonus_fast_gas')
frames =
{
  { dx=37.0, dy=28.0, u=0, v=0.578125, cu=0.2929688, cv=0.2109375, w=75.0, h=54.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('bonus_fast_offroader')
frames =
{
  { dx=37.0, dy=27.0, u=0.2929688, v=0.3789063, cu=0.2929688, cv=0.2070313, w=75.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = createTextureInfoEx('delivery_package')
frames =
{
  { dx=22.0, dy=58.0, u=0.5859375, v=0.5195313, cu=0.1835938, cv=0.2734375, w=47.0, h=70.0 },
}
addTextureInfoExNodes(texinfo, 'items', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
