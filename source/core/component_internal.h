#ifndef __COMPONENT_INTERNAL_H__
#define __COMPONENT_INTERNAL_H__

// Internal definition of struct component — available to core module internals.
// External callers must use the public CMP_* API in orca.h.
struct component
{
  struct component* next;
  lpcClassDesc_t pcls;
  lpObject_t pobj;
  char pUserData[];
};

#endif /* __COMPONENT_INTERNAL_H__ */
