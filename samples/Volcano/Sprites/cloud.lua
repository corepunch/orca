-- cloud.lua
-- copyright (c) 2004-2011 rionix
-- version: 1.00
-- date: 03.09.2012
-- source: Z:\cloud.spt (256x256)

texinfo = createTextureInfoEx('magic_cloud')
frames =
{
  { dx=128.0, dy=128.0, u=0, v=0, cu=1, cv=1, w=256.0, h=256.0 },
}
addTextureInfoExNodes(texinfo, 'cloud', frames, 0)

texinfo = nil
frames = nil
