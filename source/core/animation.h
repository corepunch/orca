#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include <include/api.h>
#include <include/orca.h>

size_t
animation_size(uint32_t num_keyframes);

float
animation_evaluate(struct curve* animation, float time, int xyzw);

void
ANIM_Release(lpKeyframeAnim_t anim);

struct tween_animation
{
  lpProperty_t target;
  struct tween_animation* next;
  void* from;
  void* to;
  byte_t data[];
};

struct tween
{
  enum ipo_type ipo;
  enum easing easing;
  struct
  {
    longTime_t start;
    longTime_t pause;
  } timestamp;
  int duration;
  struct tween_animation* animations;
};

#endif
