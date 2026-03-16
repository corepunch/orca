#include "fs_local.h"
#include "filesystem.h"

#define MONITOR_FILES

#ifdef MONITOR_FILES
struct _MONITOREDFILE
{
  path_t Filename;
  longTime_t Modified;
  struct _MONITOREDFILE* next;
};
bool_t
FS_GetModifiedTime(lpcString_t, longTime_t*);

static int
FS_FileLength(FILE* f)
{
  size_t pos, end;
  pos = ftell(f);
  fseek(f, 0, SEEK_END);
  end = ftell(f);
  fseek(f, pos, SEEK_SET);
  return (int)end;
}

struct file *
_ReadOnDisk(FILE *fp)
{
  uint32_t dwFileSize = FS_FileLength(fp);
  // add 1 to size to be able to add '/0' to the end
  struct file *pFile = ZeroAlloc(sizeof(struct file) + dwFileSize + 1);
  pFile->size = dwFileSize;
  fread(pFile->data, dwFileSize, 1, fp);
  return pFile;
}

static void
_WatchFile(struct Directory* psrch, lpcString_t filename)
{
  struct _MONITOREDFILE* files = psrch->_monitoredfiles;
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, files)
  {
    if (!strcmp(mf->Filename, filename)) {
      return;
    }
  }
  struct _MONITOREDFILE* mf = ZeroAlloc(sizeof(struct _MONITOREDFILE));
  strncpy(mf->Filename, filename, sizeof(mf->Filename));
  FS_GetModifiedTime(filename, &mf->Modified);
  ADD_TO_LIST(mf, files);
  psrch->_monitoredfiles = files;
}
#endif

HANDLER(Directory, OpenFile) {
  FILE* fp = fopen(FS_JoinPaths(pDirectory->Path, pOpenFile->FileName), "rb");
  if (!fp)
    return 0;
#ifdef MONITOR_FILES
  _WatchFile(pDirectory, FS_JoinPaths(pDirectory->Path, pOpenFile->FileName));
#endif
  uint32_t dwFileSize = FS_FileLength(fp);
  // add 1 to size to be able to add '/0' to the end
  struct file *pFile = ZeroAlloc(sizeof(struct file) + dwFileSize + 1);
  pFile->size = dwFileSize;
  fread(pFile->data, dwFileSize, 1, fp);
  fclose(fp);
  return (LRESULT)pFile;
}

HANDLER(Directory, FileExists) {
  FILE* file = fopen(FS_JoinPaths(pDirectory->Path, pFileExists->FileName), "rb");
  if (file) {
    fclose(file);
    return TRUE;
  } else {
    return FALSE;
  }
}

HANDLER(Directory, HasChangedFiles) {
#ifdef MONITOR_FILES
  longTime_t time;
  struct _MONITOREDFILE* files = pDirectory->_monitoredfiles;
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, files) {
    if (FS_GetModifiedTime(mf->Filename, &time) && mf->Modified != time) {
      mf->Modified = time;
      return TRUE;
    }
  }
  return FALSE;
#else
  return FALSE;
#endif
}

HANDLER(Directory, Destroy) {
#ifdef MONITOR_FILES
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, (struct _MONITOREDFILE*)pDirectory->_monitoredfiles) free(mf);
#endif
  return FALSE;
}

