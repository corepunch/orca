#include "../ed_local.h"

typedef struct _SPLITVIEW {
  HEDWND left, right;
  SPLITVIEWDESC desc;
} SPLITVIEW, *LPSPLITVIEW;

static LPCRECT RECT_VSplit(LPCRECT self, FLOAT d, BOOL left) {
  d = MAX(d, CONSOLE_CHAR_HEIGHT / self->height);
  d = MIN(d, 1 - CONSOLE_CHAR_HEIGHT / self->height);
  static RECT tmp;
  if (left) {
    tmp = (RECT){ self->x, self->y, self->width, self->height * d };
  } else {
    tmp = (RECT){ self->x, self->y + self->height * d, self->width, self->height * (1 - d) };
  }
  return &tmp;
}

static LPCRECT RECT_HSplit(LPCRECT self, FLOAT d, BOOL left) {
  static RECT tmp;
  if (left) {
    tmp = (RECT){ self->x, self->y, self->width * d, self->height };
  } else {
    tmp = (RECT){ self->x + self->width * d, self->y, self->width * (1 - d), self->height };
  }
  return &tmp;
}

static LPCRECT _GetLeft(HEDWND wnd, SPLITVIEWDESC const *desc) {
  RECT self = ED_GetClientRect(wnd);
  return (desc->horizontal?RECT_HSplit:RECT_VSplit)(&self, desc->split, TRUE);
}
static LPCRECT _GetRight(HEDWND wnd, SPLITVIEWDESC const *desc) {
  RECT self = ED_GetClientRect(wnd);
  return (desc->horizontal?RECT_HSplit:RECT_VSplit)(&self, desc->split, FALSE);
}

LPVOID ED_GetUserDataOffset(HEDWND wnd) {
  DWORD offset = ED_SendMessage(wnd, EVT_EXTRASPACE, 0, NULL);
  LPSTR buf = ED_GetUserData(wnd);
  return buf + offset;
}

void ED_SplitView_SetContent(HEDWND wnd, EDPANELPROC proc, lParam_t param) {
  LPSPLITVIEW data = ED_GetUserDataOffset(wnd);
  ED_DestroyWindow(data->right);
  data->right = ED_CreateWindow(proc, 0, _GetRight(wnd, &data->desc), wnd, param);
}

HEDWND ED_SplitView_GetContent(HEDWND wnd) {
  LPSPLITVIEW data = ED_GetUserDataOffset(wnd);
  return data->right;
}

HEDWND ED_SplitView_GetSideBar(HEDWND wnd) {
  LPSPLITVIEW data = ED_GetUserDataOffset(wnd);
  return data->left;
}

EDWINPROC(SplitView) {
  switch (msg) {
    case EVT_CREATE: {
      SPLITVIEWDESC *parm = lparm;
      ED_AllocUserData(wnd, sizeof(SPLITVIEW)+ED_SendMessage(wnd, EVT_EXTRASPACE, 0, NULL));
      LPSPLITVIEW data = ED_GetUserDataOffset(wnd);
      data->left = ED_CreateWindow(parm->left.proc, parm->left.flags, _GetLeft(wnd, parm), wnd, parm->left.parm);
      data->right = ED_CreateWindow(parm->right.proc, parm->right.flags, _GetRight(wnd, parm), wnd, parm->right.parm);
      data->desc = *parm;
      return TRUE;
    }
    case EVT_MOVE:
    case EVT_SIZE: {
      LPSPLITVIEW data = ED_GetUserDataOffset(wnd);
      ED_SetWindowRect(data->left, _GetLeft(wnd, &data->desc));
      ED_SetWindowRect(data->right, _GetRight(wnd, &data->desc));
      return TRUE;
    }
    case EVT_DESTROY:
      free(ED_GetUserData(wnd));
      return TRUE;
    default:
      return FALSE;
  }
}
