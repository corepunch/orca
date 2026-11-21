#ifndef ORCA_IOS_ACCESSIBILITY_H
#define ORCA_IOS_ACCESSIBILITY_H

#include <include/orca.h>

// Private iOS adapter (W11): mirrors the drawn screen's accessibility
// elements (plugins/UIKit/Accessibility.c) as UIAccessibilityElements in a
// transparent container over the ORCA view, while VoiceOver or Switch
// Control runs. Activating one taps its centre through the engine's queue.
ORCA_API void ORCA_IOS_SyncAccessibility(struct Object *screen);
ORCA_API void ORCA_IOS_ClearAccessibility(void);

#endif
