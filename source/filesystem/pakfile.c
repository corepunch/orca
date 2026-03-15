#ifndef ORCA_NO_LZ4
#ifdef __QNX__
#include <lz4/lz4.h>
#else
#include <lz4.h>
#endif
#endif

#ifndef ORCA_NO_ZIP
#include <zlib.h>
#endif

#include <include/orca.h>
#include <include/plugapi.h>

#define IDPAKHEADER MAKE_FOURCC('P', 'A', 'C', 'K')
#define MAX_READ 0x10000

struct dpackheader
{
  uint32_t identifier; // == IDPAKHEADER
  uint32_t version;
  uint32_t dirofs;
  uint32_t dirlen;
};

typedef struct _PACK
{
  char filename[MAX_OSPATH];
  FILE* handle;
  int numfiles;
  struct _PACKFILE* files;
}* PPACK;

typedef struct _PACKFILE
{
  uint32_t identifier;
  uint32_t flags;
  uint32_t filepos;
  uint32_t filelen;
  uint32_t decompsize;
  struct WI_Size imagesize;
}* PPACKFILE;

#ifndef ORCA_NO_ZIP
static HRESULT
FS_DecompressZlib(FILE* inputfile, PPACKFILE pf, struct file** ppFile)
{
  // Initialize zlib structures
  z_stream stream;
  memset(&stream, 0, sizeof(stream));
  
  if (inflateInit(&stream) != Z_OK) {
    Con_Error("Error initializing zlib");
    return E_ABORT;
  }
  
  // Allocate memory for the compressed data
  LPSTR in_buffer = malloc(pf->filelen);
  if (!in_buffer) {
    Con_Error("Error allocating memory");
    return E_OUTOFMEMORY;
  }
  stream.avail_in = (int)fread(in_buffer, 1, pf->filelen, inputfile);
  stream.next_in = (unsigned char *)in_buffer;
  
  // Allocate memory for the decompressed data
  *ppFile = ZeroAlloc(sizeof(struct file) + pf->decompsize + 1);
  if (!*ppFile) {
    Con_Error("Error allocating memory");
    inflateEnd(&stream);
    free(in_buffer);
    return E_OUTOFMEMORY;
  }
  
  stream.avail_out = pf->decompsize;
  stream.next_out = (*ppFile)->data;
  
  // Decompress
  int ret = inflate(&stream, Z_FINISH);
  
  if (ret != Z_STREAM_END || stream.total_out != pf->decompsize) {
    Con_Error("Error decompressing data");
    free(*ppFile);
    free(in_buffer);
    inflateEnd(&stream);
    return E_FAIL;
  }
  
  // Finish decompression
  if (inflateEnd(&stream) != Z_OK) {
    Con_Error("Error finishing decompression");
    free(*ppFile);
    free(in_buffer);
    return E_FAIL;
  }
  
  (*ppFile)->size = pf->decompsize;
  
  free(in_buffer);
  
  return NOERROR;
}
#endif

#ifndef ORCA_NO_LZ4
static HRESULT
FS_DecompressLZ4(FILE* inputfile, PPACKFILE pf, struct file** ppFile)
{
  // Allocate memory for the compressed data
  LPSTR compressed_data = (LPSTR)malloc(pf->filelen);
  if (!compressed_data) {
    Con_Error("Error allocating memory");
    return E_OUTOFMEMORY;
  }
  // Read the compressed data from the input file
  if (fread(compressed_data, 1, pf->filelen, inputfile) != pf->filelen) {
    Con_Error("Error reading input file");
    free(compressed_data);
    return E_INVALIDARG;
  }
  // Allocate memory for the decompressed data
  *ppFile = ZeroAlloc(sizeof(struct file) + pf->decompsize + 1);
  if (!*ppFile) {
    Con_Error("Error allocating memory");
    free(compressed_data);
    return E_OUTOFMEMORY;
  }
  // Decompress the data
  int decompressed_size = LZ4_decompress_safe(
                                              compressed_data, (LPSTR)(*ppFile)->data, pf->filelen, pf->decompsize);
  if (decompressed_size < 0) {
    Con_Error("Error decompressing data");
    free(compressed_data);
    free(*ppFile);
    return E_FAIL;
  }
  
  (*ppFile)->size = decompressed_size;
  
  // Cleanup
  free(compressed_data);
  
  return NOERROR;
}
#endif

PPACK
_LoadPackFile(lpcString_t szPackfile)
{
  struct dpackheader header;
  FILE* packhandle = fopen(szPackfile, "rb");
  
  if (!packhandle)
    return NULL;
  
  fread(&header, 1, sizeof(header), packhandle);
  if (header.identifier != IDPAKHEADER) {
    Con_Error("%s is not a PACK file", szPackfile);
    fclose(packhandle);
    return NULL;
  }
  if (header.version != ORCA_PACK_VERSION) {
    Con_Error("Only version %d pak files supported, not %d\n",
              ORCA_PACK_VERSION,
              header.version);
    fclose(packhandle);
    return NULL;
  }
  
  PPACK pack = ZeroAlloc(sizeof(struct _PACK));
  strcpy(pack->filename, szPackfile);
  
  pack->numfiles = header.dirlen / sizeof(struct _PACKFILE);
  pack->files = ZeroAlloc(pack->numfiles * sizeof(struct _PACKFILE));
  pack->handle = packhandle;
  
  fseek(packhandle, header.dirofs, SEEK_SET);
  fread(pack->files, 1, header.dirlen, packhandle);
  
  return pack;
}

// Binary search function
static PPACKFILE
FindPackFile(uint32_t id, PPACKFILE array, size_t size)
{
  if (size == 0)
    return NULL;
  
  size_t low = 0;
  size_t high = size - 1;
  
  while (low <= high) {
    size_t mid = (low + high) / 2;
    uint32_t midIdentifier = array[mid].identifier;
    
    if (midIdentifier == id) {
      return &array[mid];
    } else if (midIdentifier < id) {
      low = mid + 1;
    } else if (mid > 0) {
      high = mid - 1;
    } else {
      break;
    }
  }
  
  // Element not found
  return NULL;
}

static void
_Read(void* buffer, int len, FILE* f)
{
  uint8_t* buf = (uint8_t*)buffer;
  for (int remaining = len; remaining;) {
    int block = MIN(remaining, MAX_READ);
    int read = (int)fread(buf, 1, block, f);
    remaining -= read;
    buf += read;
  }
}

static struct file *_ReadFromPak(FILE *fp, PPACKFILE pf) {
  struct file *file = 0;
  switch (pf->flags & 0xff) {
    case PACK_LZ4:
#ifndef ORCA_NO_LZ4
      FS_DecompressLZ4(fp, pf, &file);
      return file;
#else
      Con_Error("LZ4 is not supported");
      return NULL;
#endif
    case PACK_ZIP:
#ifndef ORCA_NO_ZIP
      FS_DecompressZlib(fp, pf, &file);
      return file;
#else
      Con_Error("ZIP is not supported");
      return NULL;
#endif
    case PACK_FLAT:
      // add 1 to size to be able to add '/0' to the end
      file = ZeroAlloc(sizeof(struct file) + pf->filelen + 1);
      file->size = pf->filelen;
      _Read(file->data, pf->filelen, fp);
      return file;
      
    default:
      return NULL;
  }
}

FILE*
_OpenPakFile(lpcString_t filename, PPACK pack, PPACKFILE desc)
{
  PPACKFILE pf;
  uint32_t identifier = fnv1a32(filename);
  if ((pf = FindPackFile(identifier, pack->files, pack->numfiles))) {
    fseek(pack->handle, pf->filepos, SEEK_SET);
    *desc = *pf;
    return pack->handle;
  }
  return NULL;
}

bool_t _FindPackFile(lpcString_t basename, PPACK pak) {
  uint32_t identifier = fnv1a32(basename);
  for (int i = 0; i < pak->numfiles; i++) {
    if (pak->files[i].identifier == identifier) {
      return TRUE;
    }
  }
  return FALSE;
}

struct file* _ReadPakFile(lpcString_t filename, PPACK pack) {
  FILE* fp;
  struct file* file = NULL;
  struct _PACKFILE pf = { 0 };
  if ((fp = _OpenPakFile(filename, pack, &pf))) {
    if ((file = _ReadFromPak(fp, &pf))) {
      return file;
    } else {
      Con_Error("Unknown compression in file %s", filename);
      return NULL;
    }
  }
  return NULL;
}

void _FreePack(PPACK pack) {
  fclose(pack->handle);
  free(pack->files);
  free(pack);
}

/*
 * pz2 PackageLoader plugin
 *
 * Exposes the pz2 format as a ClassDesc with SuperClassID == SCLASS_FILESYSTEM
 * so that the filesystem module discovers it through the plugin registry.
 */

static void*
Pz2_LoadPackage(lpcString_t path)
{
  path_t pakfile = {0};
  snprintf(pakfile, sizeof(pakfile), "%s.pz2", path);
  return _LoadPackFile(pakfile);
}

static void
Pz2_FreePackage(void* pack)
{
  _FreePack((PPACK)pack);
}

static struct file*
Pz2_ReadFile(lpcString_t filename, void* pack)
{
  return _ReadPakFile(filename, (PPACK)pack);
}

static bool_t
Pz2_FindFile(lpcString_t filename, void* pack)
{
  return _FindPackFile(filename, (PPACK)pack);
}

static PackageLoaderDesc_t _Pz2LoaderDesc = {
  .LoadPackage = Pz2_LoadPackage,
  .FreePackage = Pz2_FreePackage,
  .ReadFile    = Pz2_ReadFile,
  .FindFile    = Pz2_FindFile,
};

/* ClassID == fnv1a32("Pz2PackageLoader") */
#define ID_Pz2PackageLoader 0x97f7e9da

ORCA_API struct ClassDesc _Pz2PackageLoader = {
  .ClassName   = "Pz2PackageLoader",
  .DefaultName = "Pz2PackageLoader",
  .ClassID     = ID_Pz2PackageLoader,
  .SuperClassID = SCLASS_FILESYSTEM,
  .ClassData   = &_Pz2LoaderDesc,
};

