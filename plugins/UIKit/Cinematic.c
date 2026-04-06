#include <include/orca.h>

#include <plugins/UIKit/UIKit.h>


typedef struct Cinematic* PCINEMATIC;

longTime_t
UI_GetTime(void);

HANDLER(Cinematic, Node2D, DrawBrush)
{
	if (!memcmp(&pDrawBrush->brush,
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

  Node2D_GetViewEntity(GetNode2D(hObject), &entity, 0, &pDrawBrush->brush);

  entity.bbox = BOX3_FromRect(GetNode2D(hObject)->_rect);
  entity.mesh = BOX_PTR(Mesh, MD_RECTANGLE);
  entity.shader = BOX_PTR(Shader, SHADER_CINEMATIC);

  if (realtime > endtime) {
    entity.frame = pCinematic->NumFrames - 1;
    entity.material.opacity =
      1 - MIN(1, (float)(realtime - endtime) / pCinematic->FadeOut);
  } else {
    entity.frame =
      (uint32_t)(realtime - pCinematic->_starttime) / pCinematic->FrameRate;
  }

  if (entity.material.opacity > 0) {
    memset(&entity.ninepatch, 0, sizeof(entity.ninepatch));
    if (pCinematic->FileName) {
      strncpy((char*)&entity.ninepatch, pCinematic->FileName, sizeof(entity.ninepatch) - 1);
    }
    R_DrawEntity(pDrawBrush->viewdef, &entity);
  }

  return TRUE;
}
