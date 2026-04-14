#include "ed_local.h"

#define UNDO_DATA_SIZE 64
#define MAX_UNDO_STEPS 256

typedef struct _UNDOSTEP {
  BYTE size;
  BYTE data[UNDO_DATA_SIZE];
  LPVOID dest;
} UNDOSTEP;

static struct _UNDOSYSTEM {
  UNDOSTEP steps[MAX_UNDO_STEPS];
  int head;   // Next slot for new undo (also defines boundary of redo)
  int cursor; // Current view in the stack (can be <= head)
} undo = { 0 };

static BYTE prev(BYTE i) {
  return --i;
}

void ED_StoreUndo(LPVOID dest, LPVOID value, BYTE size) {
  UNDOSTEP step = {
    .size = size,
    .dest = dest,
  };
  if (undo.steps[prev(undo.cursor)].dest != dest) { // merge steps if possible
    memcpy(step.data, dest, size);
    undo.steps[undo.cursor++] = step;
  }
  undo.head = undo.cursor;
  memcpy(dest, value, size);
}

static void ED_ApplyStep(UNDOSTEP *step) {
  BYTE tmp[UNDO_DATA_SIZE];
  memcpy(tmp, step->dest, step->size);
  memcpy(step->dest, step->data, step->size);
  memcpy(step->data, tmp, step->size);
}

BOOL ED_Undo(void) {
  if (prev(undo.cursor) == undo.head ||
      !undo.steps[prev(undo.cursor)].dest)
    return FALSE;
  --undo.cursor;
  ED_ApplyStep(undo.steps+undo.cursor);
  return TRUE;
}

BOOL ED_Redo(void) {
  if (undo.cursor == undo.head)
    return FALSE;
  ED_ApplyStep(undo.steps+undo.cursor);
  ++undo.cursor;
  return TRUE;
}
