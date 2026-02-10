#include <include/orca.h>

#include <source/UIKit/UIKit.h>


typedef struct Cinematic* PCINEMATIC;

longTime_t
UI_GetTime(void);

HANDLER(Cinematic, DrawBrush)
{
	if (!memcmp(pDrawBrush->brush,
							&(struct BrushShorthand){0},
							sizeof(struct BrushShorthand)) &&
			!pDrawBrush->foreground)
		return FALSE;

  struct ViewEntity entity;

  longTime_t realtime = UI_GetTime();
  longTime_t endtime =
    pCinematic->_starttime + pCinematic->FrameRate * pCinematic->NumFrames;

  if (!pCinematic->_playing) {
    pCinematic->_playing = TRUE;
    pCinematic->_starttime = realtime;
  }

  Node2D_GetViewEntity(hObject, &entity, 0, pDrawBrush->brush);

  entity.rect = GetNode2D(hObject)->_rect;
  entity.type = ET_CINEMATIC;

  if (realtime > endtime) {
    entity.frame = pCinematic->NumFrames - 1;
    entity.opacity =
      1 - MIN(1, (float)(realtime - endtime) / pCinematic->FadeOut);
  } else {
    entity.frame =
      (uint32_t)(realtime - pCinematic->_starttime) / pCinematic->FrameRate;
  }

  if (entity.opacity > 0) {
    assert(sizeof(entity.ninepatch) == MAX_PROPERTY_STRING);
    memcpy(&entity.ninepatch, pCinematic->FileName, MAX_PROPERTY_STRING);
    R_DrawEntity(pDrawBrush->viewdef, &entity);
  }

  return TRUE;
}
