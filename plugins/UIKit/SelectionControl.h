#ifndef ORCA_SELECTION_CONTROL_H
#define ORCA_SELECTION_CONTROL_H

#include <include/orca.h>
#include <UIKit/UIKit.h>

void SelectionControl_Build(struct Object *object, char const *css, bool_t isSwitch);
bool_t SelectionControl_IsEnabled(struct Object *object);
void SelectionControl_SetState(struct Object *object, bool_t checked, bool_t mixed);

#endif
