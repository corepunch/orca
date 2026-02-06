//
//  ed_stab_filesystem.h
//  orca
//
//  Created by Igor Chernakov on 01.07.25.
//

#include "ed_local.h"

#define MAX_FILE_HASHES (1 << 12)
#define MAX_FILENAME_BUFFER (MAX_FILE_HASHES*MAX_PROPERTY_STRING)

typedef struct _FILEHASH {
  DWORD dwHash;
  DWORD dwParent;
  lpcString_t szFileName;
} FILEHASH, *LPFILEHASH;

static struct {
  FILEHASH *filehashes;
  LPSTR filebuffer;
  int writehash;
  int writebuffer;
} _fh;

//HRESULT FS_EnumSearchPaths(EnumChildProc, LPVOID);
DWORD FS_GetFileIdentifier(lpcString_t, lpcString_t);
BOOL FS_GetFileName(HANDLE, LPSTR, BOOL);
lpcString_t FS_GetFileNamesPointer(void);

struct _file {
  lpcString_t name;
  int style;
  int identifier;
  int d_type;
  int add_button;
};

#include <dirent.h>

int compareByName(const void* a, const void* b) {
  struct _file* fileA = (struct _file*)a;
  struct _file* fileB = (struct _file*)b;
  if (fileA->d_type == DT_DIR && fileB->d_type != DT_DIR) {
    return -1; // fileA should come first
  }
  if (fileA->d_type != DT_DIR && fileB->d_type == DT_DIR) {
    return 1; // fileB should come first
  }
  return strcasecmp(fileA->name, fileB->name);
}

static DWORD GetFileStyle(DWORD d_type, lpcString_t fulltext) {
  DWORD style = 15;
  if (d_type == DT_DIR) {
    style = 7;
  }
  return style;
}

//static inline int find_if(const void *a, size_t n, size_t s, size_t off, const char *v) {
//  for (size_t i = 0; i < n; i++) {
//    const char *f = *(const char **)((const char *)a + i * s + off);
//    if (f && strcmp(f, v) == 0) return (int)i;
//  }
//  return -1;
//}
//
//#define strfind(a,n,f,v) find_if(a, n, sizeof(*(a)), offsetof(__typeof__(*(a)), f), v)

ORCA_API uint32_t FS_FindLibraryType(lpcString_t libname) {
  lpcString_t shortname = strrchr(libname, '/');
  FOR_LOOP(i, kNumLibraries) {
    if (!strcmp(libraries[i].name, shortname?shortname+1:libname)) {
      return i;
    }
  }
  return -1;
}

xmlNodePtr FS_FindLibrary(lpcString_t libname) {
  if (FS_FindLibraryType(libname) < kNumLibraries) {
    return MainBundle->libraries[FS_FindLibraryType(libname)];
  } else {
    return NULL;
  }
}

int FS_HasChildren(lpcString_t parent, lpcString_t filename) {
  char dir_path[1024]={0};
  snprintf(dir_path, sizeof(dir_path), "%s/%s", parent, filename);
  if (!MainBundle) {
    return FALSE;
  }
  if (!strcmp(parent, MainBundle->path) && FS_FindLibrary(filename)) {
    return FS_FindLibrary(filename)->children != NULL;
//    uint32_t library = strfind(libraries, kNumLibraries, name, szLibrary);
  }
  DIR *dir = opendir(dir_path);
  if (!dir) {
    return FALSE;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      closedir(dir);
      return TRUE;
    }
  }
  closedir(dir);
  return FALSE;
}

#include <ctype.h>

static struct _file *_AddFile(lpcString_t name, DWORD d_type, struct _file *file, lpcString_t root_dir) {
  PATHSTR fulltext;
  snprintf(fulltext, sizeof(fulltext), "%s/%s", root_dir, name);
  DWORD style = GetFileStyle(d_type, fulltext);
  file->style = style;
  file->d_type = d_type;
  file->name = FS_GetFileNamesPointer();
  file->identifier = FS_GetFileIdentifier(root_dir, name);
  file->add_button = FALSE;
  return file;
}

ORCA_API void
FS_EnumDir2(lpcString_t root_dir, void (*callback)(lpcString_t,LPVOID), LPVOID parm)
{
  xmlWith(DIR, dir, opendir(root_dir), closedir) {
    for (struct dirent *entry = readdir(dir); entry; entry = readdir(dir)) {
      if (*entry->d_name == '.')
        continue;
      callback(entry->d_name, parm);
    }
  }
}

ORCA_API void
FS_EnumDir(lpcString_t root_dir,
           EnumChildProc callback,
           LPVOID parm)
{
  DIR *dir;
  xmlNodePtr lib;
  struct _file files[1024];
  struct dirent *entry;
  int _numfiles=0;
  if (!root_dir) {
    PATHSTR project={0};
    FS_GetFileName(0, project, TRUE);
    FOR_LOOP(i, sizeof(libraries)/sizeof(*libraries)) {
      uint32_t file_id = FS_GetFileIdentifier(root_dir, libraries[i].name);
      callback(&(struct _OBJDEF) {
        .hIdentifier = file_id,
        .szName = libraries[i].name,
        .bHasChildren = FS_HasChildren(project, libraries[i].name),
        .dwStyle = 7,
        //        .lpUserData = NULL,
        .lpObjectItself = (void*)(intptr_t)file_id,
      }, parm);
    }
    //    FS_EnumSearchPaths(search, callback, parm);
  } else if ((lib = FS_FindLibrary(FS_GetBaseName(root_dir)))) {
    xmlForEach(item, lib) {
      xmlWith(xmlChar, Name, xmlGetProp(item, XMLSTR("Name")), xmlFree) {
        _AddFile((lpcString_t)Name, DT_CHR, &files[_numfiles++], root_dir);
      }
    }
  } else if ((dir = opendir(root_dir))) {
    while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      if (*entry->d_name == '.')
        continue;
      _AddFile(entry->d_name, entry->d_type, &files[_numfiles++], root_dir);
    }
    closedir(dir);
    qsort(files, _numfiles, sizeof(struct _file), compareByName);
  }
  
  FOR_LOOP(i, _numfiles) {
    callback(&(struct _OBJDEF) {
      .hIdentifier = files[i].identifier,
      .szName = files[i].name,
      .hParent = fnv1a32(root_dir),
      .bHasChildren = files[i].d_type == DT_DIR ? FS_HasChildren(root_dir, files[i].name) : FALSE,
      .dwStyle = files[i].style,
      .lpUserData = (DWORD*)1,
      .bHasAddButton = files[i].add_button,
      .lpObjectItself = (void*)(intptr_t)files[i].identifier,
    }, parm);
  }
}

void FS_FilterFilesInDir(lpcString_t szRoot, lpcString_t szFilter, EnumChildProc lpProc, LPVOID lpParm) {
  DIR *dir;
  struct dirent *entry;
  if (!(dir = opendir(szRoot)))
    return;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;
    if (*entry->d_name == '.')
      continue;
    CHAR fulltext[1024];
    snprintf(fulltext, sizeof(fulltext), "%s/%s", szRoot, entry->d_name);
    if (entry->d_type == DT_DIR) {
      FS_FilterFilesInDir(fulltext, szFilter, lpProc, lpParm);
    } else if (strstr(entry->d_name, szFilter)) {
//      Con_Error("%s", entry->d_name);
      lpProc(&(struct _OBJDEF) {
        .hIdentifier = FS_GetFileIdentifier(szRoot, entry->d_name),
        .szName = entry->d_name,
        .hParent = fnv1a32(szRoot),
        .bHasChildren = FALSE,
        .dwStyle = GetFileStyle(entry->d_type, fulltext),
        .lpUserData = (DWORD*)1,
      }, lpParm);
    }
  }
  closedir(dir);
}

lpcString_t FS_GetFileNamesPointer(void) {
  return _fh.filebuffer + _fh.writebuffer;
}

DWORD FS_GetFileIdentifier(lpcString_t szDirectory, lpcString_t name) {
  char path[1024];
  snprintf(path, sizeof(path), "%s/%s", szDirectory, name);
  LPFILEHASH lpfh = &_fh.filehashes[(_fh.writehash++)%MAX_FILE_HASHES];
  lpfh->dwHash = fnv1a32(path);
  if (szDirectory) {
    lpfh->dwParent = fnv1a32(szDirectory);
  } else {
    lpfh->dwParent = 0;
  }
  strcpy(_fh.filebuffer + _fh.writebuffer, name);
  lpfh->szFileName = _fh.filebuffer + _fh.writebuffer;
  _fh.writebuffer += strlen(name)+1;
  if (_fh.writebuffer > MAX_FILENAME_BUFFER) {
    _fh.writebuffer = 0;
  }
  return lpfh->dwHash;
}

//HRESULT FS_EnumSearchPaths(EnumChildProc fnProc, LPVOID lpParam)
//{
//  FOR_EACH_LIST(struct Package, fs, MainBundle) {
//    if (fs->pack)
//      continue;
//    LPFILEHASH lpfh = &_fh.filehashes[(_fh.writehash++)%MAX_FILE_HASHES];
//    lpfh->dwHash = fnv1a32(fs->path);
//    lpfh->dwParent = 0;
//    lpfh->szFileName = fs->path;
//    fnProc(&(struct _OBJDEF) {
//      .hIdentifier = lpfh->dwHash,
//      .szName = *fs->path ?  FS_GetBaseName(fs->path) : "(local)",
//      .bHasChildren = TRUE,
//      .dwStyle = 7,
//      .lpUserData = (DWORD*)fs,
//      .lpObjectItself = (void*)(intptr_t)lpfh->dwHash,
//
//    }, lpParam);
//  }
//  return NOERROR;
//}

void FS_FilterFilesInDir(lpcString_t szPath,
                         lpcString_t szFilter,
                         EnumChildProc lpProc,
                         LPVOID lpParm);

ORCA_API void
FS_FilterFiles(lpcString_t szFilter,
               EnumChildProc lpProc,
               LPVOID lpParm)
{
  if (strlen(szFilter) < 3)
    return;
  FOR_EACH_LIST(struct Package, fs, MainBundle) {
    if (fs->pack)
      continue;
    FS_FilterFilesInDir(fs->path, szFilter, lpProc, lpParm);
  }
}

ORCA_API BOOL FS_GetFileName(HANDLE ident, LPSTR path, BOOL fullpath)
{
  if (!ident) {
    if (MainBundle) {
      LPFILEHASH lpfh = &_fh.filehashes[(_fh.writehash++)%MAX_FILE_HASHES];
      lpfh->dwHash = fnv1a32(MainBundle->path);
      lpfh->dwParent = 0;
      lpfh->szFileName = MainBundle->path;
//      strcpy(path, fullpath ? search->path : search->relative);
      strcpy(path, MainBundle->path);
      return TRUE;
    } else {
      return FALSE;
    }
  }
  if (!_fh.filehashes)
    return FALSE;
  FOR_LOOP(i, MAX_FILE_HASHES) {
    LPFILEHASH fh = _fh.filehashes+i;
    if (fh->dwHash == ident) {
      if (fh->dwParent && fullpath) {
        FS_GetFileName(fh->dwParent, path, fullpath);
        strcat(path, "/");
      }
      strcat(path, fh->szFileName);
      return TRUE;
    }
  }
  return FALSE;
}

ORCA_API BOOL
FS_FindFileByHandle(HANDLE hIdentifier, LPOBJDEF lpObjDef)
{
  static char fullpath[1024];
  static char name[256];
  memset(fullpath, 0, sizeof(fullpath));
  memset(name, 0, sizeof(name));
  if (FS_GetFileName(hIdentifier, fullpath, TRUE)&&
      FS_GetFileName(hIdentifier, name, FALSE))
  {
    memset(lpObjDef, 0, sizeof(struct _OBJDEF));
    lpObjDef->hIdentifier = hIdentifier;
    lpObjDef->szName = name;
    lpObjDef->szFullPath = fullpath;
    lpObjDef->dwItemType = -1;
    lpObjDef->lpObjectItself = (void*)(intptr_t)hIdentifier;;
    return TRUE;
  } else {
    return FALSE;
  }
}

ORCA_API BOOL
FS_RenameFile(HANDLE handle, lpcString_t szFilename)
{
  struct _OBJDEF objdef;
  if (FS_FindFileByHandle(handle, &objdef)) {
    PATHSTR path;
    strcpy(path, objdef.szFullPath);
    LPSTR lastpiece = strchr(path, '/');
    while (strchr(lastpiece, '/')) {
      lastpiece = strchr(lastpiece, '/')+1;
    }
    strcpy(lastpiece, szFilename);
    rename(objdef.szFullPath, path);
    return TRUE;
  }
  return FALSE;
}

ORCA_API DWORD
FS_NewFile(HANDLE hHandle, lpcString_t szName, DWORD dwType)
{
  struct _OBJDEF objdef;
  if (dwType == PROJITEM_PROJECT) {
    xmlWith(xmlDoc, doc, xmlReadFile(szName, NULL, 0), xmlFreeDoc) {
      xmlFindAllText(_, Name, xmlDocGetRootElement(doc), XMLSTR("Name")) {
        path_t tmp={0};
        strncpy(tmp, szName, sizeof(tmp));
        if (strrchr(tmp, '/')) {
          *strrchr(tmp, '/') = 0;
        }
        xmlChar const name[] = "ProjectItemReference";
        xmlNodePtr lib = MainBundle->libraries[kProjectReferenceLibrary];
        xmlNodePtr itm = xmlNewChild(lib, NULL, name, XMLSTR(tmp));//szName));
        xmlSetProp(itm, XMLSTR("Name"), Name);
      }
    }
    return TRUE;
  } else if (FS_FindFileByHandle(hHandle, &objdef)) {
    PATHSTR path = {0};
    strcpy(path, objdef.szFullPath);
    strcat(path, "/");
    strcat(path, szName);
    
    switch (dwType) {
      case PROJITEM_SCENE:
      case PROJITEM_MATERIAL:
      case PROJITEM_SHADER:
        strcat(path, ".xml");
        break;
      default:
        Con_Error("Could not find type %08x", dwType);
        return 0;
    }
    FS_MakeDirectory(objdef.szFullPath);
    xmlWith(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFreeDoc) {
      xmlNodePtr root=NULL;
      switch (dwType) {
        case PROJITEM_SCENE:
          root = xmlNewNode(NULL, XMLSTR("Screen"));
          xmlSetProp(root, XMLSTR("Name"), XMLSTR(szName));
          break;
        case PROJITEM_MATERIAL:
          root = xmlNewNode(NULL, XMLSTR("Material"));
          xmlSetProp(root, XMLSTR("Name"), XMLSTR(szName));
          xmlSetProp(root, XMLSTR("Shader"), XMLSTR("$Default"));
          break;
        default:
          break;
      }
      if (root) {
        xmlDocSetRootElement(doc, root);
        xmlSaveFormatFile(path, doc, TRUE);
      }
    }
    return fnv1a32(path);
  } else {
    Con_Error("Could not find find by handle %08x", hHandle);
    return 0;
  }
}

void FS_InitHash(void) {
  memset(&_fh, 0, sizeof(_fh));
  _fh.filehashes = ZeroAlloc(sizeof(FILEHASH)*MAX_FILE_HASHES);
  _fh.filebuffer = ZeroAlloc(MAX_FILENAME_BUFFER);
}

void FS_ShutdownHash(void) {
  free(_fh.filehashes);
  free(_fh.filebuffer);
}

ORCA_API BOOL
FS_GetProjectReference(lpcString_t szName, LPSTR pOut, DWORD nMaxLen)
{
  if (!MainBundle->libraries[kProjectReferenceLibrary])
    return FALSE;
  BOOL bWritten = TRUE;
  xmlForEach(item, MainBundle->libraries[kProjectReferenceLibrary]) {
    xmlWith(xmlChar, Name, xmlGetProp(item, XMLSTR("Name")), xmlFree) {
      if (!xmlStrcmp(Name, XMLSTR(szName))) {
        xmlWith(xmlChar, Content, xmlNodeGetContent(item), xmlFree) {
          lpcString_t path = FS_JoinPaths(MainBundle->path, (const char *)Content);
          strncpy(pOut, path, nMaxLen);
        }
      }
    }
  }
  return bWritten;
}

//static int cmp(const void *a, const void *b) {
//  xmlChar *na = xmlGetProp(*(xmlNodePtr*)a, (xmlChar*)"name");
//  xmlChar *nb = xmlGetProp(*(xmlNodePtr*)b, (xmlChar*)"name");
//  int r = strcmp((char*)(na?na:(xmlChar*)""), (char*)(nb?nb:(xmlChar*)""));
//  if (na) xmlFree(na); if (nb) xmlFree(nb);
//  return r;
//}
//
//static void sortChildrenByName(xmlNodePtr root) {
//  int n=0; for(xmlNodePtr c=root->children;c;c=c->next) n++;
//  xmlNodePtr *arr=malloc(n*sizeof(*arr));
//  int i=0; for(xmlNodePtr c=root->children;c;c=c->next) arr[i++]=c;
//  qsort(arr,n,sizeof(*arr),cmp);
//  xmlNodePtr last=NULL;
//  for(i=0;i<n;i++){
//    if(last) xmlAddNextSibling(last,arr[i]);
//    else xmlAddChild(root,arr[i]);
//    last=arr[i];
//  }
//  free(arr);
//}

//static void
//collect(lpcString_t szDirectory, lpcString_t szType, xmlNodePtr parent)
//{
//  xmlWith(DIR, dir, opendir(szDirectory), closedir) {
//    for (struct dirent *e = readdir(dir); e != NULL; e = readdir(dir)) {
//      if (*e->d_name == '.')
//        continue;
//      xmlNodePtr node = xmlNewChild(parent, NULL, XMLSTR("File"), NULL);
//      path_t name={0};
//      strncpy(name, e->d_name, sizeof(name));
//      if (strrchr(name, '.')) *strrchr(name, '.') = '\0';
//      xmlSetProp(node, XMLSTR("name"), XMLSTR(name));
//      xmlSetProp(node, XMLSTR("data-type"), XMLSTR(szType));
//      xmlSetProp(node, XMLSTR("data-content"), XMLSTR(szType));
//      if (e->d_type == DT_DIR) {
//        node->name = XMLSTR("Folder");
//        collect(FS_JoinPaths(szDirectory, e->d_name), szType, node);
//      }
//    }
//  }
//  sortChildrenByName(parent);
//}

enum { no_match, exact_match, partial_match };

//static int match(lpcString_t args, lpcString_t src, lpcString_t *rest) {
//  if (src && !strncmp(args, src, strlen(src))) {
//    if (args[strlen(src)] == '\0') {
//      return exact_match;
//    } else if (args[strlen(src)] == '/') {
//      *rest = args+strlen(src)+1;
//      return partial_match;
//    } else {
//      return no_match;
//    }
//  } else {
//    return no_match;
//  }
//}

xmlNodePtr
__xmlNewChild(xmlNodePtr p, lpcString_t name, lpcString_t args[])
{
  xmlNodePtr n = xmlNewChild(p, NULL, XMLSTR(name), NULL);
  for (int i = 0; args[i];) {
    int key = i++;
    if (args[i]) {
      int value = i++;
      xmlSetProp(n, XMLSTR(args[key]), XMLSTR(args[value]));
    }
  }
  return n;
}

//ORCA_API SV_CMD(GET, project_overview) {
//  FOR_LOOP(i, kNumLibraries) {
//    lpcString_t id = FS_JoinPaths(MainBundle->name, libraries[i].name);
//    xmlNodePtr node = _xmlNewChild(parent, "Library",
//      "name", libraries[i].name,
//      "data-type", libraries[i].id,
//      "data-content", libraries[i].type,
//      "data-source", id);
//    if (libraries[i].ondisk) {
//      collect(FS_JoinPaths(MainBundle->path, libraries[i].name), libraries[i].type, node);
//    } else if (MainBundle->libraries[i]) {
//      xmlForEach(item, MainBundle->libraries[i]) {
//        xmlWith(xmlChar, Name, xmlGetProp(item, XMLSTR("Name")), xmlFree) {
//          _xmlNewChild(node, "File", "name", (lpcString_t)Name);
//        }
//      }
//    }
//  }
//  sortChildrenByName(parent);
//}

ORCA_API lpcString_t FS_GetMainBundleName(void) {
  return MainBundle->name;
}
