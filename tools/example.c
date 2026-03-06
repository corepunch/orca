#ifndef __UIKIT_PROPERTIES_H__
#define __UIKIT_PROPERTIES_H__

// DataObject
#define ID_DataObject 0xeb3560da

// AnimationPlayer
#define ID_AnimationPlayer 0x81ac71d8
#define ID_AnimationPlayer_AutoplayEnabled 0xbb4b7f90 // AnimationPlayer.AutoplayEnabled
#define ID_AnimationPlayer_DurationScale 0x742d0c5c // AnimationPlayer.DurationScale
#define ID_AnimationPlayer_PlaybackMode 0xb886a1a0 // AnimationPlayer.PlaybackMode
#define ID_AnimationPlayer_RelativePlayback 0xb10f4317 // AnimationPlayer.RelativePlayback
#define ID_AnimationPlayer_RepeatCount 0x02afaefc // AnimationPlayer.RepeatCount
#define ID_AnimationPlayer_RestoreOriginalValuesAfterPlayback 0xb982b350 // AnimationPlayer.RestoreOriginalValuesAfterPlayback
#define ID_AnimationPlayer_Timeline 0x17c5cb89 // AnimationPlayer.Timeline

// Trigger
#define ID_Trigger 0xa5ea0da3
#define ID_Trigger_Property 0x7a6c1a46 // Trigger.Property
#define ID_Trigger_Value 0x5229f3cc // Trigger.Value

// OnPropertyChangedTrigger
#define ID_OnPropertyChangedTrigger 0xde0f7949
#define ID_OnPropertyChangedTrigger_SourceNode 0x5d55b664 // OnPropertyChangedTrigger.SourceNode
#define ID_OnPropertyChangedTrigger_Property 0x8664fe08 // OnPropertyChangedTrigger.Property

// OnAttachedTrigger
#define ID_OnAttachedTrigger 0x677fe670

// EventTrigger
#define ID_EventTrigger 0x88bab1a1
#define ID_EventTrigger_RoutedEvent 0x0c5f2982 // EventTrigger.RoutedEvent

// Setter
#define ID_Setter 0xf849aee6
#define ID_Setter_Trigger 0xd1fad954 // Setter.Trigger
#define ID_Setter_Property 0x89d17a41 // Setter.Property
#define ID_Setter_Value 0x8fd76d09 // Setter.Value

// Handler
#define ID_Handler 0x04d66a13
#define ID_Handler_Trigger 0x0ef3b2a9 // Handler.Trigger
#define ID_Handler_Target 0x2798724a // Handler.Target
#define ID_Handler_Function 0x595dd2a7 // Handler.Function

// Brush
#define ID_Brush 0xccbef093

// ColorBrush
#define ID_ColorBrush 0x2077ee58
#define ID_ColorBrush_Color 0xb7f26275 // ColorBrush.Color

// Node
#define ID_Node 0x3468032d
#define ID_Node_Size 0xc8371588 // Node.Size
#define ID_Node_Size.Axis[0] 0x4565a1f3 // Node.Size.Axis[0]
#define ID_Node_Size.Axis[0].Requested 0x102de6d9 // Node.Size.Axis[0].Requested
#define ID_Node_Size.Axis[0].Desired 0x09eee753 // Node.Size.Axis[0].Desired
#define ID_Node_Size.Axis[0].Min 0xa074cf61 // Node.Size.Axis[0].Min
#define ID_Node_Size.Axis[0].Actual 0x224b6e8d // Node.Size.Axis[0].Actual
#define ID_Node_Size.Axis[0].Scroll 0xa41791f6 // Node.Size.Axis[0].Scroll
#define ID_Node_Size.Axis[1] 0x09691516 // Node.Size.Axis[1]
#define ID_Node_Size.Axis[1].Requested 0xe4b79290 // Node.Size.Axis[1].Requested
#define ID_Node_Size.Axis[1].Desired 0xe84b117a // Node.Size.Axis[1].Desired
#define ID_Node_Size.Axis[1].Min 0x31c7f6a4 // Node.Size.Axis[1].Min
#define ID_Node_Size.Axis[1].Actual 0x186d2dde // Node.Size.Axis[1].Actual
#define ID_Node_Size.Axis[1].Scroll 0xd4ebc049 // Node.Size.Axis[1].Scroll
#define ID_Node_Size.Axis[2] 0x496bb86d // Node.Size.Axis[2]
#define ID_Node_Size.Axis[2].Requested 0x143c453f // Node.Size.Axis[2].Requested
#define ID_Node_Size.Axis[2].Desired 0x0c302d01 // Node.Size.Axis[2].Desired
#define ID_Node_Size.Axis[2].Min 0xd1741313 // Node.Size.Axis[2].Min
#define ID_Node_Size.Axis[2].Actual 0xdd86a333 // Node.Size.Axis[2].Actual
#define ID_Node_Size.Axis[2].Scroll 0x5a88ee38 // Node.Size.Axis[2].Scroll
#define ID_Node_Margin 0x3b2ed3f7 // Node.Margin
#define ID_Node_Margin.Axis[0] 0x62964df8 // Node.Margin.Axis[0]
#define ID_Node_Margin.Axis[0].Left 0x96985d4f // Node.Margin.Axis[0].Left
#define ID_Node_Margin.Axis[0].Right 0xa4384dc0 // Node.Margin.Axis[0].Right
#define ID_Node_Margin.Axis[1] 0x669415ad // Node.Margin.Axis[1]
#define ID_Node_Margin.Axis[1].Left 0x4aef8ff4 // Node.Margin.Axis[1].Left
#define ID_Node_Margin.Axis[1].Right 0x830ad5a1 // Node.Margin.Axis[1].Right
#define ID_Node_Margin.Axis[2] 0x26917256 // Node.Margin.Axis[2]
#define ID_Node_Margin.Axis[2].Left 0x9f495f39 // Node.Margin.Axis[2].Left
#define ID_Node_Margin.Axis[2].Right 0xa321559a // Node.Margin.Axis[2].Right
#define ID_Node_Padding 0xc7721c32 // Node.Padding
#define ID_Node_Padding.Axis[0] 0x5257e3d9 // Node.Padding.Axis[0]
#define ID_Node_Padding.Axis[0].Left 0x07befdc0 // Node.Padding.Axis[0].Left
#define ID_Node_Padding.Axis[0].Right 0x0f8248f5 // Node.Padding.Axis[0].Right
#define ID_Node_Padding.Axis[1] 0x0e59b764 // Node.Padding.Axis[1]
#define ID_Node_Padding.Axis[1].Left 0x60cf938b // Node.Padding.Axis[1].Left
#define ID_Node_Padding.Axis[1].Right 0xf2faf894 // Node.Padding.Axis[1].Right
#define ID_Node_Padding.Axis[2] 0x0e52fb9f // Node.Padding.Axis[2]
#define ID_Node_Padding.Axis[2].Left 0x4bef887a // Node.Padding.Axis[2].Left
#define ID_Node_Padding.Axis[2].Right 0x9c193a47 // Node.Padding.Axis[2].Right
#define ID_Node_Border 0x2a3ce9f3 // Node.Border
#define ID_Node_Border.Width 0x46604ca9 // Node.Border.Width
#define ID_Node_Border.Width.Axis[0] 0x03cd783e // Node.Border.Width.Axis[0]
#define ID_Node_Border.Width.Axis[0].Left 0x9a761251 // Node.Border.Width.Axis[0].Left
#define ID_Node_Border.Width.Axis[0].Right 0xf779d672 // Node.Border.Width.Axis[0].Right
#define ID_Node_Border.Width.Axis[1] 0xffcb335b // Node.Border.Width.Axis[1]
#define ID_Node_Border.Width.Axis[1].Left 0x2a47f706 // Node.Border.Width.Axis[1].Left
#define ID_Node_Border.Width.Axis[1].Right 0xd1204ebb // Node.Border.Width.Axis[1].Right
#define ID_Node_Border.Width.Axis[2] 0x3fd253e0 // Node.Border.Width.Axis[2]
#define ID_Node_Border.Width.Axis[2].Left 0xf25e25e7 // Node.Border.Width.Axis[2].Left
#define ID_Node_Border.Width.Axis[2].Right 0x5dccab98 // Node.Border.Width.Axis[2].Right
#define ID_Node_Border.Color 0x6a219616 // Node.Border.Color
#define ID_Node_Border.Style 0x17c24518 // Node.Border.Style
#define ID_Node_Border.Radius 0xc3baa535 // Node.Border.Radius
#define ID_Node_Border.Radius.TopLeftRadius 0xaf7f40ef // Node.Border.Radius.TopLeftRadius
#define ID_Node_Border.Radius.TopRightRadius 0xd60f4820 // Node.Border.Radius.TopRightRadius
#define ID_Node_Border.Radius.BottomRightRadius 0xc289781a // Node.Border.Radius.BottomRightRadius
#define ID_Node_Border.Radius.BottomLeftRadius 0xf4b6eb19 // Node.Border.Radius.BottomLeftRadius
#define ID_Node_Alignment 0x6ae48d82 // Node.Alignment
#define ID_Node_Alignment.Axis[0] 0x6723f449 // Node.Alignment.Axis[0]
#define ID_Node_Alignment.Axis[1] 0x23275ad4 // Node.Alignment.Axis[1]
#define ID_Node_Alignment.Axis[2] 0x231f0c0f // Node.Alignment.Axis[2]
#define ID_Node_Visible 0xe1936ee5 // Node.Visible
#define ID_Node_QuickHide 0x7e26e1b0 // Node.QuickHide
#define ID_Node_VisibleAmountInParent 0xbe0696b2 // Node.VisibleAmountInParent
#define ID_Node_Opacity 0xb6882472 // Node.Opacity
#define ID_Node_Tags 0xec56af24 // Node.Tags
#define ID_Node_DataContext 0x80b43db0 // Node.DataContext

// TextRun
#define ID_TextRun 0x4362c3d7
#define ID_TextRun_Text 0xcba1ea6c // TextRun.Text
#define ID_TextRun_Font 0x3900dfa2 // TextRun.Font
#define ID_TextRun_Font.Weight 0xec282e94 // TextRun.Font.Weight
#define ID_TextRun_Font.Style 0x2f6edf49 // TextRun.Font.Style
#define ID_TextRun_Font.Size 0xbe496cfd // TextRun.Font.Size
#define ID_TextRun_Font.Family 0x4539c02c // TextRun.Font.Family
#define ID_TextRun_Underline 0x0b0d856d // TextRun.Underline
#define ID_TextRun_Underline.Offset 0xdbc60cc6 // TextRun.Underline.Offset
#define ID_TextRun_Underline.Width 0x01ca4593 // TextRun.Underline.Width
#define ID_TextRun_Underline.Color 0xebeacd1c // TextRun.Underline.Color
#define ID_TextRun_LetterSpacing 0x83b2ad0c // TextRun.LetterSpacing
#define ID_TextRun_LineHeight 0x45c78196 // TextRun.LineHeight
#define ID_TextRun_CharacterSpacing 0xda8217e7 // TextRun.CharacterSpacing
#define ID_TextRun_FixedCharacterWidth 0xb71765fa // TextRun.FixedCharacterWidth
#define ID_TextRun_RemoveSideBearingsProperty 0xc20f6de6 // TextRun.RemoveSideBearingsProperty

// TextBlockConcept
#define ID_TextBlockConcept 0x4903089d
#define ID_TextBlockConcept_TextResourceID 0x7617ef4f // TextBlockConcept.TextResourceID
#define ID_TextBlockConcept_TextResourceConfiguration 0x445231c0 // TextBlockConcept.TextResourceConfiguration
#define ID_TextBlockConcept_PlaceholderText 0xadd54a35 // TextBlockConcept.PlaceholderText
#define ID_TextBlockConcept_TextOverflow 0x9717f5b0 // TextBlockConcept.TextOverflow
#define ID_TextBlockConcept_Placeholder 0x78889986 // TextBlockConcept.Placeholder
#define ID_TextBlockConcept_Placeholder.Color 0x05a37cef // TextBlockConcept.Placeholder.Color
#define ID_TextBlockConcept_Placeholder.Image 0xd25ba905 // TextBlockConcept.Placeholder.Image
#define ID_TextBlockConcept_Placeholder.Material 0xecc2c701 // TextBlockConcept.Placeholder.Material
#define ID_TextBlockConcept_UseFullFontHeight 0x95fcf16b // TextBlockConcept.UseFullFontHeight
#define ID_TextBlockConcept_ConstrainContentHeight 0x8468e688 // TextBlockConcept.ConstrainContentHeight
#define ID_TextBlockConcept_WordWrap 0x34b71f41 // TextBlockConcept.WordWrap
#define ID_TextBlockConcept_TextWrapping 0x709f2f06 // TextBlockConcept.TextWrapping
#define ID_TextBlockConcept_TextHorizontalAlignment 0xe480096b // TextBlockConcept.TextHorizontalAlignment
#define ID_TextBlockConcept_TextVerticalAlignment 0xbf0260e5 // TextBlockConcept.TextVerticalAlignment

// Node2D
#define ID_Node2D 0x6c63a2ab
#define ID_Node2D_LayoutTransform 0x7c78c87b // Node2D.LayoutTransform
#define ID_Node2D_LayoutTransform.translation 0x83fe0fe6 // Node2D.LayoutTransform.translation
#define ID_Node2D_LayoutTransform.rotation 0xce3121d1 // Node2D.LayoutTransform.rotation
#define ID_Node2D_LayoutTransform.scale 0xa7a1580f // Node2D.LayoutTransform.scale
#define ID_Node2D_RenderTransform 0xa5faec05 // Node2D.RenderTransform
#define ID_Node2D_RenderTransform.translation 0xcf2e8290 // Node2D.RenderTransform.translation
#define ID_Node2D_RenderTransform.rotation 0x3e3f58b3 // Node2D.RenderTransform.rotation
#define ID_Node2D_RenderTransform.scale 0x0bb2c7e5 // Node2D.RenderTransform.scale
#define ID_Node2D_RenderTransformOrigin 0x43a9dbaf // Node2D.RenderTransformOrigin
#define ID_Node2D_ContentOffset 0xb5cb609b // Node2D.ContentOffset
#define ID_Node2D_Matrix 0x09a64b02 // Node2D.Matrix
#define ID_Node2D_RenderTarget 0x441af9f6 // Node2D.RenderTarget
#define ID_Node2D_Background 0x59353973 // Node2D.Background
#define ID_Node2D_Background.Color 0x13db6996 // Node2D.Background.Color
#define ID_Node2D_Background.Image 0xce4f21dc // Node2D.Background.Image
#define ID_Node2D_Background.Material 0x0f9d3f1e // Node2D.Background.Material
#define ID_Node2D_Foreground 0xaeaca316 // Node2D.Foreground
#define ID_Node2D_Foreground.Color 0x18d562df // Node2D.Foreground.Color
#define ID_Node2D_Foreground.Image 0x0809bd55 // Node2D.Foreground.Image
#define ID_Node2D_Foreground.Material 0x4f4b36b1 // Node2D.Foreground.Material
#define ID_Node2D_BoxShadow 0x47152f84 // Node2D.BoxShadow
#define ID_Node2D_BoxShadow.Offset 0x2b0a9c29 // Node2D.BoxShadow.Offset
#define ID_Node2D_BoxShadow.BlurRadius 0xb24a754d // Node2D.BoxShadow.BlurRadius
#define ID_Node2D_BoxShadow.SpreadRadius 0xfcf0bd53 // Node2D.BoxShadow.SpreadRadius
#define ID_Node2D_BoxShadow.Color 0x5a8a6021 // Node2D.BoxShadow.Color
#define ID_Node2D_Overflow 0xd5704155 // Node2D.Overflow
#define ID_Node2D_Overflow.x 0x0735d19b // Node2D.Overflow.x
#define ID_Node2D_Overflow.y 0x0635d008 // Node2D.Overflow.y
#define ID_Node2D_Ring 0x039ac541 // Node2D.Ring
#define ID_Node2D_Ring.Offset 0xddfeb552 // Node2D.Ring.Offset
#define ID_Node2D_Ring.Width 0x56b97a67 // Node2D.Ring.Width
#define ID_Node2D_Ring.Color 0x74eaf630 // Node2D.Ring.Color
#define ID_Node2D_CompositionBrush 0x04f99755 // Node2D.CompositionBrush
#define ID_Node2D_CompositionDesignSize 0x8a2688dc // Node2D.CompositionDesignSize
#define ID_Node2D_SizeToContent 0xe55ab2cc // Node2D.SizeToContent
#define ID_Node2D_OffscreenRendering 0x893fea40 // Node2D.OffscreenRendering
#define ID_Node2D_ForceComposition 0x987f2560 // Node2D.ForceComposition
#define ID_Node2D_CacheResult 0xfa85e55a // Node2D.CacheResult
#define ID_Node2D_SnapToPixel 0x752cbd46 // Node2D.SnapToPixel
#define ID_Node2D_ClipChildren 0xe3db0cc6 // Node2D.ClipChildren
#define ID_Node2D_ContentStretch 0x09a28d25 // Node2D.ContentStretch
#define ID_Node2D_Hovered 0x982d5e3e // Node2D.Hovered
#define ID_Node2D_IgnoreHitTest 0x0943bf6a // Node2D.IgnoreHitTest
#define ID_Node2D_ForegroundHint 0x1a0ea5e3 // Node2D.ForegroundHint

// PrefabView2D
#define ID_PrefabView2D 0xe741d328
#define ID_PrefabView2D_SCA 0x1ab11f83 // PrefabView2D.SCA
#define ID_PrefabView2D_Prefab 0xef0b7c70 // PrefabView2D.Prefab

// TextBlock
#define ID_TextBlock 0x40f4d77b

// Input
#define ID_Input 0x9f93e11b
#define ID_Input_Name 0x56849c4c // Input.Name
#define ID_Input_Type 0x84bfe237 // Input.Type
#define ID_Input_Cursor 0xb89f7b0d // Input.Cursor
#define ID_Input_Multiline 0xa42cd2d2 // Input.Multiline
#define ID_Input_Checked 0x62ce5658 // Input.Checked

// Button
#define ID_Button 0x33881a91
#define ID_Button_Type 0x843eb785 // Button.Type

// Label
#define ID_Label 0x9eccf29d
#define ID_Label_For 0x8ea77d4c // Label.For

// StackView
#define ID_StackView 0x56aa550a
#define ID_StackView_Reversed 0x4f7ea66a // StackView.Reversed
#define ID_StackView_Direction 0x4f1430fd // StackView.Direction
#define ID_StackView_AlignItems 0x4d502153 // StackView.AlignItems
#define ID_StackView_JustifyContent 0x6b245371 // StackView.JustifyContent
#define ID_StackView_Spacing 0xf9ff1755 // StackView.Spacing

// Form
#define ID_Form 0xc0851367

// Control
#define ID_Control 0x8347bf3e
#define ID_Control_Pressed 0x0bfbf446 // Control.Pressed
#define ID_Control_Disabled 0x08680018 // Control.Disabled
#define ID_Control_Hovered 0xd61b4c93 // Control.Hovered
#define ID_Control_Focused 0x9d297c69 // Control.Focused
#define ID_Control_Value 0xbb44f9c1 // Control.Value
#define ID_Control_Scale 0xb46d6752 // Control.Scale

// Screen
#define ID_Screen 0x9bd8c631
#define ID_Screen_ClearColor 0x1bfc36dd // Screen.ClearColor
#define ID_Screen_ResizeMode 0xc3203446 // Screen.ResizeMode

// Cinematic
#define ID_Cinematic 0xc04cee0e
#define ID_Cinematic_FileName 0xabe998d5 // Cinematic.FileName
#define ID_Cinematic_FrameRate 0xead5c9cf // Cinematic.FrameRate
#define ID_Cinematic_NumFrames 0x46a2037a // Cinematic.NumFrames
#define ID_Cinematic_FadeOut 0x314b07da // Cinematic.FadeOut

// Grid
#define ID_Grid 0x2fb366b1
#define ID_Grid_Columns 0x5d28e334 // Grid.Columns
#define ID_Grid_Rows 0x1a2b78b0 // Grid.Rows
#define ID_Grid_Direction 0x252effe2 // Grid.Direction
#define ID_Grid_Spacing 0x5d2ffdd6 // Grid.Spacing
#define ID_Grid_CellWidth 0x534e3267 // Grid.CellWidth
#define ID_Grid_CellHeight 0xa97554ea // Grid.CellHeight

// ImageView
#define ID_ImageView 0xfe686eb3
#define ID_ImageView_Src 0xe2534f6b // ImageView.Src
#define ID_ImageView_Image 0x0b666f9c // ImageView.Image
#define ID_ImageView_Edges 0xbc1ab5c3 // ImageView.Edges
#define ID_ImageView_Insets 0xddc76755 // ImageView.Insets
#define ID_ImageView_Viewbox 0x4fff923f // ImageView.Viewbox
#define ID_ImageView_Stretch 0x13aa1da4 // ImageView.Stretch

// NinePatchImage
#define ID_NinePatchImage 0xbd86cb1e
#define ID_NinePatchImage_StretchTypeTop 0x9375dc9c // NinePatchImage.StretchTypeTop
#define ID_NinePatchImage_StretchTypeBottom 0x400be60a // NinePatchImage.StretchTypeBottom
#define ID_NinePatchImage_StretchTypeLeft 0xee979d50 // NinePatchImage.StretchTypeLeft
#define ID_NinePatchImage_StretchTypeRight 0x659e0345 // NinePatchImage.StretchTypeRight
#define ID_NinePatchImage_StretchTypeCenter 0xcab1ae64 // NinePatchImage.StretchTypeCenter
#define ID_NinePatchImage_ImageTopLeft 0x4b2c37e5 // NinePatchImage.ImageTopLeft
#define ID_NinePatchImage_ImageTop 0x5e5bf7e4 // NinePatchImage.ImageTop
#define ID_NinePatchImage_ImageTopRight 0xc001b5be // NinePatchImage.ImageTopRight
#define ID_NinePatchImage_ImageLeft 0x6b2a74a8 // NinePatchImage.ImageLeft
#define ID_NinePatchImage_ImageCenter 0x1811e0dc // NinePatchImage.ImageCenter
#define ID_NinePatchImage_ImageRight 0x04cb7dbd // NinePatchImage.ImageRight
#define ID_NinePatchImage_ImageBottomLeft 0xe446356f // NinePatchImage.ImageBottomLeft
#define ID_NinePatchImage_ImageBottom 0xc398b8c2 // NinePatchImage.ImageBottom
#define ID_NinePatchImage_ImageBottomRight 0x87575d60 // NinePatchImage.ImageBottomRight

// TerminalView
#define ID_TerminalView 0xb1477a0c
#define ID_TerminalView_BufferWidth 0xdf51183c // TerminalView.BufferWidth
#define ID_TerminalView_BufferHeight 0xfd169aab // TerminalView.BufferHeight
#define ID_TerminalView_Cursor 0x550c9034 // TerminalView.Cursor
#define ID_TerminalView_SelectedIndex 0xc6c048a5 // TerminalView.SelectedIndex
#define ID_TerminalView_DropShadow 0x8c72c3cb // TerminalView.DropShadow

// Page
#define ID_Page 0xe83d9196
#define ID_Page_Title 0x31e209ce // Page.Title
#define ID_Page_Path 0x20109b7f // Page.Path
#define ID_Page_Transition 0xb60425fb // Page.Transition

// PageHost
#define ID_PageHost 0x6f2dc6c0
#define ID_PageHost_ActivePage 0xb276c4f7 // PageHost.ActivePage

// PageViewport
#define ID_PageViewport 0x61a2c1e0

// Style
#define ID_Style 0x5467ec76
#define ID_Style_TargetType 0x26cf4319 // Style.TargetType
#define ID_Style_Type 0x8f053d80 // Style.Type
#endif