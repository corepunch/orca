-- train.lua
-- copyright (c) 2004-2014 rionix
-- version: 1.20
-- date: 18.06.2014
-- source: E:\train.spt (512x256)

texinfo = createTextureInfoEx('train')
frames =
{
  { dx=113.0, dy=11.0, u=0.4882813, v=0.6328125, cu=0.2363281, cv=0.3671875, w=121.0, h=94.0 },
  { dx=109.0, dy=10.0, u=0.2578125, v=0.609375, cu=0.2304688, cv=0.390625, w=118.0, h=100.0 },
  { dx=113.0, dy=10.0, u=0, v=0.1953125, cu=0.2382813, cv=0.3945313, w=122.0, h=101.0 },
  { dx=108.0, dy=10.0, u=0.7246094, v=0.65625, cu=0.2285156, cv=0.34375, w=117.0, h=88.0 },
  { dx=117.0, dy=11.0, u=0, v=0.5898438, cu=0.2578125, cv=0.4101563, w=132.0, h=105.0 },
}
addTextureInfoExNodes(texinfo, 'train', frames, 0)

texinfo = nil
frames = nil
bounds = nil
points = nil
