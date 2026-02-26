-- boat.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 25.06.2014
-- source: E:\boat.spt (512x512)

texinfo = createTextureInfoEx('boat_fish_idle')
frames =
{
  { dx=11.0, dy=6.0, u=0.4335938, v=0.6289063, cu=0.03710938, cv=0.02148438, w=19.0, h=11.0 },
  { dx=14.0, dy=7.0, u=0.09375, v=0.005859375, cu=0.05273438, cv=0.02539063, w=27.0, h=13.0 },
  { dx=18.0, dy=13.0, u=0.4589844, v=0.6738281, cu=0.06640625, cv=0.0390625, w=34.0, h=20.0 },
  { dx=22.0, dy=10.0, u=0.5253906, v=0.6816406, cu=0.08203125, cv=0.03710938, w=42.0, h=19.0 },
  { dx=26.0, dy=10.0, u=0.3203125, v=0.5761719, cu=0.09765625, cv=0.03710938, w=50.0, h=19.0 },
  { dx=30.0, dy=10.0, u=0.3203125, v=0.6132813, cu=0.1132813, cv=0.03710938, w=58.0, h=19.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 5)

texinfo = createTextureInfoEx('boat_fish_action0')
frames =
{
  { dx=-1.0, dy=16.0, u=0.4335938, v=0.6152344, cu=0.03515625, cv=0.01367188, w=18.0, h=7.0 },
  { dx=6.0, dy=17.0, u=0.4589844, v=0.6523438, cu=0.0625, cv=0.02148438, w=32.0, h=11.0 },
  { dx=5.0, dy=17.0, u=0.234375, v=0.005859375, cu=0.05859375, cv=0.02929688, w=30.0, h=15.0 },
  { dx=5.0, dy=16.0, u=0.6074219, v=0.6796875, cu=0.05273438, cv=0.0390625, w=27.0, h=20.0 },
  { dx=6.0, dy=14.0, u=0.390625, v=0.1582031, cu=0.05273438, cv=0.046875, w=27.0, h=24.0 },
  { dx=7.0, dy=13.0, u=0.390625, v=0.109375, cu=0.046875, cv=0.04882813, w=24.0, h=25.0 },
  { dx=14.0, dy=-2.0, u=0.703125, v=0.6757813, cu=0.04296875, cv=0.0234375, w=22.0, h=12.0 },
  { dx=16.0, dy=-1.0, u=0.390625, v=0, cu=0.04296875, cv=0.02539063, w=22.0, h=13.0 },
  { dx=17.0, dy=-1.0, u=0.3515625, v=0, cu=0.0390625, cv=0.02929688, w=20.0, h=15.0 },
  { dx=19.0, dy=4.0, u=0.2050781, v=0, cu=0.02929688, cv=0.03515625, w=15.0, h=18.0 },
  { dx=25.0, dy=10.0, u=0.3730469, v=0.5371094, cu=0.04492188, cv=0.0390625, w=23.0, h=20.0 },
  { dx=24.0, dy=15.0, u=0.390625, v=0.06445313, cu=0.0390625, cv=0.04492188, w=20.0, h=23.0 },
  { dx=25.0, dy=17.0, u=0.6601563, v=0.6796875, cu=0.04296875, cv=0.0390625, w=22.0, h=20.0 },
  { dx=25.0, dy=17.0, u=0.390625, v=0.02539063, cu=0.04296875, cv=0.0390625, w=22.0, h=20.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 10)

texinfo = createTextureInfoEx('boat_fish_action1')
frames =
{
  { dx=-1.0, dy=16.0, u=0.4707031, v=0.6386719, cu=0.03515625, cv=0.01367188, w=18.0, h=7.0 },
  { dx=6.0, dy=17.0, u=0.7636719, v=0.703125, cu=0.0625, cv=0.02148438, w=32.0, h=11.0 },
  { dx=5.0, dy=17.0, u=0.2929688, v=0, cu=0.05859375, cv=0.02929688, w=30.0, h=15.0 },
  { dx=6.0, dy=17.0, u=0, v=0, cu=0.0546875, cv=0.03125, w=28.0, h=16.0 },
  { dx=8.0, dy=13.0, u=0.8261719, v=0.6972656, cu=0.04882813, cv=0.02734375, w=25.0, h=14.0 },
  { dx=12.0, dy=11.0, u=0.703125, v=0.6992188, cu=0.06054688, cv=0.02539063, w=31.0, h=13.0 },
  { dx=23.0, dy=13.0, u=0.1464844, v=0, cu=0.05859375, cv=0.03515625, w=30.0, h=18.0 },
  { dx=24.0, dy=17.0, u=0.3203125, v=0.5351563, cu=0.05273438, cv=0.04101563, w=27.0, h=21.0 },
  { dx=23.0, dy=11.0, u=0.0546875, v=0, cu=0.0390625, cv=0.03125, w=20.0, h=16.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 10)

texinfo = createTextureInfoEx('boat_wood')
frames =
{
  { dx=32.0, dy=15.0, u=0.3359375, v=0.6503906, cu=0.1230469, cv=0.0625, w=63.0, h=32.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat0')
frames =
{
  { dx=40.0, dy=29.0, u=0.7011719, v=0.8613281, cu=0.1816406, cv=0.1386719, w=93.0, h=71.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat1')
frames =
{
  { dx=40.0, dy=29.0, u=0.5195313, v=0.71875, cu=0.1796875, cv=0.1386719, w=92.0, h=71.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat2')
frames =
{
  { dx=44.0, dy=37.0, u=0.1640625, v=0.8457031, cu=0.1757813, cv=0.1542969, w=90.0, h=79.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat3')
frames =
{
  { dx=44.0, dy=43.0, u=0, v=0.8320313, cu=0.1640625, cv=0.1679688, w=84.0, h=86.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat4')
frames =
{
  { dx=47.0, dy=43.0, u=0, v=0.359375, cu=0.1523438, cv=0.1660156, w=78.0, h=85.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat5')
frames =
{
  { dx=40.0, dy=43.0, u=0.1523438, v=0.359375, cu=0.1308594, cv=0.1660156, w=67.0, h=85.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat6')
frames =
{
  { dx=30.0, dy=43.0, u=0.2929688, v=0.02929688, cu=0.09765625, cv=0.1640625, w=50.0, h=84.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat7')
frames =
{
  { dx=21.0, dy=43.0, u=0.3652344, v=0.2050781, cu=0.09570313, cv=0.1640625, w=49.0, h=84.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat8')
frames =
{
  { dx=27.0, dy=43.0, u=0.15625, v=0.1933594, cu=0.125, cv=0.1660156, w=64.0, h=85.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat9')
frames =
{
  { dx=32.0, dy=43.0, u=0, v=0.03125, cu=0.1464844, cv=0.1660156, w=75.0, h=85.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat10')
frames =
{
  { dx=39.0, dy=42.0, u=0, v=0.1972656, cu=0.15625, cv=0.1621094, w=80.0, h=83.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat11')
frames =
{
  { dx=47.0, dy=35.0, u=0, v=0.5253906, cu=0.1699219, cv=0.1503906, w=87.0, h=77.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat12')
frames =
{
  { dx=51.0, dy=28.0, u=0.3398438, v=0.7128906, cu=0.1796875, cv=0.1367188, w=92.0, h=70.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat13')
frames =
{
  { dx=53.0, dy=28.0, u=0.7011719, v=0.7246094, cu=0.1816406, cv=0.1367188, w=93.0, h=70.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat14')
frames =
{
  { dx=51.0, dy=31.0, u=0.5195313, v=0.8574219, cu=0.1816406, cv=0.1425781, w=93.0, h=73.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat15')
frames =
{
  { dx=45.0, dy=34.0, u=0.1699219, v=0.6972656, cu=0.1660156, cv=0.1484375, w=85.0, h=76.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat16')
frames =
{
  { dx=37.0, dy=39.0, u=0.1464844, v=0.03515625, cu=0.1464844, cv=0.1582031, w=75.0, h=81.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat17')
frames =
{
  { dx=29.0, dy=42.0, u=0.8828125, v=0.8359375, cu=0.1152344, cv=0.1640625, w=59.0, h=84.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat18')
frames =
{
  { dx=22.0, dy=43.0, u=0.2832031, v=0.3691406, cu=0.0859375, cv=0.1660156, w=44.0, h=85.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat19')
frames =
{
  { dx=19.0, dy=43.0, u=0.28125, v=0.1933594, cu=0.08398438, cv=0.1660156, w=43.0, h=85.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat20')
frames =
{
  { dx=29.0, dy=43.0, u=0.8828125, v=0.6699219, cu=0.1152344, cv=0.1660156, w=59.0, h=85.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat21')
frames =
{
  { dx=41.0, dy=41.0, u=0.1699219, v=0.5351563, cu=0.1503906, cv=0.1621094, w=77.0, h=83.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat22')
frames =
{
  { dx=42.0, dy=38.0, u=0, v=0.6757813, cu=0.1699219, cv=0.15625, w=87.0, h=80.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = createTextureInfoEx('boat23')
frames =
{
  { dx=40.0, dy=34.0, u=0.3398438, v=0.8496094, cu=0.1796875, cv=0.1503906, w=92.0, h=77.0 },
}
addTextureInfoExNodes(texinfo, 'boat', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
