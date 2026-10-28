#ifndef ORCA_ANIMATION_H
#define ORCA_ANIMATION_H

#include <include/orca.h>

// Private shared clock. A platform supplies time, never animation semantics.
// Source changes and suspension preserve elapsed animation time.
typedef uint32_t (*AnimationClock)(void *context);
ORCA_API uint32_t ANIM_GetTime(void);
ORCA_API void ANIM_SetClock(AnimationClock clock, void *context);
ORCA_API void ANIM_SetPaused(bool_t paused);
ORCA_API void ANIM_BeginFrame(void);
ORCA_API void ANIM_EndFrame(void);

#endif
