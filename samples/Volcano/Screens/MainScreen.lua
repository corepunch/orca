local sk = require "orca.SpriteKit"
local ui = require "orca.ui"
local cg = require "orca.geometry"
local fs = require "orca.filesystem"
local Level = require "GameKit.Level"

local ImageCtrl = sk.SKSpriteNode
local LabelCtrl = sk.SKLabelNode
local ButtonSysCtrl = sk.SKSpriteNode
local ProgressBarSysCtrl = sk.SKSpriteNode

local align = {
  Left = 1,
  Right = 2,
  Top = 4,
  Bottom = 8,
  Middle = 16,
}

return ui.Node2D {
  Width = 1024,
  Height = 768,
  Name = "mainWindow",
  sk.SKView {
    Name = "GameView",
    Width = 1366,
    Height = 768,
    HorizontalAlignment = "Center",
    Scene = "Level",
    Level {
      Name = "Level",
    },
  },
  sk.SKView {
    Name = "InterfaceView",
    Width = 1024,
    Height = 768,
    ReferenceWidth = 1366,
    ReferenceHeight = 768,
    HorizontalAlignment = "Center",
    Scene = "Root",
    sk.SKScene {
      Name = "Root",

      ImageCtrl{
        Name = "noname",
        Rect = cg.Rect { x=-47, y=668, width=488, height=115 },
        UvRect = cg.Rect { x=0.0020, y=0.5332, width=0.4766, height=0.1123 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Left + align.Top,

      },
      ButtonSysCtrl{
        Name = "menu",
        Rect = cg.Rect { x=1, y=681, width=82, height=82 },
        Image = fs:Image "texture/gui/gui00.png",
        UvRect = cg.Rect { x=0.7441, y=0.9189, width=0.0801, height=0.0801 },
        Anchor = align.Left + align.Top,

      },
      ImageCtrl{
        Name = "noname",
        Rect = cg.Rect { x=1010, y=671, width=454, height=153 },
        UvRect = cg.Rect { x=0.4805, y=0.5176, width=0.4434, height=0.1494 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Right + align.Top,

      },
      ImageCtrl{
        Name = "task0_more",
        Rect = cg.Rect { x=1035, y=741, width=20, height=21 },
        UvRect = cg.Rect { x=0.8330, y=0.7373, width=0.0195, height=0.0205 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Right + align.Top,

      },
      ImageCtrl{
        Name = "task0_done",
        Rect = cg.Rect { x=1036, y=741, width=21, height=22 },
        UvRect = cg.Rect { x=0.8535, y=0.7373, width=0.0205, height=0.0215 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Right + align.Top,

      },
      ImageCtrl{
        Name = "task1_more",
        Rect = cg.Rect { x=1035, y=720, width=20, height=21 },
        UvRect = cg.Rect { x=0.8330, y=0.7373, width=0.0195, height=0.0205 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Right + align.Top,

      },
      ImageCtrl{
        Name = "task2_more",
        Rect = cg.Rect { x=1035, y=699, width=20, height=21 },
        UvRect = cg.Rect { x=0.8330, y=0.7373, width=0.0195, height=0.0205 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Right + align.Top,

      },
      ImageCtrl{
        Name = "task1_done",
        Rect = cg.Rect { x=1036, y=720, width=21, height=22 },
        UvRect = cg.Rect { x=0.8535, y=0.7373, width=0.0205, height=0.0215 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Right + align.Top,

      },
      ImageCtrl{
        Name = "task2_done",
        Rect = cg.Rect { x=1036, y=699, width=21, height=22 },
        UvRect = cg.Rect { x=0.8535, y=0.7373, width=0.0205, height=0.0215 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Right + align.Top,

      },
      LabelCtrl{
        Name = "task0_label",
        Rect = cg.Rect { x=1059, y=738, width=320, height=25 },
        Color = cg.Color(1, 1, 1, 1),
        Text = "Rebuild 2 gasoline stations",
        Anchor = align.Right + align.Top,

      },
      LabelCtrl{
        Name = "task1_label",
        Rect = cg.Rect { x=1059, y=717, width=320, height=25 },
        Color = cg.Color(1, 1, 1, 1),
        Text = "Rebuild 2 gasoline stations",
        Anchor = align.Right + align.Top,

      },
      LabelCtrl{
        Name = "task2_label",
        Rect = cg.Rect { x=1059, y=696, width=320, height=25 },
        Color = cg.Color(1, 1, 1, 1),
        Text = "Rebuild 2 gasoline stations",
        Anchor = align.Right + align.Top,

      },
      ImageCtrl{
        Name = "noname",
        Rect = cg.Rect { x=24, y=704, width=36, height=37 },
        UvRect = cg.Rect { x=0.6201, y=0.7031, width=0.0352, height=0.2891 },
        Anchor = align.Left + align.Top,

      },
      ImageCtrl{
        Name = "time_back",
        Rect = cg.Rect { x=87, y=707, width=359, height=28 },
        UvRect = cg.Rect { x=0.0020, y=0.6504, width=0.3506, height=0.0273 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Left + align.Top,

      },
      ProgressBarSysCtrl{
        Name = "time",
        Rect = cg.Rect { x=89, y=715, width=352, height=12 },
        Image = fs:Image "texture/gui/gui00.png",
        UvRect = cg.Rect { x=0.3662, y=0.6719, width=0.3438, height=0.0117 },
        Anchor = align.Left + align.Top,

      },
      ImageCtrl{
        Name = "time_spin",
        Rect = cg.Rect { x=200, y=708, width=10, height=26 },
        UvRect = cg.Rect { x=0.3545, y=0.6514, width=0.0098, height=0.0254 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Left + align.Top,

      },
      LabelCtrl{
        Name = "food",
        Rect = cg.Rect { x=123, y=739, width=49, height=20 },
        Color = cg.Color(1, 1, 1, 1),
        Text = "0000",
        Anchor = align.Left + align.Top,

      },
      LabelCtrl{
        Name = "wood",
        Rect = cg.Rect { x=211, y=739, width=49, height=20 },
        Color = cg.Color(1, 1, 1, 1),
        Text = "0000",
        Anchor = align.Left + align.Top,

      },
      LabelCtrl{
        Name = "money",
        Rect = cg.Rect { x=303, y=739, width=49, height=20 },
        Color = cg.Color(1, 1, 1, 1),
        Text = "0000",
        Anchor = align.Left + align.Top,

      },
      LabelCtrl{
        Name = "gas",
        Rect = cg.Rect { x=383, y=739, width=49, height=20 },
        Color = cg.Color(1, 1, 1, 1),
        Text = "0000",
        Anchor = align.Left + align.Top,

      },
      ImageCtrl{
        Name = "icon_food",
        Rect = cg.Rect { x=87, y=732, width=37, height=36 },
        UvRect = cg.Rect { x=0.9590, y=0.6221, width=0.0361, height=0.0352 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Left + align.Top,

      },
      ImageCtrl{
        Name = "icon_wood",
        Rect = cg.Rect { x=173, y=733, width=41, height=35 },
        UvRect = cg.Rect { x=0.9590, y=0.5869, width=0.0400, height=0.0342 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Left + align.Top,

      },
      ImageCtrl{
        Name = "icon_money",
        Rect = cg.Rect { x=262, y=733, width=44, height=35 },
        UvRect = cg.Rect { x=0.9551, y=0.5508, width=0.0430, height=0.0342 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Left + align.Top,

      },
      ImageCtrl{
        Name = "icon_gas",
        Rect = cg.Rect { x=358, y=733, width=23, height=35 },
        UvRect = cg.Rect { x=0.9336, y=0.6221, width=0.0225, height=0.0342 },
        Image = fs:Image "texture/gui/gui00.png",
        Anchor = align.Left + align.Top,
      },
    },
  },
}
