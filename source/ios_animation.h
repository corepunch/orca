#ifndef ORCA_IOS_ANIMATION_H
#define ORCA_IOS_ANIMATION_H

#include <include/orca.h>

// The native adapter only supplies the shared clock's scalar timeline.
void ORCA_IOS_SyncAnimationClock(void);
void ORCA_IOS_ClearAnimationClock(void);
void ORCA_IOS_PauseAnimationClock(bool_t paused);

#endif
