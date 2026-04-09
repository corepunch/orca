// Auto-generated from core.xml by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __CORE_PROPERTIES_H__
#define __CORE_PROPERTIES_H__


#define ID_Object_Create ((0x5ad07ee8&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.Create
#define ID_Object_Start ((0x01629a90&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.Start
#define ID_Object_ThemeChanged ((0x6d440f67&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.ThemeChanged
#define ID_Object_PropertyChanged ((0x00d8cdf3&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.PropertyChanged
#define ID_Object_Attached ((0xc920ef48&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.Attached
#define ID_Object_Release ((0x10360cb1&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.Release
#define ID_Object_Destroy ((0x8ef01bbe&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.Destroy
#define ID_Object_Timer ((0x6e6ade9d&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.Timer
#define ID_Object_Animate ((0xc8c0972b&MSG_DATA_MASK)|ROUTING_DIRECT) // Object.Animate

	

#define ID_Object_CreateEventArgs 0xd370143c

#define ID_Object_StartEventArgs 0x23dcf2aa

#define ID_Object_ThemeChangedEventArgs 0x22275cf1

#define ID_Object_PropertyChangedEventArgs 0x530ec22f

#define ID_Object_PropertyChangedEventArgs_Property 0xc7308dfa // Object_PropertyChangedEventArgs.Property
#define ID_Object_AttachedEventArgs 0x421be758

#define ID_Object_ReleaseEventArgs 0x7710fc41

#define ID_Object_DestroyEventArgs 0x3eec8a44

#define ID_Object_TimerEventArgs 0x75284ba1

	

// AnimationCurve component IDs
#define ID_AnimationCurve 0xdbe5783c
#define ID_AnimationCurve_Path 0xdda91d5d // AnimationCurve.Path
#define ID_AnimationCurve_Property 0x525d5123 // AnimationCurve.Property
#define ID_AnimationCurve_Keyframes 0x161acd67 // AnimationCurve.Keyframes
#define ID_AnimationCurve_NumKeyframes 0x7cda066f // AnimationCurve.NumKeyframes
enum { kAnimationCurveNumProperties = 4 };
enum {
	kAnimationCurvePath,
	kAnimationCurveProperty,
	kAnimationCurveKeyframes,
	kAnimationCurveNumKeyframes,
};

// AnimationClip component IDs
#define ID_AnimationClip 0xac597053
#define ID_AnimationClip_Mode 0x2a6a9d40 // AnimationClip.Mode
#define ID_AnimationClip_StartTime 0xd35a8f6c // AnimationClip.StartTime
#define ID_AnimationClip_StopTime 0x52f0f22e // AnimationClip.StopTime
enum { kAnimationClipNumProperties = 3 };
enum {
	kAnimationClipMode,
	kAnimationClipStartTime,
	kAnimationClipStopTime,
};

// AnimationPlayer component IDs
#define ID_AnimationPlayer 0x81ac71d8
#define ID_AnimationPlayer_Clip 0xbc835a74 // AnimationPlayer.Clip
#define ID_AnimationPlayer_Playing 0xdd2ed06c // AnimationPlayer.Playing
#define ID_AnimationPlayer_Looping 0x02535bc8 // AnimationPlayer.Looping
#define ID_AnimationPlayer_Speed 0x4a1ae835 // AnimationPlayer.Speed
#define ID_AnimationPlayer_CurrentTime 0xbc48783a // AnimationPlayer.CurrentTime
#define ID_AnimationPlayer_Play ((0x389b07cc&MSG_DATA_MASK)|ROUTING_DIRECT) // AnimationPlayer.Play
#define ID_AnimationPlayer_Stop ((0x59228a5a&MSG_DATA_MASK)|ROUTING_DIRECT) // AnimationPlayer.Stop
#define ID_AnimationPlayer_Pause ((0xafb93ba4&MSG_DATA_MASK)|ROUTING_DIRECT) // AnimationPlayer.Pause
enum { kAnimationPlayerNumProperties = 6 };
enum {
	kAnimationPlayerClip,
	kAnimationPlayerPlaying,
	kAnimationPlayerLooping,
	kAnimationPlayerSpeed,
	kAnimationPlayerCurrentTime,
	kAnimationPlayerPlayEvent,
	kAnimationPlayerStopEvent,
	kAnimationPlayerPauseEvent,
};

#define ID_AnimationPlayer_PlayEventArgs 0x1e984720
#define ID_AnimationPlayer_StopEventArgs 0xa977f29a
#define ID_AnimationPlayer_PauseEventArgs 0x68215542

#endif