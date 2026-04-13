// Auto-generated from UIKit.xml by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __UIKIT_PROPERTIES_H__
#define __UIKIT_PROPERTIES_H__

// DataObject
#define ID_DataObject 0xeb3560da
#define GetDataObject(_P) ((struct DataObject*)((_P)?OBJ_GetComponent(_P,ID_DataObject):NULL))
#define DataObject_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_DataObject,sizeof(struct DataObject),_N)
#define kDataObjectNumProperties 0
// Trigger
#define ID_Trigger 0xa5ea0da3
#define GetTrigger(_P) ((struct Trigger*)((_P)?OBJ_GetComponent(_P,ID_Trigger):NULL))
#define Trigger_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Trigger,sizeof(struct Trigger),_N)
#define ID_Trigger_Property 0x7a6c1a46 // Trigger.Property
#define ID_Trigger_Value 0x5229f3cc // Trigger.Value
#define ID_Trigger_Triggered 0x729dabf0 // Trigger.Triggered
#define kTriggerNumProperties 3
enum TriggerProperties {
	kTriggerProperty,
	kTriggerValue,
	kTriggerTriggered,
};
// OnPropertyChangedTrigger
#define ID_OnPropertyChangedTrigger 0xde0f7949
#define GetOnPropertyChangedTrigger(_P) ((struct OnPropertyChangedTrigger*)((_P)?OBJ_GetComponent(_P,ID_OnPropertyChangedTrigger):NULL))
#define OnPropertyChangedTrigger_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_OnPropertyChangedTrigger,sizeof(struct OnPropertyChangedTrigger),_N)
#define ID_OnPropertyChangedTrigger_SourceNode 0x5d55b664 // OnPropertyChangedTrigger.SourceNode
#define ID_OnPropertyChangedTrigger_Property 0x8664fe08 // OnPropertyChangedTrigger.Property
#define kOnPropertyChangedTriggerNumProperties 2
enum OnPropertyChangedTriggerProperties {
	kOnPropertyChangedTriggerSourceNode,
	kOnPropertyChangedTriggerProperty,
};
// OnAttachedTrigger
#define ID_OnAttachedTrigger 0x677fe670
#define GetOnAttachedTrigger(_P) ((struct OnAttachedTrigger*)((_P)?OBJ_GetComponent(_P,ID_OnAttachedTrigger):NULL))
#define OnAttachedTrigger_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_OnAttachedTrigger,sizeof(struct OnAttachedTrigger),_N)
#define kOnAttachedTriggerNumProperties 0
// EventTrigger
#define ID_EventTrigger 0x88bab1a1
#define GetEventTrigger(_P) ((struct EventTrigger*)((_P)?OBJ_GetComponent(_P,ID_EventTrigger):NULL))
#define EventTrigger_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_EventTrigger,sizeof(struct EventTrigger),_N)
#define ID_EventTrigger_RoutedEvent 0x0c5f2982 // EventTrigger.RoutedEvent
#define kEventTriggerNumProperties 1
enum EventTriggerProperties {
	kEventTriggerRoutedEvent,
};
// Setter
#define ID_Setter 0xf849aee6
#define GetSetter(_P) ((struct Setter*)((_P)?OBJ_GetComponent(_P,ID_Setter):NULL))
#define Setter_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Setter,sizeof(struct Setter),_N)
#define ID_Setter_Trigger 0xd1fad954 // Setter.Trigger
#define ID_Setter_Property 0x89d17a41 // Setter.Property
#define ID_Setter_Value 0x8fd76d09 // Setter.Value
#define kSetterNumProperties 3
enum SetterProperties {
	kSetterTrigger,
	kSetterProperty,
	kSetterValue,
};
// Handler
#define ID_Handler 0x04d66a13
#define GetHandler(_P) ((struct Handler*)((_P)?OBJ_GetComponent(_P,ID_Handler):NULL))
#define Handler_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Handler,sizeof(struct Handler),_N)
#define ID_Handler_Trigger 0x0ef3b2a9 // Handler.Trigger
#define ID_Handler_Target 0x2798724a // Handler.Target
#define ID_Handler_Function 0x595dd2a7 // Handler.Function
#define kHandlerNumProperties 3
enum HandlerProperties {
	kHandlerTrigger,
	kHandlerTarget,
	kHandlerFunction,
};
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
// Node
#define ID_Node 0x3468032d
#define GetNode(_P) ((struct Node*)((_P)?OBJ_GetComponent(_P,ID_Node):NULL))
#define Node_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Node,sizeof(struct Node),_N)
#define ID_Node_Size 0xc8371588 // Node.Size
#define ID_Node_HorizontalSize 0x8dd5feec // Node.HorizontalSize
#define ID_Node_Width 0xc28a97d3 // Node.Width
#define ID_Node_DesiredWidth 0x3aae910b // Node.DesiredWidth
#define ID_Node_MinWidth 0x1ebf4605 // Node.MinWidth
#define ID_Node_ActualWidth 0xf66f0265 // Node.ActualWidth
#define ID_Node_ScrollWidth 0x7ccec714 // Node.ScrollWidth
#define ID_Node_VerticalSize 0x41e70316 // Node.VerticalSize
#define ID_Node_Height 0x5615e70e // Node.Height
#define ID_Node_DesiredHeight 0x18b527e6 // Node.DesiredHeight
#define ID_Node_MinHeight 0x7a45235c // Node.MinHeight
#define ID_Node_ActualHeight 0xfca3503c // Node.ActualHeight
#define ID_Node_ScrollHeight 0x4288c2c3 // Node.ScrollHeight
#define ID_Node_DepthSize 0x86dbf73f // Node.DepthSize
#define ID_Node_Depth 0x9aed1cde // Node.Depth
#define ID_Node_DesiredDepth 0xbbf1c3e6 // Node.DesiredDepth
#define ID_Node_MinDepth 0x63a6319c // Node.MinDepth
#define ID_Node_ActualDepth 0x38d9f1fc // Node.ActualDepth
#define ID_Node_ScrollDepth 0x580ae93d // Node.ScrollDepth
#define ID_Node_Margin 0x3b2ed3f7 // Node.Margin
#define ID_Node_HorizontalMargin 0xe4abc3eb // Node.HorizontalMargin
#define ID_Node_MarginLeft 0xae3827de // Node.MarginLeft
#define ID_Node_MarginRight 0xf5c8d933 // Node.MarginRight
#define ID_Node_VerticalMargin 0x73189bc9 // Node.VerticalMargin
#define ID_Node_MarginTop 0x720b58b6 // Node.MarginTop
#define ID_Node_MarginBottom 0xd1367ec8 // Node.MarginBottom
#define ID_Node_DepthMargin 0x45ad5268 // Node.DepthMargin
#define ID_Node_MarginFront 0xb567a99e // Node.MarginFront
#define ID_Node_MarginBack 0x81d2d3e0 // Node.MarginBack
#define ID_Node_Padding 0xc7721c32 // Node.Padding
#define ID_Node_HorizontalPadding 0x18bd87c6 // Node.HorizontalPadding
#define ID_Node_PaddingLeft 0xbf34d9ff // Node.PaddingLeft
#define ID_Node_PaddingRight 0x6f0935b0 // Node.PaddingRight
#define ID_Node_VerticalPadding 0xb2aba4fc // Node.VerticalPadding
#define ID_Node_PaddingTop 0x42f6b495 // Node.PaddingTop
#define ID_Node_PaddingBottom 0x06044a21 // Node.PaddingBottom
#define ID_Node_DepthPadding 0x70b717ef // Node.DepthPadding
#define ID_Node_PaddingFront 0x0ac7b2b5 // Node.PaddingFront
#define ID_Node_PaddingBack 0xd22b7a31 // Node.PaddingBack
#define ID_Node_Border 0x2a3ce9f3 // Node.Border
#define ID_Node_BorderWidth 0x52e9a9e5 // Node.BorderWidth
#define ID_Node_HorizontalBorderWidth 0xae7c2f49 // Node.HorizontalBorderWidth
#define ID_Node_BorderWidthLeft 0x86dcb870 // Node.BorderWidthLeft
#define ID_Node_BorderWidthRight 0xfa6751e5 // Node.BorderWidthRight
#define ID_Node_VerticalBorderWidth 0xd5d44d93 // Node.VerticalBorderWidth
#define ID_Node_BorderWidthTop 0x4587603c // Node.BorderWidthTop
#define ID_Node_BorderWidthBottom 0x31ac172a // Node.BorderWidthBottom
#define ID_Node_DepthBorderWidth 0x0c1d04f8 // Node.DepthBorderWidth
#define ID_Node_BorderWidthFront 0x62fdffd8 // Node.BorderWidthFront
#define ID_Node_BorderWidthBack 0xd045eda2 // Node.BorderWidthBack
#define ID_Node_BorderColor 0x85e5d732 // Node.BorderColor
#define ID_Node_BorderStyle 0x88c40c44 // Node.BorderStyle
#define ID_Node_BorderRadius 0x867eed01 // Node.BorderRadius
#define ID_Node_BorderTopLeftRadius 0xa13859b5 // Node.BorderTopLeftRadius
#define ID_Node_BorderTopRightRadius 0xbe725f4e // Node.BorderTopRightRadius
#define ID_Node_BorderBottomRightRadius 0xe9173b64 // Node.BorderBottomRightRadius
#define ID_Node_BorderBottomLeftRadius 0x9511fd4b // Node.BorderBottomLeftRadius
#define ID_Node_Alignment 0x6ae48d82 // Node.Alignment
#define ID_Node_HorizontalAlignment 0xe230b1ee // Node.HorizontalAlignment
#define ID_Node_VerticalAlignment 0x1c45d168 // Node.VerticalAlignment
#define ID_Node_DepthAlignment 0x5b191ce3 // Node.DepthAlignment
#define ID_Node_Visible 0xe1936ee5 // Node.Visible
#define ID_Node_QuickHide 0x7e26e1b0 // Node.QuickHide
#define ID_Node_VisibleAmountInParent 0xbe0696b2 // Node.VisibleAmountInParent
#define ID_Node_Opacity 0xb6882472 // Node.Opacity
#define ID_Node_Tags 0xec56af24 // Node.Tags
#define ID_Node_DataContext 0x80b43db0 // Node.DataContext
#define ID_Node_Resources 0xa9ef04ee // Node.Resources
#define ID_Node_NumResources 0xce1c8f30 // Node.NumResources
#define ID_Node_Awake 0x2facb9c8 // Node.Awake
#define ID_Node_UpdateMatrix 0x35cdb821 // Node.UpdateMatrix
#define ID_Node_LoadView 0xe3c6ed08 // Node.LoadView
#define ID_Node_HitTest 0xbc0e5546 // Node.HitTest
#define ID_Node_IsVisible 0xa03cfb85 // Node.IsVisible
#define ID_Node_ViewDidLoad 0x71bab7e1 // Node.ViewDidLoad
#define ID_Node_KillFocus 0x2bdb09fb // Node.KillFocus
#define ID_Node_SetFocus 0x5f1c2071 // Node.SetFocus
#define ID_Node_GetSize 0x2282f1da // Node.GetSize
#define ID_Node_MouseMessage 0x8633bad7 // Node.MouseMessage
#define ID_Node_LeftMouseDown 0x34ecec23 // Node.LeftMouseDown
#define ID_Node_RightMouseDown 0x379ed064 // Node.RightMouseDown
#define ID_Node_OtherMouseDown 0x5b61ba9c // Node.OtherMouseDown
#define ID_Node_LeftMouseUp 0x87d20362 // Node.LeftMouseUp
#define ID_Node_RightMouseUp 0xf135e605 // Node.RightMouseUp
#define ID_Node_OtherMouseUp 0x51ca895d // Node.OtherMouseUp
#define ID_Node_LeftMouseDragged 0x845ee787 // Node.LeftMouseDragged
#define ID_Node_RightMouseDragged 0xc2e805f6 // Node.RightMouseDragged
#define ID_Node_OtherMouseDragged 0xbab6f63e // Node.OtherMouseDragged
#define ID_Node_LeftDoubleClick 0x4de79e13 // Node.LeftDoubleClick
#define ID_Node_RightDoubleClick 0x9104b1b4 // Node.RightDoubleClick
#define ID_Node_OtherDoubleClick 0x4a967bfc // Node.OtherDoubleClick
#define ID_Node_MouseMoved 0x9922448b // Node.MouseMoved
#define ID_Node_ScrollWheel 0x54e1d1f7 // Node.ScrollWheel
#define ID_Node_DragDrop 0xc40ddaee // Node.DragDrop
#define ID_Node_DragEnter 0xadc27c43 // Node.DragEnter
#define ID_Node_KeyMessage 0x8c3d8ec7 // Node.KeyMessage
#define ID_Node_KeyDown 0xfc8dc15c // Node.KeyDown
#define ID_Node_KeyUp 0x9282bc1d // Node.KeyUp
#define ID_Node_TextInput 0xaf29a9ae // Node.TextInput
#define kNodeNumProperties 99
enum NodeProperties {
	kNodeSize,
	kNodeHorizontalSize,
	kNodeWidth,
	kNodeDesiredWidth,
	kNodeMinWidth,
	kNodeActualWidth,
	kNodeScrollWidth,
	kNodeVerticalSize,
	kNodeHeight,
	kNodeDesiredHeight,
	kNodeMinHeight,
	kNodeActualHeight,
	kNodeScrollHeight,
	kNodeDepthSize,
	kNodeDepth,
	kNodeDesiredDepth,
	kNodeMinDepth,
	kNodeActualDepth,
	kNodeScrollDepth,
	kNodeMargin,
	kNodeHorizontalMargin,
	kNodeMarginLeft,
	kNodeMarginRight,
	kNodeVerticalMargin,
	kNodeMarginTop,
	kNodeMarginBottom,
	kNodeDepthMargin,
	kNodeMarginFront,
	kNodeMarginBack,
	kNodePadding,
	kNodeHorizontalPadding,
	kNodePaddingLeft,
	kNodePaddingRight,
	kNodeVerticalPadding,
	kNodePaddingTop,
	kNodePaddingBottom,
	kNodeDepthPadding,
	kNodePaddingFront,
	kNodePaddingBack,
	kNodeBorder,
	kNodeBorderWidth,
	kNodeHorizontalBorderWidth,
	kNodeBorderWidthLeft,
	kNodeBorderWidthRight,
	kNodeVerticalBorderWidth,
	kNodeBorderWidthTop,
	kNodeBorderWidthBottom,
	kNodeDepthBorderWidth,
	kNodeBorderWidthFront,
	kNodeBorderWidthBack,
	kNodeBorderColor,
	kNodeBorderStyle,
	kNodeBorderRadius,
	kNodeBorderTopLeftRadius,
	kNodeBorderTopRightRadius,
	kNodeBorderBottomRightRadius,
	kNodeBorderBottomLeftRadius,
	kNodeAlignment,
	kNodeHorizontalAlignment,
	kNodeVerticalAlignment,
	kNodeDepthAlignment,
	kNodeVisible,
	kNodeQuickHide,
	kNodeVisibleAmountInParent,
	kNodeOpacity,
	kNodeTags,
	kNodeDataContext,
	kNodeResources,
	kNodeNumResources,
	kNodeAwake,
	kNodeUpdateMatrix,
	kNodeLoadView,
	kNodeHitTest,
	kNodeIsVisible,
	kNodeViewDidLoad,
	kNodeKillFocus,
	kNodeSetFocus,
	kNodeGetSize,
	kNodeMouseMessage,
	kNodeLeftMouseDown,
	kNodeRightMouseDown,
	kNodeOtherMouseDown,
	kNodeLeftMouseUp,
	kNodeRightMouseUp,
	kNodeOtherMouseUp,
	kNodeLeftMouseDragged,
	kNodeRightMouseDragged,
	kNodeOtherMouseDragged,
	kNodeLeftDoubleClick,
	kNodeRightDoubleClick,
	kNodeOtherDoubleClick,
	kNodeMouseMoved,
	kNodeScrollWheel,
	kNodeDragDrop,
	kNodeDragEnter,
	kNodeKeyMessage,
	kNodeKeyDown,
	kNodeKeyUp,
	kNodeTextInput,
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
#define kButtonNumProperties 1
enum ButtonProperties {
	kButtonType,
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
#define kConsoleViewNumProperties 9
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


#define ID_ResourceEntry 0xf0906883
#define ID_ResourceEntry_Key 0x97d0396e // ResourceEntry.Key
#define ID_ResourceEntry_Value 0xd86983ec // ResourceEntry.Value
#define ID_ResourceEntry_Type 0x5085e4af // ResourceEntry.Type
#define ID_CornerRadius 0x0485b456
#define ID_CornerRadius_TopLeftRadius 0xd5413658 // CornerRadius.TopLeftRadius
#define ID_CornerRadius_TopRightRadius 0x6387c115 // CornerRadius.TopRightRadius
#define ID_CornerRadius_BottomRightRadius 0x3b94d08d // CornerRadius.BottomRightRadius
#define ID_CornerRadius_BottomLeftRadius 0x8ca4dfb0 // CornerRadius.BottomLeftRadius
#define ID_EdgeShorthand 0x3cbf2c0b
#define ID_EdgeShorthand_Left 0xb21cdbb6 // EdgeShorthand.Left
#define ID_EdgeShorthand_Right 0xee8e65eb // EdgeShorthand.Right
#define ID_AlignmentShorthand 0x5d30eea9
#define ID_AlignmentShorthand_Horizontal 0xfa3ad949 // AlignmentShorthand.Horizontal
#define ID_AlignmentShorthand_Vertical 0x455c521b // AlignmentShorthand.Vertical
#define ID_AlignmentShorthand_Depth 0xdf90274a // AlignmentShorthand.Depth
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
#define ID_Thickness 0xce483eb5
#define ID_Thickness_Horizontal 0x0f36bd4d // Thickness.Horizontal
#define ID_Thickness_Left 0x03d812ac // Thickness.Left
#define ID_Thickness_Right 0xe24fcbb9 // Thickness.Right
#define ID_Thickness_Vertical 0x55c6bb67 // Thickness.Vertical
#define ID_Thickness_Top 0x06162ab0 // Thickness.Top
#define ID_Thickness_Bottom 0x5a2f9fae // Thickness.Bottom
#define ID_Thickness_Depth 0x8ecdafa6 // Thickness.Depth
#define ID_Thickness_Front 0xd745e23c // Thickness.Front
#define ID_Thickness_Back 0x143862ae // Thickness.Back
#define ID_BorderShorthand 0x317c0db6
#define ID_BorderShorthand_Width 0x0f2cb544 // BorderShorthand.Width
#define ID_BorderShorthand_HorizontalWidth 0xd6fbc4b4 // BorderShorthand.HorizontalWidth
#define ID_BorderShorthand_WidthLeft 0x608ae485 // BorderShorthand.WidthLeft
#define ID_BorderShorthand_WidthRight 0x7e80ddde // BorderShorthand.WidthRight
#define ID_BorderShorthand_VerticalWidth 0x96d0e7ee // BorderShorthand.VerticalWidth
#define ID_BorderShorthand_WidthTop 0xab886547 // BorderShorthand.WidthTop
#define ID_BorderShorthand_WidthBottom 0x432470af // BorderShorthand.WidthBottom
#define ID_BorderShorthand_DepthWidth 0xc868f077 // BorderShorthand.DepthWidth
#define ID_BorderShorthand_WidthFront 0x04871823 // BorderShorthand.WidthFront
#define ID_BorderShorthand_WidthBack 0xa404adcf // BorderShorthand.WidthBack
#define ID_BorderShorthand_Color 0x77ce2ebf // BorderShorthand.Color
#define ID_BorderShorthand_Style 0x60399c3d // BorderShorthand.Style
#define ID_BorderShorthand_Radius 0xaf3f00de // BorderShorthand.Radius
#define ID_BorderShorthand_RadiusTopLeftRadius 0xaa72a01a // BorderShorthand.RadiusTopLeftRadius
#define ID_BorderShorthand_RadiusTopRightRadius 0xc38f432f // BorderShorthand.RadiusTopRightRadius
#define ID_BorderShorthand_RadiusBottomRightRadius 0x8e31b4f3 // BorderShorthand.RadiusBottomRightRadius
#define ID_BorderShorthand_RadiusBottomLeftRadius 0x00bb0796 // BorderShorthand.RadiusBottomLeftRadius
#define ID_SizeAxisShorthand 0xa05ccb28
#define ID_SizeAxisShorthand_Requested 0xf9a8147e // SizeAxisShorthand.Requested
#define ID_SizeAxisShorthand_Desired 0x48f908f0 // SizeAxisShorthand.Desired
#define ID_SizeAxisShorthand_Min 0x796bc446 // SizeAxisShorthand.Min
#define ID_SizeAxisShorthand_Actual 0x18fecf94 // SizeAxisShorthand.Actual
#define ID_SizeAxisShorthand_Scroll 0xe09d850b // SizeAxisShorthand.Scroll
#define ID_SizeShorthand 0x02bfd76b
#define ID_SizeShorthand_Horizontal 0xa8ee570f // SizeShorthand.Horizontal
#define ID_SizeShorthand_HorizontalRequested 0x17543bb1 // SizeShorthand.HorizontalRequested
#define ID_SizeShorthand_HorizontalDesired 0x385f735b // SizeShorthand.HorizontalDesired
#define ID_SizeShorthand_HorizontalMin 0xf14ba409 // SizeShorthand.HorizontalMin
#define ID_SizeShorthand_HorizontalActual 0x228bb6a5 // SizeShorthand.HorizontalActual
#define ID_SizeShorthand_HorizontalScroll 0x772de84e // SizeShorthand.HorizontalScroll
#define ID_SizeShorthand_Vertical 0x032084c1 // SizeShorthand.Vertical
#define ID_SizeShorthand_VerticalRequested 0xe6ff1c17 // SizeShorthand.VerticalRequested
#define ID_SizeShorthand_VerticalDesired 0x855e9309 // SizeShorthand.VerticalDesired
#define ID_SizeShorthand_VerticalMin 0x5c248b5b // SizeShorthand.VerticalMin
#define ID_SizeShorthand_VerticalActual 0x793bc5eb // SizeShorthand.VerticalActual
#define ID_SizeShorthand_VerticalScroll 0xf4aafd10 // SizeShorthand.VerticalScroll
#define ID_SizeShorthand_Depth 0x6026d818 // SizeShorthand.Depth
#define ID_SizeShorthand_DepthRequested 0x4cfbea00 // SizeShorthand.DepthRequested
#define ID_SizeShorthand_DepthDesired 0xfce7044a // SizeShorthand.DepthDesired
#define ID_SizeShorthand_DepthMin 0x1a9d1274 // SizeShorthand.DepthMin
#define ID_SizeShorthand_DepthActual 0x3967040e // SizeShorthand.DepthActual
#define ID_SizeShorthand_DepthScroll 0x60ff3f79 // SizeShorthand.DepthScroll
	

#define ID_Trigger_TriggeredEventArgs 0x86976046

#define ID_Trigger_TriggeredEventArgs_Trigger 0xa8841074 // Trigger_TriggeredEventArgs.Trigger
#define ID_Node_AwakeEventArgs 0x53009d32

#define ID_Node_UpdateMatrixEventArgs 0xb9c33fb7

#define ID_Node_UpdateMatrixEventArgs_parent 0xe966f5d3 // Node_UpdateMatrixEventArgs.parent
#define ID_Node_UpdateMatrixEventArgs_opacity 0x1bb1efd0 // Node_UpdateMatrixEventArgs.opacity
#define ID_Node_UpdateMatrixEventArgs_force 0x04845b66 // Node_UpdateMatrixEventArgs.force
#define ID_Node_LoadViewEventArgs 0x361e935c

#define ID_Node_LoadViewEventArgs_lua_state 0x96df0d0e // Node_LoadViewEventArgs.lua_state
#define ID_Node_HitTestEventArgs 0xbac9d590

#define ID_Node_HitTestEventArgs_x 0xf421a846 // Node_HitTestEventArgs.x
#define ID_Node_HitTestEventArgs_y 0xf521a9d9 // Node_HitTestEventArgs.y
#define ID_Node_IsVisibleEventArgs 0x6a63fae9

#define ID_Node_ViewDidLoadEventArgs 0x2d915aa5

#define ID_Node_KillFocusEventArgs 0xb6f00bfb

#define ID_Node_SetFocusEventArgs 0x9a763fc7

#define ID_Node_GetSizeEventArgs 0x81cb33d8

#define ID_Node_MouseMessageEventArgs 0x4b089af9

#define ID_Node_MouseMessageEventArgs_x 0x97ff5477 // Node_MouseMessageEventArgs.x
#define ID_Node_MouseMessageEventArgs_y 0x96ff52e4 // Node_MouseMessageEventArgs.y
#define ID_Node_MouseMessageEventArgs_deltaX 0xaa801d6b // Node_MouseMessageEventArgs.deltaX
#define ID_Node_MouseMessageEventArgs_deltaY 0xa9801bd8 // Node_MouseMessageEventArgs.deltaY
#define ID_Node_MouseMessageEventArgs_button 0x563796c1 // Node_MouseMessageEventArgs.button
#define ID_Node_MouseMessageEventArgs_clickCount 0x457c8aa6 // Node_MouseMessageEventArgs.clickCount
#define ID_Node_LeftMouseDownEventArgs 0xeaaeccfb

#define ID_Node_LeftMouseDownEventArgs_x 0xaef95f95 // Node_LeftMouseDownEventArgs.x
#define ID_Node_LeftMouseDownEventArgs_y 0xadf95e02 // Node_LeftMouseDownEventArgs.y
#define ID_Node_LeftMouseDownEventArgs_deltaX 0x3654571d // Node_LeftMouseDownEventArgs.deltaX
#define ID_Node_LeftMouseDownEventArgs_deltaY 0x3554558a // Node_LeftMouseDownEventArgs.deltaY
#define ID_Node_LeftMouseDownEventArgs_button 0x2c230f5f // Node_LeftMouseDownEventArgs.button
#define ID_Node_LeftMouseDownEventArgs_clickCount 0xfbb27808 // Node_LeftMouseDownEventArgs.clickCount
#define ID_Node_RightMouseDownEventArgs 0xa21e5cb8

#define ID_Node_RightMouseDownEventArgs_x 0x2bbfcdae // Node_RightMouseDownEventArgs.x
#define ID_Node_RightMouseDownEventArgs_y 0x2cbfcf41 // Node_RightMouseDownEventArgs.y
#define ID_Node_RightMouseDownEventArgs_deltaX 0xf69f68a4 // Node_RightMouseDownEventArgs.deltaX
#define ID_Node_RightMouseDownEventArgs_deltaY 0xf79f6a37 // Node_RightMouseDownEventArgs.deltaY
#define ID_Node_RightMouseDownEventArgs_button 0x41ea39ea // Node_RightMouseDownEventArgs.button
#define ID_Node_RightMouseDownEventArgs_clickCount 0x52c29d69 // Node_RightMouseDownEventArgs.clickCount
#define ID_Node_OtherMouseDownEventArgs 0x7ca437d0

#define ID_Node_OtherMouseDownEventArgs_x 0x4e05ab06 // Node_OtherMouseDownEventArgs.x
#define ID_Node_OtherMouseDownEventArgs_y 0x4f05ac99 // Node_OtherMouseDownEventArgs.y
#define ID_Node_OtherMouseDownEventArgs_deltaX 0x6265dc3c // Node_OtherMouseDownEventArgs.deltaX
#define ID_Node_OtherMouseDownEventArgs_deltaY 0x6365ddcf // Node_OtherMouseDownEventArgs.deltaY
#define ID_Node_OtherMouseDownEventArgs_button 0x4efec082 // Node_OtherMouseDownEventArgs.button
#define ID_Node_OtherMouseDownEventArgs_clickCount 0x777f1591 // Node_OtherMouseDownEventArgs.clickCount
#define ID_Node_LeftMouseUpEventArgs 0x77942658

#define ID_Node_LeftMouseUpEventArgs_x 0x6f03f5ce // Node_LeftMouseUpEventArgs.x
#define ID_Node_LeftMouseUpEventArgs_y 0x7003f761 // Node_LeftMouseUpEventArgs.y
#define ID_Node_LeftMouseUpEventArgs_deltaX 0x47173104 // Node_LeftMouseUpEventArgs.deltaX
#define ID_Node_LeftMouseUpEventArgs_deltaY 0x48173297 // Node_LeftMouseUpEventArgs.deltaY
#define ID_Node_LeftMouseUpEventArgs_button 0x4fe7348a // Node_LeftMouseUpEventArgs.button
#define ID_Node_LeftMouseUpEventArgs_clickCount 0xd07b9e49 // Node_LeftMouseUpEventArgs.clickCount
#define ID_Node_RightMouseUpEventArgs 0xdd5538ef

#define ID_Node_RightMouseUpEventArgs_x 0x54820231 // Node_RightMouseUpEventArgs.x
#define ID_Node_RightMouseUpEventArgs_y 0x5382009e // Node_RightMouseUpEventArgs.y
#define ID_Node_RightMouseUpEventArgs_deltaX 0x0e0af2f1 // Node_RightMouseUpEventArgs.deltaX
#define ID_Node_RightMouseUpEventArgs_deltaY 0x0d0af15e // Node_RightMouseUpEventArgs.deltaY
#define ID_Node_RightMouseUpEventArgs_button 0x3bd7d75b // Node_RightMouseUpEventArgs.button
#define ID_Node_RightMouseUpEventArgs_clickCount 0xff0d7974 // Node_RightMouseUpEventArgs.clickCount
#define ID_Node_OtherMouseUpEventArgs 0x2bc11d87

#define ID_Node_OtherMouseUpEventArgs_x 0xb3aa5809 // Node_OtherMouseUpEventArgs.x
#define ID_Node_OtherMouseUpEventArgs_y 0xb2aa5676 // Node_OtherMouseUpEventArgs.y
#define ID_Node_OtherMouseUpEventArgs_deltaX 0x593eb3a9 // Node_OtherMouseUpEventArgs.deltaX
#define ID_Node_OtherMouseUpEventArgs_deltaY 0x583eb216 // Node_OtherMouseUpEventArgs.deltaY
#define ID_Node_OtherMouseUpEventArgs_button 0xf3978bb3 // Node_OtherMouseUpEventArgs.button
#define ID_Node_OtherMouseUpEventArgs_clickCount 0x9fa5dfdc // Node_OtherMouseUpEventArgs.clickCount
#define ID_Node_LeftMouseDraggedEventArgs 0xd1c109f9

#define ID_Node_LeftMouseDraggedEventArgs_x 0x9c8adb77 // Node_LeftMouseDraggedEventArgs.x
#define ID_Node_LeftMouseDraggedEventArgs_y 0x9b8ad9e4 // Node_LeftMouseDraggedEventArgs.y
#define ID_Node_LeftMouseDraggedEventArgs_deltaX 0xe438f26b // Node_LeftMouseDraggedEventArgs.deltaX
#define ID_Node_LeftMouseDraggedEventArgs_deltaY 0xe338f0d8 // Node_LeftMouseDraggedEventArgs.deltaY
#define ID_Node_LeftMouseDraggedEventArgs_button 0x8ff06bc1 // Node_LeftMouseDraggedEventArgs.button
#define ID_Node_LeftMouseDraggedEventArgs_clickCount 0x332dafa6 // Node_LeftMouseDraggedEventArgs.clickCount
#define ID_Node_RightMouseDraggedEventArgs 0x8fdfdde4

#define ID_Node_RightMouseDraggedEventArgs_x 0xf2177ef2 // Node_RightMouseDraggedEventArgs.x
#define ID_Node_RightMouseDraggedEventArgs_y 0xf3178085 // Node_RightMouseDraggedEventArgs.y
#define ID_Node_RightMouseDraggedEventArgs_deltaX 0xd4c9d1a0 // Node_RightMouseDraggedEventArgs.deltaX
#define ID_Node_RightMouseDraggedEventArgs_deltaY 0xd5c9d333 // Node_RightMouseDraggedEventArgs.deltaY
#define ID_Node_RightMouseDraggedEventArgs_button 0x6d81777e // Node_RightMouseDraggedEventArgs.button
#define ID_Node_RightMouseDraggedEventArgs_clickCount 0xf395ed2d // Node_RightMouseDraggedEventArgs.clickCount
#define ID_Node_OtherMouseDraggedEventArgs 0x891d3a4c

#define ID_Node_OtherMouseDraggedEventArgs_x 0x2393999a // Node_OtherMouseDraggedEventArgs.x
#define ID_Node_OtherMouseDraggedEventArgs_y 0x24939b2d // Node_OtherMouseDraggedEventArgs.y
#define ID_Node_OtherMouseDraggedEventArgs_deltaX 0xf2dbed68 // Node_OtherMouseDraggedEventArgs.deltaX
#define ID_Node_OtherMouseDraggedEventArgs_deltaY 0xf3dbeefb // Node_OtherMouseDraggedEventArgs.deltaY
#define ID_Node_OtherMouseDraggedEventArgs_button 0xef7fbb06 // Node_OtherMouseDraggedEventArgs.button
#define ID_Node_OtherMouseDraggedEventArgs_clickCount 0x35f6b465 // Node_OtherMouseDraggedEventArgs.clickCount
#define ID_Node_LeftDoubleClickEventArgs 0x031dea9b

#define ID_Node_LeftDoubleClickEventArgs_x 0xe488fbb5 // Node_LeftDoubleClickEventArgs.x
#define ID_Node_LeftDoubleClickEventArgs_y 0xe388fa22 // Node_LeftDoubleClickEventArgs.y
#define ID_Node_LeftDoubleClickEventArgs_deltaX 0x676b4c7d // Node_LeftDoubleClickEventArgs.deltaX
#define ID_Node_LeftDoubleClickEventArgs_deltaY 0x666b4aea // Node_LeftDoubleClickEventArgs.deltaY
#define ID_Node_LeftDoubleClickEventArgs_button 0x1d3b32ff // Node_LeftDoubleClickEventArgs.button
#define ID_Node_LeftDoubleClickEventArgs_clickCount 0xf8711868 // Node_LeftDoubleClickEventArgs.clickCount
#define ID_Node_RightDoubleClickEventArgs 0x3b464ab8

#define ID_Node_RightDoubleClickEventArgs_x 0xe08c6bae // Node_RightDoubleClickEventArgs.x
#define ID_Node_RightDoubleClickEventArgs_y 0xe18c6d41 // Node_RightDoubleClickEventArgs.y
#define ID_Node_RightDoubleClickEventArgs_deltaX 0x804dc2a4 // Node_RightDoubleClickEventArgs.deltaX
#define ID_Node_RightDoubleClickEventArgs_deltaY 0x814dc437 // Node_RightDoubleClickEventArgs.deltaY
#define ID_Node_RightDoubleClickEventArgs_button 0xcb9893ea // Node_RightDoubleClickEventArgs.button
#define ID_Node_RightDoubleClickEventArgs_clickCount 0xd77c9769 // Node_RightDoubleClickEventArgs.clickCount
#define ID_Node_OtherDoubleClickEventArgs 0x2eeca4a0

#define ID_Node_OtherDoubleClickEventArgs_x 0xca97cdf6 // Node_OtherDoubleClickEventArgs.x
#define ID_Node_OtherDoubleClickEventArgs_y 0xcb97cf89 // Node_OtherDoubleClickEventArgs.y
#define ID_Node_OtherDoubleClickEventArgs_deltaX 0x906143ac // Node_OtherDoubleClickEventArgs.deltaX
#define ID_Node_OtherDoubleClickEventArgs_deltaY 0x9161453f // Node_OtherDoubleClickEventArgs.deltaY
#define ID_Node_OtherDoubleClickEventArgs_button 0xf76bfe52 // Node_OtherDoubleClickEventArgs.button
#define ID_Node_OtherDoubleClickEventArgs_clickCount 0xc9ffe6c1 // Node_OtherDoubleClickEventArgs.clickCount
#define ID_Node_MouseMovedEventArgs 0xdeaf328d

#define ID_Node_MouseMovedEventArgs_x 0xb343e433 // Node_MouseMovedEventArgs.x
#define ID_Node_MouseMovedEventArgs_y 0xb243e2a0 // Node_MouseMovedEventArgs.y
#define ID_Node_MouseMovedEventArgs_deltaX 0x4cdbf8ef // Node_MouseMovedEventArgs.deltaX
#define ID_Node_MouseMovedEventArgs_deltaY 0x4bdbf75c // Node_MouseMovedEventArgs.deltaY
#define ID_Node_MouseMovedEventArgs_button 0x4e82eb0d // Node_MouseMovedEventArgs.button
#define ID_Node_MouseMovedEventArgs_clickCount 0xefc1d982 // Node_MouseMovedEventArgs.clickCount
#define ID_Node_ScrollWheelEventArgs 0x00b0123b

#define ID_Node_ScrollWheelEventArgs_x 0xdb001055 // Node_ScrollWheelEventArgs.x
#define ID_Node_ScrollWheelEventArgs_y 0xda000ec2 // Node_ScrollWheelEventArgs.y
#define ID_Node_ScrollWheelEventArgs_deltaX 0xf28672dd // Node_ScrollWheelEventArgs.deltaX
#define ID_Node_ScrollWheelEventArgs_deltaY 0xf186714a // Node_ScrollWheelEventArgs.deltaY
#define ID_Node_ScrollWheelEventArgs_button 0x8d22851f // Node_ScrollWheelEventArgs.button
#define ID_Node_ScrollWheelEventArgs_clickCount 0x62fcf1c8 // Node_ScrollWheelEventArgs.clickCount
#define ID_Node_DragDropEventArgs 0x34528692

#define ID_Node_DragDropEventArgs_x 0x57863764 // Node_DragDropEventArgs.x
#define ID_Node_DragDropEventArgs_y 0x588638f7 // Node_DragDropEventArgs.y
#define ID_Node_DragDropEventArgs_deltaX 0x555d57ee // Node_DragDropEventArgs.deltaX
#define ID_Node_DragDropEventArgs_deltaY 0x565d5981 // Node_DragDropEventArgs.deltaY
#define ID_Node_DragDropEventArgs_button 0x3ad3bd30 // Node_DragDropEventArgs.button
#define ID_Node_DragDropEventArgs_clickCount 0x4304c7f3 // Node_DragDropEventArgs.clickCount
#define ID_Node_DragEnterEventArgs 0x469d4d03

#define ID_Node_DragEnterEventArgs_x 0x0da5da7d // Node_DragEnterEventArgs.x
#define ID_Node_DragEnterEventArgs_y 0x0ca5d8ea // Node_DragEnterEventArgs.y
#define ID_Node_DragEnterEventArgs_deltaX 0x49c75605 // Node_DragEnterEventArgs.deltaX
#define ID_Node_DragEnterEventArgs_deltaY 0x48c75472 // Node_DragEnterEventArgs.deltaY
#define ID_Node_DragEnterEventArgs_button 0xd7ad1647 // Node_DragEnterEventArgs.button
#define ID_Node_DragEnterEventArgs_clickCount 0x9f171300 // Node_DragEnterEventArgs.clickCount
#define ID_Node_KeyMessageEventArgs 0x31a320d9

#define ID_Node_KeyMessageEventArgs_keyCode 0xc45b7b79 // Node_KeyMessageEventArgs.keyCode
#define ID_Node_KeyMessageEventArgs_character 0xd10511a0 // Node_KeyMessageEventArgs.character
#define ID_Node_KeyMessageEventArgs_modifiers 0xc57fbc09 // Node_KeyMessageEventArgs.modifiers
#define ID_Node_KeyMessageEventArgs_text 0x7246b7ae // Node_KeyMessageEventArgs.text
#define ID_Node_KeyMessageEventArgs_modifiersString 0xcdc743c4 // Node_KeyMessageEventArgs.modifiersString
#define ID_Node_KeyMessageEventArgs_hotKey 0xa26f0a63 // Node_KeyMessageEventArgs.hotKey
#define ID_Node_KeyDownEventArgs 0x768f6172

#define ID_Node_KeyDownEventArgs_keyCode 0x86146eca // Node_KeyDownEventArgs.keyCode
#define ID_Node_KeyDownEventArgs_character 0x0ba694db // Node_KeyDownEventArgs.character
#define ID_Node_KeyDownEventArgs_modifiers 0x2a28bbe2 // Node_KeyDownEventArgs.modifiers
#define ID_Node_KeyDownEventArgs_text 0x74b1d6c3 // Node_KeyDownEventArgs.text
#define ID_Node_KeyDownEventArgs_modifiersString 0x12417f3b // Node_KeyDownEventArgs.modifiersString
#define ID_Node_KeyDownEventArgs_hotKey 0x5ec0ec16 // Node_KeyDownEventArgs.hotKey
#define ID_Node_KeyUpEventArgs 0x83e649ad

#define ID_Node_KeyUpEventArgs_keyCode 0x2ad2ffc5 // Node_KeyUpEventArgs.keyCode
#define ID_Node_KeyUpEventArgs_character 0x5ed502ec // Node_KeyUpEventArgs.character
#define ID_Node_KeyUpEventArgs_modifiers 0x81da073d // Node_KeyUpEventArgs.modifiers
#define ID_Node_KeyUpEventArgs_text 0x47e68ffa // Node_KeyUpEventArgs.text
#define ID_Node_KeyUpEventArgs_modifiersString 0xc2676db0 // Node_KeyUpEventArgs.modifiersString
#define ID_Node_KeyUpEventArgs_hotKey 0xe53c200f // Node_KeyUpEventArgs.hotKey
#define ID_Node_TextInputEventArgs 0xe5666e98

#define ID_Node_TextInputEventArgs_keyCode 0x7806eba0 // Node_TextInputEventArgs.keyCode
#define ID_Node_TextInputEventArgs_character 0x1ca6a7f5 // Node_TextInputEventArgs.character
#define ID_Node_TextInputEventArgs_modifiers 0x65aed464 // Node_TextInputEventArgs.modifiers
#define ID_Node_TextInputEventArgs_text 0x4f7a5129 // Node_TextInputEventArgs.text
#define ID_Node_TextInputEventArgs_modifiersString 0x3a2b2e05 // Node_TextInputEventArgs.modifiersString
#define ID_Node_TextInputEventArgs_hotKey 0xf8911dfc // Node_TextInputEventArgs.hotKey
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

#define ID_PageHost_NavigateToPageEventArgs 0x0b5e4b0d

#define ID_PageHost_NavigateToPageEventArgs_URL 0x5085360a // PageHost_NavigateToPageEventArgs.URL
#define ID_PageHost_NavigateToPageEventArgs_TransitionType 0x972d66de // PageHost_NavigateToPageEventArgs.TransitionType
#define ID_PageHost_NavigateBackEventArgs 0xa3f6a342

#define ID_PageHost_NavigateBackEventArgs_TransitionType 0x0531cd67 // PageHost_NavigateBackEventArgs.TransitionType
	

#endif