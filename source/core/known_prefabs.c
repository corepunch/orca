#include "core_local.h"

#define MAX_PREFABS 2048

typedef struct
{
  char path[256];
  lpObject_t object;
} knownPrefab_t;

knownPrefab_t known_prefabs[MAX_PREFABS] = { 0 };

void Init_KnownPrefabs(void) {
  memset(known_prefabs, 0, sizeof(known_prefabs));
}

ORCA_API lpObject_t
OBJ_FindKnownPrefab(lpcString_t szFileName, lpcString_t* ppRemaining)
{
  for (uint32_t i = 1; i < MAX_PREFABS && *known_prefabs[i].path; i++) {
    size_t size = strlen(known_prefabs[i].path);
    
    if (!strncmp(known_prefabs[i].path, szFileName, size)) {
      if (ppRemaining)
        *ppRemaining = szFileName + size;
      return known_prefabs[i].object;
    }
  }
  return NULL;
}

ORCA_API void
OBJ_RegisterPrefab(lpObject_t hobj, lpcString_t szFileName)
{
  uint32_t i = 1;
  path_t filename={0};
  strncpy(filename, szFileName, sizeof(filename));
  if (filename[strlen(filename) - 1] != '/') strcat(filename, "/");
  for (; i < MAX_PREFABS && *known_prefabs[i].path; i++)
    if (!strcmp(known_prefabs[i].path, filename))
      break;
  if (i != MAX_PREFABS) {
    strncpy(known_prefabs[i].path, filename, sizeof(known_prefabs[i].path));
    known_prefabs[i].object = hobj;
  } else {
    Con_Error("Too many prefabs registered");
//    for (int i = 0; i < MAX_PREFABS; i++) {
//      Con_Error("%s", known_prefabs[i].path);
//    }
  }
  
  //  if (szPackageDir) {
  //    path_t url;
  //    snprintf(url, sizeof(path_t), "%s/%s", szPackageDir, szFileName);
  //    OBJ_SetSourceFile(hobj, url);
  //  } else {
  OBJ_SetSourceFile(hobj, szFileName);
  //  }
}
