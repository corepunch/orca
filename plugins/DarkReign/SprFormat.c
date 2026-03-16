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
    anim->Palette   = R_GetSpritePalette();

    // fprintf(stderr, "SPR '%s': loaded %d frames into %ux%u atlas\n",
    //         name, total_frames, atlas_w, atlas_h);

    return obj;

fail:
    free(atlas);
    free(sects);
    return NULL;
}

