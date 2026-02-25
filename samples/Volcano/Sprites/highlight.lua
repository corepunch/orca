-- highlight.lua
-- copyright (c) 2004-2011 rionix
-- version: 1.00
-- date: 28.06.2012
-- source: Z:\highlight.spt (256x256)

texinfo = createTextureInfoEx('highlight')
frames =
{
  { dx=79.0, dy=54.0, u=0.0078125, v=0.5703125, cu=0.6171875, cv=0.421875, w=158.0, h=108.0 },
  { dx=79.0, dy=54.0, u=0.0078125, v=0.140625, cu=0.6171875, cv=0.421875, w=158.0, h=108.0 },
}
addTextureInfoExNodes(texinfo, 'highlight', frames, 0)

texinfo = createTextureInfoEx('highlight2')
frames =
{
  { dx=79.0, dy=54.0, u=0.0078125, v=0.140625, cu=0.6171875, cv=0.421875, w=158.0, h=108.0 },
}
addTextureInfoExNodes(texinfo, 'highlight', frames, 0)

texinfo = nil
frames = nil
