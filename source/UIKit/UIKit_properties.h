#define ID_DataObject 0xeb3560da
#define GetDataObject(_P)((struct DataObject*)((_P)?OBJ_GetComponent(_P,ID_DataObject):NULL))
#define DataObject_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_DataObject,sizeof(struct DataObject),_N)
enum DataObjectProperties {
	kDataObjectNumProperties
};

#define ID_AnimationPlayer 0x81ac71d8
#define GetAnimationPlayer(_P)((struct AnimationPlayer*)((_P)?OBJ_GetComponent(_P,ID_AnimationPlayer):NULL))
#define AnimationPlayer_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_AnimationPlayer,sizeof(struct AnimationPlayer),_N)
enum AnimationPlayerProperties {
	kAnimationPlayerAutoplayEnabled,
	kAnimationPlayerDurationScale,
	kAnimationPlayerPlaybackMode,
	kAnimationPlayerRelativePlayback,
	kAnimationPlayerRepeatCount,
	kAnimationPlayerRestoreOriginalValuesAfterPlayback,
	kAnimationPlayerTimeline,
	kAnimationPlayerNumProperties
};

#define ID_AnimationPlayer_AutoplayEnabled 0xbb4b7f90
#define ID_AnimationPlayer_DurationScale 0x742d0c5c
#define ID_AnimationPlayer_PlaybackMode 0xb886a1a0
#define ID_AnimationPlayer_RelativePlayback 0xb10f4317
#define ID_AnimationPlayer_RepeatCount 0x02afaefc
#define ID_AnimationPlayer_RestoreOriginalValuesAfterPlayback 0xb982b350
#define ID_AnimationPlayer_Timeline 0x17c5cb89
#define ID_Trigger 0xa5ea0da3
#define GetTrigger(_P)((struct Trigger*)((_P)?OBJ_GetComponent(_P,ID_Trigger):NULL))
#define Trigger_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Trigger,sizeof(struct Trigger),_N)
enum TriggerProperties {
	kTriggerProperty,
	kTriggerValue,
	kTriggerNumProperties
};

#define ID_Trigger_Property 0x7a6c1a46
#define ID_Trigger_Value 0x5229f3cc
#define ID_OnPropertyChangedTrigger 0xde0f7949
#define GetOnPropertyChangedTrigger(_P)((struct OnPropertyChangedTrigger*)((_P)?OBJ_GetComponent(_P,ID_OnPropertyChangedTrigger):NULL))
#define OnPropertyChangedTrigger_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_OnPropertyChangedTrigger,sizeof(struct OnPropertyChangedTrigger),_N)
enum OnPropertyChangedTriggerProperties {
	kOnPropertyChangedTriggerSourceNode,
	kOnPropertyChangedTriggerProperty,
	kOnPropertyChangedTriggerNumProperties
};

#define ID_OnPropertyChangedTrigger_SourceNode 0x5d55b664
#define ID_OnPropertyChangedTrigger_Property 0x8664fe08
#define ID_OnAttachedTrigger 0x677fe670
#define GetOnAttachedTrigger(_P)((struct OnAttachedTrigger*)((_P)?OBJ_GetComponent(_P,ID_OnAttachedTrigger):NULL))
#define OnAttachedTrigger_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_OnAttachedTrigger,sizeof(struct OnAttachedTrigger),_N)
enum OnAttachedTriggerProperties {
	kOnAttachedTriggerNumProperties
};

#define ID_EventTrigger 0x88bab1a1
#define GetEventTrigger(_P)((struct EventTrigger*)((_P)?OBJ_GetComponent(_P,ID_EventTrigger):NULL))
#define EventTrigger_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_EventTrigger,sizeof(struct EventTrigger),_N)
enum EventTriggerProperties {
	kEventTriggerRoutedEvent,
	kEventTriggerNumProperties
};

#define ID_EventTrigger_RoutedEvent 0x0c5f2982
#define ID_Setter 0xf849aee6
#define GetSetter(_P)((struct Setter*)((_P)?OBJ_GetComponent(_P,ID_Setter):NULL))
#define Setter_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Setter,sizeof(struct Setter),_N)
enum SetterProperties {
	kSetterTrigger,
	kSetterProperty,
	kSetterValue,
	kSetterNumProperties
};

#define ID_Setter_Trigger 0xd1fad954
#define ID_Setter_Property 0x89d17a41
#define ID_Setter_Value 0x8fd76d09
#define ID_Handler 0x04d66a13
#define GetHandler(_P)((struct Handler*)((_P)?OBJ_GetComponent(_P,ID_Handler):NULL))
#define Handler_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Handler,sizeof(struct Handler),_N)
enum HandlerProperties {
	kHandlerTrigger,
	kHandlerTarget,
	kHandlerFunction,
	kHandlerNumProperties
};

#define ID_Handler_Trigger 0x0ef3b2a9
#define ID_Handler_Target 0x2798724a
#define ID_Handler_Function 0x595dd2a7
#define ID_Brush 0xccbef093
#define GetBrush(_P)((struct Brush*)((_P)?OBJ_GetComponent(_P,ID_Brush):NULL))
#define Brush_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Brush,sizeof(struct Brush),_N)
enum BrushProperties {
	kBrushNumProperties
};

#define ID_ColorBrush 0x2077ee58
#define GetColorBrush(_P)((struct ColorBrush*)((_P)?OBJ_GetComponent(_P,ID_ColorBrush):NULL))
#define ColorBrush_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_ColorBrush,sizeof(struct ColorBrush),_N)
enum ColorBrushProperties {
	kColorBrushColor,
	kColorBrushNumProperties
};

#define ID_ColorBrush_Color 0xb7f26275
#define ID_Node 0x3468032d
#define GetNode(_P)((struct Node*)((_P)?OBJ_GetComponent(_P,ID_Node):NULL))
#define Node_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Node,sizeof(struct Node),_N)
enum NodeProperties {
	kNodeSize,
	kNodeHorizontalSize,
	kNodeWidth,
	kNodeMinWidth,
	kNodeActualWidth,
	kNodeScrollWidth,
	kNodeVerticalSize,
	kNodeHeight,
	kNodeMinHeight,
	kNodeActualHeight,
	kNodeScrollHeight,
	kNodeDepthSize,
	kNodeDepth,
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
	kNodeBorderStyle,
	kNodeBorderColor,
	kNodeBorderRadius,
	kNodeBorderTopLeftRadius,
	kNodeBorderTopRightRadius,
	kNodeBorderBottomRightRadius,
	kNodeBorderBottomLeftRadius,
	kNodeHorizontalBorder,
	kNodeBorderLeft,
	kNodeBorderLeftWidth,
	kNodeBorderLeftStyle,
	kNodeBorderLeftColor,
	kNodeBorderRight,
	kNodeBorderRightWidth,
	kNodeBorderRightStyle,
	kNodeBorderRightColor,
	kNodeVerticalBorder,
	kNodeBorderTop,
	kNodeBorderTopWidth,
	kNodeBorderTopStyle,
	kNodeBorderTopColor,
	kNodeBorderBottom,
	kNodeBorderBottomWidth,
	kNodeBorderBottomStyle,
	kNodeBorderBottomColor,
	kNodeDepthBorder,
	kNodeBorderFront,
	kNodeBorderFrontWidth,
	kNodeBorderFrontStyle,
	kNodeBorderFrontColor,
	kNodeBorderBack,
	kNodeBorderBackWidth,
	kNodeBorderBackStyle,
	kNodeBorderBackColor,
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
	kNodeNumProperties
};

#define ID_Node_Size 0xc8371588
#define ID_Node_HorizontalSize 0x8dd5feec
#define ID_Node_Width 0xc28a97d3
#define ID_Node_MinWidth 0x1ebf4605
#define ID_Node_ActualWidth 0xf66f0265
#define ID_Node_ScrollWidth 0x7ccec714
#define ID_Node_VerticalSize 0x41e70316
#define ID_Node_Height 0x5615e70e
#define ID_Node_MinHeight 0x7a45235c
#define ID_Node_ActualHeight 0xfca3503c
#define ID_Node_ScrollHeight 0x4288c2c3
#define ID_Node_DepthSize 0x86dbf73f
#define ID_Node_Depth 0x9aed1cde
#define ID_Node_MinDepth 0x63a6319c
#define ID_Node_ActualDepth 0x38d9f1fc
#define ID_Node_ScrollDepth 0x580ae93d
#define ID_Node_Margin 0x3b2ed3f7
#define ID_Node_HorizontalMargin 0xe4abc3eb
#define ID_Node_MarginLeft 0xae3827de
#define ID_Node_MarginRight 0xf5c8d933
#define ID_Node_VerticalMargin 0x73189bc9
#define ID_Node_MarginTop 0x720b58b6
#define ID_Node_MarginBottom 0xd1367ec8
#define ID_Node_DepthMargin 0x45ad5268
#define ID_Node_MarginFront 0xb567a99e
#define ID_Node_MarginBack 0x81d2d3e0
#define ID_Node_Padding 0xc7721c32
#define ID_Node_HorizontalPadding 0x18bd87c6
#define ID_Node_PaddingLeft 0xbf34d9ff
#define ID_Node_PaddingRight 0x6f0935b0
#define ID_Node_VerticalPadding 0xb2aba4fc
#define ID_Node_PaddingTop 0x42f6b495
#define ID_Node_PaddingBottom 0x06044a21
#define ID_Node_DepthPadding 0x70b717ef
#define ID_Node_PaddingFront 0x0ac7b2b5
#define ID_Node_PaddingBack 0xd22b7a31
#define ID_Node_Border 0x2a3ce9f3
#define ID_Node_BorderWidth 0x52e9a9e5
#define ID_Node_BorderStyle 0x88c40c44
#define ID_Node_BorderColor 0x85e5d732
#define ID_Node_BorderRadius 0x867eed01
#define ID_Node_BorderTopLeftRadius 0xa13859b5
#define ID_Node_BorderTopRightRadius 0xbe725f4e
#define ID_Node_BorderBottomRightRadius 0xe9173b64
#define ID_Node_BorderBottomLeftRadius 0x9511fd4b
#define ID_Node_HorizontalBorder 0x011608c7
#define ID_Node_BorderLeft 0xc27a852a
#define ID_Node_BorderLeftWidth 0x02c8fa36
#define ID_Node_BorderLeftStyle 0x418d537f
#define ID_Node_BorderLeftColor 0x9af0561d
#define ID_Node_BorderRight 0xf30cc2f7
#define ID_Node_BorderRightWidth 0x9bfa14d9
#define ID_Node_BorderRightStyle 0xf41a8468
#define ID_Node_BorderRightColor 0x50c65ea6
#define ID_Node_VerticalBorder 0xd17e2079
#define ID_Node_BorderTop 0xe260ceca
#define ID_Node_BorderTopWidth 0x243bba96
#define ID_Node_BorderTopStyle 0x7ec59edf
#define ID_Node_BorderTopColor 0xbedd7f7d
#define ID_Node_BorderBottom 0x822aba1c
#define ID_Node_BorderBottomWidth 0x5f38b988
#define ID_Node_BorderBottomStyle 0x1db18d51
#define ID_Node_BorderBottomColor 0x4c77d2c3
#define ID_Node_DepthBorder 0xc5ae22ec
#define ID_Node_BorderFront 0xd98795c2
#define ID_Node_BorderFrontWidth 0x1000a25e
#define ID_Node_BorderFrontStyle 0x20a52967
#define ID_Node_BorderFrontColor 0x21677875
#define ID_Node_BorderBack 0x51f9245c
#define ID_Node_BorderBackWidth 0x437e0ec8
#define ID_Node_BorderBackStyle 0x47843b91
#define ID_Node_BorderBackColor 0x2e445203
#define ID_Node_Alignment 0x6ae48d82
#define ID_Node_HorizontalAlignment 0xe230b1ee
#define ID_Node_VerticalAlignment 0x1c45d168
#define ID_Node_DepthAlignment 0x5b191ce3
#define ID_Node_Visible 0xe1936ee5
#define ID_Node_QuickHide 0x7e26e1b0
#define ID_Node_VisibleAmountInParent 0xbe0696b2
#define ID_Node_Opacity 0xb6882472
#define ID_Node_Tags 0xec56af24
#define ID_Node_DataContext 0x80b43db0
#define ID_TextRun 0x4362c3d7
#define GetTextRun(_P)((struct TextRun*)((_P)?OBJ_GetComponent(_P,ID_TextRun):NULL))
#define TextRun_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_TextRun,sizeof(struct TextRun),_N)
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
	kTextRunNumProperties
};

#define ID_TextRun_Text 0xcba1ea6c
#define ID_TextRun_Font 0x3900dfa2
#define ID_TextRun_FontWeight 0x69432eea
#define ID_TextRun_FontStyle 0xbcedda87
#define ID_TextRun_FontSize 0x307249cb
#define ID_TextRun_FontFamily 0x2991bcb6
#define ID_TextRun_Underline 0x0b0d856d
#define ID_TextRun_UnderlineOffset 0x0a823d42
#define ID_TextRun_UnderlineWidth 0xc80a55b7
#define ID_TextRun_UnderlineColor 0xcacb7660
#define ID_TextRun_LetterSpacing 0x83b2ad0c
#define ID_TextRun_LineHeight 0x45c78196
#define ID_TextRun_CharacterSpacing 0xda8217e7
#define ID_TextRun_FixedCharacterWidth 0xb71765fa
#define ID_TextRun_RemoveSideBearingsProperty 0xc20f6de6
#define ID_TextBlockConcept 0x4903089d
#define GetTextBlockConcept(_P)((struct TextBlockConcept*)((_P)?OBJ_GetComponent(_P,ID_TextBlockConcept):NULL))
#define TextBlockConcept_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_TextBlockConcept,sizeof(struct TextBlockConcept),_N)
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
	kTextBlockConceptTextHorizontalAlignment,
	kTextBlockConceptTextVerticalAlignment,
	kTextBlockConceptNumProperties
};

#define ID_TextBlockConcept_TextResourceID 0x7617ef4f
#define ID_TextBlockConcept_TextResourceConfiguration 0x445231c0
#define ID_TextBlockConcept_PlaceholderText 0xadd54a35
#define ID_TextBlockConcept_TextOverflow 0x9717f5b0
#define ID_TextBlockConcept_Placeholder 0x78889986
#define ID_TextBlockConcept_PlaceholderColor 0x8f53db89
#define ID_TextBlockConcept_PlaceholderImage 0x2275c70b
#define ID_TextBlockConcept_PlaceholderMaterial 0x16a577db
#define ID_TextBlockConcept_UseFullFontHeight 0x95fcf16b
#define ID_TextBlockConcept_ConstrainContentHeight 0x8468e688
#define ID_TextBlockConcept_WordWrap 0x34b71f41
#define ID_TextBlockConcept_TextHorizontalAlignment 0xe480096b
#define ID_TextBlockConcept_TextVerticalAlignment 0xbf0260e5
#define ID_Node2D 0x6c63a2ab
#define GetNode2D(_P)((struct Node2D*)((_P)?OBJ_GetComponent(_P,ID_Node2D):NULL))
#define Node2D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Node2D,sizeof(struct Node2D),_N)
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
	kNode2DNumProperties
};

#define ID_Node2D_LayoutTransform 0x7c78c87b
#define ID_Node2D_LayoutTransformTranslation 0x2407475a
#define ID_Node2D_LayoutTransformRotation 0x40a04c55
#define ID_Node2D_LayoutTransformScale 0xae9265d3
#define ID_Node2D_RenderTransform 0xa5faec05
#define ID_Node2D_RenderTransformTranslation 0xa5d5540c
#define ID_Node2D_RenderTransformRotation 0x5387d0bf
#define ID_Node2D_RenderTransformScale 0xa0bbe951
#define ID_Node2D_RenderTransformOrigin 0x43a9dbaf
#define ID_Node2D_ContentOffset 0xb5cb609b
#define ID_Node2D_Matrix 0x09a64b02
#define ID_Node2D_RenderTarget 0x441af9f6
#define ID_Node2D_Background 0x59353973
#define ID_Node2D_BackgroundColor 0x0796e5b2
#define ID_Node2D_BackgroundImage 0xeb919ee8
#define ID_Node2D_BackgroundMaterial 0x973fabea
#define ID_Node2D_Foreground 0xaeaca316
#define ID_Node2D_ForegroundColor 0xf890bd19
#define ID_Node2D_ForegroundImage 0xd8914fbb
#define ID_Node2D_ForegroundMaterial 0x4f98b2ab
#define ID_Node2D_BoxShadow 0x47152f84
#define ID_Node2D_BoxShadowOffset 0xfa0a729f
#define ID_Node2D_BoxShadowBlurRadius 0x16c10efb
#define ID_Node2D_BoxShadowSpreadRadius 0x8804c4f1
#define ID_Node2D_BoxShadowColor 0xc855fc8b
#define ID_Node2D_Overflow 0xd5704155
#define ID_Node2D_OverflowX 0x0cb66777
#define ID_Node2D_OverflowY 0x0bb665e4
#define ID_Node2D_Ring 0x039ac541
#define ID_Node2D_RingOffset 0x674cacfe
#define ID_Node2D_RingWidth 0x1a88410b
#define ID_Node2D_RingColor 0xecbe3144
#define ID_Node2D_CompositionBrush 0x04f99755
#define ID_Node2D_CompositionDesignSize 0x8a2688dc
#define ID_Node2D_SizeToContent 0xe55ab2cc
#define ID_Node2D_OffscreenRendering 0x893fea40
#define ID_Node2D_ForceComposition 0x987f2560
#define ID_Node2D_CacheResult 0xfa85e55a
#define ID_Node2D_SnapToPixel 0x752cbd46
#define ID_Node2D_ClipChildren 0xe3db0cc6
#define ID_Node2D_ContentStretch 0x09a28d25
#define ID_Node2D_Hovered 0x982d5e3e
#define ID_Node2D_IgnoreHitTest 0x0943bf6a
#define ID_Node2D_ForegroundHint 0x1a0ea5e3
#define ID_PrefabView2D 0xe741d328
#define GetPrefabView2D(_P)((struct PrefabView2D*)((_P)?OBJ_GetComponent(_P,ID_PrefabView2D):NULL))
#define PrefabView2D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_PrefabView2D,sizeof(struct PrefabView2D),_N)
enum PrefabView2DProperties {
	kPrefabView2DSCA,
	kPrefabView2DPrefab,
	kPrefabView2DNumProperties
};

#define ID_PrefabView2D_SCA 0x1ab11f83
#define ID_PrefabView2D_Prefab 0xef0b7c70
#define ID_TextBlock 0x40f4d77b
#define GetTextBlock(_P)((struct TextBlock*)((_P)?OBJ_GetComponent(_P,ID_TextBlock):NULL))
#define TextBlock_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_TextBlock,sizeof(struct TextBlock),_N)
enum TextBlockProperties {
	kTextBlockNumProperties
};

#define ID_Input 0x9f93e11b
#define GetInput(_P)((struct Input*)((_P)?OBJ_GetComponent(_P,ID_Input):NULL))
#define Input_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Input,sizeof(struct Input),_N)
enum InputProperties {
	kInputName,
	kInputType,
	kInputCursor,
	kInputMultiline,
	kInputChecked,
	kInputNumProperties
};

#define ID_Input_Name 0x56849c4c
#define ID_Input_Type 0x84bfe237
#define ID_Input_Cursor 0xb89f7b0d
#define ID_Input_Multiline 0xa42cd2d2
#define ID_Input_Checked 0x62ce5658
#define ID_Button 0x33881a91
#define GetButton(_P)((struct Button*)((_P)?OBJ_GetComponent(_P,ID_Button):NULL))
#define Button_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Button,sizeof(struct Button),_N)
enum ButtonProperties {
	kButtonType,
	kButtonNumProperties
};

#define ID_Button_Type 0x843eb785
#define ID_Label 0x9eccf29d
#define GetLabel(_P)((struct Label*)((_P)?OBJ_GetComponent(_P,ID_Label):NULL))
#define Label_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Label,sizeof(struct Label),_N)
enum LabelProperties {
	kLabelFor,
	kLabelNumProperties
};

#define ID_Label_For 0x8ea77d4c
#define ID_StackView 0x56aa550a
#define GetStackView(_P)((struct StackView*)((_P)?OBJ_GetComponent(_P,ID_StackView):NULL))
#define StackView_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_StackView,sizeof(struct StackView),_N)
enum StackViewProperties {
	kStackViewReversed,
	kStackViewDirection,
	kStackViewAlignItems,
	kStackViewJustifyContent,
	kStackViewSpacing,
	kStackViewNumProperties
};

#define ID_StackView_Reversed 0x4f7ea66a
#define ID_StackView_Direction 0x4f1430fd
#define ID_StackView_AlignItems 0x4d502153
#define ID_StackView_JustifyContent 0x6b245371
#define ID_StackView_Spacing 0xf9ff1755
#define ID_Form 0xc0851367
#define GetForm(_P)((struct Form*)((_P)?OBJ_GetComponent(_P,ID_Form):NULL))
#define Form_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Form,sizeof(struct Form),_N)
enum FormProperties {
	kFormNumProperties
};

#define ID_CollectionView 0xda7bf4ac
#define GetCollectionView(_P)((struct CollectionView*)((_P)?OBJ_GetComponent(_P,ID_CollectionView):NULL))
#define CollectionView_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_CollectionView,sizeof(struct CollectionView),_N)
enum CollectionViewProperties {
	kCollectionViewDirection,
	kCollectionViewSpacing,
	kCollectionViewNumProperties
};

#define ID_CollectionView_Direction 0x63122e5b
#define ID_CollectionView_Spacing 0x8d21d4db
#define ID_Control 0x8347bf3e
#define GetControl(_P)((struct Control*)((_P)?OBJ_GetComponent(_P,ID_Control):NULL))
#define Control_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Control,sizeof(struct Control),_N)
enum ControlProperties {
	kControlPressed,
	kControlDisabled,
	kControlHovered,
	kControlFocused,
	kControlValue,
	kControlScale,
	kControlNumProperties
};

#define ID_Control_Pressed 0x0bfbf446
#define ID_Control_Disabled 0x08680018
#define ID_Control_Hovered 0xd61b4c93
#define ID_Control_Focused 0x9d297c69
#define ID_Control_Value 0xbb44f9c1
#define ID_Control_Scale 0xb46d6752
#define ID_Screen 0x9bd8c631
#define GetScreen(_P)((struct Screen*)((_P)?OBJ_GetComponent(_P,ID_Screen):NULL))
#define Screen_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Screen,sizeof(struct Screen),_N)
enum ScreenProperties {
	kScreenClearColor,
	kScreenResizeMode,
	kScreenNumProperties
};

#define ID_Screen_ClearColor 0x1bfc36dd
#define ID_Screen_ResizeMode 0xc3203446
#define ID_Cinematic 0xc04cee0e
#define GetCinematic(_P)((struct Cinematic*)((_P)?OBJ_GetComponent(_P,ID_Cinematic):NULL))
#define Cinematic_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Cinematic,sizeof(struct Cinematic),_N)
enum CinematicProperties {
	kCinematicFileName,
	kCinematicFrameRate,
	kCinematicNumFrames,
	kCinematicFadeOut,
	kCinematicNumProperties
};

#define ID_Cinematic_FileName 0xabe998d5
#define ID_Cinematic_FrameRate 0xead5c9cf
#define ID_Cinematic_NumFrames 0x46a2037a
#define ID_Cinematic_FadeOut 0x314b07da
#define ID_Grid 0x2fb366b1
#define GetGrid(_P)((struct Grid*)((_P)?OBJ_GetComponent(_P,ID_Grid):NULL))
#define Grid_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Grid,sizeof(struct Grid),_N)
enum GridProperties {
	kGridColumns,
	kGridRows,
	kGridDirection,
	kGridSpacing,
	kGridCellWidth,
	kGridCellHeight,
	kGridNumProperties
};

#define ID_Grid_Columns 0x5d28e334
#define ID_Grid_Rows 0x1a2b78b0
#define ID_Grid_Direction 0x252effe2
#define ID_Grid_Spacing 0x5d2ffdd6
#define ID_Grid_CellWidth 0x534e3267
#define ID_Grid_CellHeight 0xa97554ea
#define ID_ImageView 0xfe686eb3
#define GetImageView(_P)((struct ImageView*)((_P)?OBJ_GetComponent(_P,ID_ImageView):NULL))
#define ImageView_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_ImageView,sizeof(struct ImageView),_N)
enum ImageViewProperties {
	kImageViewSrc,
	kImageViewImage,
	kImageViewEdges,
	kImageViewInsets,
	kImageViewViewbox,
	kImageViewStretch,
	kImageViewNumProperties
};

#define ID_ImageView_Src 0xe2534f6b
#define ID_ImageView_Image 0x0b666f9c
#define ID_ImageView_Edges 0xbc1ab5c3
#define ID_ImageView_Insets 0xddc76755
#define ID_ImageView_Viewbox 0x4fff923f
#define ID_ImageView_Stretch 0x13aa1da4
#define ID_NinePatchImage 0xbd86cb1e
#define GetNinePatchImage(_P)((struct NinePatchImage*)((_P)?OBJ_GetComponent(_P,ID_NinePatchImage):NULL))
#define NinePatchImage_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_NinePatchImage,sizeof(struct NinePatchImage),_N)
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
	kNinePatchImageNumProperties
};

#define ID_NinePatchImage_StretchTypeTop 0x9375dc9c
#define ID_NinePatchImage_StretchTypeBottom 0x400be60a
#define ID_NinePatchImage_StretchTypeLeft 0xee979d50
#define ID_NinePatchImage_StretchTypeRight 0x659e0345
#define ID_NinePatchImage_StretchTypeCenter 0xcab1ae64
#define ID_NinePatchImage_ImageTopLeft 0x4b2c37e5
#define ID_NinePatchImage_ImageTop 0x5e5bf7e4
#define ID_NinePatchImage_ImageTopRight 0xc001b5be
#define ID_NinePatchImage_ImageLeft 0x6b2a74a8
#define ID_NinePatchImage_ImageCenter 0x1811e0dc
#define ID_NinePatchImage_ImageRight 0x04cb7dbd
#define ID_NinePatchImage_ImageBottomLeft 0xe446356f
#define ID_NinePatchImage_ImageBottom 0xc398b8c2
#define ID_NinePatchImage_ImageBottomRight 0x87575d60
#define ID_TerminalView 0xb1477a0c
#define GetTerminalView(_P)((struct TerminalView*)((_P)?OBJ_GetComponent(_P,ID_TerminalView):NULL))
#define TerminalView_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_TerminalView,sizeof(struct TerminalView),_N)
enum TerminalViewProperties {
	kTerminalViewBufferWidth,
	kTerminalViewBufferHeight,
	kTerminalViewCursor,
	kTerminalViewSelectedIndex,
	kTerminalViewDropShadow,
	kTerminalViewNumProperties
};

#define ID_TerminalView_BufferWidth 0xdf51183c
#define ID_TerminalView_BufferHeight 0xfd169aab
#define ID_TerminalView_Cursor 0x550c9034
#define ID_TerminalView_SelectedIndex 0xc6c048a5
#define ID_TerminalView_DropShadow 0x8c72c3cb
#define ID_Page 0xe83d9196
#define GetPage(_P)((struct Page*)((_P)?OBJ_GetComponent(_P,ID_Page):NULL))
#define Page_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Page,sizeof(struct Page),_N)
enum PageProperties {
	kPageTitle,
	kPageTransition,
	kPageNumProperties
};

#define ID_Page_Title 0x31e209ce
#define ID_Page_Transition 0xb60425fb
#define ID_PageHost 0x6f2dc6c0
#define GetPageHost(_P)((struct PageHost*)((_P)?OBJ_GetComponent(_P,ID_PageHost):NULL))
#define PageHost_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_PageHost,sizeof(struct PageHost),_N)
enum PageHostProperties {
	kPageHostActivePage,
	kPageHostNumProperties
};

#define ID_PageHost_ActivePage 0xb276c4f7
#define ID_Style 0x5467ec76
#define GetStyle(_P)((struct Style*)((_P)?OBJ_GetComponent(_P,ID_Style):NULL))
#define Style_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Style,sizeof(struct Style),_N)
enum StyleProperties {
	kStyleTargetType,
	kStyleType,
	kStyleNumProperties
};

#define ID_Style_TargetType 0x26cf4319
#define ID_Style_Type 0x8f053d80
