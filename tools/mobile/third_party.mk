# Bundled font and image dependencies for the native iOS and Android builds.
# Compile individual translation units: these libraries cannot be unity-built.
VENDOR_ROOT := $(ROOT)/third-party
VENDOR_CONFIG := $(VENDOR_ROOT)/include
VENDOR_INCLUDES := -I$(VENDOR_CONFIG) -Ilibs/freetype/include -Ilibs/libpng -Ilibs/libjpeg -Ilibs/zlib -Ilibs/monocypher/src
VENDOR_FREETYPE := base/ftbase base/ftinit base/ftsystem base/ftglyph base/ftbitmap base/ftdebug base/ftmm gzip/ftgzip autofit/autofit truetype/truetype cff/cff psaux/psaux psnames/psnames pshinter/pshinter sfnt/sfnt smooth/smooth raster/raster
VENDOR_PNG := png pngerror pngget pngmem pngpread pngread pngrio pngrtran pngrutil pngset pngtrans pngwio pngwrite pngwtran pngwutil
VENDOR_JPEG := jaricom jcomapi jutils jerror jmemmgr jmemnobs jcapimin jcapistd jcarith jctrans jcparam jdatadst jcinit jcmaster jcmarker jcmainct jcprepct jccoefct jccolor jcsample jchuff jcdctmgr jfdctfst jfdctflt jfdctint jdapimin jdapistd jdarith jdtrans jdatasrc jdmaster jdinput jdmarker jdhuff jdmainct jdcoefct jdpostct jddctmgr jidctfst jidctflt jidctint jdsample jdcolor jquant1 jquant2 jdmerge
VENDOR_ZLIB := adler32 compress crc32 deflate gzclose gzlib gzread gzwrite infback inffast inflate inftrees trees uncompr zutil
VENDOR_OBJECTS := $(addprefix $(VENDOR_ROOT)/freetype/,$(addsuffix .o,$(VENDOR_FREETYPE))) \
	$(addprefix $(VENDOR_ROOT)/png/,$(addsuffix .o,$(VENDOR_PNG))) \
	$(addprefix $(VENDOR_ROOT)/jpeg/,$(addsuffix .o,$(VENDOR_JPEG))) \
	$(addprefix $(VENDOR_ROOT)/zlib/,$(addsuffix .o,$(VENDOR_ZLIB))) \
	$(VENDOR_ROOT)/monocypher/monocypher.o $(VENDOR_ROOT)/monocypher/monocypher-ed25519.o
VENDOR_HEADERS := $(VENDOR_CONFIG)/pnglibconf.h $(VENDOR_CONFIG)/jconfig.h

$(VENDOR_CONFIG)/pnglibconf.h: libs/libpng/scripts/pnglibconf.h.prebuilt
	@mkdir -p "$(@D)"
	cp "$<" "$@"
$(VENDOR_CONFIG)/jconfig.h: libs/libjpeg/jconfig.txt
	@mkdir -p "$(@D)"
	cp "$<" "$@"
$(VENDOR_ROOT)/freetype/%.o: libs/freetype/src/%.c source/renderer/third_party/freetype_modules.h $(ROOT)/settings
	@mkdir -p "$(@D)"
	$(CC) $(VENDOR_FLAGS) $(VENDOR_INCLUDES) -I. -DFT2_BUILD_LIBRARY -DFT_CONFIG_MODULES_H='"source/renderer/third_party/freetype_modules.h"' -c "$<" -o "$@"
$(VENDOR_ROOT)/png/%.o: libs/libpng/%.c $(VENDOR_HEADERS) $(ROOT)/settings
	@mkdir -p "$(@D)"
	$(CC) $(VENDOR_FLAGS) $(VENDOR_INCLUDES) -DPNG_ARM_NEON_OPT=0 -c "$<" -o "$@"
$(VENDOR_ROOT)/jpeg/%.o: libs/libjpeg/%.c $(VENDOR_HEADERS) $(ROOT)/settings
	@mkdir -p "$(@D)"
	$(CC) $(VENDOR_FLAGS) $(VENDOR_INCLUDES) -c "$<" -o "$@"
$(VENDOR_ROOT)/zlib/%.o: libs/zlib/%.c $(ROOT)/settings
	@mkdir -p "$(@D)"
	$(CC) $(VENDOR_FLAGS) $(VENDOR_INCLUDES) -DHAVE_UNISTD_H=1 -c "$<" -o "$@"
$(VENDOR_ROOT)/monocypher/%.o: libs/monocypher/src/%.c $(ROOT)/settings
	@mkdir -p "$(@D)"
	$(CC) $(VENDOR_FLAGS) $(VENDOR_INCLUDES) -c "$<" -o "$@"
$(VENDOR_ROOT)/monocypher/%.o: libs/monocypher/src/optional/%.c $(ROOT)/settings
	@mkdir -p "$(@D)"
	$(CC) $(VENDOR_FLAGS) $(VENDOR_INCLUDES) -c "$<" -o "$@"

.PHONY: vendor-libraries
vendor-libraries: $(VENDOR_OBJECTS)
