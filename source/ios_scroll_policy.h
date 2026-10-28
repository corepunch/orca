#ifndef ORCA_IOS_SCROLL_POLICY_H
#define ORCA_IOS_SCROLL_POLICY_H

#include <math.h>

// UIKit's geometric deceleration projects distance as v * r / (1 - r).
// Scaling that distance preserves the finger's release velocity.
static inline double
ORCA_IOS_ScrollRetention(double retention, double distanceMultiplier)
{
  if (!(retention > 0 && retention < 1) || !(distanceMultiplier > 0) || !isfinite(distanceMultiplier)) return retention;
  return distanceMultiplier * retention / (1 + (distanceMultiplier - 1) * retention);
}

static inline double ORCA_IOS_ScrollOffset(double contentOffset) { return -contentOffset; }

static inline double
ORCA_IOS_ScrollExtent(double contentHeight, double viewportHeight)
{
  if (!isfinite(contentHeight) || !isfinite(viewportHeight)) return 0;
  return fmax(0, contentHeight - viewportHeight);
}

#endif
