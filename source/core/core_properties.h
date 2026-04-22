// Auto-generated from core.xml by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __CORE_PROPERTIES_H__
#define __CORE_PROPERTIES_H__

// AnimationCurve
#define ID_AnimationCurve 0xdbe5783c
#define GetAnimationCurve(_P) ((struct AnimationCurve*)((_P)?OBJ_GetComponent(_P,ID_AnimationCurve):NULL))
#define AnimationCurve_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_AnimationCurve,sizeof(struct AnimationCurve),_N)
#define ID_AnimationCurve_Path 0xdda91d5d // AnimationCurve.Path
#define ID_AnimationCurve_Property 0x525d5123 // AnimationCurve.Property
#define ID_AnimationCurve_Keyframes 0x161acd67 // AnimationCurve.Keyframes
#define ID_AnimationCurve_NumKeyframes 0x7cda066f // AnimationCurve.NumKeyframes
#define kAnimationCurveNumProperties 4
enum AnimationCurveProperties {
	kAnimationCurvePath,
	kAnimationCurveProperty,
	kAnimationCurveKeyframes,
	kAnimationCurveNumKeyframes,
};
// AnimationClip
#define ID_AnimationClip 0xac597053
#define GetAnimationClip(_P) ((struct AnimationClip*)((_P)?OBJ_GetComponent(_P,ID_AnimationClip):NULL))
#define AnimationClip_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_AnimationClip,sizeof(struct AnimationClip),_N)
#define ID_AnimationClip_Mode 0x2a6a9d40 // AnimationClip.Mode
#define ID_AnimationClip_StartTime 0xd35a8f6c // AnimationClip.StartTime
#define ID_AnimationClip_StopTime 0x52f0f22e // AnimationClip.StopTime
#define kAnimationClipNumProperties 3
enum AnimationClipProperties {
	kAnimationClipMode,
	kAnimationClipStartTime,
	kAnimationClipStopTime,
};
// AnimationPlayer
#define ID_AnimationPlayer 0x81ac71d8
#define GetAnimationPlayer(_P) ((struct AnimationPlayer*)((_P)?OBJ_GetComponent(_P,ID_AnimationPlayer):NULL))
#define AnimationPlayer_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_AnimationPlayer,sizeof(struct AnimationPlayer),_N)
#define ID_AnimationPlayer_Clip 0xbc835a74 // AnimationPlayer.Clip
#define ID_AnimationPlayer_Clips 0xc9c6b905 // AnimationPlayer.Clips
#define ID_AnimationPlayer_NumClips 0x7a1df25d // AnimationPlayer.NumClips
#define ID_AnimationPlayer_Playing 0xdd2ed06c // AnimationPlayer.Playing
#define ID_AnimationPlayer_Looping 0x02535bc8 // AnimationPlayer.Looping
#define ID_AnimationPlayer_Speed 0x4a1ae835 // AnimationPlayer.Speed
#define ID_AnimationPlayer_CurrentTime 0xbc48783a // AnimationPlayer.CurrentTime
#define ID_AnimationPlayer_AutoplayEnabled 0xbb4b7f90 // AnimationPlayer.AutoplayEnabled
#define ID_AnimationPlayer_RelativePlayback 0xb10f4317 // AnimationPlayer.RelativePlayback
#define ID_AnimationPlayer_RestoreOriginalValuesAfterPlayback 0xb982b350 // AnimationPlayer.RestoreOriginalValuesAfterPlayback
#define ID_AnimationPlayer_PlaybackMode 0xb886a1a0 // AnimationPlayer.PlaybackMode
#define ID_AnimationPlayer_DurationScale 0x742d0c5c // AnimationPlayer.DurationScale
#define ID_AnimationPlayer_RepeatCount 0x02afaefc // AnimationPlayer.RepeatCount
#define ID_AnimationPlayer_Play 0x389b07cc // AnimationPlayer.Play
#define ID_AnimationPlayer_Resume 0x6c36db2d // AnimationPlayer.Resume
#define ID_AnimationPlayer_Stop 0x59228a5a // AnimationPlayer.Stop
#define ID_AnimationPlayer_Pause 0xafb93ba4 // AnimationPlayer.Pause
#define ID_AnimationPlayer_Started 0x6f97130b // AnimationPlayer.Started
#define ID_AnimationPlayer_Stopped 0x845c891f // AnimationPlayer.Stopped
#define ID_AnimationPlayer_Completed 0xa2ffe357 // AnimationPlayer.Completed
#define kAnimationPlayerNumProperties 20
enum AnimationPlayerProperties {
	kAnimationPlayerClip,
	kAnimationPlayerClips,
	kAnimationPlayerNumClips,
	kAnimationPlayerPlaying,
	kAnimationPlayerLooping,
	kAnimationPlayerSpeed,
	kAnimationPlayerCurrentTime,
	kAnimationPlayerAutoplayEnabled,
	kAnimationPlayerRelativePlayback,
	kAnimationPlayerRestoreOriginalValuesAfterPlayback,
	kAnimationPlayerPlaybackMode,
	kAnimationPlayerDurationScale,
	kAnimationPlayerRepeatCount,
	kAnimationPlayerPlay,
	kAnimationPlayerResume,
	kAnimationPlayerStop,
	kAnimationPlayerPause,
	kAnimationPlayerStarted,
	kAnimationPlayerStopped,
	kAnimationPlayerCompleted,
};
// PropertyAnimation
#define ID_PropertyAnimation 0xd2fdd366
#define GetPropertyAnimation(_P) ((struct PropertyAnimation*)((_P)?OBJ_GetComponent(_P,ID_PropertyAnimation):NULL))
#define PropertyAnimation_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PropertyAnimation,sizeof(struct PropertyAnimation),_N)
#define ID_PropertyAnimation_Property 0x02301fc1 // PropertyAnimation.Property
#define ID_PropertyAnimation_From 0x642f4068 // PropertyAnimation.From
#define ID_PropertyAnimation_To 0xec0b5ad1 // PropertyAnimation.To
#define ID_PropertyAnimation_Interpolation 0xadb6aa4e // PropertyAnimation.Interpolation
#define ID_PropertyAnimation_Easing 0xa4c33685 // PropertyAnimation.Easing
#define ID_PropertyAnimation_Start 0x238f4404 // PropertyAnimation.Start
#define ID_PropertyAnimation_Duration 0x88219fc8 // PropertyAnimation.Duration
#define kPropertyAnimationNumProperties 7
enum PropertyAnimationProperties {
	kPropertyAnimationProperty,
	kPropertyAnimationFrom,
	kPropertyAnimationTo,
	kPropertyAnimationInterpolation,
	kPropertyAnimationEasing,
	kPropertyAnimationStart,
	kPropertyAnimationDuration,
};
// StyleSheet
#define ID_StyleSheet 0x6546e1e1
#define GetStyleSheet(_P) ((struct StyleSheet*)((_P)?OBJ_GetComponent(_P,ID_StyleSheet):NULL))
#define StyleSheet_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StyleSheet,sizeof(struct StyleSheet),_N)
#define kStyleSheetNumProperties 0
// StyleRule
#define ID_StyleRule 0x96a4d210
#define GetStyleRule(_P) ((struct StyleRule*)((_P)?OBJ_GetComponent(_P,ID_StyleRule):NULL))
#define StyleRule_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StyleRule,sizeof(struct StyleRule),_N)
#define ID_StyleRule_ClassName 0x3396a471 // StyleRule.ClassName
#define ID_StyleRule_PseudoClass 0xc7313550 // StyleRule.PseudoClass
#define kStyleRuleNumProperties 2
enum StyleRuleProperties {
	kStyleRuleClassName,
	kStyleRulePseudoClass,
};
// StyleController
#define ID_StyleController 0x70b793e6
#define GetStyleController(_P) ((struct StyleController*)((_P)?OBJ_GetComponent(_P,ID_StyleController):NULL))
#define StyleController_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StyleController,sizeof(struct StyleController),_N)
#define ID_StyleController_StyleSheet 0x6ae4d4d0 // StyleController.StyleSheet
#define ID_StyleController_ThemeChanged 0xe51e8073 // StyleController.ThemeChanged
#define ID_StyleController_AddClass 0x8534805d // StyleController.AddClass
#define ID_StyleController_AddClasses 0x2b139741 // StyleController.AddClasses
#define kStyleControllerNumProperties 4
enum StyleControllerProperties {
	kStyleControllerStyleSheet,
	kStyleControllerThemeChanged,
	kStyleControllerAddClass,
	kStyleControllerAddClasses,
};
// StateManager
#define ID_StateManager 0xe76f2815
#define GetStateManager(_P) ((struct StateManager*)((_P)?OBJ_GetComponent(_P,ID_StateManager):NULL))
#define StateManager_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StateManager,sizeof(struct StateManager),_N)
#define kStateManagerNumProperties 0
// StateGroup
#define ID_StateGroup 0xc71ce3c9
#define GetStateGroup(_P) ((struct StateGroup*)((_P)?OBJ_GetComponent(_P,ID_StateGroup):NULL))
#define StateGroup_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StateGroup,sizeof(struct StateGroup),_N)
#define ID_StateGroup_ControllerProperty 0xa362303c // StateGroup.ControllerProperty
#define kStateGroupNumProperties 1
enum StateGroupProperties {
	kStateGroupControllerProperty,
};
// State
#define ID_State 0x1deca496
#define GetState(_P) ((struct State*)((_P)?OBJ_GetComponent(_P,ID_State):NULL))
#define State_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_State,sizeof(struct State),_N)
#define ID_State_Value 0x60339e79 // State.Value
#define ID_State_Path 0xf7b1147f // State.Path
#define kStateNumProperties 2
enum StateProperties {
	kStateValue,
	kStatePath,
};
// StateManagerController
#define ID_StateManagerController 0x69d899d1
#define GetStateManagerController(_P) ((struct StateManagerController*)((_P)?OBJ_GetComponent(_P,ID_StateManagerController):NULL))
#define StateManagerController_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StateManagerController,sizeof(struct StateManagerController),_N)
#define ID_StateManagerController_StateManager 0xb09e0f5d // StateManagerController.StateManager
#define ID_StateManagerController_ControllerChanged 0x0d0ee8c7 // StateManagerController.ControllerChanged
#define kStateManagerControllerNumProperties 2
enum StateManagerControllerProperties {
	kStateManagerControllerStateManager,
	kStateManagerControllerControllerChanged,
};
// ResourceDictionary
#define ID_ResourceDictionary 0xde7febc5
#define GetResourceDictionary(_P) ((struct ResourceDictionary*)((_P)?OBJ_GetComponent(_P,ID_ResourceDictionary):NULL))
#define ResourceDictionary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ResourceDictionary,sizeof(struct ResourceDictionary),_N)
#define kResourceDictionaryNumProperties 0
// Locale
#define ID_Locale 0xdd6e5bb7
#define GetLocale(_P) ((struct Locale*)((_P)?OBJ_GetComponent(_P,ID_Locale):NULL))
#define Locale_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Locale,sizeof(struct Locale),_N)
#define ID_Locale_Language 0x665b5d0d // Locale.Language
#define ID_Locale_Entries 0x8c51cbd9 // Locale.Entries
#define ID_Locale_NumEntries 0x883914cf // Locale.NumEntries
#define kLocaleNumProperties 3
enum LocaleProperties {
	kLocaleLanguage,
	kLocaleEntries,
	kLocaleNumEntries,
};
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
#define ID_Node_UpdateMatrix 0x35cdb821 // Node.UpdateMatrix
#define ID_Node_LoadView 0xe3c6ed08 // Node.LoadView
#define ID_Node_HitTest 0xbc0e5546 // Node.HitTest
#define ID_Node_IsVisible 0xa03cfb85 // Node.IsVisible
#define ID_Node_ViewDidLoad 0x71bab7e1 // Node.ViewDidLoad
#define ID_Node_KillFocus 0x2bdb09fb // Node.KillFocus
#define ID_Node_SetFocus 0x5f1c2071 // Node.SetFocus
#define ID_Node_GetSize 0x2282f1da // Node.GetSize
#define ID_Node_MouseMessage 0x8633bad7 // Node.MouseMessage
#define ID_Node_LeftButtonDown 0xd4990efc // Node.LeftButtonDown
#define ID_Node_RightButtonDown 0xfac5b9c5 // Node.RightButtonDown
#define ID_Node_OtherButtonDown 0x18d5d88d // Node.OtherButtonDown
#define ID_Node_LeftButtonUp 0xa94c62bd // Node.LeftButtonUp
#define ID_Node_RightButtonUp 0xda69d730 // Node.RightButtonUp
#define ID_Node_OtherButtonUp 0xdfe48738 // Node.OtherButtonUp
#define ID_Node_LeftButtonDragged 0xbc03661e // Node.LeftButtonDragged
#define ID_Node_RightButtonDragged 0x60b4eb2d // Node.RightButtonDragged
#define ID_Node_OtherButtonDragged 0x7733e4c5 // Node.OtherButtonDragged
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
#define kNodeNumProperties 98
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
	kNodeUpdateMatrix,
	kNodeLoadView,
	kNodeHitTest,
	kNodeIsVisible,
	kNodeViewDidLoad,
	kNodeKillFocus,
	kNodeSetFocus,
	kNodeGetSize,
	kNodeMouseMessage,
	kNodeLeftButtonDown,
	kNodeRightButtonDown,
	kNodeOtherButtonDown,
	kNodeLeftButtonUp,
	kNodeRightButtonUp,
	kNodeOtherButtonUp,
	kNodeLeftButtonDragged,
	kNodeRightButtonDragged,
	kNodeOtherButtonDragged,
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

#define ID_Object_Create 0x5ad07ee8 // Object.Create
#define ID_Object_Start 0x01629a90 // Object.Start
#define ID_Object_Animate 0xc8c0972b // Object.Animate
#define ID_Object_ThemeChanged 0x6d440f67 // Object.ThemeChanged
#define ID_Object_PropertyChanged 0x00d8cdf3 // Object.PropertyChanged
#define ID_Object_Attached 0xc920ef48 // Object.Attached
#define ID_Object_Release 0x10360cb1 // Object.Release
#define ID_Object_Destroy 0x8ef01bbe // Object.Destroy
#define ID_Object_Timer 0x6e6ade9d // Object.Timer

#define ID_AnimationClipReference 0x9309ab18
#define ID_AnimationClipReference_Name 0x5b67cfe9 // AnimationClipReference.Name
#define ID_AnimationClipReference_Clip 0xce2a9734 // AnimationClipReference.Clip
#define ID_Keyframe 0xf2057289
#define ID_Keyframe_Time 0x5e026ae4 // Keyframe.Time
#define ID_Keyframe_Value 0x79c3b0ca // Keyframe.Value
#define ID_Keyframe_InSlope 0x5cd2bd51 // Keyframe.InSlope
#define ID_Keyframe_OutSlope 0xf51b53ae // Keyframe.OutSlope
#define ID_Keyframe_InWeight 0x0d596b26 // Keyframe.InWeight
#define ID_Keyframe_OutWeight 0x1061c43b // Keyframe.OutWeight
#define ID_Keyframe_TangentMode 0x075e3c89 // Keyframe.TangentMode
#define ID_Keyframe_WeightedMode 0x7101cd83 // Keyframe.WeightedMode
#define ID_LocaleEntry 0x4e4d5c51
#define ID_LocaleEntry_Key 0xb57c4b84 // LocaleEntry.Key
#define ID_LocaleEntry_Value 0xb599c6f2 // LocaleEntry.Value
#define ID_LocaleEntry_Type 0xddf9bec5 // LocaleEntry.Type
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
	

#define ID_Object_CreateEventArgs 0xd370143c

#define ID_Object_StartEventArgs 0x23dcf2aa

#define ID_Object_AnimateEventArgs 0xc80c1167

#define ID_StyleController_ThemeChangedEventArgs 0x0ee8d735

#define ID_StyleController_ThemeChangedEventArgs_recursive 0xa1d9af6b // StyleController_ThemeChangedEventArgs.recursive
#define ID_Object_PropertyChangedEventArgs 0x530ec22f

#define ID_Object_PropertyChangedEventArgs_Property 0xc7308dfa // Object_PropertyChangedEventArgs.Property
#define ID_Object_AttachedEventArgs 0x421be758

#define ID_Object_ReleaseEventArgs 0x7710fc41

#define ID_Object_DestroyEventArgs 0x3eec8a44

#define ID_Object_TimerEventArgs 0x75284ba1

#define ID_Object_TimerEventArgs_timerId 0x9555bf27 // Object_TimerEventArgs.timerId
#define ID_AnimationPlayer_PlayEventArgs 0x1e984720

#define ID_AnimationPlayer_PlayEventArgs_Name 0xc73b23f1 // AnimationPlayer_PlayEventArgs.Name
#define ID_AnimationPlayer_ResumeEventArgs 0xf37671df

#define ID_AnimationPlayer_StopEventArgs 0xa977f29a

#define ID_AnimationPlayer_PauseEventArgs 0x68215542

#define ID_AnimationPlayer_StartedEventArgs 0x9649588b

#define ID_AnimationPlayer_StoppedEventArgs 0x272b2fdb

#define ID_AnimationPlayer_CompletedEventArgs 0xacaaf6c7

#define ID_StyleController_AddClassEventArgs 0x6838e65b

#define ID_StyleController_AddClassEventArgs_ClassName 0xb3e910ea // StyleController_AddClassEventArgs.ClassName
#define ID_StyleController_AddClassesEventArgs 0xc1f36253

#define ID_StyleController_AddClassesEventArgs_ClassNames 0xbeac48c3 // StyleController_AddClassesEventArgs.ClassNames
#define ID_StateManagerController_ControllerChangedEventArgs 0x12c11cb3

#define ID_StateManagerController_ControllerChangedEventArgs_Property 0x1ed8c3b6 // StateManagerController_ControllerChangedEventArgs.Property
#define ID_Trigger_TriggeredEventArgs 0x86976046

#define ID_Trigger_TriggeredEventArgs_Trigger 0xa8841074 // Trigger_TriggeredEventArgs.Trigger
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
#define ID_Node_LeftButtonDownEventArgs 0x6caa3f0c

#define ID_Node_LeftButtonDownEventArgs_x 0xb62ed5da // Node_LeftButtonDownEventArgs.x
#define ID_Node_LeftButtonDownEventArgs_y 0xb72ed76d // Node_LeftButtonDownEventArgs.y
#define ID_Node_LeftButtonDownEventArgs_deltaX 0x2f27e0a8 // Node_LeftButtonDownEventArgs.deltaX
#define ID_Node_LeftButtonDownEventArgs_deltaY 0x3027e23b // Node_LeftButtonDownEventArgs.deltaY
#define ID_Node_LeftButtonDownEventArgs_button 0x1b554e46 // Node_LeftButtonDownEventArgs.button
#define ID_Node_LeftButtonDownEventArgs_clickCount 0x1e50b5a5 // Node_LeftButtonDownEventArgs.clickCount
#define ID_Node_RightButtonDownEventArgs 0x37f5e871

#define ID_Node_RightButtonDownEventArgs_x 0x775838af // Node_RightButtonDownEventArgs.x
#define ID_Node_RightButtonDownEventArgs_y 0x7658371c // Node_RightButtonDownEventArgs.y
#define ID_Node_RightButtonDownEventArgs_deltaX 0x3191ea23 // Node_RightButtonDownEventArgs.deltaX
#define ID_Node_RightButtonDownEventArgs_deltaY 0x3091e890 // Node_RightButtonDownEventArgs.deltaY
#define ID_Node_RightButtonDownEventArgs_button 0xb2b28fb9 // Node_RightButtonDownEventArgs.button
#define ID_Node_RightButtonDownEventArgs_clickCount 0xf21b9fce // Node_RightButtonDownEventArgs.clickCount
#define ID_Node_OtherButtonDownEventArgs 0x18279d79

#define ID_Node_OtherButtonDownEventArgs_x 0x85e85af7 // Node_OtherButtonDownEventArgs.x
#define ID_Node_OtherButtonDownEventArgs_y 0x84e85964 // Node_OtherButtonDownEventArgs.y
#define ID_Node_OtherButtonDownEventArgs_deltaX 0xc14d90eb // Node_OtherButtonDownEventArgs.deltaX
#define ID_Node_OtherButtonDownEventArgs_deltaY 0xc04d8f58 // Node_OtherButtonDownEventArgs.deltaY
#define ID_Node_OtherButtonDownEventArgs_button 0x6d050a41 // Node_OtherButtonDownEventArgs.button
#define ID_Node_OtherButtonDownEventArgs_clickCount 0x328db626 // Node_OtherButtonDownEventArgs.clickCount
#define ID_Node_LeftButtonUpEventArgs 0xf6a581db

#define ID_Node_LeftButtonUpEventArgs_x 0x93bb4e75 // Node_LeftButtonUpEventArgs.x
#define ID_Node_LeftButtonUpEventArgs_y 0x92bb4ce2 // Node_LeftButtonUpEventArgs.y
#define ID_Node_LeftButtonUpEventArgs_deltaX 0xc29b573d // Node_LeftButtonUpEventArgs.deltaX
#define ID_Node_LeftButtonUpEventArgs_deltaY 0xc19b55aa // Node_LeftButtonUpEventArgs.deltaY
#define ID_Node_LeftButtonUpEventArgs_button 0x4c2a99bf // Node_LeftButtonUpEventArgs.button
#define ID_Node_LeftButtonUpEventArgs_clickCount 0x4595c428 // Node_LeftButtonUpEventArgs.clickCount
#define ID_Node_RightButtonUpEventArgs 0x4140a59a

#define ID_Node_RightButtonUpEventArgs_x 0x5de366ac // Node_RightButtonUpEventArgs.x
#define ID_Node_RightButtonUpEventArgs_y 0x5ee3683f // Node_RightButtonUpEventArgs.y
#define ID_Node_RightButtonUpEventArgs_deltaX 0xb9422e16 // Node_RightButtonUpEventArgs.deltaX
#define ID_Node_RightButtonUpEventArgs_deltaY 0xba422fa9 // Node_RightButtonUpEventArgs.deltaY
#define ID_Node_RightButtonUpEventArgs_button 0xc356b138 // Node_RightButtonUpEventArgs.button
#define ID_Node_RightButtonUpEventArgs_clickCount 0x61ebff0b // Node_RightButtonUpEventArgs.clickCount
#define ID_Node_OtherButtonUpEventArgs 0x650283a2

#define ID_Node_OtherButtonUpEventArgs_x 0x27d03b14 // Node_OtherButtonUpEventArgs.x
#define ID_Node_OtherButtonUpEventArgs_y 0x28d03ca7 // Node_OtherButtonUpEventArgs.y
#define ID_Node_OtherButtonUpEventArgs_deltaX 0xafc9125e // Node_OtherButtonUpEventArgs.deltaX
#define ID_Node_OtherButtonUpEventArgs_deltaY 0xb0c913f1 // Node_OtherButtonUpEventArgs.deltaY
#define ID_Node_OtherButtonUpEventArgs_button 0x95466cc0 // Node_OtherButtonUpEventArgs.button
#define ID_Node_OtherButtonUpEventArgs_clickCount 0x1d0d30e3 // Node_OtherButtonUpEventArgs.clickCount
#define ID_Node_LeftButtonDraggedEventArgs 0x5d72bb08

#define ID_Node_LeftButtonDraggedEventArgs_x 0xf38a9f9e // Node_LeftButtonDraggedEventArgs.x
#define ID_Node_LeftButtonDraggedEventArgs_y 0xf48aa131 // Node_LeftButtonDraggedEventArgs.y
#define ID_Node_LeftButtonDraggedEventArgs_deltaX 0x885ef494 // Node_LeftButtonDraggedEventArgs.deltaX
#define ID_Node_LeftButtonDraggedEventArgs_deltaY 0x895ef627 // Node_LeftButtonDraggedEventArgs.deltaY
#define ID_Node_LeftButtonDraggedEventArgs_button 0xb8f3463a // Node_LeftButtonDraggedEventArgs.button
#define ID_Node_LeftButtonDraggedEventArgs_clickCount 0x94331719 // Node_LeftButtonDraggedEventArgs.clickCount
#define ID_Node_RightButtonDraggedEventArgs 0x8ec28ae3

#define ID_Node_RightButtonDraggedEventArgs_x 0xdf8ac5dd // Node_RightButtonDraggedEventArgs.x
#define ID_Node_RightButtonDraggedEventArgs_y 0xde8ac44a // Node_RightButtonDraggedEventArgs.y
#define ID_Node_RightButtonDraggedEventArgs_deltaX 0xe33c2ba5 // Node_RightButtonDraggedEventArgs.deltaX
#define ID_Node_RightButtonDraggedEventArgs_deltaY 0xe23c2a12 // Node_RightButtonDraggedEventArgs.deltaY
#define ID_Node_RightButtonDraggedEventArgs_button 0x311ff427 // Node_RightButtonDraggedEventArgs.button
#define ID_Node_RightButtonDraggedEventArgs_clickCount 0xb9eabba0 // Node_RightButtonDraggedEventArgs.clickCount
#define ID_Node_OtherButtonDraggedEventArgs 0x9e10189b

#define ID_Node_OtherButtonDraggedEventArgs_x 0x37c9d9b5 // Node_OtherButtonDraggedEventArgs.x
#define ID_Node_OtherButtonDraggedEventArgs_y 0x36c9d822 // Node_OtherButtonDraggedEventArgs.y
#define ID_Node_OtherButtonDraggedEventArgs_deltaX 0xa166667d // Node_OtherButtonDraggedEventArgs.deltaX
#define ID_Node_OtherButtonDraggedEventArgs_deltaY 0xa06664ea // Node_OtherButtonDraggedEventArgs.deltaY
#define ID_Node_OtherButtonDraggedEventArgs_button 0x57364cff // Node_OtherButtonDraggedEventArgs.button
#define ID_Node_OtherButtonDraggedEventArgs_clickCount 0x1583d268 // Node_OtherButtonDraggedEventArgs.clickCount
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
	

#endif