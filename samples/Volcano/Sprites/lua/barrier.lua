-- barrier.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 22.08.2014
-- source: E:\barrier.spt (512x512)

texinfo = createTextureInfoEx('bar_stone00')
frames =
{
  { dx=31.0, dy=23.0, u=0.3203125, v=0.3359375, cu=0.1210938, cv=0.09960938, w=62.0, h=51.0 },
  { dx=31.0, dy=23.0, u=0.375, v=0.0625, cu=0.1210938, cv=0.09179688, w=62.0, h=47.0 },
  { dx=27.0, dy=23.0, u=0.6054688, v=0.1972656, cu=0.1035156, cv=0.08203125, w=53.0, h=42.0 },
  { dx=27.0, dy=18.0, u=0.6757813, v=0.8046875, cu=0.09960938, cv=0.06640625, w=51.0, h=34.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('bar_stone_big00')
frames =
{
  { dx=45.0, dy=35.0, u=0, v=0.6582031, cu=0.1738281, cv=0.1503906, w=89.0, h=77.0 },
  { dx=40.0, dy=29.0, u=0.1289063, v=0.2578125, cu=0.1679688, cv=0.1308594, w=86.0, h=67.0 },
  { dx=36.0, dy=32.0, u=0.7753906, v=0.7460938, cu=0.1542969, cv=0.1289063, w=79.0, h=66.0 },
  { dx=24.0, dy=28.0, u=0.4960938, v=0.05078125, cu=0.1230469, cv=0.1035156, w=63.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('stone_dynamite')
frames =
{
  { dx=56.0, dy=47.0, u=0, v=0.8085938, cu=0.1914063, cv=0.1914063, w=98.0, h=98.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('bridge_auto00')
frames =
{
  { dx=49.0, dy=38.0, u=0.1914063, v=0.8710938, cu=0.1953125, cv=0.1289063, w=100.0, h=66.0 },
  { dx=49.0, dy=38.0, u=0.3867188, v=0.8730469, cu=0.1953125, cv=0.1269531, w=100.0, h=65.0 },
  { dx=49.0, dy=39.0, u=0.5820313, v=0.8710938, cu=0.1933594, cv=0.1289063, w=99.0, h=66.0 },
  { dx=49.0, dy=38.0, u=0.7753906, v=0.875, cu=0.1933594, cv=0.125, w=99.0, h=64.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('bridge_wood00')
frames =
{
  { dx=48.0, dy=36.0, u=0.1914063, v=0.7441406, cu=0.1894531, cv=0.1269531, w=97.0, h=65.0 },
  { dx=48.0, dy=33.0, u=0.1738281, v=0.6230469, cu=0.1894531, cv=0.1210938, w=97.0, h=62.0 },
  { dx=48.0, dy=33.0, u=0.1308594, v=0.5058594, cu=0.1894531, cv=0.1171875, w=97.0, h=60.0 },
  { dx=48.0, dy=33.0, u=0.1308594, v=0.3886719, cu=0.1894531, cv=0.1171875, w=97.0, h=60.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('bridge_auto01')
frames =
{
  { dx=28.0, dy=48.0, u=0, v=0.4707031, cu=0.1308594, cv=0.1875, w=67.0, h=96.0 },
  { dx=30.0, dy=48.0, u=0, v=0.2832031, cu=0.1289063, cv=0.1875, w=66.0, h=96.0 },
  { dx=29.0, dy=48.0, u=0, v=0.09570313, cu=0.1191406, cv=0.1875, w=61.0, h=96.0 },
  { dx=28.0, dy=48.0, u=0.3203125, v=0.4355469, cu=0.1132813, cv=0.1875, w=58.0, h=96.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('bridge_wood01')
frames =
{
  { dx=26.0, dy=46.0, u=0.265625, v=0.078125, cu=0.109375, cv=0.1796875, w=56.0, h=92.0 },
  { dx=29.0, dy=46.0, u=0.375, v=0.1542969, cu=0.1171875, cv=0.1816406, w=60.0, h=93.0 },
  { dx=27.0, dy=46.0, u=0.4335938, v=0.4785156, cu=0.109375, cv=0.1816406, w=56.0, h=93.0 },
  { dx=26.0, dy=46.0, u=0.5429688, v=0.4902344, cu=0.1015625, cv=0.1796875, w=52.0, h=92.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('bar_tree00')
frames =
{
  { dx=38.0, dy=23.0, u=0.7753906, v=0.6464844, cu=0.1328125, cv=0.09960938, w=68.0, h=51.0 },
  { dx=35.0, dy=23.0, u=0.6191406, v=0.01171875, cu=0.1210938, cv=0.08203125, w=62.0, h=42.0 },
  { dx=34.0, dy=20.0, u=0, v=0.0234375, cu=0.1191406, cv=0.07226563, w=61.0, h=37.0 },
  { dx=30.0, dy=18.0, u=0.4921875, v=0.1542969, cu=0.1035156, cv=0.06445313, w=53.0, h=33.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('meteor0')
frames =
{
  { dx=26.0, dy=28.0, u=0.578125, v=0.3847656, cu=0.1015625, cv=0.1054688, w=52.0, h=54.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('meteor1')
frames =
{
  { dx=26.0, dy=27.0, u=0.6054688, v=0.2792969, cu=0.1015625, cv=0.1054688, w=52.0, h=54.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('swarm0')
frames =
{
  { dx=28.0, dy=28.0, u=0.4921875, v=0.21875, cu=0.1132813, cv=0.1445313, w=58.0, h=74.0 },
  { dx=27.0, dy=25.0, u=0.6445313, v=0.6015625, cu=0.109375, cv=0.1015625, w=56.0, h=52.0 },
  { dx=27.0, dy=24.0, u=0.6445313, v=0.5039063, cu=0.109375, cv=0.09765625, w=56.0, h=50.0 },
  { dx=17.0, dy=18.0, u=0.296875, v=0.2597656, cu=0.078125, cv=0.07617188, w=40.0, h=39.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('oil_slick')
frames =
{
  { dx=37.0, dy=35.0, u=0.1191406, v=0.1191406, cu=0.1464844, cv=0.1386719, w=75.0, h=71.0 },
  { dx=31.0, dy=28.0, u=0.5390625, v=0.7558594, cu=0.1367188, cv=0.1152344, w=70.0, h=59.0 },
  { dx=31.0, dy=28.0, u=0.4414063, v=0.3632813, cu=0.1367188, cv=0.1152344, w=70.0, h=59.0 },
  { dx=17.0, dy=31.0, u=0.6191406, v=0.09375, cu=0.1015625, cv=0.1035156, w=52.0, h=53.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('broken_road0')
frames =
{
  { dx=24.0, dy=26.0, u=0.6796875, v=0.703125, cu=0.09570313, cv=0.1015625, w=49.0, h=52.0 },
  { dx=19.0, dy=21.0, u=0.6796875, v=0.4238281, cu=0.07226563, cv=0.08007813, w=37.0, h=41.0 },
  { dx=14.0, dy=16.0, u=0.7539063, v=0.5, cu=0.04882813, cv=0.06054688, w=25.0, h=31.0 },
  { dx=11.0, dy=12.0, u=0.2304688, v=0.07421875, cu=0.03515625, cv=0.04492188, w=18.0, h=23.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('broken_road1')
frames =
{
  { dx=38.0, dy=28.0, u=0.3867188, v=0.7617188, cu=0.1523438, cv=0.1113281, w=78.0, h=57.0 },
  { dx=34.0, dy=26.0, u=0.3808594, v=0.6601563, cu=0.1386719, cv=0.1015625, w=71.0, h=52.0 },
  { dx=30.0, dy=21.0, u=0.5195313, v=0.6699219, cu=0.1210938, cv=0.0859375, w=62.0, h=44.0 },
  { dx=21.0, dy=15.0, u=0.2304688, v=0, cu=0.0859375, cv=0.0625, w=44.0, h=32.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('broken_road2')
frames =
{
  { dx=30.0, dy=32.0, u=0.1191406, v=0, cu=0.1113281, cv=0.1191406, w=57.0, h=61.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = createTextureInfoEx('bonfire')
frames =
{
  { dx=29.0, dy=23.0, u=0.7539063, v=0.5605469, cu=0.1113281, cv=0.0859375, w=57.0, h=44.0 },
}
addTextureInfoExNodes(texinfo, 'barrier', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
