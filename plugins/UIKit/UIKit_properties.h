// Auto-generated from UIKit.xml by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __UIKIT_PROPERTIES_H__
#define __UIKIT_PROPERTIES_H__

// Brush
#define ID_Brush 0xccbef093
#define GetBrush(_P) ((struct Brush*)((_P)?OBJ_GetComponent(_P,ID_Brush):NULL))
#define Brush_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Brush,sizeof(struct Brush),_N)
#define kBrushNumProperties 0
// ColorBrush
#define ID_ColorBrush 0x2077ee58
#define GetColorBrush(_P) ((struct ColorBrush*)((_P)?OBJ_GetComponent(_P,ID_ColorBrush):NULL))
#define ColorBrush_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ColorBrush,sizeof(struct ColorBrush),_N)
#define ID_ColorBrush_Color 0xb7f26275 // ColorBrush.Color
#define kColorBrushNumProperties 1
enum ColorBrushProperties {
	kColorBrushColor,
};
// TextRun
#define ID_TextRun 0x4362c3d7
#define GetTextRun(_P) ((struct TextRun*)((_P)?OBJ_GetComponent(_P,ID_TextRun):NULL))
#define TextRun_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TextRun,sizeof(struct TextRun),_N)
#define ID_TextRun_Text 0xcba1ea6c // TextRun.Text
#define ID_TextRun_Font 0x3900dfa2 // TextRun.Font
#define ID_TextRun_FontWeight 0x69432eea // TextRun.FontWeight
#define ID_TextRun_FontStyle 0xbcedda87 // TextRun.FontStyle
#define ID_TextRun_FontSize 0x307249cb // TextRun.FontSize
#define ID_TextRun_FontFamily 0x2991bcb6 // TextRun.FontFamily
#define ID_TextRun_Underline 0x0b0d856d // TextRun.Underline
#define ID_TextRun_UnderlineOffset 0x0a823d42 // TextRun.UnderlineOffset
#define ID_TextRun_UnderlineWidth 0xc80a55b7 // TextRun.UnderlineWidth
#define ID_TextRun_UnderlineColor 0xcacb7660 // TextRun.UnderlineColor
#define ID_TextRun_LetterSpacing 0x83b2ad0c // TextRun.LetterSpacing
#define ID_TextRun_LineHeight 0x45c78196 // TextRun.LineHeight
#define ID_TextRun_CharacterSpacing 0xda8217e7 // TextRun.CharacterSpacing
#define ID_TextRun_FixedCharacterWidth 0xb71765fa // TextRun.FixedCharacterWidth
#define ID_TextRun_RemoveSideBearingsProperty 0xc20f6de6 // TextRun.RemoveSideBearingsProperty
#define kTextRunNumProperties 15
enum TextRunProperties {
	kTextRunText,
	kTextRunFont,
	kTextRunFontWeight,
	kTextRunFontStyle,
	kTextRunFontSize,
	kTextRunFontFamily,
	kTextRunUnderline,
	kTextRunUnderlineOffset,
	kTextRunUnderlineWidth,
	kTextRunUnderlineColor,
	kTextRunLetterSpacing,
	kTextRunLineHeight,
	kTextRunCharacterSpacing,
	kTextRunFixedCharacterWidth,
	kTextRunRemoveSideBearingsProperty,
};
// TextBlockConcept
#define ID_TextBlockConcept 0x4903089d
#define GetTextBlockConcept(_P) ((struct TextBlockConcept*)((_P)?OBJ_GetComponent(_P,ID_TextBlockConcept):NULL))
#define TextBlockConcept_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TextBlockConcept,sizeof(struct TextBlockConcept),_N)
#define ID_TextBlockConcept_TextResourceID 0x7617ef4f // TextBlockConcept.TextResourceID
#define ID_TextBlockConcept_TextResourceConfiguration 0x445231c0 // TextBlockConcept.TextResourceConfiguration
#define ID_TextBlockConcept_PlaceholderText 0xadd54a35 // TextBlockConcept.PlaceholderText
#define ID_TextBlockConcept_TextOverflow 0x9717f5b0 // TextBlockConcept.TextOverflow
#define ID_TextBlockConcept_Placeholder 0x78889986 // TextBlockConcept.Placeholder
#define ID_TextBlockConcept_PlaceholderColor 0x8f53db89 // TextBlockConcept.PlaceholderColor
#define ID_TextBlockConcept_PlaceholderImage 0x2275c70b // TextBlockConcept.PlaceholderImage
#define ID_TextBlockConcept_PlaceholderMaterial 0x16a577db // TextBlockConcept.PlaceholderMaterial
#define ID_TextBlockConcept_UseFullFontHeight 0x95fcf16b // TextBlockConcept.UseFullFontHeight
#define ID_TextBlockConcept_ConstrainContentHeight 0x8468e688 // TextBlockConcept.ConstrainContentHeight
#define ID_TextBlockConcept_WordWrap 0x34b71f41 // TextBlockConcept.WordWrap
#define ID_TextBlockConcept_TextWrapping 0x709f2f06 // TextBlockConcept.TextWrapping
#define ID_TextBlockConcept_TextHorizontalAlignment 0xe480096b // TextBlockConcept.TextHorizontalAlignment
#define ID_TextBlockConcept_TextVerticalAlignment 0xbf0260e5 // TextBlockConcept.TextVerticalAlignment
#define ID_TextBlockConcept_MakeText 0x66218f84 // TextBlockConcept.MakeText
#define kTextBlockConceptNumProperties 15
enum TextBlockConceptProperties {
	kTextBlockConceptTextResourceID,
	kTextBlockConceptTextResourceConfiguration,
	kTextBlockConceptPlaceholderText,
	kTextBlockConceptTextOverflow,
	kTextBlockConceptPlaceholder,
	kTextBlockConceptPlaceholderColor,
	kTextBlockConceptPlaceholderImage,
	kTextBlockConceptPlaceholderMaterial,
	kTextBlockConceptUseFullFontHeight,
	kTextBlockConceptConstrainContentHeight,
	kTextBlockConceptWordWrap,
	kTextBlockConceptTextWrapping,
	kTextBlockConceptTextHorizontalAlignment,
	kTextBlockConceptTextVerticalAlignment,
	kTextBlockConceptMakeText,
};
// Node2D
#define ID_Node2D 0x6c63a2ab
#define GetNode2D(_P) ((struct Node2D*)((_P)?OBJ_GetComponent(_P,ID_Node2D):NULL))
#define Node2D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Node2D,sizeof(struct Node2D),_N)
#define ID_Node2D_LayoutTransform 0x7c78c87b // Node2D.LayoutTransform
#define ID_Node2D_LayoutTransformTranslation 0x2407475a // Node2D.LayoutTransformTranslation
#define ID_Node2D_LayoutTransformRotation 0x40a04c55 // Node2D.LayoutTransformRotation
#define ID_Node2D_LayoutTransformScale 0xae9265d3 // Node2D.LayoutTransformScale
#define ID_Node2D_RenderTransform 0xa5faec05 // Node2D.RenderTransform
#define ID_Node2D_RenderTransformTranslation 0xa5d5540c // Node2D.RenderTransformTranslation
#define ID_Node2D_RenderTransformRotation 0x5387d0bf // Node2D.RenderTransformRotation
#define ID_Node2D_RenderTransformScale 0xa0bbe951 // Node2D.RenderTransformScale
#define ID_Node2D_RenderTransformOrigin 0x43a9dbaf // Node2D.RenderTransformOrigin
#define ID_Node2D_ContentOffset 0xb5cb609b // Node2D.ContentOffset
#define ID_Node2D_Matrix 0x09a64b02 // Node2D.Matrix
#define ID_Node2D_RenderTarget 0x441af9f6 // Node2D.RenderTarget
#define ID_Node2D_Background 0x59353973 // Node2D.Background
#define ID_Node2D_BackgroundColor 0x0796e5b2 // Node2D.BackgroundColor
#define ID_Node2D_BackgroundImage 0xeb919ee8 // Node2D.BackgroundImage
#define ID_Node2D_BackgroundMaterial 0x973fabea // Node2D.BackgroundMaterial
#define ID_Node2D_Foreground 0xaeaca316 // Node2D.Foreground
#define ID_Node2D_ForegroundColor 0xf890bd19 // Node2D.ForegroundColor
#define ID_Node2D_ForegroundImage 0xd8914fbb // Node2D.ForegroundImage
#define ID_Node2D_ForegroundMaterial 0x4f98b2ab // Node2D.ForegroundMaterial
#define ID_Node2D_BoxShadow 0x47152f84 // Node2D.BoxShadow
#define ID_Node2D_BoxShadowOffset 0xfa0a729f // Node2D.BoxShadowOffset
#define ID_Node2D_BoxShadowBlurRadius 0x16c10efb // Node2D.BoxShadowBlurRadius
#define ID_Node2D_BoxShadowSpreadRadius 0x8804c4f1 // Node2D.BoxShadowSpreadRadius
#define ID_Node2D_BoxShadowColor 0xc855fc8b // Node2D.BoxShadowColor
#define ID_Node2D_Overflow 0xd5704155 // Node2D.Overflow
#define ID_Node2D_OverflowX 0x0cb66777 // Node2D.OverflowX
#define ID_Node2D_OverflowY 0x0bb665e4 // Node2D.OverflowY
#define ID_Node2D_Ring 0x039ac541 // Node2D.Ring
#define ID_Node2D_RingOffset 0x674cacfe // Node2D.RingOffset
#define ID_Node2D_RingWidth 0x1a88410b // Node2D.RingWidth
#define ID_Node2D_RingColor 0xecbe3144 // Node2D.RingColor
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
#define ID_Node2D_DrawBrush 0x8272196b // Node2D.DrawBrush
#define ID_Node2D_Measure 0x5ea52a4d // Node2D.Measure
#define ID_Node2D_Arrange 0x211d274d // Node2D.Arrange
#define ID_Node2D_MeasureOverride 0x6e565d45 // Node2D.MeasureOverride
#define ID_Node2D_ArrangeOverride 0xe3b0fa45 // Node2D.ArrangeOverride
#define ID_Node2D_ForegroundContent 0x61bfb2f7 // Node2D.ForegroundContent
#define ID_Node2D_UpdateGeometry 0x0cc412c2 // Node2D.UpdateGeometry
#define ID_Node2D_SetScrollTop 0xa3d79fef // Node2D.SetScrollTop
#define kNode2DNumProperties 52
enum Node2DProperties {
	kNode2DLayoutTransform,
	kNode2DLayoutTransformTranslation,
	kNode2DLayoutTransformRotation,
	kNode2DLayoutTransformScale,
	kNode2DRenderTransform,
	kNode2DRenderTransformTranslation,
	kNode2DRenderTransformRotation,
	kNode2DRenderTransformScale,
	kNode2DRenderTransformOrigin,
	kNode2DContentOffset,
	kNode2DMatrix,
	kNode2DRenderTarget,
	kNode2DBackground,
	kNode2DBackgroundColor,
	kNode2DBackgroundImage,
	kNode2DBackgroundMaterial,
	kNode2DForeground,
	kNode2DForegroundColor,
	kNode2DForegroundImage,
	kNode2DForegroundMaterial,
	kNode2DBoxShadow,
	kNode2DBoxShadowOffset,
	kNode2DBoxShadowBlurRadius,
	kNode2DBoxShadowSpreadRadius,
	kNode2DBoxShadowColor,
	kNode2DOverflow,
	kNode2DOverflowX,
	kNode2DOverflowY,
	kNode2DRing,
	kNode2DRingOffset,
	kNode2DRingWidth,
	kNode2DRingColor,
	kNode2DCompositionBrush,
	kNode2DCompositionDesignSize,
	kNode2DSizeToContent,
	kNode2DOffscreenRendering,
	kNode2DForceComposition,
	kNode2DCacheResult,
	kNode2DSnapToPixel,
	kNode2DClipChildren,
	kNode2DContentStretch,
	kNode2DHovered,
	kNode2DIgnoreHitTest,
	kNode2DForegroundHint,
	kNode2DDrawBrush,
	kNode2DMeasure,
	kNode2DArrange,
	kNode2DMeasureOverride,
	kNode2DArrangeOverride,
	kNode2DForegroundContent,
	kNode2DUpdateGeometry,
	kNode2DSetScrollTop,
};
// PrefabView2D
#define ID_PrefabView2D 0xe741d328
#define GetPrefabView2D(_P) ((struct PrefabView2D*)((_P)?OBJ_GetComponent(_P,ID_PrefabView2D):NULL))
#define PrefabView2D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PrefabView2D,sizeof(struct PrefabView2D),_N)
#define ID_PrefabView2D_SCA 0x1ab11f83 // PrefabView2D.SCA
#define ID_PrefabView2D_Prefab 0xef0b7c70 // PrefabView2D.Prefab
#define kPrefabView2DNumProperties 2
enum PrefabView2DProperties {
	kPrefabView2DSCA,
	kPrefabView2DPrefab,
};
// TextBlock
#define ID_TextBlock 0x40f4d77b
#define GetTextBlock(_P) ((struct TextBlock*)((_P)?OBJ_GetComponent(_P,ID_TextBlock):NULL))
#define TextBlock_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TextBlock,sizeof(struct TextBlock),_N)
#define kTextBlockNumProperties 0
// Input
#define ID_Input 0x9f93e11b
#define GetInput(_P) ((struct Input*)((_P)?OBJ_GetComponent(_P,ID_Input):NULL))
#define Input_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Input,sizeof(struct Input),_N)
#define ID_Input_Name 0x56849c4c // Input.Name
#define ID_Input_Type 0x84bfe237 // Input.Type
#define ID_Input_Cursor 0xb89f7b0d // Input.Cursor
#define ID_Input_Multiline 0xa42cd2d2 // Input.Multiline
#define ID_Input_Checked 0x62ce5658 // Input.Checked
#define kInputNumProperties 5
enum InputProperties {
	kInputName,
	kInputType,
	kInputCursor,
	kInputMultiline,
	kInputChecked,
};
// Button
#define ID_Button 0x33881a91
#define GetButton(_P) ((struct Button*)((_P)?OBJ_GetComponent(_P,ID_Button):NULL))
#define Button_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Button,sizeof(struct Button),_N)
#define ID_Button_Type 0x843eb785 // Button.Type
#define ID_Button_Click 0xeb91f197 // Button.Click
#define kButtonNumProperties 2
enum ButtonProperties {
	kButtonType,
	kButtonClick,
};
// Label
#define ID_Label 0x9eccf29d
#define GetLabel(_P) ((struct Label*)((_P)?OBJ_GetComponent(_P,ID_Label):NULL))
#define Label_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Label,sizeof(struct Label),_N)
#define ID_Label_For 0x8ea77d4c // Label.For
#define kLabelNumProperties 1
enum LabelProperties {
	kLabelFor,
};
// StackView
#define ID_StackView 0x56aa550a
#define GetStackView(_P) ((struct StackView*)((_P)?OBJ_GetComponent(_P,ID_StackView):NULL))
#define StackView_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StackView,sizeof(struct StackView),_N)
#define ID_StackView_Reversed 0x4f7ea66a // StackView.Reversed
#define ID_StackView_Direction 0x4f1430fd // StackView.Direction
#define ID_StackView_AlignItems 0x4d502153 // StackView.AlignItems
#define ID_StackView_JustifyContent 0x6b245371 // StackView.JustifyContent
#define ID_StackView_Spacing 0xf9ff1755 // StackView.Spacing
#define kStackViewNumProperties 5
enum StackViewProperties {
	kStackViewReversed,
	kStackViewDirection,
	kStackViewAlignItems,
	kStackViewJustifyContent,
	kStackViewSpacing,
};
// Form
#define ID_Form 0xc0851367
#define GetForm(_P) ((struct Form*)((_P)?OBJ_GetComponent(_P,ID_Form):NULL))
#define Form_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Form,sizeof(struct Form),_N)
#define ID_Form_Submit 0xef0f357b // Form.Submit
#define kFormNumProperties 1
// Control
#define ID_Control 0x8347bf3e
#define GetControl(_P) ((struct Control*)((_P)?OBJ_GetComponent(_P,ID_Control):NULL))
#define Control_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Control,sizeof(struct Control),_N)
#define ID_Control_Pressed 0x0bfbf446 // Control.Pressed
#define ID_Control_Disabled 0x08680018 // Control.Disabled
#define ID_Control_Hovered 0xd61b4c93 // Control.Hovered
#define ID_Control_Focused 0x9d297c69 // Control.Focused
#define ID_Control_Value 0xbb44f9c1 // Control.Value
#define ID_Control_Scale 0xb46d6752 // Control.Scale
#define kControlNumProperties 6
enum ControlProperties {
	kControlPressed,
	kControlDisabled,
	kControlHovered,
	kControlFocused,
	kControlValue,
	kControlScale,
};
// Screen
#define ID_Screen 0x9bd8c631
#define GetScreen(_P) ((struct Screen*)((_P)?OBJ_GetComponent(_P,ID_Screen):NULL))
#define Screen_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Screen,sizeof(struct Screen),_N)
#define ID_Screen_ClearColor 0x1bfc36dd // Screen.ClearColor
#define ID_Screen_ResizeMode 0xc3203446 // Screen.ResizeMode
#define ID_Screen_DialogResult 0x2f02ab20 // Screen.DialogResult
#define ID_Screen_UpdateLayout 0x7d9d5a12 // Screen.UpdateLayout
#define ID_Screen_RenderScreen 0xd1601cf1 // Screen.RenderScreen
#define kScreenNumProperties 5
enum ScreenProperties {
	kScreenClearColor,
	kScreenResizeMode,
	kScreenDialogResult,
	kScreenUpdateLayout,
	kScreenRenderScreen,
};
// Cinematic
#define ID_Cinematic 0xc04cee0e
#define GetCinematic(_P) ((struct Cinematic*)((_P)?OBJ_GetComponent(_P,ID_Cinematic):NULL))
#define Cinematic_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Cinematic,sizeof(struct Cinematic),_N)
#define ID_Cinematic_FileName 0xabe998d5 // Cinematic.FileName
#define ID_Cinematic_FrameRate 0xead5c9cf // Cinematic.FrameRate
#define ID_Cinematic_NumFrames 0x46a2037a // Cinematic.NumFrames
#define ID_Cinematic_FadeOut 0x314b07da // Cinematic.FadeOut
#define kCinematicNumProperties 4
enum CinematicProperties {
	kCinematicFileName,
	kCinematicFrameRate,
	kCinematicNumFrames,
	kCinematicFadeOut,
};
// Grid
#define ID_Grid 0x2fb366b1
#define GetGrid(_P) ((struct Grid*)((_P)?OBJ_GetComponent(_P,ID_Grid):NULL))
#define Grid_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Grid,sizeof(struct Grid),_N)
#define ID_Grid_Columns 0x5d28e334 // Grid.Columns
#define ID_Grid_Rows 0x1a2b78b0 // Grid.Rows
#define ID_Grid_Direction 0x252effe2 // Grid.Direction
#define ID_Grid_Spacing 0x5d2ffdd6 // Grid.Spacing
#define ID_Grid_CellWidth 0x534e3267 // Grid.CellWidth
#define ID_Grid_CellHeight 0xa97554ea // Grid.CellHeight
#define kGridNumProperties 6
enum GridProperties {
	kGridColumns,
	kGridRows,
	kGridDirection,
	kGridSpacing,
	kGridCellWidth,
	kGridCellHeight,
};
// ImageView
#define ID_ImageView 0xfe686eb3
#define GetImageView(_P) ((struct ImageView*)((_P)?OBJ_GetComponent(_P,ID_ImageView):NULL))
#define ImageView_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ImageView,sizeof(struct ImageView),_N)
#define ID_ImageView_Src 0xe2534f6b // ImageView.Src
#define ID_ImageView_Source 0xec613792 // ImageView.Source
#define ID_ImageView_Edges 0xbc1ab5c3 // ImageView.Edges
#define ID_ImageView_Insets 0xddc76755 // ImageView.Insets
#define ID_ImageView_Viewbox 0x4fff923f // ImageView.Viewbox
#define ID_ImageView_Stretch 0x13aa1da4 // ImageView.Stretch
#define kImageViewNumProperties 6
enum ImageViewProperties {
	kImageViewSrc,
	kImageViewSource,
	kImageViewEdges,
	kImageViewInsets,
	kImageViewViewbox,
	kImageViewStretch,
};
// NinePatchImage
#define ID_NinePatchImage 0xbd86cb1e
#define GetNinePatchImage(_P) ((struct NinePatchImage*)((_P)?OBJ_GetComponent(_P,ID_NinePatchImage):NULL))
#define NinePatchImage_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_NinePatchImage,sizeof(struct NinePatchImage),_N)
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
#define kNinePatchImageNumProperties 14
enum NinePatchImageProperties {
	kNinePatchImageStretchTypeTop,
	kNinePatchImageStretchTypeBottom,
	kNinePatchImageStretchTypeLeft,
	kNinePatchImageStretchTypeRight,
	kNinePatchImageStretchTypeCenter,
	kNinePatchImageImageTopLeft,
	kNinePatchImageImageTop,
	kNinePatchImageImageTopRight,
	kNinePatchImageImageLeft,
	kNinePatchImageImageCenter,
	kNinePatchImageImageRight,
	kNinePatchImageImageBottomLeft,
	kNinePatchImageImageBottom,
	kNinePatchImageImageBottomRight,
};
// ConsoleView
#define ID_ConsoleView 0x2ad7c563
#define GetConsoleView(_P) ((struct ConsoleView*)((_P)?OBJ_GetComponent(_P,ID_ConsoleView):NULL))
#define ConsoleView_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ConsoleView,sizeof(struct ConsoleView),_N)
#define ID_ConsoleView_BufferWidth 0x83573a57 // ConsoleView.BufferWidth
#define ID_ConsoleView_BufferHeight 0xce20e99a // ConsoleView.BufferHeight
#define ID_ConsoleView_Cursor 0x0e1d2fa5 // ConsoleView.Cursor
#define ID_ConsoleView_SelectedIndex 0x8f71f73a // ConsoleView.SelectedIndex
#define ID_ConsoleView_DropShadow 0x861e7f0e // ConsoleView.DropShadow
#define ID_ConsoleView_ContentHeight 0xcbb6893b // ConsoleView.ContentHeight
#define ID_ConsoleView_Println 0xb19dac78 // ConsoleView.Println
#define ID_ConsoleView_Erase 0xab38bbeb // ConsoleView.Erase
#define ID_ConsoleView_Invalidate 0xdc68fe5a // ConsoleView.Invalidate
#define ID_ConsoleView_Unpack 0xb7f62399 // ConsoleView.Unpack
#define ID_ConsoleView_GetIndexPosition 0x0848e624 // ConsoleView.GetIndexPosition
#define kConsoleViewNumProperties 11
enum ConsoleViewProperties {
	kConsoleViewBufferWidth,
	kConsoleViewBufferHeight,
	kConsoleViewCursor,
	kConsoleViewSelectedIndex,
	kConsoleViewDropShadow,
	kConsoleViewContentHeight,
	kConsoleViewPrintln,
	kConsoleViewErase,
	kConsoleViewInvalidate,
	kConsoleViewUnpack,
	kConsoleViewGetIndexPosition,
};
// Page
#define ID_Page 0xe83d9196
#define GetPage(_P) ((struct Page*)((_P)?OBJ_GetComponent(_P,ID_Page):NULL))
#define Page_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Page,sizeof(struct Page),_N)
#define ID_Page_Title 0x31e209ce // Page.Title
#define ID_Page_Path 0x20109b7f // Page.Path
#define ID_Page_Transition 0xb60425fb // Page.Transition
#define kPageNumProperties 3
enum PageProperties {
	kPageTitle,
	kPagePath,
	kPageTransition,
};
// PageHost
#define ID_PageHost 0x6f2dc6c0
#define GetPageHost(_P) ((struct PageHost*)((_P)?OBJ_GetComponent(_P,ID_PageHost):NULL))
#define PageHost_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PageHost,sizeof(struct PageHost),_N)
#define ID_PageHost_ActivePage 0xb276c4f7 // PageHost.ActivePage
#define ID_PageHost_NavigateToPage 0x3e8b196b // PageHost.NavigateToPage
#define ID_PageHost_NavigateBack 0x94f6fcce // PageHost.NavigateBack
#define kPageHostNumProperties 3
enum PageHostProperties {
	kPageHostActivePage,
	kPageHostNavigateToPage,
	kPageHostNavigateBack,
};
// PageViewport
#define ID_PageViewport 0x61a2c1e0
#define GetPageViewport(_P) ((struct PageViewport*)((_P)?OBJ_GetComponent(_P,ID_PageViewport):NULL))
#define PageViewport_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PageViewport,sizeof(struct PageViewport),_N)
#define kPageViewportNumProperties 0
// Style
#define ID_Style 0x5467ec76
#define GetStyle(_P) ((struct Style*)((_P)?OBJ_GetComponent(_P,ID_Style):NULL))
#define Style_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Style,sizeof(struct Style),_N)
#define ID_Style_TargetType 0x26cf4319 // Style.TargetType
#define ID_Style_Type 0x8f053d80 // Style.Type
#define kStyleNumProperties 2
enum StyleProperties {
	kStyleTargetType,
	kStyleType,
};


#define ID_FontShorthand 0x23a65347
#define ID_FontShorthand_Weight 0x5eb4d5c7 // FontShorthand.Weight
#define ID_FontShorthand_Style 0xcb02aa6c // FontShorthand.Style
#define ID_FontShorthand_Size 0x40564eca // FontShorthand.Size
#define ID_FontShorthand_Family 0xb31e25cb // FontShorthand.Family
#define ID_BrushShorthand 0x3da999da
#define ID_BrushShorthand_Color 0x278f75c3 // BrushShorthand.Color
#define ID_BrushShorthand_Image 0x4fae4039 // BrushShorthand.Image
#define ID_BrushShorthand_Material 0xcc574095 // BrushShorthand.Material
#define ID_ShadowShorthand 0x81dd8884
#define ID_ShadowShorthand_Offset 0x5a75df29 // ShadowShorthand.Offset
#define ID_ShadowShorthand_BlurRadius 0xc1e6e84d // ShadowShorthand.BlurRadius
#define ID_ShadowShorthand_SpreadRadius 0xd8e9e853 // ShadowShorthand.SpreadRadius
#define ID_ShadowShorthand_Color 0x6247f121 // ShadowShorthand.Color
#define ID_RingShorthand 0xd8587082
#define ID_RingShorthand_Offset 0x7b28e2df // RingShorthand.Offset
#define ID_RingShorthand_Width 0x04fa90e0 // RingShorthand.Width
#define ID_RingShorthand_Color 0xf820b9cb // RingShorthand.Color
#define ID_OverflowShorthand 0x05e8dc82
#define ID_OverflowShorthand_X 0x7f043b94 // OverflowShorthand.X
#define ID_OverflowShorthand_Y 0x80043d27 // OverflowShorthand.Y
#define ID_UnderlineShorthand 0x2fc1b56e
#define ID_UnderlineShorthand_Offset 0xa54f0483 // UnderlineShorthand.Offset
#define ID_UnderlineShorthand_Width 0xa5db78cc // UnderlineShorthand.Width
#define ID_UnderlineShorthand_Color 0x33df11f7 // UnderlineShorthand.Color
	

#define ID_TextBlockConcept_MakeTextEventArgs 0x57b8c928

#define ID_TextBlockConcept_MakeTextEventArgs_text 0x45faff79 // TextBlockConcept_MakeTextEventArgs.text
#define ID_TextBlockConcept_MakeTextEventArgs_availableSpace 0x674d6dc1 // TextBlockConcept_MakeTextEventArgs.availableSpace
#define ID_Node2D_DrawBrushEventArgs 0x4c43d1ab

#define ID_Node2D_DrawBrushEventArgs_projection 0x068baeea // Node2D_DrawBrushEventArgs.projection
#define ID_Node2D_DrawBrushEventArgs_image 0x86f13a44 // Node2D_DrawBrushEventArgs.image
#define ID_Node2D_DrawBrushEventArgs_brush 0x5e46bb1d // Node2D_DrawBrushEventArgs.brush
#define ID_Node2D_DrawBrushEventArgs_borderOffset 0xfcefe6a6 // Node2D_DrawBrushEventArgs.borderOffset
#define ID_Node2D_DrawBrushEventArgs_borderWidth 0x84bfc1f3 // Node2D_DrawBrushEventArgs.borderWidth
#define ID_Node2D_DrawBrushEventArgs_foreground 0xadede5f6 // Node2D_DrawBrushEventArgs.foreground
#define ID_Node2D_DrawBrushEventArgs_viewdef 0xd518da01 // Node2D_DrawBrushEventArgs.viewdef
#define ID_Node2D_MeasureEventArgs 0x40c88ef1

#define ID_Node2D_MeasureEventArgs_Width 0x141da717 // Node2D_MeasureEventArgs.Width
#define ID_Node2D_MeasureEventArgs_Height 0x06f6d85a // Node2D_MeasureEventArgs.Height
#define ID_Node2D_ArrangeEventArgs 0x6fd4edb1

#define ID_Node2D_ArrangeEventArgs_X 0x94d0e40f // Node2D_ArrangeEventArgs.X
#define ID_Node2D_ArrangeEventArgs_Y 0x93d0e27c // Node2D_ArrangeEventArgs.Y
#define ID_Node2D_ArrangeEventArgs_Width 0xc723b657 // Node2D_ArrangeEventArgs.Width
#define ID_Node2D_ArrangeEventArgs_Height 0x89081d9a // Node2D_ArrangeEventArgs.Height
#define ID_Node2D_MeasureOverrideEventArgs 0xcfdd08d9

#define ID_Node2D_MeasureOverrideEventArgs_Width 0x7640f96f // Node2D_MeasureOverrideEventArgs.Width
#define ID_Node2D_MeasureOverrideEventArgs_Height 0xa36a2d12 // Node2D_MeasureOverrideEventArgs.Height
#define ID_Node2D_ArrangeOverrideEventArgs 0xfc6e4f99

#define ID_Node2D_ArrangeOverrideEventArgs_X 0x1caa4037 // Node2D_ArrangeOverrideEventArgs.X
#define ID_Node2D_ArrangeOverrideEventArgs_Y 0x1baa3ea4 // Node2D_ArrangeOverrideEventArgs.Y
#define ID_Node2D_ArrangeOverrideEventArgs_Width 0xa8339daf // Node2D_ArrangeOverrideEventArgs.Width
#define ID_Node2D_ArrangeOverrideEventArgs_Height 0x01e78b52 // Node2D_ArrangeOverrideEventArgs.Height
#define ID_Node2D_ForegroundContentEventArgs 0x955c578f

#define ID_Node2D_UpdateGeometryEventArgs 0x13a29e26

#define ID_Node2D_SetScrollTopEventArgs 0x64d56721

#define ID_Node2D_SetScrollTopEventArgs_Value 0xce539fc2 // Node2D_SetScrollTopEventArgs.Value
#define ID_Button_ClickEventArgs 0x620fb91f

#define ID_Form_SubmitEventArgs 0x940ec545

#define ID_Screen_UpdateLayoutEventArgs 0xe31efe36

#define ID_Screen_UpdateLayoutEventArgs_Width 0xe73421c4 // Screen_UpdateLayoutEventArgs.Width
#define ID_Screen_UpdateLayoutEventArgs_Height 0x8cbbbaf3 // Screen_UpdateLayoutEventArgs.Height
#define ID_Screen_RenderScreenEventArgs 0xe82fdc27

#define ID_Screen_RenderScreenEventArgs_width 0x3b5d04ad // Screen_RenderScreenEventArgs.width
#define ID_Screen_RenderScreenEventArgs_height 0x4a6e2404 // Screen_RenderScreenEventArgs.height
#define ID_Screen_RenderScreenEventArgs_stereo 0x14234527 // Screen_RenderScreenEventArgs.stereo
#define ID_Screen_RenderScreenEventArgs_angle 0xd956701a // Screen_RenderScreenEventArgs.angle
#define ID_Screen_RenderScreenEventArgs_target 0xf85bed2e // Screen_RenderScreenEventArgs.target
#define ID_ConsoleView_PrintlnEventArgs 0xeefac1f6

#define ID_ConsoleView_PrintlnEventArgs_Index 0x1bb76f64 // ConsoleView_PrintlnEventArgs.Index
#define ID_ConsoleView_PrintlnEventArgs_Text 0x123171ef // ConsoleView_PrintlnEventArgs.Text
#define ID_ConsoleView_EraseEventArgs 0xfc2f86cb

#define ID_ConsoleView_InvalidateEventArgs 0xd692d03a

#define ID_ConsoleView_UnpackEventArgs 0x23743f27

#define ID_ConsoleView_UnpackEventArgs_X 0x0cca9609 // ConsoleView_UnpackEventArgs.X
#define ID_ConsoleView_UnpackEventArgs_Y 0x0bca9476 // ConsoleView_UnpackEventArgs.Y
#define ID_ConsoleView_GetIndexPositionEventArgs 0x69be81e4

#define ID_ConsoleView_GetIndexPositionEventArgs_Index 0xb627bac2 // ConsoleView_GetIndexPositionEventArgs.Index
#define ID_ConsoleView_GetIndexPositionEventArgs_OffsetX 0x1cc262c3 // ConsoleView_GetIndexPositionEventArgs.OffsetX
#define ID_ConsoleView_GetIndexPositionEventArgs_OffsetY 0x1bc26130 // ConsoleView_GetIndexPositionEventArgs.OffsetY
#define ID_ConsoleView_GetIndexPositionEventArgs_Global 0x20a91c49 // ConsoleView_GetIndexPositionEventArgs.Global
#define ID_PageHost_NavigateToPageEventArgs 0x0b5e4b0d

#define ID_PageHost_NavigateToPageEventArgs_URL 0x5085360a // PageHost_NavigateToPageEventArgs.URL
#define ID_PageHost_NavigateToPageEventArgs_TransitionType 0x972d66de // PageHost_NavigateToPageEventArgs.TransitionType
#define ID_PageHost_NavigateBackEventArgs 0xa3f6a342

#define ID_PageHost_NavigateBackEventArgs_TransitionType 0x0531cd67 // PageHost_NavigateBackEventArgs.TransitionType
	

#endif