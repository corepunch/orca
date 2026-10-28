#ifndef ORCA_IOS_SCROLL_H
#define ORCA_IOS_SCROLL_H

#include <include/orca.h>

// Private iOS adapter. UIKit owns the gesture and its clock; queued Animate
// messages only commit its latest offset on the engine's normal message path.
ORCA_API void ORCA_IOS_SyncScrollViews(struct Object *screen);
ORCA_API void ORCA_IOS_ClearScrollViews(void);
ORCA_API void ORCA_IOS_SetScrollViewsEnabled(bool_t enabled);
ORCA_API void ORCA_IOS_ForgetScrollView(struct Object *object);
ORCA_API bool_t ORCA_IOS_IsNativeScroll(struct Object *object);
ORCA_API bool_t ORCA_IOS_IsNativeScrollActive(struct Object *object);
ORCA_API bool_t ORCA_IOS_ApplyNativeScroll(struct Object *object);
ORCA_API void ORCA_IOS_NativeScrollOffsetChanged(struct Object *object);

#endif
