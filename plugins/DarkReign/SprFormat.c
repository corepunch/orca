/*
 * SPR sprite file format (Dark Reign, palette-indexed RLE, 8 bits per pixel)
 *
 * All multi-byte integers are little-endian.
 *
 * Header (32 bytes):
 *   char	 magic[4];	// "RSPR" = normal sprite, "SSPR" = shadow sprite
 *   int32_t  version;	 // must be 0x0210
 *   int32_t  nanims;	  // number of animation frames per rotation
 *   int32_t  nrots;	   // number of rotations
 *   int32_t  szx;		 // frame width  (all frames share this size)
 *   int32_t  szy;		 // frame height (all frames share this size)
 *   int32_t  npics;	   // total number of compressed pictures
 *   int32_t  nsects;	  // number of animation sections
 *
 * Computed offsets (nothing is stored; derive from header):
 *   off_sections = 32 + 4 * nanims * nrots
 *   off_anims	= off_sections + 16 * nsects
 *   off_picoffs  = off_anims   + 4 * nanims
 *   off_bits	 = off_picoffs + 8 * npics + 4
 *
 * Anim-index table (4 bytes × nanims × nrots, immediately after header):
 *   int32_t  picnr[nanims * nrots]
 *   Index: picnr[anim * nrots + rot] → which compressed picture to use.
 *
 * Section table (16 bytes × nsects, at off_sections):
 *   int32_t  firstanim;   // first animation index in this section
 *   int32_t  lastanim;	// last  animation index in this section (inclusive)
 *   int32_t  framerate;   // playback rate (unused here, kept for completeness)
 *   int32_t  numhotspots; // number of hotspot channels in this section
 *
 * Animation table (4 bytes × nanims, at off_anims):
 *   int32_t  anim_data[nanims]  // (unused in this loader)
 *
 * Picture-offset table (8 bytes × npics + 4 sentinel, at off_picoffs):
 *   int32_t  picoff[npics + 1]  // byte offsets into the pixel-data block;
 *								// element npics is a sentinel so that
 *								// size = picoff[i+1] - picoff[i]
 *   NOTE: the table is actually (8 * npics + 4) bytes; entries are 4 bytes
 *   apart (not 8) when walking by index.  The +4 after 8*npics accounts for
 *   the sentinel entry at the end.  Each picture descriptor is two int32_t
 *   values:
 *	 [0] picoff   – compressed data offset from off_bits
 *	 [1] hotoff   – hotspot table offset (ignored unless numhotspots > 0)
 *
 * Compressed pixel data (at off_bits):
 *   Each picture is independently RLE-compressed, one scanline at a time.
 *   For scanline l of picture p:
 *	 - Read bytes sequentially; maintain step counter (starts at 0 each line).
 *	 - Read cnt = next byte.
 *	 - If step is odd: cnt &= 0x7F  (clear the high bit).
 *	 - If step is even (0, 2, 4, …): transparent run of cnt pixels – skip.
 *	 - If step is odd  (1, 3, 5, …): opaque run of cnt pixels.
 *		 Normal  sprite: copy cnt literal bytes from the stream.
 *		 Shadow  sprite: fill cnt pixels with palette index 47.
 *	 - Advance currx += cnt; step++.
 *	 - Repeat until currx == szx.
 *
 * Rotation mapping:
 *   Sprites face right at rotation 0 on disk.  To make them face the
 *   conventional "up" direction, rotations are shifted by nrots/4:
 *	 display_rot r  →  disk_rot = (r + nrots/4) % nrots   (only when nrots >= 4)
 *
 * Texture atlas strategy (horizontal strip):
 *   Sections are laid out left-to-right in rotation-major, anim-minor order:
 *	 for each section:
 *	   for r in [0, nrots):
 *		 for a in [firstanim, lastanim]:
 *		   place frame at x_cursor; x_cursor += szx
 *   atlas_width  = total number of frames × szx
 *   atlas_height = szy
 *   All frames are the same size (szx × szy), so UvRects are uniform.
 */

#include <include/orca.h>
#include <include/api.h>
#include <include/renderer.h>
#include <source/core/core.h>
#include <plugins/SpriteKit/SpriteKit.h>
#include "DarkReign.h"

#define SPR_DEFAULT_FRAMERATE 10.0f

/* Simple Dark Reign palette (256 entries, 0x00RRGGBB).
 * Index 0 is always transparent. The remaining entries cover the principal
 * hues used in the Dark Reign campaign art so that sprites render with
 * recognisable colours while a proper per-level palette is not yet wired up. */
static uint32_t const spr_palette[256] = {
  /* 0 – transparent (only alpha=0 matters; RGB values are ignored) */ 0x00000000,
  /* 1-15: greens (terrain / foliage) */
  0x003A5F2A, 0x00496E35, 0x00587D40, 0x00678C4B, 0x00769B56,
  0x0085AA61, 0x0094B96C, 0x00A3C877, 0x00B2D782, 0x00C1E68D,
  0x00D0F598, 0x00C8EE90, 0x00BFE788, 0x00B6E080, 0x00ADD978,
  /* 16-31: browns / earth */
  0x006B4226, 0x007A5130, 0x0089603A, 0x00986F44, 0x00A77E4E,
  0x00B68D58, 0x00C59C62, 0x00D4AB6C, 0x00E3BA76, 0x00F2C980,
  0x00D4A96A, 0x00BB9060, 0x00A27756, 0x008A5F4D, 0x00724844,
  0x005A313B,
  /* 32-47: greys / concrete */
  0x00181818, 0x00222222, 0x002C2C2C, 0x00363636, 0x00404040,
  0x004A4A4A, 0x00545454, 0x005E5E5E, 0x00686868, 0x00727272,
  0x007C7C7C, 0x00868686, 0x00909090, 0x009A9A9A, 0x00A4A4A4,
  0x00AEAEAE,
  /* 48-63: blues (water / sky) */
  0x00102040, 0x00182840, 0x00203050, 0x00283860, 0x00304070,
  0x00384880, 0x00405090, 0x004858A0, 0x005060B0, 0x005868C0,
  0x006070D0, 0x006878E0, 0x0070A0E0, 0x0078B0E8, 0x0080C0F0,
  0x0088D0F8,
  /* 64-79: reds / rust (TCA military) */
  0x00500000, 0x00600000, 0x00700010, 0x00800020, 0x00901030,
  0x00A02040, 0x00B03050, 0x00C04060, 0x00D05070, 0x00E06080,
  0x00F07090, 0x00E86888, 0x00E06080, 0x00D85878, 0x00D05070,
  0x00C84868,
  /* 80-95: yellows / gold (Imperium) */
  0x00705000, 0x00806000, 0x00907010, 0x00A08020, 0x00B09030,
  0x00C0A040, 0x00D0B050, 0x00E0C060, 0x00F0D070, 0x00FFE080,
  0x00FFD060, 0x00FFC040, 0x00FFB020, 0x00FFA000, 0x00E09000,
  0x00C08000,
  /* 96-127: mixed mid-tones */
  0x00204000, 0x00284800, 0x00305000, 0x00385800, 0x00406000,
  0x00486800, 0x00507000, 0x00587800, 0x00608000, 0x00688800,
  0x00709000, 0x00789800, 0x0080A000, 0x0088A800, 0x0090B000,
  0x0098B800, 0x0038200A, 0x00482814, 0x0058301E, 0x00683828,
  0x00784032, 0x0088483C, 0x00985046, 0x00A85850, 0x00B8605A,
  0x00C86864, 0x00D8706E, 0x00A06040, 0x00906050, 0x00806060,
  0x00706070, 0x00606080,
  /* 128-191: 64 ramp entries (general shading) */
  0x00080808, 0x00101010, 0x00181818, 0x00202020,
  0x00282828, 0x00303030, 0x00383838, 0x00404040,
  0x00484848, 0x00505050, 0x00585858, 0x00606060,
  0x00686868, 0x00707070, 0x00787878, 0x00808080,
  0x00888888, 0x00909090, 0x00989898, 0x00A0A0A0,
  0x00A8A8A8, 0x00B0B0B0, 0x00B8B8B8, 0x00C0C0C0,
  0x00C8C8C8, 0x00D0D0D0, 0x00D8D8D8, 0x00E0E0E0,
  0x00E8E8E8, 0x00F0F0F0, 0x00F8F8F8, 0x00FFFFFF,
  0x00102030, 0x00203040, 0x00304050, 0x00405060,
  0x00506070, 0x00607080, 0x00708090, 0x008090A0,
  0x0090A0B0, 0x00A0B0C0, 0x00B0C0D0, 0x00C0D0E0,
  0x00D0E0F0, 0x00001020, 0x00002030, 0x00003040,
  0x00300010, 0x00400020, 0x00500030, 0x00600040,
  0x00700050, 0x00800060, 0x00900070, 0x00A00080,
  0x00203000, 0x00304000, 0x00405000, 0x00506000,
  0x00607000, 0x00708000, 0x00809000, 0x0090A000,
  /* 192-255: fill with a generic palette ramp */
  0x00C00000, 0x00C80800, 0x00D01000, 0x00D81800,
  0x00E02000, 0x00E82800, 0x00F03000, 0x00F83800,
  0x0000C000, 0x0008C800, 0x0010D000, 0x0018D800,
  0x0020E000, 0x0028E800, 0x0030F000, 0x0038F800,
  0x000000C0, 0x000008C8, 0x000010D0, 0x000018D8,
  0x000020E0, 0x000028E8, 0x000030F0, 0x000038F8,
  0x00C0C000, 0x00C8C800, 0x00D0D000, 0x00D8D800,
  0x00E0E000, 0x00E8E800, 0x00F0F000, 0x00F8F800,
  0x00C000C0, 0x00C800C8, 0x00D000D0, 0x00D800D8,
  0x00E000E0, 0x00E800E8, 0x00F000F0, 0x00F800F8,
  0x0000C0C0, 0x0000C8C8, 0x0000D0D0, 0x0000D8D8,
  0x0000E0E0, 0x0000E8E8, 0x0000F0F0, 0x0000F8F8,
  0x00804000, 0x00884800, 0x00905000, 0x00985800,
  0x00A06000, 0x00A86800, 0x00B07000, 0x00B87800,
  0x00408000, 0x00488800, 0x00509000, 0x00589800,
  0x0060A000, 0x0068A800, 0x0070B000, 0x0078B800,
};


/* ---- on-disk structs (packed to prevent padding) ---- */

#pragma pack(push, 1)
struct _spr_header {
	char	magic[4];   /* "RSPR" or "SSPR"  */
	int32_t version;	/* must be 0x0210	*/
	int32_t nanims;
	int32_t nrots;
	int32_t szx;
	int32_t szy;
	int32_t npics;
	int32_t nsects;
};
#pragma pack(pop)

#define SPR_HEADER_SIZE   32	/* sizeof _spr_header – explicit for offset math */
#define SPR_SECTION_SIZE  16	/* 4 × int32_t per section entry				 */
#define SPR_SHADOW_INDEX  47	/* palette index used to fill shadow pixels	   */

/* A parsed section from the section table. */
struct _spr_section {
	int32_t firstanim;
	int32_t lastanim;
	int32_t framerate;
	int32_t numhotspots;
};

/* ------------------------------------------------------------------ */
/*  Helpers to read little-endian int32 from a byte buffer at offset  */
/* ------------------------------------------------------------------ */
static inline int32_t
_read_i32(uint8_t const *buf, uint32_t off)
{
	/* unaligned-safe, endian-explicit */
	return (int32_t)( (uint32_t)buf[off + 0]
					| (uint32_t)buf[off + 1] << 8
					| (uint32_t)buf[off + 2] << 16
					| (uint32_t)buf[off + 3] << 24 );
}

/* ------------------------------------------------------------------ */
/*  _SprFile_Load													   */
/*																	  */
/*  Parse a Dark Reign .spr file, decompress all frames into a		 */
/*  horizontal-strip texture atlas, and return a SpriteAnimation	   */
/*  Object.  Returns NULL on any error.								 */
/* ------------------------------------------------------------------ */

lpObject_t
_SprFile_Load(lua_State* L, uint8_t const *data, uint32_t size, lpcString_t name)
{
    /* ---- 1. validate header ---- */
    if (size < (uint32_t)SPR_HEADER_SIZE) {
        fprintf(stderr, "SPR '%s': too small for header (%u bytes)\n", name, size);
        return NULL;
    }

    struct _spr_header const *hdr = (struct _spr_header const *)data;

    if (memcmp(hdr->magic, "RSPR", 4) != 0 &&
        memcmp(hdr->magic, "SSPR", 4) != 0 &&
        memcmp(hdr->magic, "LSPR", 4) != 0) {
        fprintf(stderr, "SPR '%s': bad magic '%.4s'\n", name, hdr->magic);
        return NULL;
    }
    bool const is_shadow = (memcmp(hdr->magic, "SSPR", 4) == 0);

    if (hdr->version != 0x0210 && hdr->version != 0x0200) {
        fprintf(stderr, "SPR '%s': unsupported version 0x%04X\n", name, hdr->version);
        return NULL;
    }

    int32_t const nanims = hdr->nanims;
    int32_t const nrots  = hdr->nrots;
    int32_t const szx    = hdr->szx;
    int32_t const szy    = hdr->szy;
    int32_t const npics  = hdr->npics;
    int32_t const nsects = hdr->nsects;

    if (nanims <= 0 || nrots <= 0 || szx <= 0 || szy <= 0 ||
        npics  <= 0 || nsects <= 0) {
        fprintf(stderr, "SPR '%s': invalid header fields\n", name);
        return NULL;
    }

    /* ---- 2. compute derived offsets ---- */
    int32_t const off_sections = SPR_HEADER_SIZE + 4 * nanims * nrots;
    int32_t const off_anims    = off_sections     + SPR_SECTION_SIZE * nsects;
    int32_t const off_picoffs  = off_anims        + 4 * nanims;
    int32_t const off_bits     = off_picoffs      + 8 * npics + 4;

    if ((uint32_t)off_bits > size) {
        fprintf(stderr, "SPR '%s': file too small for declared structure\n", name);
        return NULL;
    }

    /* ---- 3. parse section table ---- */
    struct _spr_section *sects =
        (struct _spr_section *)malloc((size_t)nsects * sizeof(struct _spr_section));
    if (!sects) {
        fprintf(stderr, "SPR '%s': OOM for section table\n", name);
        return NULL;
    }
    for (int32_t s = 0; s < nsects; s++) {
        uint32_t base = (uint32_t)(off_sections + SPR_SECTION_SIZE * s);
        sects[s].firstanim   = _read_i32(data, base +  0);
        sects[s].lastanim    = _read_i32(data, base +  4);
        sects[s].framerate   = _read_i32(data, base +  8);
        sects[s].numhotspots = _read_i32(data, base + 12);
    }

    /* ---- 4. count total frames ---- */
    int32_t total_frames = 0;
    for (int32_t s = 0; s < nsects; s++)
        total_frames += nrots * (sects[s].lastanim - sects[s].firstanim + 1);

    if (total_frames <= 0) {
        fprintf(stderr, "SPR '%s': no frames\n", name);
        free(sects);
        return NULL;
    }

    /* ---- 5. compute near-square atlas dimensions ---- */
    uint32_t const cols    = (uint32_t)ceilf(sqrtf((float)total_frames));
    uint32_t const rows    = ((uint32_t)total_frames + cols - 1) / cols;
    uint32_t const atlas_w = cols * (uint32_t)szx;
    uint32_t const atlas_h = rows * (uint32_t)szy;

    if (atlas_w == 0 || atlas_h == 0) {
        fprintf(stderr, "SPR '%s': zero atlas dimension\n", name);
        free(sects);
        return NULL;
    }

    // {
    //     int max_tex = 0;
    //     glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex);
    //     if (max_tex > 0 && ((int)atlas_w > max_tex || (int)atlas_h > max_tex)) {
    //         fprintf(stderr, "SPR '%s': atlas %ux%u exceeds GL_MAX_TEXTURE_SIZE %d\n",
    //                 name, atlas_w, atlas_h, max_tex);
    //         free(sects);
    //         return NULL;
    //     }
    // }

    /* ---- 6. allocate atlas ---- */
    uint8_t *atlas = (uint8_t *)calloc(1, atlas_w * atlas_h);
    if (!atlas) {
        fprintf(stderr, "SPR '%s': OOM for atlas (%u x %u)\n", name, atlas_w, atlas_h);
        free(sects);
        return NULL;
    }

    /* ---- 7. decompress each frame into the atlas ---- */
    int32_t const rot_offset = (nrots >= 4) ? nrots / 4 : 0;
    int32_t       frame_cursor = 0;

    for (int32_t s = 0; s < nsects; s++) {
        int32_t const fa = sects[s].firstanim;
        int32_t const la = sects[s].lastanim;

        for (int32_t r = 0; r < nrots; r++) {
            int32_t disk_r = r + rot_offset;
            if (disk_r >= nrots) disk_r -= nrots;

            for (int32_t a = fa; a <= la; a++) {
                /* look up compressed picture index */
                int32_t const picindex = a * nrots + disk_r;
                if (picindex < 0 || picindex >= nanims * nrots) {
                    fprintf(stderr, "SPR '%s': picindex %d out of range\n",
                            name, picindex);
                    goto fail;
                }

                uint32_t const idx_off = (uint32_t)(SPR_HEADER_SIZE + picindex * 4);
                int32_t  const picnr   = _read_i32(data, idx_off);

                if (picnr < 0 || picnr >= npics) {
                    fprintf(stderr, "SPR '%s': picnr %d out of range\n", name, picnr);
                    goto fail;
                }

                /* compressed data bounds */
                uint32_t const poff_base = (uint32_t)(off_picoffs + 8 * picnr);
                int32_t  const pic_start = _read_i32(data, poff_base);
                int32_t  const pic_end   = _read_i32(data, poff_base + 8);

                if (pic_start < 0 || pic_end < pic_start ||
                    (uint32_t)(off_bits + pic_end) > size) {
                    fprintf(stderr, "SPR '%s': pic %d data out of bounds\n",
                            name, picnr);
                    goto fail;
                }

                uint8_t const *compressed = data + off_bits + pic_start;
                uint32_t const comp_size  = (uint32_t)(pic_end - pic_start);
                uint32_t       comp_pos   = 0;

                /* frame position in the square atlas */
                uint32_t const col_idx        = (uint32_t)frame_cursor % cols;
                uint32_t const row_idx        = (uint32_t)frame_cursor / cols;
                uint32_t const frame_origin_x = col_idx * (uint32_t)szx;
                uint32_t const frame_origin_y = row_idx * (uint32_t)szy;

                /* decompress scanline by scanline */
                for (int32_t l = 0; l < szy; l++) {
                    int32_t currx = 0;
                    int32_t step  = 0;

                    while (currx < szx) {
                        if (comp_pos >= comp_size) {
                            fprintf(stderr, "SPR '%s': RLE overrun at line %d\n",
                                    name, l);
                            goto fail;
                        }
                        int32_t cnt = (int32_t)compressed[comp_pos++];
                        if (step & 1)
                            cnt &= 0x7F;

                        if (step & 1) {
                            uint8_t *row_dst = atlas
                                + (frame_origin_y + (uint32_t)l) * atlas_w
                                +  frame_origin_x + (uint32_t)currx;
                            if (is_shadow) {
                                memset(row_dst, SPR_SHADOW_INDEX, (size_t)cnt);
                            } else {
                                if (comp_pos + (uint32_t)cnt > comp_size) {
                                    fprintf(stderr, "SPR '%s': RLE pixel overrun\n",
                                            name);
                                    goto fail;
                                }
                                memcpy(row_dst, compressed + comp_pos, (size_t)cnt);
                                comp_pos += (uint32_t)cnt;
                            }
                        }

                        currx += cnt;
                        step++;
                    }

                    if (currx != szx) {
                        fprintf(stderr, "SPR '%s': line %d width mismatch "
                                "(got %d, expected %d)\n", name, l, currx, szx);
                        goto fail;
                    }
                }

                frame_cursor++;
            }
        }
    }
    free(sects);
    sects = NULL;

    /* ---- 8. upload atlas to GL ---- */
    struct Texture *tex = NULL;
    HRESULT hr = Texture_Create(
        &(CREATEIMGSTRUCT){
            .Width     = atlas_w,
            .Height    = atlas_h,
            .Scale     = 1,
            .Format    = kTextureFormatAlpha8,
            .ImageData = atlas,
            .MinFilter = kTextureFilterNearest,
            .MagFilter = kTextureFilterNearest,
            .WrapMode  = kTextureWrapClamp,
        },
        &tex);
    free(atlas);
    atlas = NULL;

    if (FAILED(hr) || !tex) {
        fprintf(stderr, "SPR '%s': failed to create texture\n", name);
        return NULL;
    }

    /* ---- 9. build per-frame SpriteFrame descriptors ---- */
    struct SpriteFrame *frames =
        (struct SpriteFrame *)malloc((size_t)total_frames * sizeof(struct SpriteFrame));
    if (!frames) {
        fprintf(stderr, "SPR '%s': OOM for frame descriptors\n", name);
        Texture_Release(tex);
        return NULL;
    }

    {
        float const fw       = (float)szx;
        float const fh       = (float)szy;
        float const inv_aw   = 1.0f / (float)atlas_w;
        float const inv_ah   = 1.0f / (float)atlas_h;
        float const uv_w     = fw * inv_aw;
        float const uv_h     = fh * inv_ah;

        for (int32_t i = 0; i < total_frames; i++) {
            uint32_t const col_idx = (uint32_t)i % cols;
            uint32_t const row_idx = (uint32_t)i / cols;

            frames[i].Rect.x      = 0.0f;
            frames[i].Rect.y      = 0.0f;
            frames[i].Rect.width  = fw;
            frames[i].Rect.height = fh;

            frames[i].UvRect.x      = (float)(col_idx * (uint32_t)szx) * inv_aw;
            frames[i].UvRect.y      = (float)(row_idx * (uint32_t)szy) * inv_ah;
            frames[i].UvRect.width  = uv_w;
            frames[i].UvRect.height = uv_h;
        }
    }

    /* ---- 10. create SpriteAnimation Object ---- */
		lua_getglobal(L, "require");
		lua_pushstring(L, "orca.SpriteKit");
		if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
			fprintf(stderr, "SPR '%s': failed to require SpriteKit: %s\n",
					name, lua_tostring(L, -1));
			lua_pop(L, 1);
			free(frames);
			Texture_Release(tex);
			return NULL;
		}
		lua_getfield(L, -1, "SpriteAnimation");
		if (!lua_isfunction(L, -1) && !lua_istable(L, -1)) {
			fprintf(stderr, "SPR '%s': SpriteKit.SpriteAnimation not found\n", name);
			lua_pop(L, 2);
			free(frames);
			Texture_Release(tex);
			return NULL;
		}
		if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
			fprintf(stderr, "SPR '%s': failed to call SpriteKit.SpriteAnimation: %s\n",
					name, lua_tostring(L, -1));
			lua_pop(L, 1);
			free(frames);
			Texture_Release(tex);
			return NULL;
		}
    lpObject_t obj = luaX_checkObject(L, -1);
    if (!obj) {
        fprintf(stderr, "SPR '%s': failed to create SpriteAnimation object\n", name);
        free(frames);
        Texture_Release(tex);
        return NULL;
    }
    OBJ_SetName(obj, name);

    struct SpriteAnimation *anim = GetSpriteAnimation(obj);
    anim->Image     = tex;
    anim->Framerate = SPR_DEFAULT_FRAMERATE;
    anim->NumFrames = total_frames;
    anim->Frames    = frames;
    anim->Palette   = spr_palette;

    // fprintf(stderr, "SPR '%s': loaded %d frames into %ux%u atlas\n",
    //         name, total_frames, atlas_w, atlas_h);

    return obj;

fail:
    free(atlas);
    free(sects);
    return NULL;
}

