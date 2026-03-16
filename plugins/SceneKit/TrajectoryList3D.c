#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include "SceneKit.h"

HANDLER(TrajectoryList3D, UpdateMatrix)
{
  float index = -pTrajectoryList3D->SelectedItemIndex;
  FOR_EACH_OBJECT(child, hObject)
  {
    float const offset = fmin(1, fmax(-1, -index));
    GetNode3D(child)->CalculatedOffset = offset * 0.5f + 0.5f;
    GetNode3D(child)->LayoutTransform.translation.x = index * 8;
    index++;
  }
  return FALSE;
}
