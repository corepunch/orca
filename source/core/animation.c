#include "animation.h"

static AnimationClock animation_clock;
static void *animation_clock_context;
static uint32_t animation_offset, animation_frame_time, animation_pause_time;
static unsigned animation_frame_depth;
static bool_t animation_paused;

static uint32_t Animation_SourceTime(void)
{
  return animation_clock ? animation_clock(animation_clock_context) : (uint32_t)axGetMilliseconds();
}

uint32_t ANIM_GetTime(void)
{
  if (animation_frame_depth) return animation_frame_time;
  return animation_paused ? animation_pause_time : Animation_SourceTime() - animation_offset;
}

void ANIM_SetClock(AnimationClock clock, void *context)
{
  uint32_t time = ANIM_GetTime();
  animation_clock = clock;
  animation_clock_context = context;
  animation_offset = Animation_SourceTime() - time;
}

void ANIM_SetPaused(bool_t paused)
{
  if (animation_paused == paused) return;
  if (paused) animation_pause_time = ANIM_GetTime();
  else animation_offset = Animation_SourceTime() - animation_pause_time;
  animation_paused = paused;
}

void ANIM_BeginFrame(void)
{
  if (!animation_frame_depth) animation_frame_time = ANIM_GetTime();
  animation_frame_depth++;
}

void ANIM_EndFrame(void)
{
  if (animation_frame_depth) animation_frame_depth--;
}
