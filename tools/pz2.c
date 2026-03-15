#include <dirent.h>
#include <errno.h>
#include <include/orca.h>
#include <libpng16/png.h>
#include <sys/stat.h>

#include <include/api.h>
#include <source/filesystem/pakfile.c>

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

#define MAX_READ 0x10000
#define MAX_FILES_IN_PACK 100000

#define INCLUDE_ROOT_FOLDER 1

struct dpackfile
{
  char filename[MAX_RESOURCEPATH];
  char fullpath[MAX_RESOURCEPATH];
  uint32_t identifier;
};

static int read_big_endian_int(FILE* fp)
{
  unsigned char buffer[4];
  fread(buffer, 4, 1, fp);
  return (int)buffer[3] | (int)buffer[2] << 8 | (int)buffer[1] << 16 |
         (int)buffer[0] << 24;
}

struct WI_Size
read_image_size(FILE* fp)
{
  unsigned char PNG[] = "\x89PNG\r\n\x1A\n";
  unsigned char IHDR[] = "IHDR";
  unsigned char header[20];
  struct WI_Size size = { 0, 0 };
  fread(header, 8, 1, fp);
  if (memcmp(header, PNG, 8))
    return size;
  fread(header, 4, 1, fp);
  fread(header, 4, 1, fp);
  if (memcmp(header, IHDR, 4))
    return size;
  size.width = read_big_endian_int(fp);
  size.height = read_big_endian_int(fp);
  return size;
}

static struct dpackfile pakfiles[MAX_FILES_IN_PACK];
static int num_files = 0;

int arg_dxtc = 0;
int arg_luac = 0;
int arg_lz4 = 0;
int arg_zip = 0;

// extern void EncodeCompressedTexture(lpcString_t  input, FILE *file);

int FS_FileLength(FILE* f)
{
  size_t pos, end;
  pos = ftell(f);
  fseek(f, 0, SEEK_END);
  end = ftell(f);
  fseek(f, pos, SEEK_SET);
  return (int)end;
}

void
FS_CollectPack(lpcString_t directory,
               lpcString_t path,
               int argc,
               lpcString_t argv[])
{
  char ospath[MAX_OSPATH];
  char pakpath[MAX_RESOURCEPATH];
  DIR* dir = opendir(directory);
  if (!dir) {
    Con_Error("No such directory %s", directory);
    return;
  }
  for (struct dirent* entry = readdir(dir); entry; entry = readdir(dir)) {
    snprintf(ospath, sizeof(ospath), "%s/%s", directory, entry->d_name);
    if (path) {
      snprintf(pakpath, sizeof(pakpath), "%s/%s", path, entry->d_name);
    } else {
      strcpy(pakpath, entry->d_name);
    }
    if (entry->d_name[0] == '.') {
      continue;
    } else if (strstr(entry->d_name, ".pz2")) {
    } else if (entry->d_type == DT_REG) {
      struct dpackfile* f = &pakfiles[num_files++];
      strcpy(f->fullpath, ospath);
      strcpy(f->filename, entry->d_name);
      f->identifier = fnv1a32(pakpath);
    } else if (entry->d_type == DT_DIR) {
      for (int i = 0; i < argc; i++) {
        if (!strcmp(ospath, argv[i])) {
          goto skip_folder;
        }
      }
      FS_CollectPack(ospath, pakpath, argc, argv);
    }
  skip_folder:;
  }
  closedir(dir);
}

int FS_AppendFile(FILE* packhandle, lpcString_t filename)
{
  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    return 0;
  }
  char buffer[MAX_READ];
  int nread;
  while ((nread = (int)fread(buffer, 1, sizeof(buffer), fp)) > 0) {
    fwrite(buffer, nread, 1, packhandle);
  }
  fclose(fp);
  return 1;
}

void
FS_CompressFileZIP(FILE* outputfile, lpcString_t filename)
{
  // Open the input file
  FILE* input = fopen(filename, "rb");
  if (!input) {
    Con_Error("Error opening input file");
    return;
  }

  // Initialize zlib structures
  z_stream stream;
  memset(&stream, 0, sizeof(stream));

  if (deflateInit(&stream, Z_BEST_COMPRESSION) != Z_OK) {
    Con_Error("Error initializing zlib");
    fclose(input);
    return;
  }

  // Allocate memory for the input data
  char in_buffer[CHUNK_SIZE];
  stream.avail_in = (uInt)fread(in_buffer, 1, sizeof(in_buffer), input);
  stream.next_in = (void*)in_buffer;

  // Allocate memory for the compressed data
  char out_buffer[CHUNK_SIZE];

  // Loop to compress and write to the output file
  int ret;
  do {
    stream.avail_out = sizeof(out_buffer);
    stream.next_out = (void*)out_buffer;

    ret = deflate(&stream, stream.avail_in == 0 ? Z_FINISH : Z_NO_FLUSH);

    if (ret == Z_STREAM_ERROR) {
      Con_Error("Error compressing data: Z_STREAM_ERROR");
      break;
    }

    size_t compressed_size = sizeof(out_buffer) - stream.avail_out;
    if (fwrite(out_buffer, 1, compressed_size, outputfile) != compressed_size) {
      Con_Error("Error writing to output file");
      break;
    }

    if (ret == Z_BUF_ERROR) {
      // Output buffer is full, continue compressing
      continue;
    }

    // More data to compress or compression is finished
    stream.avail_in = (uInt)fread(in_buffer, 1, sizeof(in_buffer), input);
    stream.next_in = (void*)in_buffer;

  } while (ret != Z_STREAM_END);

  // Finish compression
  if (deflateEnd(&stream) != Z_OK) {
    Con_Error("Error finishing compression");
  }

  fclose(input);
}

void
FS_CompressFileLZ4(FILE* outputfile, lpcString_t filename)
{
  // Open the input file
  FILE* input = fopen(filename, "rb");
  if (!input) {
    Con_Error("Error opening input file");
    return;
  }

  // Determine the size of the input file
  fseek(input, 0, SEEK_END);
  int input_size = (int)ftell(input);
  fseek(input, 0, SEEK_SET);

  // Allocate memory for the input data
  LPSTR input_data = (LPSTR)malloc(input_size);
  if (!input_data) {
    Con_Error("Error allocating memory");
    fclose(input);
    return;
  }

  // Read the input data
  if (fread(input_data, 1, input_size, input) != input_size) {
    Con_Error("Error reading input file");
    free(input_data);
    fclose(input);
    return;
  }

  fclose(input);

  // Determine the maximum compressed size
  int max_compressed_size = LZ4_compressBound(input_size);

  // Allocate memory for the compressed data
  LPSTR compressed_data = (LPSTR)malloc(max_compressed_size);
  if (!compressed_data) {
    Con_Error("Error allocating memory");
    free(input_data);
    return;
  }

  // Compress the data
  int compressed_size = LZ4_compress_default(
    input_data, compressed_data, input_size, max_compressed_size);
  if (compressed_size <= 0) {
    Con_Error("Error compressing data");
    free(input_data);
    free(compressed_data);
    return;
  }

  // Write the compressed data to the output file
  if (fwrite(compressed_data, 1, compressed_size, outputfile) !=
      compressed_size) {
    Con_Error("Error writing to output file");
    free(input_data);
    free(compressed_data);
    return;
  }

  // Cleanup
  free(input_data);
  free(compressed_data);

  // frintf(stdout, "Compression complete. Compressed size: %d bytes\n",
  // compressed_size);
}

// Comparison function for qsort
int compareByIdentifier(const void* a, const void* b)
{
  uint32_t idA = ((PPACKFILE)a)->identifier;
  uint32_t idB = ((PPACKFILE)b)->identifier;
  if (idA < idB)
    return -1;
  else if (idA > idB)
    return 1;
  else
    return 0; // Elements are equal
}

int luac_Writer(lua_State* L, const void* p, size_t sz, void* ud)
{
  fwrite(p, sz, 1, (FILE*)ud);
  return 0;
}

void
FS_WritePackFile(FILE* packhandle)
{
  struct _PACK pack;
  struct dpackheader header;

  pack.files = ZeroAlloc(num_files * sizeof(struct _PACKFILE));
  pack.numfiles = num_files;

  header.identifier = IDPAKHEADER;
  header.version = ORCA_PACK_VERSION;
  header.dirlen = pack.numfiles * sizeof(struct _PACKFILE);
  header.dirofs = sizeof(struct dpackheader);

  fwrite(&header, sizeof(struct dpackheader), 1, packhandle);
  fwrite(pack.files, sizeof(struct _PACKFILE), pack.numfiles, packhandle);

  for (int i = 0; i < pack.numfiles; i++) {
    char buffer[MAX_READ];
    int nread;

    struct _PACKFILE* pf = &pack.files[i];
    struct dpackfile* dpf = &pakfiles[i];

    pf->filepos = (int)ftell(packhandle);
    pf->identifier = dpf->identifier;

    bool_t bRemoveFile = FALSE;
    lpcString_t szFileName = dpf->fullpath;

    if (arg_luac && strstr(dpf->fullpath, ".lua") &&
        !strstr(dpf->fullpath, ".luac")) {
      char out[1024];
      char path[PATH_MAX];
      snprintf(out, sizeof(out), "%sc", dpf->fullpath);
      lua_State* L = luaL_newstate();
      if (luaL_loadfile(L, dpf->fullpath) == LUA_OK) {
        FILE* outputFile = fopen(out, "wb");
        size_t size;
        lua_dump(L, luac_Writer, outputFile, 0);
        fclose(outputFile);
        bRemoveFile = TRUE;
        szFileName = out;
      } else {
        Con_Error("Error loading Lua file: %s", lua_tostring(L, -1));
      }
      lua_close(L);
#if 0
		} else if (arg_dxtc && strstr(dpf->fullpath, ".png")) {
			FILE       *fp = fopen(dpf->fullpath, "rb");
			png_structp png_ptr =
			png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			png_infop info_ptr = png_create_info_struct(png_ptr);
			setjmp(png_jmpbuf(png_ptr));
			png_init_io(png_ptr, fp);
			png_set_sig_bytes(png_ptr, 0);
			png_read_info(png_ptr, info_ptr);
			
			int const width  = png_get_image_width(png_ptr, info_ptr);
			int const height = png_get_image_height(png_ptr, info_ptr);
			
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			
			if (width % 4 == 0 && height % 4 == 0) {
				EncodeCompressedTexture(dpf->fullpath, packhandle);
			} else {
				frintf(stdout, "Can't compress %s (size %dx%d)\n",
							 dpf->fullpath, width, height);
				fseek(fp, 0, SEEK_SET);
				while ((nread = (int)fread(buffer, 1, sizeof(buffer), fp)) >
							 0) {
					fwrite(buffer, nread, 1, packhandle);
				}
			}
			fclose(fp);
#endif
    }
    if (arg_lz4) {
      FS_CompressFileLZ4(packhandle, szFileName);
      pf->flags = PACK_LZ4;
    } else if (arg_zip) {
      FS_CompressFileZIP(packhandle, szFileName);
      pf->flags = PACK_ZIP;
    } else {
      FS_AppendFile(packhandle, szFileName);
      pf->flags = PACK_FLAT;
    }
    FILE* fp = fopen(szFileName, "rb");
    if (strstr(szFileName, ".png")) {
      pf->imagesize = read_image_size(fp);
    }
    pf->decompsize = FS_FileLength(fp);
    pf->filelen = (int)ftell(packhandle) - pf->filepos;
    fclose(fp);
    if (bRemoveFile) {
      remove(szFileName);
    }
    Con_Error("[ %02d%% ]: %s (compressed: %d%%)\n",
            100 * i / pack.numfiles,
            dpf->filename,
            100 * pf->filelen / pf->decompsize);
  }

  qsort(
    pack.files, pack.numfiles, sizeof(struct _PACKFILE), compareByIdentifier);
  fseek(packhandle, 0, SEEK_SET);
  fwrite(&header, sizeof(struct dpackheader), 1, packhandle);
  fwrite(pack.files, sizeof(struct _PACKFILE), pack.numfiles, packhandle);

  free(pack.files);
}

void
usage(void)
{
  Con_Error("Usage: pz2 [options] <folders...>");
  Con_Error("\nOptions:");
  Con_Error("\t-?       Show this help message");
  Con_Error("\t-lz4     Enable LZ4 compression");
  Con_Error("\t-gz      Enable Gzip compression");
  Con_Error("\t-luac    Compile Lua scripts");
  Con_Error("\t-dxtc    Convert PNG files to DXT1/DXT5 format");
  Con_Error("\nArguments:");
  Con_Error("\t<folders...>  One or more directories to be packed");
}

int main(int argc, lpcString_t argv[])
{
  FOR_LOOP(i, argc - 1)
  {
    lpcString_t arg = argv[i + 1];
    if (!strcmp(arg, "-dxtc")) {
      arg_dxtc = 1;
    } else if (!strcmp(arg, "-luac")) {
      arg_luac = 1;
    } else if (!strcmp(arg, "-lz4")) {
      arg_lz4 = 1;
    } else if (!strcmp(arg, "-gz")) {
      arg_zip = 1;
    } else if (!strcmp(arg, "-?")) {
      usage();
      return 0;
    } else if (*arg != '-') {
      memset(pakfiles, 0, sizeof(pakfiles));
      num_files = 0;
      FS_CollectPack(arg, NULL, argc, argv);
      char output[1024] = { 0 };
      snprintf(output, sizeof(output), "%s.pz2", arg);
      FILE* fp = fopen(output, "wb");
      FS_WritePackFile(fp);
      fclose(fp);
    } else {
      Con_Error("Unknown option %s", arg);
    }
  }
  //	if (output == NULL) {
  //		usage();
  //		return 1;
  //	}
  //
  //	FILE *fp = fopen(output, "wb");
  //	if (!fp) {
  //		Con_Error("Can't open output file");
  //		return 1;
  //	}
  //	FS_WritePackFile(fp);
  //	fclose(fp);
  return 0;
}
