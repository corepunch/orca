#ifndef ORCA_IOS_PAGE_TRANSITION_H
#define ORCA_IOS_PAGE_TRANSITION_H

#include <include/orca.h>

// Private renderer/UIKit bridge. Geometry is in view points; capture happens
// while the freshly rendered default framebuffer is still available.
struct OrcaIOSPageTransition {
  float x, y, width, height;
  float background[4];
  float fromAngle, toAngle, perspective, duration;
  float curve[4];
  bool_t hold;
};

typedef void (*OrcaIOSPageTransitionCompletion)(uint64_t generation, bool_t finished);
ORCA_API bool_t ORCA_IOS_CanPageTransition(void);
ORCA_API bool_t ORCA_IOS_StartPageTransition(struct OrcaIOSPageTransition const *spec,
  uint64_t generation, OrcaIOSPageTransitionCompletion completion);
// Refresh newly rendered incoming content without restarting its native turn.
ORCA_API void ORCA_IOS_UpdatePageTransition(uint64_t generation);
// A coalesced content refresh requests one paint if its entrance still owns
// the screen. Stale timers carry only a generation, never an engine pointer.
ORCA_API void ORCA_IOS_RequestPageTransitionRefresh(uint64_t generation);
ORCA_API void ORCA_IOS_ClearPageTransition(void);
// Lifecycle teardown also cancels a request that has not reached its capture
// paint yet. This private hook is implemented by UIKit's transition owner.
ORCA_API void ORCA_IOS_CancelPageTransitionRequests(void);
ORCA_API bool_t ORCA_IOS_PageTransitionActive(void);

#endif
