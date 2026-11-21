// ORCA renderer — Metal backend. See metal.h. Compiled with ARC.
//
// Model:
// - Renderer objects are Objective-C objects in per-kind name tables.
// - A render pass starts lazily at the first draw on the bound framebuffer.
//   mtlClear before that becomes the pass's load action; mtlClear inside an
//   open pass ends it and the next draw starts a cleared pass.
// - Offscreen framebuffers are rendered with the vertex stage's Y flip, so
//   their textures keep bottom-up rows: textures uploaded by the engine and
//   textures it renders to then sample the same way. The default framebuffer
//   is the CAMetalLayer's drawable and is rendered unflipped.
// - Uniform values live in each program's per-stage block and are copied
//   into the command stream with every draw (setVertexBytes).
// - An upload to a texture the GPU may still read is encoded as a blit in
//   the command stream, so draws before and after it see submission order.

#if !__has_feature(objc_arc)
#error metal.m must be compiled with -fobjc-arc
#endif

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <TargetConditionals.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libs/platform/platform.h>
#include "metal.h"
#include "msl_program.h"
#include "r_msl_shaders.h"

#define MAX_TEXTURE_UNITS 16
#define MAX_VERTEX_ATTRIBS MSL_MAX_ATTRIBUTES
#define MAX_SET_BYTES 4096

#if TARGET_OS_IPHONE
#import <UIKit/UIApplication.h>
#endif

// Metal and QuartzCore return autoreleased drawables, command buffers and
// descriptors. macOS paints from the engine's loop outside any pool, so a
// frame without one would keep every drawable and stall nextDrawable.
void *objc_autoreleasePoolPush(void);
void objc_autoreleasePoolPop(void *pool);

#pragma mark - Objects

@interface OrcaTexture : NSObject {
@public
  mtlEnum target;                 // 0 until first bound
  id<MTLTexture> texture;
  id<MTLTexture> swizzled;        // view for a non-identity swizzle
  int width, height, levels;
  mtlEnum internalFormat;
  mtlEnum minFilter, magFilter, wrapS, wrapT, wrapR;
  int maxLevel;
  mtlEnum swizzle[4];
  id<MTLSamplerState> sampler;
  uint64_t lastUse;               // command buffer serial that last read or wrote it
  BOOL renderbuffer;
}
@end
@implementation OrcaTexture
- (instancetype)init
{
  if ((self = [super init])) {
    minFilter = MTL_NEAREST_MIPMAP_LINEAR;
    magFilter = MTL_LINEAR;
    wrapS = wrapT = wrapR = MTL_REPEAT;
    maxLevel = 1000;
    swizzle[0] = MTL_RED, swizzle[1] = MTL_GREEN, swizzle[2] = MTL_BLUE, swizzle[3] = MTL_ALPHA;
  }
  return self;
}
@end

@interface OrcaBuffer : NSObject {
@public
  id<MTLBuffer> buffer;
  NSUInteger size;
  // Metal needs vertex strides in multiples of 4: a copy of the contents with
  // padded elements, for the source stride, offset and element size it was made for.
  id<MTLBuffer> padded;
  NSUInteger paddedStride, paddedOffset, paddedElement;
}
@end
@implementation OrcaBuffer
@end

typedef struct
{
  OrcaBuffer *__unsafe_unretained buffer;  // owned by OrcaVertexArray.retained
  int size;
  mtlEnum type;
  bool normalized;
  int stride;
  uintptr_t offset;
  BOOL enabled;
} attrib_t;

@interface OrcaVertexArray : NSObject {
@public
  attrib_t attribs[MAX_VERTEX_ATTRIBS];
  NSMutableArray<OrcaBuffer *> *retained;
  OrcaBuffer *elements;
}
@end
@implementation OrcaVertexArray
- (instancetype)init
{
  if ((self = [super init])) retained = [NSMutableArray new];
  return self;
}
@end

@interface OrcaFramebuffer : NSObject {
@public
  OrcaTexture *color;
  OrcaTexture *depth;    // depth, or depth and stencil
  OrcaTexture *stencil;
}
@end
@implementation OrcaFramebuffer
@end

@interface OrcaShader : NSObject {
@public
  mtlEnum type;
  NSString *source;
}
@end
@implementation OrcaShader
@end

@interface OrcaProgram : NSObject {
@public
  OrcaShader *vs, *fs;
  BOOL linked;
  NSString *log;
  NSString *name;                          // set by mtlProgramLabel before mtlLinkProgram
  struct msl_program const *refl;          // looked up by name in mtlLinkProgram
  id<MTLFunction> functions[MSL_STAGES];
  uint8_t *data[MSL_STAGES];
  int units[MSL_MAX_UNIFORMS];
  int32_t flipY;                         // uniform index of MSL_FLIP_Y
  uint32_t serial;
  NSMutableDictionary<NSData *, id<MTLRenderPipelineState>> *pipelines;
}
@end
@implementation OrcaProgram
- (void)reset
{
  refl = NULL;
  name = nil;
  for (int s = 0; s < MSL_STAGES; s++) free(data[s]), data[s] = NULL, functions[s] = nil;
  linked = NO;
  [pipelines removeAllObjects];
}
- (void)dealloc { [self reset]; }
@end

#pragma mark - State

typedef struct
{
  uint32_t program;
  uint32_t color, depth, stencil;
  uint8_t blend, src, dst, mask;
  struct { uint16_t format, stride; } attrs[MAX_VERTEX_ATTRIBS];
} pipeline_key_t;

typedef struct
{
  mtlEnum target;
  OrcaTexture *__unsafe_unretained bound2D, *__unsafe_unretained boundCube;
} unit_t;

static struct
{
  id<MTLDevice> device;
  id<MTLCommandQueue> queue;
  id<MTLCommandBuffer> cb;
  id<MTLRenderCommandEncoder> encoder;
  OrcaFramebuffer *encoderFramebuffer;  // nil: the default framebuffer
  BOOL encoderOpen, encoderDefault;
  uint64_t serial;
  _Atomic uint64_t completed;

  CAMetalLayer *layer;
  id<CAMetalDrawable> drawable;
  id<MTLTexture> offscreen;             // default framebuffer without a layer
  id<MTLTexture> defaultDepth;
  BOOL srgb;

  NSMutableDictionary<NSNumber *, id> *objects[7];
  uint32_t nextName;

  OrcaBuffer *arrayBuffer, *unpackBuffer;
  OrcaVertexArray *vertexArray, *defaultVertexArray;
  OrcaFramebuffer *drawFramebuffer, *readFramebuffer;
  OrcaTexture *renderbuffer;
  OrcaProgram *program;
  uint32_t drawName, readName, renderbufferName, programName, vertexArrayName;
  unit_t units[MAX_TEXTURE_UNITS];
  uint32_t activeUnit;
  int unpackAlignment;

  BOOL blend, depthTest, stencilTest, cull;
  mtlEnum blendSrc, blendDst;
  uint8_t colorMask;
  mtlEnum cullFace, depthFunc;
  BOOL depthMask;
  mtlEnum stencilFunc, stencilFail, stencilZFail, stencilZPass;
  int stencilRef;
  uint32_t stencilReadMask, stencilWriteMask;
  int viewport[4];

  float clearColor[4];
  float clearDepth;
  int clearStencil;
  uint32_t pendingClear;                // for drawFramebuffer, before its pass starts
  float pendingColor[4];
  float pendingDepth;
  int pendingStencil;

  mtlEnum error;
  id<MTLRenderPipelineState> lastPipeline;
  NSMutableDictionary<NSData *, id<MTLDepthStencilState>> *depthStates;
  NSMutableDictionary<NSNumber *, id<MTLSamplerState>> *samplers;
  NSMutableDictionary<NSNumber *, id<MTLRenderPipelineState>> *blitPipelines;
  id<MTLLibrary> blitLibrary;
  id<MTLDepthStencilState> depthAlways, depthDisabled;
  id<MTLBuffer> constantAttribute;
  OrcaTexture *blackTexture, *blackCube;
  void *framePool;
  pipeline_key_t lastKey;
} mtl;

enum { OBJ_TEXTURE, OBJ_BUFFER, OBJ_FRAMEBUFFER, OBJ_RENDERBUFFER, OBJ_VERTEXARRAY, OBJ_SHADER, OBJ_PROGRAM };

static void set_error(mtlEnum error) { if (!mtl.error) mtl.error = error; }

static void log_once(char const *message)
{
  static NSMutableSet *logged;
  if (!logged) logged = [NSMutableSet new];
  NSString *key = @(message);
  if ([logged containsObject:key]) return;
  [logged addObject:key];
  axLog("Metal: %s", message);
}

// The window's layer, configured for this device. The platform replaces it
// when the window is recreated, as after a reload, so it is looked up again.
static CAMetalLayer *current_layer(void)
{
  CAMetalLayer *layer = (__bridge CAMetalLayer *)axGetMetalLayer();
  if (layer == mtl.layer) return layer;
  mtl.layer = layer;
  mtl.drawable = nil;
  if (layer) {
    layer.device = mtl.device;
    layer.framebufferOnly = NO;  // the drawable is read by backdrop filters
    layer.pixelFormat = mtl.srgb ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
  }
  return layer;
}

static BOOL ensure_device(void)
{
  if (mtl.device) return YES;
  CAMetalLayer *layer = (__bridge CAMetalLayer *)axGetMetalLayer();
  mtl.device = layer.device ? layer.device : MTLCreateSystemDefaultDevice();
  if (!mtl.device) return log_once("no Metal device"), NO;
  mtl.queue = [mtl.device newCommandQueue];
  current_layer();
  for (int i = 0; i < 7; i++) mtl.objects[i] = [NSMutableDictionary new];
  mtl.depthStates = [NSMutableDictionary new];
  mtl.samplers = [NSMutableDictionary new];
  mtl.blitPipelines = [NSMutableDictionary new];
  mtl.defaultVertexArray = mtl.vertexArray = [OrcaVertexArray new];
  mtl.unpackAlignment = 4;
  mtl.blendSrc = MTL_ONE, mtl.blendDst = MTL_ZERO;
  mtl.colorMask = 0xf;
  mtl.cullFace = MTL_BACK;
  mtl.depthFunc = MTL_LESS;
  mtl.depthMask = YES;
  mtl.stencilFunc = MTL_ALWAYS;
  mtl.stencilFail = mtl.stencilZFail = mtl.stencilZPass = MTL_KEEP;
  mtl.stencilReadMask = mtl.stencilWriteMask = 0xffffffff;
  mtl.clearDepth = 1;
  float constant[4] = { 0, 0, 0, 1 };
  mtl.constantAttribute = [mtl.device newBufferWithBytes:constant length:sizeof(constant)
                                               options:MTLResourceStorageModeShared];
  MTLDepthStencilDescriptor *ds = [MTLDepthStencilDescriptor new];
  ds.depthCompareFunction = MTLCompareFunctionAlways;
  ds.depthWriteEnabled = YES;
  mtl.depthAlways = [mtl.device newDepthStencilStateWithDescriptor:ds];
  ds.depthWriteEnabled = NO;
  mtl.depthDisabled = [mtl.device newDepthStencilStateWithDescriptor:ds];
  return YES;
}

static id object(int kind, uint32_t name) { return name ? mtl.objects[kind][@(name)] : nil; }

static void gen(int kind, int n, uint32_t *names, Class cls)
{
  if (!ensure_device() || n < 0) return set_error(MTL_INVALID_VALUE);
  for (int i = 0; i < n; i++) {
    names[i] = ++mtl.nextName;
    mtl.objects[kind][@(names[i])] = cls ? [cls new] : [NSNull null];
  }
}

static void forget(int kind, int n, uint32_t const *names)
{
  for (int i = 0; names && i < n; i++) if (names[i]) [mtl.objects[kind] removeObjectForKey:@(names[i])];
}

#pragma mark - Command buffers and passes

static id<MTLCommandBuffer> command_buffer(void)
{
  if (mtl.cb) return mtl.cb;
  mtl.cb = [mtl.queue commandBuffer];
  uint64_t serial = ++mtl.serial;
  [mtl.cb addCompletedHandler:^(id<MTLCommandBuffer> cb) {
    (void)cb;
    uint64_t done = atomic_load(&mtl.completed);
    while (done < serial && !atomic_compare_exchange_weak(&mtl.completed, &done, serial)) {}
  }];
  return mtl.cb;
}

static void end_pass(void)
{
  if (!mtl.encoderOpen) return;
  [mtl.encoder endEncoding];
  mtl.encoder = nil;
  mtl.encoderFramebuffer = nil;
  mtl.encoderOpen = NO;
  mtl.lastPipeline = nil;
}

static void commit(BOOL wait)
{
  end_pass();
  if (!mtl.cb) return;
  id<MTLCommandBuffer> cb = mtl.cb;
  mtl.cb = nil;
  [cb commit];
  if (wait) [cb waitUntilCompleted];
}

static void use(OrcaTexture *t) { if (t) t->lastUse = mtl.serial; }

// The texture the default framebuffer draws into this frame.
static id<MTLTexture> default_color(void)
{
  CAMetalLayer *layer = current_layer();
  if (layer) {
    if (!mtl.drawable) {
      CGSize size = layer.drawableSize;
      if (size.width < 1 || size.height < 1) return nil;
      mtl.drawable = [layer nextDrawable];
    }
    return mtl.drawable.texture;
  }
  struct AXsize size;
  axGetSize(&size);
  NSUInteger w = MAX(1, (NSUInteger)(size.width * axGetScaling()));
  NSUInteger h = MAX(1, (NSUInteger)(size.height * axGetScaling()));
  if (!mtl.offscreen || mtl.offscreen.width != w || mtl.offscreen.height != h) {
    MTLPixelFormat format = mtl.srgb ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm;
    MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                                                  width:w height:h mipmapped:NO];
    d.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    d.storageMode = MTLStorageModePrivate;
    mtl.offscreen = [mtl.device newTextureWithDescriptor:d];
  }
  return mtl.offscreen;
}

static id<MTLTexture> default_depth(id<MTLTexture> color)
{
  if (!color) return nil;
  if (!mtl.defaultDepth || mtl.defaultDepth.width != color.width || mtl.defaultDepth.height != color.height) {
    MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
                                                                                  width:color.width
                                                                                 height:color.height
                                                                              mipmapped:NO];
    d.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    d.storageMode = MTLStorageModePrivate;
    mtl.defaultDepth = [mtl.device newTextureWithDescriptor:d];
  }
  return mtl.defaultDepth;
}

static BOOL has_stencil(MTLPixelFormat f)
{
  return f == MTLPixelFormatDepth32Float_Stencil8 || f == MTLPixelFormatStencil8
#if TARGET_OS_OSX
      || f == MTLPixelFormatDepth24Unorm_Stencil8
#endif
      ;
}

typedef struct
{
  id<MTLTexture> color, depth, stencil;
  OrcaTexture *__unsafe_unretained colorObject, *__unsafe_unretained depthObject;
  BOOL flipped;   // offscreen: rows in bottom-up order
} targets_t;

static targets_t framebuffer_targets(OrcaFramebuffer *fb)
{
  targets_t t = { 0 };
  if (!fb) {
    t.color = default_color();
    t.depth = t.stencil = default_depth(t.color);
    return t;
  }
  t.flipped = YES;
  t.colorObject = fb->color;
  t.depthObject = fb->depth;
  t.color = fb->color ? fb->color->texture : nil;
  t.depth = fb->depth ? fb->depth->texture : nil;
  t.stencil = fb->stencil ? fb->stencil->texture
            : (t.depth && has_stencil(t.depth.pixelFormat) ? t.depth : nil);
  return t;
}

// Starts a pass on the draw framebuffer, applying a pending clear.
static BOOL begin_pass(void)
{
  if (mtl.encoderOpen && mtl.encoderFramebuffer == mtl.drawFramebuffer &&
      mtl.encoderDefault == !mtl.drawFramebuffer)
    return YES;
  end_pass();
  targets_t t = framebuffer_targets(mtl.drawFramebuffer);
  if (!t.color && !t.depth) return NO;
  MTLRenderPassDescriptor *rp = [MTLRenderPassDescriptor renderPassDescriptor];
  if (t.color) {
    rp.colorAttachments[0].texture = t.color;
    rp.colorAttachments[0].storeAction = MTLStoreActionStore;
    if (mtl.pendingClear & MTL_COLOR_BUFFER_BIT) {
      rp.colorAttachments[0].loadAction = MTLLoadActionClear;
      rp.colorAttachments[0].clearColor = MTLClearColorMake(mtl.pendingColor[0], mtl.pendingColor[1],
                                                            mtl.pendingColor[2], mtl.pendingColor[3]);
    } else {
      rp.colorAttachments[0].loadAction = MTLLoadActionLoad;
    }
  }
  if (t.depth) {
    rp.depthAttachment.texture = t.depth;
    rp.depthAttachment.storeAction = MTLStoreActionStore;
    rp.depthAttachment.loadAction = mtl.pendingClear & MTL_DEPTH_BUFFER_BIT ? MTLLoadActionClear : MTLLoadActionLoad;
    rp.depthAttachment.clearDepth = mtl.pendingDepth;
  }
  if (t.stencil) {
    rp.stencilAttachment.texture = t.stencil;
    rp.stencilAttachment.storeAction = MTLStoreActionStore;
    rp.stencilAttachment.loadAction = mtl.pendingClear & MTL_STENCIL_BUFFER_BIT ? MTLLoadActionClear : MTLLoadActionLoad;
    rp.stencilAttachment.clearStencil = (uint32_t)mtl.pendingStencil;
  }
  mtl.pendingClear = 0;
  mtl.encoder = [command_buffer() renderCommandEncoderWithDescriptor:rp];
  if (!mtl.encoder) return NO;
  mtl.encoderOpen = YES;
  mtl.encoderFramebuffer = mtl.drawFramebuffer;
  mtl.encoderDefault = !mtl.drawFramebuffer;
  use(t.colorObject);
  use(t.depthObject);
  return YES;
}

// A clear recorded for a framebuffer that is no longer bound must still happen.
static void flush_pending_clear(void)
{
  if (mtl.pendingClear && begin_pass()) end_pass();
  mtl.pendingClear = 0;
}

#pragma mark - Pixel formats and uploads

static int is_compressed(mtlEnum f) { return (f >= 0x93B0 && f <= 0x93DD) || (f >= 0x83F0 && f <= 0x83F3); }

static MTLPixelFormat pixel_format(mtlEnum internal, mtlEnum format)
{
  switch (internal) {
    case MTL_RGBA: case MTL_RGBA8: case MTL_RGB: case MTL_RGB8:
      return format == MTL_BGRA ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatRGBA8Unorm;
    case MTL_SRGB8_ALPHA8: case MTL_SRGB8: case MTL_SRGB:
      return format == MTL_BGRA ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm_sRGB;
    case MTL_RED: case MTL_R8: case MTL_ALPHA: case MTL_LUMINANCE: return MTLPixelFormatR8Unorm;
    case MTL_DEPTH_COMPONENT: case MTL_DEPTH_COMPONENT24: case MTL_DEPTH_COMPONENT32F: return MTLPixelFormatDepth32Float;
    case MTL_DEPTH_COMPONENT16: return MTLPixelFormatDepth16Unorm;
    case MTL_DEPTH_STENCIL: case MTL_DEPTH24_STENCIL8: return MTLPixelFormatDepth32Float_Stencil8;
  }
  if (internal >= 0x93B0 && internal <= 0x93DD) {
    if (@available(macOS 11.0, iOS 13.0, *)) {
      if (![mtl.device supportsFamily:MTLGPUFamilyApple2]) return MTLPixelFormatInvalid;
      if (internal == 0x93B0) return MTLPixelFormatASTC_4x4_LDR;
      if (internal == 0x93D0) return MTLPixelFormatASTC_4x4_sRGB;
    }
    return MTLPixelFormatInvalid;
  }
  if (internal >= 0x83F0 && internal <= 0x83F3) {
    if (@available(macOS 11.0, iOS 16.4, *)) {
      if (!mtl.device.supportsBCTextureCompression) return MTLPixelFormatInvalid;
      if (internal <= 0x83F1) return MTLPixelFormatBC1_RGBA;
      return internal == 0x83F2 ? MTLPixelFormatBC2_RGBA : MTLPixelFormatBC3_RGBA;
    }
    return MTLPixelFormatInvalid;
  }
  return MTLPixelFormatInvalid;
}

static BOOL is_depth_format(MTLPixelFormat f)
{
  return f == MTLPixelFormatDepth32Float || f == MTLPixelFormatDepth16Unorm || has_stencil(f);
}

static int source_bpp(mtlEnum format, mtlEnum type)
{
  if (type == MTL_UNSIGNED_INT_8_8_8_8_REV) return 4;
  switch (format) {
    case MTL_RED: case MTL_ALPHA: case MTL_LUMINANCE: return 1;
    case MTL_RGB: return 3;
    default: return 4;
  }
}

static NSUInteger target_slice(mtlEnum target)
{
  return target >= MTL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= MTL_TEXTURE_CUBE_MAP_NEGATIVE_Z
       ? target - MTL_TEXTURE_CUBE_MAP_POSITIVE_X : 0;
}

static mtlEnum base_target(mtlEnum target)
{
  return target_slice(target) || target == MTL_TEXTURE_CUBE_MAP_POSITIVE_X ? MTL_TEXTURE_CUBE_MAP : target;
}

static OrcaTexture *bound_texture(mtlEnum target)
{
  unit_t *u = &mtl.units[mtl.activeUnit];
  return base_target(target) == MTL_TEXTURE_CUBE_MAP ? u->boundCube : u->bound2D;
}

// Mipmapped loaders (DDS, PVR) set MTL_TEXTURE_MAX_LEVEL before uploading;
// other textures get one level, whatever the default minification filter.
static BOOL wants_mipmaps(OrcaTexture *t)
{
  return t->maxLevel > 0 && t->maxLevel < 1000;
}

static NSUInteger full_chain(NSUInteger w, NSUInteger h)
{
  NSUInteger levels = 1;
  while ((w | h) > 1) w >>= 1, h >>= 1, levels++;
  return levels;
}

// (Re)creates the storage for level 0 at the given size, keeping no content.
static BOOL allocate(OrcaTexture *t, mtlEnum target, MTLPixelFormat format, int w, int h, mtlEnum internal)
{
  BOOL cube = base_target(target) == MTL_TEXTURE_CUBE_MAP;
  NSUInteger levels = wants_mipmaps(t) ? MIN(full_chain(w, h), (NSUInteger)t->maxLevel + 1) : 1;
  if (t->texture && t->texture.pixelFormat == format && t->width == w && t->height == h &&
      (t->texture.textureType == MTLTextureTypeCube) == cube && t->texture.mipmapLevelCount >= levels &&
      t->lastUse <= atomic_load(&mtl.completed))
    return YES;  // same storage, not in use by the GPU: write in place
  MTLTextureDescriptor *d = cube
    ? [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:format size:w mipmapped:NO]
    : [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:w height:h mipmapped:NO];
  d.mipmapLevelCount = levels;
  d.usage = MTLTextureUsageShaderRead;
  BOOL compressed = is_compressed(internal);
  if (!compressed) d.usage |= MTLTextureUsageRenderTarget;
  if (format == MTLPixelFormatR8Unorm) d.usage |= MTLTextureUsagePixelFormatView;
  d.storageMode = MTLStorageModeShared;
#if TARGET_OS_OSX
  d.storageMode = MTLStorageModeManaged;  // CPU-written textures on discrete GPUs
  if (@available(macOS 10.15, *)) if (mtl.device.hasUnifiedMemory) d.storageMode = MTLStorageModeShared;
#endif
  if (is_depth_format(format)) d.storageMode = MTLStorageModePrivate;
  id<MTLTexture> texture = [mtl.device newTextureWithDescriptor:d];
  if (!texture) return set_error(MTL_OUT_OF_MEMORY), NO;
  t->texture = texture;
  t->swizzled = nil;
  t->sampler = nil;
  t->width = w, t->height = h;
  t->levels = (int)levels;
  t->internalFormat = internal;
  t->lastUse = 0;
  return YES;
}

// Adds mip levels to a texture whose loader uploads more than it allocated.
static BOOL grow_levels(OrcaTexture *t, int level)
{
  if (level < (int)t->texture.mipmapLevelCount) return YES;
  MTLTextureDescriptor *d = [MTLTextureDescriptor new];
  d.textureType = t->texture.textureType;
  d.pixelFormat = t->texture.pixelFormat;
  d.width = t->texture.width, d.height = t->texture.height;
  d.mipmapLevelCount = MIN(full_chain(d.width, d.height), (NSUInteger)level + 1);
  d.usage = t->texture.usage;
  d.storageMode = t->texture.storageMode;
  id<MTLTexture> grown = [mtl.device newTextureWithDescriptor:d];
  if (!grown || level >= (int)grown.mipmapLevelCount) return set_error(MTL_INVALID_VALUE), NO;
  end_pass();
  id<MTLBlitCommandEncoder> blit = [command_buffer() blitCommandEncoder];
  NSUInteger slices = t->texture.textureType == MTLTextureTypeCube ? 6 : 1;
  for (NSUInteger s = 0; s < slices; s++)
    for (NSUInteger l = 0; l < t->texture.mipmapLevelCount; l++)
      [blit copyFromTexture:t->texture sourceSlice:s sourceLevel:l toTexture:grown destinationSlice:s
           destinationLevel:l sliceCount:1 levelCount:1];
  [blit endEncoding];
  t->texture = grown;
  t->swizzled = nil;
  t->sampler = nil;
  t->levels = (int)grown.mipmapLevelCount;
  t->lastUse = mtl.serial;
  return YES;
}

// Writes rows into a texture region in bottom-up order (first row = row 0). A
// texture the GPU may still read is updated through a blit in the stream.
static void write_region(OrcaTexture *t, NSUInteger slice, int level, int x, int y, int w, int h,
                         void const *bytes, NSUInteger rowBytes, NSUInteger imageBytes)
{
  MTLRegion region = MTLRegionMake2D(x, y, w, h);
  BOOL busy = t->lastUse > atomic_load(&mtl.completed) || t->texture.storageMode == MTLStorageModePrivate;
  if (!busy) {
    [t->texture replaceRegion:region mipmapLevel:level slice:slice withBytes:bytes
                  bytesPerRow:rowBytes bytesPerImage:0];
    return;
  }
  id<MTLBuffer> staging = [mtl.device newBufferWithBytes:bytes length:MAX(imageBytes, 1)
                                                options:MTLResourceStorageModeShared];
  end_pass();
  id<MTLBlitCommandEncoder> blit = [command_buffer() blitCommandEncoder];
  [blit copyFromBuffer:staging sourceOffset:0 sourceBytesPerRow:rowBytes sourceBytesPerImage:0
            sourceSize:MTLSizeMake(w, h, 1) toTexture:t->texture destinationSlice:slice
      destinationLevel:level destinationOrigin:MTLOriginMake(x, y, 0)];
  [blit endEncoding];
  t->lastUse = mtl.serial;
}

// Converts client pixels to the texture's layout: RGB gains alpha and rows
// lose their unpack padding. Returns a malloc'd buffer the caller frees, or
// NULL when `pixels` can be used as is (then *rowBytes is set).
static void *convert_pixels(void const *pixels, int w, int h, mtlEnum format, mtlEnum type,
                            NSUInteger *rowBytes)
{
  int bpp = source_bpp(format, type);
  NSUInteger stride = ((NSUInteger)w * bpp + mtl.unpackAlignment - 1) / mtl.unpackAlignment * mtl.unpackAlignment;
  if (format != MTL_RGB) return *rowBytes = stride, NULL;
  uint8_t *out = malloc((size_t)w * h * 4);
  if (!out) return *rowBytes = 0, NULL;
  for (int r = 0; r < h; r++) {
    uint8_t const *src = (uint8_t const *)pixels + r * stride;
    uint8_t *dst = out + (size_t)r * w * 4;
    for (int c = 0; c < w; c++)
      dst[c * 4] = src[c * 3], dst[c * 4 + 1] = src[c * 3 + 1], dst[c * 4 + 2] = src[c * 3 + 2], dst[c * 4 + 3] = 255;
  }
  *rowBytes = (NSUInteger)w * 4;
  return out;
}

static void const *unpack_source(void const *pixels)
{
  if (!mtl.unpackBuffer) return pixels;
  if (!mtl.unpackBuffer->buffer) return NULL;
  return (uint8_t const *)mtl.unpackBuffer->buffer.contents + (uintptr_t)pixels;
}

#pragma mark - Buffers

void mtlGenBuffers(int n, uint32_t *buffers) { gen(OBJ_BUFFER, n, buffers, OrcaBuffer.class); }

void mtlDeleteBuffers(int n, uint32_t const *buffers)
{
  for (int i = 0; buffers && i < n; i++) {
    OrcaBuffer *b = object(OBJ_BUFFER, buffers[i]);
    if (b && mtl.arrayBuffer == b) mtl.arrayBuffer = nil;
    if (b && mtl.unpackBuffer == b) mtl.unpackBuffer = nil;
    if (b && mtl.vertexArray->elements == b) mtl.vertexArray->elements = nil;
  }
  forget(OBJ_BUFFER, n, buffers);
}

static OrcaBuffer *__strong *buffer_binding(mtlEnum target)
{
  switch (target) {
    case MTL_ARRAY_BUFFER: return &mtl.arrayBuffer;
    case MTL_PIXEL_UNPACK_BUFFER: return &mtl.unpackBuffer;
    case MTL_ELEMENT_ARRAY_BUFFER: return mtl.vertexArray ? &mtl.vertexArray->elements : NULL;
  }
  return NULL;
}

void mtlBindBuffer(mtlEnum target, uint32_t buffer)
{
  OrcaBuffer *__strong *binding = buffer_binding(target);
  if (!binding) return set_error(MTL_INVALID_ENUM);
  *binding = object(OBJ_BUFFER, buffer);
}

// New storage every time, matching orphan semantics: draws already encoded keep
// reading the previous contents.
void mtlBufferData(mtlEnum target, ptrdiff_t size, void const *data, mtlEnum usage)
{
  (void)usage;
  OrcaBuffer *__strong *binding = buffer_binding(target);
  OrcaBuffer *b = binding ? *binding : nil;
  if (!b) return set_error(MTL_INVALID_OPERATION);
  b->size = (NSUInteger)MAX(size, 0);
  b->padded = nil;
  b->buffer = !size ? nil
            : data ? [mtl.device newBufferWithBytes:data length:size options:MTLResourceStorageModeShared]
                   : [mtl.device newBufferWithLength:size options:MTLResourceStorageModeShared];
  if (size && !b->buffer) set_error(MTL_OUT_OF_MEMORY);
}

void *mtlMapBuffer(mtlEnum target, mtlEnum access)
{
  (void)access;
  OrcaBuffer *__strong *binding = buffer_binding(target);
  OrcaBuffer *b = binding ? *binding : nil;
  return b && b->buffer ? b->buffer.contents : NULL;
}

bool mtlUnmapBuffer(mtlEnum target) { (void)target; return MTL_TRUE; }

#pragma mark - Vertex arrays

void mtlGenVertexArrays(int n, uint32_t *arrays) { gen(OBJ_VERTEXARRAY, n, arrays, OrcaVertexArray.class); }

void mtlDeleteVertexArrays(int n, uint32_t const *arrays)
{
  for (int i = 0; arrays && i < n; i++)
    if (arrays[i] && arrays[i] == mtl.vertexArrayName) mtl.vertexArray = mtl.defaultVertexArray, mtl.vertexArrayName = 0;
  forget(OBJ_VERTEXARRAY, n, arrays);
}

void mtlBindVertexArray(uint32_t array)
{
  OrcaVertexArray *va = array ? object(OBJ_VERTEXARRAY, array) : mtl.defaultVertexArray;
  if (!va) return set_error(MTL_INVALID_OPERATION);
  mtl.vertexArray = va;
  mtl.vertexArrayName = array;
}

void mtlVertexAttribPointer(uint32_t index, int size, mtlEnum type, bool normalized,
                           int stride, void const *pointer)
{
  if (index >= MAX_VERTEX_ATTRIBS || size < 1 || size > 4) return set_error(MTL_INVALID_VALUE);
  OrcaVertexArray *va = mtl.vertexArray;
  if (mtl.arrayBuffer && ![va->retained containsObject:mtl.arrayBuffer]) [va->retained addObject:mtl.arrayBuffer];
  attrib_t *a = &va->attribs[index];
  a->buffer = mtl.arrayBuffer;
  a->size = size, a->type = type, a->normalized = normalized;
  a->stride = stride;
  a->offset = (uintptr_t)pointer;
}

void mtlEnableVertexAttribArray(uint32_t index)
{
  if (index >= MAX_VERTEX_ATTRIBS) return set_error(MTL_INVALID_VALUE);
  mtl.vertexArray->attribs[index].enabled = YES;
}

#pragma mark - Textures

void mtlGenTextures(int n, uint32_t *textures) { gen(OBJ_TEXTURE, n, textures, OrcaTexture.class); }

void mtlDeleteTextures(int n, uint32_t const *textures)
{
  for (int i = 0; textures && i < n; i++) {
    OrcaTexture *t = object(OBJ_TEXTURE, textures[i]);
    for (int u = 0; t && u < MAX_TEXTURE_UNITS; u++) {
      if (mtl.units[u].bound2D == t) mtl.units[u].bound2D = nil;
      if (mtl.units[u].boundCube == t) mtl.units[u].boundCube = nil;
    }
  }
  forget(OBJ_TEXTURE, n, textures);
}

void mtlActiveTexture(mtlEnum texture)
{
  if (texture < MTL_TEXTURE0 || texture >= MTL_TEXTURE0 + MAX_TEXTURE_UNITS) return set_error(MTL_INVALID_ENUM);
  mtl.activeUnit = texture - MTL_TEXTURE0;
}

void mtlBindTexture(mtlEnum target, uint32_t texture)
{
  OrcaTexture *t = object(OBJ_TEXTURE, texture);
  if (texture && !t) return set_error(MTL_INVALID_OPERATION);
  if (t && !t->target) t->target = target;
  unit_t *u = &mtl.units[mtl.activeUnit];
  if (target == MTL_TEXTURE_CUBE_MAP) u->boundCube = t;
  else if (target == MTL_TEXTURE_2D || target == MTL_TEXTURE_RECTANGLE) u->bound2D = t;
  else set_error(MTL_INVALID_ENUM);
}

void mtlTexParameteri(mtlEnum target, mtlEnum pname, int param)
{
  OrcaTexture *t = bound_texture(target);
  if (!t) return set_error(MTL_INVALID_OPERATION);
  switch (pname) {
    case MTL_TEXTURE_MIN_FILTER: t->minFilter = param; break;
    case MTL_TEXTURE_MAG_FILTER: t->magFilter = param; break;
    case MTL_TEXTURE_WRAP_S: t->wrapS = param; break;
    case MTL_TEXTURE_WRAP_T: t->wrapT = param; break;
    case MTL_TEXTURE_WRAP_R: t->wrapR = param; break;
    case MTL_TEXTURE_MAX_LEVEL: t->maxLevel = param; return;
    case MTL_TEXTURE_BASE_LEVEL: return;
    case MTL_TEXTURE_SWIZZLE_R: case MTL_TEXTURE_SWIZZLE_G:
    case MTL_TEXTURE_SWIZZLE_B: case MTL_TEXTURE_SWIZZLE_A:
      t->swizzle[pname - MTL_TEXTURE_SWIZZLE_R] = param;
      t->swizzled = nil;
      return;
    default: return set_error(MTL_INVALID_ENUM);
  }
  t->sampler = nil;
}

void mtlPixelStorei(mtlEnum pname, int param)
{
  if (pname == MTL_UNPACK_ALIGNMENT && (param == 1 || param == 2 || param == 4 || param == 8))
    mtl.unpackAlignment = param;
}

void mtlTexImage2D(mtlEnum target, int level, int internalformat, int width, int height,
                  int border, mtlEnum format, mtlEnum type, void const *pixels)
{
  (void)border;
  OrcaTexture *t = bound_texture(target);
  if (!t || !ensure_device()) return set_error(MTL_INVALID_OPERATION);
  if (width < 1 || height < 1) return;
  MTLPixelFormat pf = pixel_format(internalformat, format);
  if (pf == MTLPixelFormatInvalid) return log_once("unsupported texture format"), set_error(MTL_INVALID_ENUM);
  if (level == 0) {
    if (!allocate(t, target, pf, width, height, internalformat)) return;
  } else if (!t->texture || !grow_levels(t, level)) {
    return;
  }
  void const *source = unpack_source(pixels);
  if (!source || is_depth_format(pf)) return;
  NSUInteger rowBytes;
  void *converted = convert_pixels(source, width, height, format, type, &rowBytes);
  if (!rowBytes) return set_error(MTL_OUT_OF_MEMORY);
  write_region(t, target_slice(target), level, 0, 0, width, height, converted ? converted : source,
               rowBytes, rowBytes * height);
  free(converted);
}

void mtlTexSubImage2D(mtlEnum target, int level, int xoffset, int yoffset, int width,
                     int height, mtlEnum format, mtlEnum type, void const *pixels)
{
  OrcaTexture *t = bound_texture(target);
  if (!t || !t->texture) return set_error(MTL_INVALID_OPERATION);
  if (width < 1 || height < 1) return;
  if (xoffset + width > (int)(t->texture.width >> level) || yoffset + height > (int)(t->texture.height >> level))
    return set_error(MTL_INVALID_VALUE);
  void const *source = unpack_source(pixels);
  if (!source) return;
  NSUInteger rowBytes;
  void *converted = convert_pixels(source, width, height, format, type, &rowBytes);
  if (!rowBytes) return set_error(MTL_OUT_OF_MEMORY);
  write_region(t, target_slice(target), level, xoffset, yoffset, width, height,
               converted ? converted : source, rowBytes, rowBytes * height);
  free(converted);
}

void mtlCompressedTexImage2D(mtlEnum target, int level, mtlEnum internalformat, int width,
                            int height, int border, int imageSize, void const *data)
{
  (void)border;
  OrcaTexture *t = bound_texture(target);
  if (!t || !ensure_device()) return set_error(MTL_INVALID_OPERATION);
  MTLPixelFormat pf = pixel_format(internalformat, 0);
  if (pf == MTLPixelFormatInvalid) return log_once("unsupported compressed texture format"), set_error(MTL_INVALID_ENUM);
  if (level == 0) {
    if (!allocate(t, target, pf, width, height, internalformat)) return;
  } else if (!t->texture || !grow_levels(t, level)) {
    return;
  }
  NSUInteger blockBytes = (internalformat == 0x83F0 || internalformat == 0x83F1) ? 8 : 16;
  NSUInteger rowBytes = (NSUInteger)(MAX(width, 1) + 3) / 4 * blockBytes;
  if (data) write_region(t, target_slice(target), level, 0, 0, MAX(width, 1), MAX(height, 1), data,
                         rowBytes, (NSUInteger)imageSize);
}

int mtlTexImageIOSurface(mtlEnum target, void *iosurface, int width, int height)
{
  OrcaTexture *t = bound_texture(target);
  if (!t || !iosurface || !ensure_device()) return -1;
  MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                width:width height:height
                                                                            mipmapped:NO];
  d.usage = MTLTextureUsageShaderRead;
  id<MTLTexture> texture = [mtl.device newTextureWithDescriptor:d iosurface:(IOSurfaceRef)iosurface plane:0];
  if (!texture) return -1;
  t->texture = texture;
  t->swizzled = nil;
  t->width = width, t->height = height, t->levels = 1;
  t->internalFormat = MTL_RGBA;
  return 0;
}

static MTLSamplerAddressMode address_mode(mtlEnum wrap)
{
  switch (wrap) {
    case MTL_CLAMP_TO_EDGE: return MTLSamplerAddressModeClampToEdge;
    case MTL_MIRRORED_REPEAT: return MTLSamplerAddressModeMirrorRepeat;
    default: return MTLSamplerAddressModeRepeat;
  }
}

static id<MTLSamplerState> sampler_for(OrcaTexture *t)
{
  if (t->sampler) return t->sampler;
  BOOL mips = t->texture && t->texture.mipmapLevelCount > 1;
  mtlEnum min = t->minFilter;
  MTLSamplerMinMagFilter minFilter =
    (min == MTL_NEAREST || min == MTL_NEAREST_MIPMAP_NEAREST || min == MTL_NEAREST_MIPMAP_LINEAR)
      ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
  MTLSamplerMipFilter mipFilter = !mips || min == MTL_NEAREST || min == MTL_LINEAR ? MTLSamplerMipFilterNotMipmapped
                                : (min == MTL_NEAREST_MIPMAP_LINEAR || min == MTL_LINEAR_MIPMAP_LINEAR)
                                  ? MTLSamplerMipFilterLinear : MTLSamplerMipFilterNearest;
  MTLSamplerMinMagFilter magFilter = t->magFilter == MTL_NEAREST ? MTLSamplerMinMagFilterNearest
                                                                : MTLSamplerMinMagFilterLinear;
  NSNumber *key = @((uint32_t)minFilter | (uint32_t)magFilter << 2 | (uint32_t)mipFilter << 4 |
                    (uint32_t)address_mode(t->wrapS) << 8 | (uint32_t)address_mode(t->wrapT) << 12 |
                    (uint32_t)address_mode(t->wrapR) << 16);
  id<MTLSamplerState> s = mtl.samplers[key];
  if (!s) {
    MTLSamplerDescriptor *d = [MTLSamplerDescriptor new];
    d.minFilter = minFilter;
    d.magFilter = magFilter;
    d.mipFilter = mipFilter;
    d.sAddressMode = address_mode(t->wrapS);
    d.tAddressMode = address_mode(t->wrapT);
    d.rAddressMode = address_mode(t->wrapR);
    s = [mtl.device newSamplerStateWithDescriptor:d];
    mtl.samplers[key] = s;
  }
  return t->sampler = s;
}

static MTLTextureSwizzle swizzle_channel(mtlEnum s)
{
  switch (s) {
    case MTL_RED: return MTLTextureSwizzleRed;
    case MTL_GREEN: return MTLTextureSwizzleGreen;
    case MTL_BLUE: return MTLTextureSwizzleBlue;
    case MTL_ALPHA: return MTLTextureSwizzleAlpha;
    case MTL_ONE: return MTLTextureSwizzleOne;
    default: return MTLTextureSwizzleZero;
  }
}

static id<MTLTexture> sampled_texture(OrcaTexture *t)
{
  if (t->swizzle[0] == MTL_RED && t->swizzle[1] == MTL_GREEN && t->swizzle[2] == MTL_BLUE && t->swizzle[3] == MTL_ALPHA)
    return t->texture;
  if (!t->swizzled && t->texture) {
    if (@available(macOS 10.15, iOS 13.0, *)) {
      MTLTextureSwizzleChannels channels = MTLTextureSwizzleChannelsMake(
        swizzle_channel(t->swizzle[0]), swizzle_channel(t->swizzle[1]),
        swizzle_channel(t->swizzle[2]), swizzle_channel(t->swizzle[3]));
      NSUInteger slices = t->texture.textureType == MTLTextureTypeCube ? 6 : 1;
      t->swizzled = [t->texture newTextureViewWithPixelFormat:t->texture.pixelFormat
                                                  textureType:t->texture.textureType
                                                       levels:NSMakeRange(0, t->texture.mipmapLevelCount)
                                                       slices:NSMakeRange(0, slices)
                                                      swizzle:channels];
    }
  }
  return t->swizzled ? t->swizzled : t->texture;
}

static OrcaTexture *fallback_texture(BOOL cube)
{
  OrcaTexture *__strong *slot = cube ? &mtl.blackCube : &mtl.blackTexture;
  if (*slot) return *slot;
  OrcaTexture *t = [OrcaTexture new];
  uint32_t black = 0xff000000;
  MTLTextureDescriptor *d = cube
    ? [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm size:1 mipmapped:NO]
    : [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:1 height:1 mipmapped:NO];
  d.usage = MTLTextureUsageShaderRead;
  t->texture = [mtl.device newTextureWithDescriptor:d];
  for (NSUInteger s = 0; s < (cube ? 6u : 1u); s++)
    [t->texture replaceRegion:MTLRegionMake2D(0, 0, 1, 1) mipmapLevel:0 slice:s withBytes:&black
                  bytesPerRow:4 bytesPerImage:4];
  t->minFilter = MTL_NEAREST;
  t->width = t->height = t->levels = 1;
  return *slot = t;
}

#pragma mark - Framebuffers

void mtlGenFramebuffers(int n, uint32_t *framebuffers) { gen(OBJ_FRAMEBUFFER, n, framebuffers, OrcaFramebuffer.class); }

void mtlDeleteFramebuffers(int n, uint32_t const *framebuffers)
{
  for (int i = 0; framebuffers && i < n; i++) {
    if (!framebuffers[i]) continue;
    if (framebuffers[i] == mtl.drawName) mtlBindFramebuffer(MTL_DRAW_FRAMEBUFFER, 0);
    if (framebuffers[i] == mtl.readName) mtlBindFramebuffer(MTL_READ_FRAMEBUFFER, 0);
  }
  forget(OBJ_FRAMEBUFFER, n, framebuffers);
}

void mtlBindFramebuffer(mtlEnum target, uint32_t framebuffer)
{
  if (!ensure_device()) return;
  OrcaFramebuffer *fb = object(OBJ_FRAMEBUFFER, framebuffer);
  if (framebuffer && !fb) return set_error(MTL_INVALID_OPERATION);
  if (target == MTL_FRAMEBUFFER || target == MTL_DRAW_FRAMEBUFFER) {
    if (framebuffer != mtl.drawName) flush_pending_clear();
    mtl.drawFramebuffer = fb;
    mtl.drawName = framebuffer;
  }
  if (target == MTL_FRAMEBUFFER || target == MTL_READ_FRAMEBUFFER) {
    mtl.readFramebuffer = fb;
    mtl.readName = framebuffer;
  }
}

static OrcaFramebuffer *framebuffer_for(mtlEnum target)
{
  return target == MTL_READ_FRAMEBUFFER ? mtl.readFramebuffer : mtl.drawFramebuffer;
}

static void attach(mtlEnum target, mtlEnum attachment, OrcaTexture *t)
{
  OrcaFramebuffer *fb = framebuffer_for(target);
  if (!fb) return set_error(MTL_INVALID_OPERATION);
  if (mtl.encoderOpen && mtl.encoderFramebuffer == fb) end_pass();
  switch (attachment) {
    case MTL_COLOR_ATTACHMENT0: fb->color = t; break;
    case MTL_DEPTH_ATTACHMENT: fb->depth = t; break;
    case MTL_STENCIL_ATTACHMENT: fb->stencil = t; break;
    case MTL_DEPTH_STENCIL_ATTACHMENT: fb->depth = t, fb->stencil = nil; break;
    default: set_error(MTL_INVALID_ENUM);
  }
}

void mtlFramebufferTexture2D(mtlEnum target, mtlEnum attachment, mtlEnum textarget, uint32_t texture, int level)
{
  (void)textarget, (void)level;
  attach(target, attachment, object(OBJ_TEXTURE, texture));
}

void mtlFramebufferRenderbuffer(mtlEnum target, mtlEnum attachment, mtlEnum renderbuffertarget, uint32_t renderbuffer)
{
  (void)renderbuffertarget;
  attach(target, attachment, object(OBJ_RENDERBUFFER, renderbuffer));
}

mtlEnum mtlCheckFramebufferStatus(mtlEnum target)
{
  OrcaFramebuffer *fb = framebuffer_for(target);
  if (!fb) return MTL_FRAMEBUFFER_COMPLETE;
  if (!fb->color && !fb->depth) return MTL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
  if ((fb->color && !fb->color->texture) || (fb->depth && !fb->depth->texture))
    return MTL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
  return MTL_FRAMEBUFFER_COMPLETE;
}

void mtlGetFramebufferAttachmentParameteriv(mtlEnum target, mtlEnum attachment, mtlEnum pname, int *params)
{
  targets_t t = framebuffer_targets(framebuffer_for(target));
  if (!params) return;
  *params = 0;
  BOOL depth = attachment == MTL_DEPTH || attachment == MTL_DEPTH_ATTACHMENT || attachment == MTL_DEPTH_STENCIL_ATTACHMENT;
  if (pname == MTL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE && depth && t.depth) *params = 24;
  if (pname == MTL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING && !depth && t.color) {
    MTLPixelFormat f = t.color.pixelFormat;
    *params = f == MTLPixelFormatRGBA8Unorm_sRGB || f == MTLPixelFormatBGRA8Unorm_sRGB ? MTL_SRGB : MTL_LINEAR;
  }
}

void mtlGenRenderbuffers(int n, uint32_t *renderbuffers)
{
  gen(OBJ_RENDERBUFFER, n, renderbuffers, OrcaTexture.class);
  for (int i = 0; i < n; i++) ((OrcaTexture *)object(OBJ_RENDERBUFFER, renderbuffers[i]))->renderbuffer = YES;
}

void mtlDeleteRenderbuffers(int n, uint32_t const *renderbuffers)
{
  for (int i = 0; renderbuffers && i < n; i++)
    if (renderbuffers[i] && renderbuffers[i] == mtl.renderbufferName) mtl.renderbuffer = nil, mtl.renderbufferName = 0;
  forget(OBJ_RENDERBUFFER, n, renderbuffers);
}

void mtlBindRenderbuffer(mtlEnum target, uint32_t renderbuffer)
{
  (void)target;
  mtl.renderbuffer = object(OBJ_RENDERBUFFER, renderbuffer);
  mtl.renderbufferName = renderbuffer;
}

void mtlRenderbufferStorage(mtlEnum target, mtlEnum internalformat, int width, int height)
{
  (void)target;
  if (!mtl.renderbuffer) return set_error(MTL_INVALID_OPERATION);
  MTLPixelFormat pf = pixel_format(internalformat, 0);
  if (pf == MTLPixelFormatInvalid) return set_error(MTL_INVALID_ENUM);
  mtl.renderbuffer->maxLevel = 0;
  allocate(mtl.renderbuffer, MTL_TEXTURE_2D, pf, width, height, internalformat);
}

#pragma mark - Blits

// A full-screen-quad copy used by mtlBlitFramebuffer and mtlCopyTexSubImage2D:
// Metal's blit encoder can neither scale nor flip rows.
static NSString *const kBlitSource =
  @"#include <metal_stdlib>\n"
   "using namespace metal;\n"
   "struct V { float4 position [[position]]; float2 uv; };\n"
   "vertex V orca_blit_vertex(uint id [[vertex_id]], constant float4 *quad [[buffer(0)]]) {\n"
   "  V v; v.position = float4(quad[id].xy, 0.0, 1.0); v.uv = quad[id].zw; return v; }\n"
   "fragment float4 orca_blit_color(V v [[stage_in]], texture2d<float> t [[texture(0)]],\n"
   "                                sampler s [[sampler(0)]]) { return t.sample(s, v.uv); }\n"
   "struct D { float depth [[depth(any)]]; };\n"
   "fragment D orca_blit_depth(V v [[stage_in]], depth2d<float> t [[texture(0)]], sampler s [[sampler(0)]]) {\n"
   "  D d; d.depth = t.sample(s, v.uv); return d; }\n";

static id<MTLRenderPipelineState> blit_pipeline(MTLPixelFormat format, BOOL depth)
{
  NSNumber *key = @((NSUInteger)format << 1 | depth);
  id<MTLRenderPipelineState> pso = mtl.blitPipelines[key];
  if (pso) return pso;
  NSError *error = nil;
  if (!mtl.blitLibrary) mtl.blitLibrary = [mtl.device newLibraryWithSource:kBlitSource options:nil error:&error];
  if (!mtl.blitLibrary) return axLog("Metal: blit shaders: %s", error.localizedDescription.UTF8String), nil;
  MTLRenderPipelineDescriptor *d = [MTLRenderPipelineDescriptor new];
  d.vertexFunction = [mtl.blitLibrary newFunctionWithName:@"orca_blit_vertex"];
  d.fragmentFunction = [mtl.blitLibrary newFunctionWithName:depth ? @"orca_blit_depth" : @"orca_blit_color"];
  if (depth) {
    d.depthAttachmentPixelFormat = format;
    if (has_stencil(format)) d.stencilAttachmentPixelFormat = format;
  } else {
    d.colorAttachments[0].pixelFormat = format;
  }
  pso = [mtl.device newRenderPipelineStateWithDescriptor:d error:&error];
  if (!pso) return axLog("Metal: blit pipeline: %s", error.localizedDescription.UTF8String), nil;
  return mtl.blitPipelines[key] = pso;
}

// Draws the window rectangle (sx0, sy0)-(sx1, sy1) of `src` over
// (dx0, dy0)-(dx1, dy1) of `dst`. Both rectangles are in bottom-up
// coordinates; `flipped` says which textures store rows bottom-up.
static void draw_quad(id<MTLTexture> src, BOOL srcFlipped, id<MTLTexture> dst, BOOL dstFlipped, BOOL depth,
                      int sx0, int sy0, int sx1, int sy1,
                      int dx0, int dy0, int dx1, int dy1, BOOL linear)
{
  id<MTLRenderPipelineState> pso = src && dst ? blit_pipeline(dst.pixelFormat, depth) : nil;
  if (!pso) return;
  end_pass();
  MTLRenderPassDescriptor *rp = [MTLRenderPassDescriptor renderPassDescriptor];
  if (depth) {
    rp.depthAttachment.texture = dst;
    rp.depthAttachment.loadAction = MTLLoadActionLoad;
    rp.depthAttachment.storeAction = MTLStoreActionStore;
    if (has_stencil(dst.pixelFormat)) {
      rp.stencilAttachment.texture = dst;
      rp.stencilAttachment.loadAction = MTLLoadActionLoad;
      rp.stencilAttachment.storeAction = MTLStoreActionStore;
    }
  } else {
    rp.colorAttachments[0].texture = dst;
    rp.colorAttachments[0].loadAction = MTLLoadActionLoad;
    rp.colorAttachments[0].storeAction = MTLStoreActionStore;
  }
  id<MTLRenderCommandEncoder> enc = [command_buffer() renderCommandEncoderWithDescriptor:rp];
  float sw = (float)src.width, sh = (float)src.height, dh = (float)dst.height;
  // Texture v of a bottom-up row coordinate, and the destination's NDC y.
  float v0 = srcFlipped ? sy0 / sh : 1 - sy0 / sh, v1 = srcFlipped ? sy1 / sh : 1 - sy1 / sh;
  float lo = MIN(dy0, dy1), hi = MAX(dy0, dy1);
  float y0 = (dy0 == lo) == dstFlipped ? 1 : -1, y1 = -y0;
  float quad[4][4] = {
    { -1, y0, sx0 / sw, v0 }, { 1, y0, sx1 / sw, v0 },
    { -1, y1, sx0 / sw, v1 }, { 1, y1, sx1 / sw, v1 },
  };
  if (dx1 < dx0) for (int i = 0; i < 4; i++) quad[i][0] = -quad[i][0];
  MTLViewport vp = { MIN(dx0, dx1), dstFlipped ? lo : dh - hi, abs(dx1 - dx0), hi - lo, 0, 1 };
  [enc setViewport:vp];
  [enc setRenderPipelineState:pso];
  if (depth) [enc setDepthStencilState:mtl.depthAlways];
  [enc setVertexBytes:quad length:sizeof(quad) atIndex:0];
  [enc setFragmentTexture:src atIndex:0];
  MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
  sd.minFilter = sd.magFilter = linear && !depth ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
  sd.sAddressMode = sd.tAddressMode = MTLSamplerAddressModeClampToEdge;
  NSNumber *key = @(linear && !depth ? 0x80000001u : 0x80000000u);
  if (!mtl.samplers[key]) mtl.samplers[key] = [mtl.device newSamplerStateWithDescriptor:sd];
  [enc setFragmentSamplerState:mtl.samplers[key] atIndex:0];
  [enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
  [enc endEncoding];
}

void mtlBlitFramebuffer(int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0,
                       int dstX1, int dstY1, uint32_t mask, mtlEnum filter)
{
  if (!(mask & MTL_COLOR_BUFFER_BIT)) return;
  flush_pending_clear();
  targets_t src = framebuffer_targets(mtl.readFramebuffer);
  targets_t dst = framebuffer_targets(mtl.drawFramebuffer);
  if (!src.color || !dst.color) return set_error(MTL_INVALID_OPERATION);
  draw_quad(src.color, src.flipped, dst.color, dst.flipped, NO, srcX0, srcY0, srcX1, srcY1,
            dstX0, dstY0, dstX1, dstY1, filter == MTL_LINEAR);
  use(src.colorObject);
  use(dst.colorObject);
}

void mtlCopyTexSubImage2D(mtlEnum target, int level, int xoffset, int yoffset, int x, int y,
                         int width, int height)
{
  (void)level;
  OrcaTexture *t = bound_texture(target);
  if (!t || !t->texture) return set_error(MTL_INVALID_OPERATION);
  flush_pending_clear();
  BOOL depth = is_depth_format(t->texture.pixelFormat);
  targets_t src = framebuffer_targets(mtl.readFramebuffer);
  id<MTLTexture> from = depth ? src.depth : src.color;
  if (!from) return set_error(MTL_INVALID_OPERATION);
  draw_quad(from, src.flipped, t->texture, YES, depth, x, y, x + width, y + height,
            xoffset, yoffset, xoffset + width, yoffset + height, NO);
  use(t);
  use(src.colorObject);
  use(src.depthObject);
}

void mtlReadPixels(int x, int y, int width, int height, mtlEnum format, mtlEnum type, void *pixels)
{
  if (format != MTL_RGBA || type != MTL_UNSIGNED_BYTE || !pixels) return set_error(MTL_INVALID_ENUM);
  flush_pending_clear();
  targets_t src = framebuffer_targets(mtl.readFramebuffer);
  if (!src.color || width < 1 || height < 1) return set_error(MTL_INVALID_OPERATION);
  // Metal rejects a copy outside the texture.
  if (x < 0 || y < 0 || x + width > (int)src.color.width || y + height > (int)src.color.height)
    return set_error(MTL_INVALID_VALUE);
  int top = src.flipped ? y : (int)src.color.height - (y + height);
  NSUInteger rowBytes = (NSUInteger)width * 4;
  id<MTLBuffer> buffer = [mtl.device newBufferWithLength:rowBytes * height options:MTLResourceStorageModeShared];
  end_pass();
  id<MTLBlitCommandEncoder> blit = [command_buffer() blitCommandEncoder];
  [blit copyFromTexture:src.color sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(x, top, 0)
             sourceSize:MTLSizeMake(width, height, 1) toBuffer:buffer destinationOffset:0
 destinationBytesPerRow:rowBytes destinationBytesPerImage:0];
  [blit endEncoding];
  commit(YES);
  BOOL bgra = src.color.pixelFormat == MTLPixelFormatBGRA8Unorm || src.color.pixelFormat == MTLPixelFormatBGRA8Unorm_sRGB;
  for (int r = 0; r < height; r++) {
    uint8_t const *row = (uint8_t const *)buffer.contents + rowBytes * (src.flipped ? r : height - 1 - r);
    uint8_t *out = (uint8_t *)pixels + rowBytes * r;
    memcpy(out, row, rowBytes);
    for (int c = 0; bgra && c < width; c++) {
      uint8_t b = out[c * 4];
      out[c * 4] = out[c * 4 + 2], out[c * 4 + 2] = b;
    }
  }
}

#pragma mark - Shaders and programs

uint32_t mtlCreateShader(mtlEnum type)
{
  uint32_t name;
  gen(OBJ_SHADER, 1, &name, OrcaShader.class);
  OrcaShader *s = object(OBJ_SHADER, name);
  if (!s) return 0;
  s->type = type;
  return name;
}

void mtlDeleteShader(uint32_t shader) { forget(OBJ_SHADER, 1, &shader); }

void mtlShaderSource(uint32_t shader, int count, char const *const *string, int const *length)
{
  OrcaShader *s = object(OBJ_SHADER, shader);
  if (!s) return set_error(MTL_INVALID_VALUE);
  NSMutableString *text = [NSMutableString new];
  for (int i = 0; i < count; i++) {
    NSString *part = length && length[i] >= 0
      ? [[NSString alloc] initWithBytes:string[i] length:length[i] encoding:NSUTF8StringEncoding]
      : [NSString stringWithUTF8String:string[i]];
    if (part) [text appendString:part];
  }
  s->source = text;
}

// Translation needs both stages, so it happens when the program links.
void mtlCompileShader(uint32_t shader) { (void)shader; }

void mtlGetShaderiv(uint32_t shader, mtlEnum pname, int *params)
{
  OrcaShader *s = object(OBJ_SHADER, shader);
  if (!params) return;
  if (pname == MTL_COMPILE_STATUS) *params = s && s->source ? MTL_TRUE : MTL_FALSE;
  else if (pname == MTL_INFO_LOG_LENGTH) *params = 0;
}

void mtlGetShaderInfoLog(uint32_t shader, int bufSize, int *length, char *infoLog)
{
  (void)shader;
  if (length) *length = 0;
  if (infoLog && bufSize > 0) *infoLog = 0;
}

uint32_t mtlCreateProgram(void)
{
  uint32_t name;
  gen(OBJ_PROGRAM, 1, &name, OrcaProgram.class);
  return object(OBJ_PROGRAM, name) ? name : 0;
}

void mtlDeleteProgram(uint32_t program)
{
  if (program && program == mtl.programName) mtl.program = nil, mtl.programName = 0;
  forget(OBJ_PROGRAM, 1, &program);
}

void mtlAttachShader(uint32_t program, uint32_t shader)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  OrcaShader *s = object(OBJ_SHADER, shader);
  if (!p || !s) return set_error(MTL_INVALID_VALUE);
  if (s->type == MTL_VERTEX_SHADER) p->vs = s;
  else p->fs = s;
}

void mtlDetachShader(uint32_t program, uint32_t shader)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  OrcaShader *s = object(OBJ_SHADER, shader);
  if (p && p->vs == s) p->vs = nil;
  if (p && p->fs == s) p->fs = nil;
}

static id<MTLFunction> compile_stage(OrcaProgram *p, int stage, NSMutableString *log)
{
  NSError *error = nil;
  MTLCompileOptions *options = [MTLCompileOptions new];
  options.languageVersion = MTLLanguageVersion2_1;
  id<MTLLibrary> library = [mtl.device newLibraryWithSource:@(p->refl->source[stage]) options:options error:&error];
  if (!library) {
    [log appendFormat:@"%s: %@\n", stage == MSL_VERTEX ? "vertex" : "fragment", error.localizedDescription];
    return nil;
  }
  return [library newFunctionWithName:@(p->refl->entry[stage])];
}

static struct { char const *name; struct msl_program const *refl; } const k_builtin_shaders[] = {
  { "Default Shader",      &k_refl_default      },
  { "UI Shader",           &k_refl_ui           },
  { "Charset Shader",      &k_refl_charset      },
  { "Cinematic Shader",    &k_refl_cinematic    },
  { "Ninepatch Shader",    &k_refl_ninepatch    },
  { "Vertex Color Shader", &k_refl_vertexcolor  },
  { "Error Shader",        &k_refl_error        },
  { "Button Shader",       &k_refl_button       },
  { "Rounded Box Shader",  &k_refl_roundedbox   },
  { "Blur H Shader",       &k_refl_blur_h       },
  { "Blur V Shader",       &k_refl_blur_v       },
  { "Backdrop Blur Shader",&k_refl_backdrop_blur},
};

static struct msl_program const *
lookup_msl_refl(NSString *name)
{
  if (!name) return NULL;
  char const *s = name.UTF8String;
  for (size_t i = 0; i < sizeof(k_builtin_shaders)/sizeof(*k_builtin_shaders); i++)
    if (!strcmp(k_builtin_shaders[i].name, s)) return k_builtin_shaders[i].refl;
  return NULL;
}

void mtlProgramLabel(uint32_t program, char const *name)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  if (p) p->name = name ? @(name) : nil;
}

void mtlLinkProgram(uint32_t program)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  if (!p || !ensure_device()) return set_error(MTL_INVALID_VALUE);
  NSString *name = p->name;
  [p reset];
  p->name = name;
  if (!p->pipelines) p->pipelines = [NSMutableDictionary new];
  static uint32_t serial;
  p->serial = ++serial;
  NSMutableString *log = [NSMutableString string];
  p->refl = lookup_msl_refl(p->name);
  if (!p->refl) {
    p->log = [NSString stringWithFormat:@"%@: no Metal shader with this name", p->name ?: @"(unnamed program)"];
    return;
  }
  int ok = YES;
  for (int s = 0; ok && s < MSL_STAGES; s++) {
    p->functions[s] = compile_stage(p, s, log);
    ok = p->functions[s] != nil;
    p->data[s] = calloc(1, MAX(p->refl->blockSize[s], 4));
    if (p->refl->blockSize[s] > MAX_SET_BYTES) {
      [log appendFormat:@"uniforms take %u bytes, more than %d\n", p->refl->blockSize[s], MAX_SET_BYTES];
      ok = NO;
    }
  }
  p->flipY = -1;
  for (uint32_t i = 0; ok && i < p->refl->numUniforms; i++) {
    p->units[i] = 0;
    if (!strcmp(p->refl->uniforms[i].name, MSL_FLIP_Y)) p->flipY = (int32_t)i;
  }
  p->log = log;
  p->linked = ok;
  if (!ok) [p reset], p->log = log;
}

void mtlUseProgram(uint32_t program)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  if (program && (!p || !p->linked)) return set_error(MTL_INVALID_OPERATION);
  mtl.program = p;
  mtl.programName = program;
}

static BOOL hidden(struct msl_uniform const *u) { return !strcmp(u->name, MSL_FLIP_Y); }

void mtlGetProgramiv(uint32_t program, mtlEnum pname, int *params)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  if (!p || !params) return set_error(MTL_INVALID_VALUE);
  switch (pname) {
    case MTL_LINK_STATUS: *params = p->linked; break;
    case MTL_INFO_LOG_LENGTH: *params = p->log.length ? (int)strlen(p->log.UTF8String) + 1 : 0; break;
    case MTL_ACTIVE_UNIFORMS:
      *params = 0;
      for (uint32_t i = 0; p->refl && i < p->refl->numUniforms; i++) *params += !hidden(&p->refl->uniforms[i]);
      break;
    case MTL_ACTIVE_ATTRIBUTES: *params = p->refl ? (int)p->refl->numAttributes : 0; break;
    default: set_error(MTL_INVALID_ENUM);
  }
}

void mtlGetProgramInfoLog(uint32_t program, int bufSize, int *length, char *infoLog)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  char const *text = p && p->log ? p->log.UTF8String : "";
  int n = bufSize > 0 ? (int)MIN(strlen(text), (size_t)bufSize - 1) : 0;
  if (infoLog && bufSize > 0) memcpy(infoLog, text, n), infoLog[n] = 0;
  if (length) *length = n;
}

void mtlGetActiveUniform(uint32_t program, uint32_t index, int bufSize, int *length, int *size,
                        mtlEnum *type, char *name)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  for (uint32_t i = 0, visible = 0; p && p->refl && i < p->refl->numUniforms; i++) {
    struct msl_uniform const *u = &p->refl->uniforms[i];
    if (hidden(u) || visible++ != index) continue;
    char text[MSL_MAX_NAME + 4];
    snprintf(text, sizeof(text), u->arraySize > 1 ? "%s[0]" : "%s", u->name);
    int n = bufSize > 0 ? (int)MIN(strlen(text), (size_t)bufSize - 1) : 0;
    if (name && bufSize > 0) memcpy(name, text, n), name[n] = 0;
    if (length) *length = n;
    if (size) *size = (int)u->arraySize;
    if (type) *type = u->type;
    return;
  }
  set_error(MTL_INVALID_VALUE);
}

// A location is the uniform's index with the array element above bit 8.
int mtlGetUniformLocation(uint32_t program, char const *name)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  if (!p || !p->refl || !name) return -1;
  char base[MSL_MAX_NAME];
  int element = 0;
  snprintf(base, sizeof(base), "%s", name);
  char *bracket = strchr(base, '[');
  if (bracket) element = atoi(bracket + 1), *bracket = 0;
  for (uint32_t i = 0; i < p->refl->numUniforms; i++) {
    struct msl_uniform const *u = &p->refl->uniforms[i];
    if (hidden(u) || strcmp(u->name, base)) continue;
    return element < 0 || (uint32_t)element >= u->arraySize ? -1 : (int)(i | (uint32_t)element << 8);
  }
  return -1;
}

int mtlGetAttribLocation(uint32_t program, char const *name)
{
  OrcaProgram *p = object(OBJ_PROGRAM, program);
  for (uint32_t i = 0; p && p->refl && name && i < p->refl->numAttributes; i++)
    if (!strcmp(p->refl->attributes[i].name, name)) return (int)p->refl->attributes[i].location;
  return -1;
}

static int components(mtlEnum type, int *columns, BOOL *integer)
{
  *columns = 1;
  *integer = NO;
  switch (type) {
    case MTL_FLOAT: return 1;
    case MTL_FLOAT_VEC2: return 2;
    case MTL_FLOAT_VEC3: return 3;
    case MTL_FLOAT_VEC4: return 4;
    case MTL_INT: case MTL_BOOL: case MTL_UNSIGNED_INT: *integer = YES; return 1;
    case MTL_INT_VEC2: case MTL_BOOL_VEC2: case MTL_UNSIGNED_INT_VEC2: *integer = YES; return 2;
    case MTL_INT_VEC3: case MTL_BOOL_VEC3: case MTL_UNSIGNED_INT_VEC3: *integer = YES; return 3;
    case MTL_INT_VEC4: case MTL_BOOL_VEC4: case MTL_UNSIGNED_INT_VEC4: *integer = YES; return 4;
    case MTL_FLOAT_MAT2: *columns = 2; return 2;
    case MTL_FLOAT_MAT3: *columns = 3; return 3;
    case MTL_FLOAT_MAT4: *columns = 4; return 4;
  }
  return 0;
}

static struct msl_uniform const *uniform_at(int location, uint32_t *index, uint32_t *element)
{
  OrcaProgram *p = mtl.program;
  if (location < 0 || !p || !p->refl) return NULL;
  *index = (uint32_t)location & 0xff;
  *element = (uint32_t)location >> 8;
  if (*index >= p->refl->numUniforms) return set_error(MTL_INVALID_OPERATION), NULL;
  return &p->refl->uniforms[*index];
}

// Writes `count` elements of `n` values each; values are floats or ints as
// the caller passes them and are converted to the uniform's type.
static void write_uniform(int location, int count, int n, float const *f, int const *i)
{
  uint32_t index, element;
  struct msl_uniform const *u = uniform_at(location, &index, &element);
  if (!u) return;
  if (u->type == MTL_SAMPLER_2D || u->type == MTL_SAMPLER_CUBE || u->type == MTL_SAMPLER_3D ||
      u->type == MTL_SAMPLER_2D_SHADOW) {
    mtl.program->units[index] = i ? i[0] : (int)f[0];
    return;
  }
  int columns;
  BOOL integer;
  int size = components(u->type, &columns, &integer);
  if (!size || columns > 1) return set_error(MTL_INVALID_OPERATION);
  for (int s = 0; s < MSL_STAGES; s++) {
    if (u->offset[s] < 0) continue;
    for (int e = 0; e < count && element + e < u->arraySize; e++) {
      uint32_t at = (uint32_t)u->offset[s] + (element + e) * u->arrayStride[s];
      if (at + size * 4 > mtl.program->refl->blockSize[s]) break;
      uint8_t *dst = mtl.program->data[s] + at;
      for (int c = 0; c < size && c < n; c++) {
        int k = e * n + c;
        if (integer) {
          int32_t v = i ? i[k] : (int32_t)f[k];
          if (u->type >= MTL_BOOL && u->type <= MTL_BOOL_VEC4) v = v != 0;
          memcpy(dst + c * 4, &v, 4);
        } else {
          float v = f ? f[k] : (float)i[k];
          memcpy(dst + c * 4, &v, 4);
        }
      }
    }
  }
}

static void write_matrix(int location, int count, int n, bool transpose, float const *value)
{
  uint32_t index, element;
  struct msl_uniform const *u = uniform_at(location, &index, &element);
  if (!u) return;
  int columns;
  BOOL integer;
  if (components(u->type, &columns, &integer) != n || columns != n) return set_error(MTL_INVALID_OPERATION);
  for (int s = 0; s < MSL_STAGES; s++) {
    if (u->offset[s] < 0) continue;
    for (int e = 0; e < count && element + e < u->arraySize; e++) {
      uint32_t at = (uint32_t)u->offset[s] + (element + e) * u->arrayStride[s];
      if (at + (n - 1) * u->matrixStride[s] + n * 4 > mtl.program->refl->blockSize[s]) break;
      float const *m = value + e * n * n;
      for (int c = 0; c < n; c++) {
        float *col = (float *)(mtl.program->data[s] + at + c * u->matrixStride[s]);
        for (int r = 0; r < n; r++) col[r] = transpose ? m[r * n + c] : m[c * n + r];
      }
    }
  }
}

void mtlUniform1f(int location, float v0) { write_uniform(location, 1, 1, &v0, NULL); }
void mtlUniform2f(int location, float v0, float v1) { float v[] = { v0, v1 }; write_uniform(location, 1, 2, v, NULL); }
void mtlUniform3f(int location, float v0, float v1, float v2) { float v[] = { v0, v1, v2 }; write_uniform(location, 1, 3, v, NULL); }
void mtlUniform4f(int location, float v0, float v1, float v2, float v3) { float v[] = { v0, v1, v2, v3 }; write_uniform(location, 1, 4, v, NULL); }
void mtlUniform1i(int location, int v0) { write_uniform(location, 1, 1, NULL, &v0); }
void mtlUniform2fv(int location, int count, float const *value) { write_uniform(location, count, 2, value, NULL); }
void mtlUniform3fv(int location, int count, float const *value) { write_uniform(location, count, 3, value, NULL); }
void mtlUniform4fv(int location, int count, float const *value) { write_uniform(location, count, 4, value, NULL); }
void mtlUniformMatrix3fv(int location, int count, bool transpose, float const *value) { write_matrix(location, count, 3, transpose, value); }
void mtlUniformMatrix4fv(int location, int count, bool transpose, float const *value) { write_matrix(location, count, 4, transpose, value); }

#pragma mark - Fixed-function state

void mtlEnable(mtlEnum cap)
{
  switch (cap) {
    case MTL_BLEND: mtl.blend = YES; break;
    case MTL_DEPTH_TEST: mtl.depthTest = YES; break;
    case MTL_STENCIL_TEST: mtl.stencilTest = YES; break;
    case MTL_CULL_FACE: mtl.cull = YES; break;
    case MTL_FRAMEBUFFER_SRGB:
      mtl.srgb = YES;
      if (ensure_device() && current_layer()) mtl.layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
      break;
  }
}

void mtlDisable(mtlEnum cap)
{
  switch (cap) {
    case MTL_BLEND: mtl.blend = NO; break;
    case MTL_DEPTH_TEST: mtl.depthTest = NO; break;
    case MTL_STENCIL_TEST: mtl.stencilTest = NO; break;
    case MTL_CULL_FACE: mtl.cull = NO; break;
    case MTL_FRAMEBUFFER_SRGB:
      mtl.srgb = NO;
      if (mtl.layer) mtl.layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
      break;
  }
}

void mtlBlendFunc(mtlEnum sfactor, mtlEnum dfactor) { mtl.blendSrc = sfactor, mtl.blendDst = dfactor; }

void mtlColorMask(bool red, bool green, bool blue, bool alpha)
{
  mtl.colorMask = (red ? MTLColorWriteMaskRed : 0) | (green ? MTLColorWriteMaskGreen : 0) |
                 (blue ? MTLColorWriteMaskBlue : 0) | (alpha ? MTLColorWriteMaskAlpha : 0);
}

void mtlCullFace(mtlEnum mode) { mtl.cullFace = mode; }
void mtlDepthFunc(mtlEnum func) { mtl.depthFunc = func; }
void mtlDepthMask(bool flag) { mtl.depthMask = flag; }
void mtlStencilFunc(mtlEnum func, int ref, uint32_t mask) { mtl.stencilFunc = func, mtl.stencilRef = ref, mtl.stencilReadMask = mask; }
void mtlStencilMask(uint32_t mask) { mtl.stencilWriteMask = mask; }
void mtlStencilOp(mtlEnum fail, mtlEnum zfail, mtlEnum zpass) { mtl.stencilFail = fail, mtl.stencilZFail = zfail, mtl.stencilZPass = zpass; }
void mtlViewport(int x, int y, int width, int height)
{
  mtl.viewport[0] = x, mtl.viewport[1] = y, mtl.viewport[2] = width, mtl.viewport[3] = height;
}
// The renderer never enables MTL_SCISSOR_TEST, so the rectangle has no effect.
void mtlScissor(int x, int y, int width, int height) { (void)x, (void)y, (void)width, (void)height; }

void mtlClearColor(float r, float g, float b, float a)
{
  mtl.clearColor[0] = r, mtl.clearColor[1] = g, mtl.clearColor[2] = b, mtl.clearColor[3] = a;
}
void mtlClearDepthf(float d) { mtl.clearDepth = d; }
void mtlClearStencil(int s) { mtl.clearStencil = s; }

// Clears the whole attachment: the renderer never enables the scissor test.
void mtlClear(uint32_t mask)
{
  if (!ensure_device()) return;
  if (mtl.encoderOpen && mtl.encoderFramebuffer == mtl.drawFramebuffer && mtl.encoderDefault == !mtl.drawFramebuffer)
    end_pass();
  mtl.pendingClear |= mask & (MTL_COLOR_BUFFER_BIT | MTL_DEPTH_BUFFER_BIT | MTL_STENCIL_BUFFER_BIT);
  if (mask & MTL_COLOR_BUFFER_BIT) memcpy(mtl.pendingColor, mtl.clearColor, sizeof(mtl.pendingColor));
  if (mask & MTL_DEPTH_BUFFER_BIT) mtl.pendingDepth = mtl.clearDepth;
  if (mask & MTL_STENCIL_BUFFER_BIT) mtl.pendingStencil = mtl.clearStencil;
}

void mtlGetIntegerv(mtlEnum pname, int *data)
{
  if (!data) return;
  switch (pname) {
    case MTL_VIEWPORT: memcpy(data, mtl.viewport, sizeof(mtl.viewport)); return;
    case MTL_FRAMEBUFFER_BINDING: *data = (int)mtl.drawName; return;
    case MTL_READ_FRAMEBUFFER_BINDING: *data = (int)mtl.readName; return;
    case MTL_RENDERBUFFER_BINDING: *data = (int)mtl.renderbufferName; return;
    case MTL_SAMPLES: *data = 0; return;
    case MTL_NUM_EXTENSIONS: *data = 0; return;
    case MTL_ACTIVE_TEXTURE: *data = MTL_TEXTURE0 + mtl.activeUnit; return;
    case MTL_UNPACK_ALIGNMENT: *data = mtl.unpackAlignment; return;
    case MTL_TEXTURE_BINDING_2D: {
      OrcaTexture *t = mtl.units[mtl.activeUnit].bound2D;
      *data = 0;
      for (NSNumber *name in mtl.objects[OBJ_TEXTURE])
        if (mtl.objects[OBJ_TEXTURE][name] == t) *data = name.intValue;
      return;
    }
    case MTL_MAX_TEXTURE_SIZE:
      *data = 8192;
      if (ensure_device()) {
#if TARGET_OS_OSX
        *data = 16384;
#else
        if (@available(iOS 13.0, *)) if ([mtl.device supportsFamily:MTLGPUFamilyApple3]) *data = 16384;
#endif
      }
      return;
  }
  *data = 0;
  set_error(MTL_INVALID_ENUM);
}

char const *mtlGetString(mtlEnum name)
{
  switch (name) {
    case MTL_VERSION: return "ORCA on Metal";
    case MTL_VENDOR: return "ORCA";
    case MTL_RENDERER: return "Metal";
    case MTL_EXTENSIONS: return "";
  }
  return NULL;
}

char const *mtlGetStringi(mtlEnum name, uint32_t index) { (void)name, (void)index; return NULL; }

mtlEnum mtlGetError(void)
{
  mtlEnum error = mtl.error;
  mtl.error = MTL_NO_ERROR;
  return error;
}

#pragma mark - Drawing

static MTLCompareFunction compare_function(mtlEnum f)
{
  switch (f) {
    case MTL_NEVER: return MTLCompareFunctionNever;
    case MTL_LESS: return MTLCompareFunctionLess;
    case MTL_EQUAL: return MTLCompareFunctionEqual;
    case MTL_LEQUAL: return MTLCompareFunctionLessEqual;
    case MTL_GREATER: return MTLCompareFunctionGreater;
    case MTL_NOTEQUAL: return MTLCompareFunctionNotEqual;
    case MTL_GEQUAL: return MTLCompareFunctionGreaterEqual;
    default: return MTLCompareFunctionAlways;
  }
}

static MTLStencilOperation stencil_operation(mtlEnum op)
{
  switch (op) {
    case MTL_ZERO: return MTLStencilOperationZero;
    case MTL_REPLACE: return MTLStencilOperationReplace;
    case MTL_INCR: return MTLStencilOperationIncrementClamp;
    case MTL_DECR: return MTLStencilOperationDecrementClamp;
    case MTL_INVERT: return MTLStencilOperationInvert;
    case MTL_INCR_WRAP: return MTLStencilOperationIncrementWrap;
    case MTL_DECR_WRAP: return MTLStencilOperationDecrementWrap;
    default: return MTLStencilOperationKeep;
  }
}

static MTLBlendFactor blend_factor(mtlEnum f)
{
  switch (f) {
    case MTL_ZERO: return MTLBlendFactorZero;
    case MTL_ONE: return MTLBlendFactorOne;
    case MTL_SRC_COLOR: return MTLBlendFactorSourceColor;
    case MTL_ONE_MINUS_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
    case MTL_SRC_ALPHA: return MTLBlendFactorSourceAlpha;
    case MTL_ONE_MINUS_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
    case MTL_DST_ALPHA: return MTLBlendFactorDestinationAlpha;
    case MTL_ONE_MINUS_DST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
    case MTL_DST_COLOR: return MTLBlendFactorDestinationColor;
    case MTL_ONE_MINUS_DST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
    default: return MTLBlendFactorOne;
  }
}

static MTLVertexFormat vertex_format(int size, mtlEnum type, bool normalized)
{
  static MTLVertexFormat const floats[] = { MTLVertexFormatFloat, MTLVertexFormatFloat2, MTLVertexFormatFloat3, MTLVertexFormatFloat4 };
  static MTLVertexFormat const halves[] = { MTLVertexFormatHalf, MTLVertexFormatHalf2, MTLVertexFormatHalf3, MTLVertexFormatHalf4 };
  static MTLVertexFormat const ubytesN[] = { MTLVertexFormatUCharNormalized, MTLVertexFormatUChar2Normalized, MTLVertexFormatUChar3Normalized, MTLVertexFormatUChar4Normalized };
  static MTLVertexFormat const bytesN[] = { MTLVertexFormatCharNormalized, MTLVertexFormatChar2Normalized, MTLVertexFormatChar3Normalized, MTLVertexFormatChar4Normalized };
  switch (type) {
    case MTL_FLOAT: return floats[size - 1];
    case MTL_HALF_FLOAT: return halves[size - 1];
    case MTL_UNSIGNED_BYTE:
      // Metal converts only normalized integers to a float input.
      if (!normalized) log_once("unnormalized byte attribute read as normalized");
      return ubytesN[size - 1];
    case MTL_BYTE:
      if (!normalized) log_once("unnormalized byte attribute read as normalized");
      return bytesN[size - 1];
  }
  return MTLVertexFormatInvalid;
}

static int type_size(mtlEnum type)
{
  return type == MTL_FLOAT ? 4 : type == MTL_HALF_FLOAT ? 2 : 1;
}

static NSUInteger source_stride(attrib_t const *a)
{
  return a->stride ? (NSUInteger)a->stride : (NSUInteger)(a->size * type_size(a->type));
}

// Tightly packed 3-byte or 6-byte elements (UINT8 or FLOAT16 vectors from a
// mesh file) are copied once into elements padded to a multiple of 4 bytes.
static id<MTLBuffer> padded_buffer(attrib_t const *a)
{
  OrcaBuffer *b = a->buffer;
  NSUInteger stride = source_stride(a), element = (NSUInteger)(a->size * type_size(a->type));
  if (b->padded && b->paddedStride == stride && b->paddedOffset == a->offset && b->paddedElement == element)
    return b->padded;
  NSUInteger count = b->size > a->offset ? (b->size - a->offset + stride - 1) / stride : 0;
  NSUInteger padded = (element + 3) & ~(NSUInteger)3;
  b->padded = [mtl.device newBufferWithLength:MAX(count * padded, 4) options:MTLResourceStorageModeShared];
  uint8_t const *src = (uint8_t const *)b->buffer.contents + a->offset;
  uint8_t *dst = b->padded.contents;
  for (NSUInteger i = 0; i < count; i++)
    memcpy(dst + i * padded, src + i * stride, MIN(element, b->size - a->offset - i * stride));
  b->paddedStride = stride, b->paddedOffset = a->offset, b->paddedElement = element;
  return b->padded;
}

static id<MTLRenderPipelineState> pipeline_for(OrcaProgram *p, targets_t const *t, pipeline_key_t *key)
{
  if (mtl.lastPipeline && !memcmp(&mtl.lastKey, key, sizeof(*key))) return mtl.lastPipeline;
  NSData *data = [NSData dataWithBytes:key length:sizeof(*key)];
  id<MTLRenderPipelineState> pso = p->pipelines[data];
  if (!pso) {
    MTLRenderPipelineDescriptor *d = [MTLRenderPipelineDescriptor new];
    d.vertexFunction = p->functions[MSL_VERTEX];
    d.fragmentFunction = p->functions[MSL_FRAGMENT];
    MTLVertexDescriptor *vd = [MTLVertexDescriptor vertexDescriptor];
    for (uint32_t i = 0; i < p->refl->numAttributes; i++) {
      uint32_t l = p->refl->attributes[i].location;
      NSUInteger index = MSL_VERTEX_BUFFER_BASE + l;
      vd.attributes[l].format = key->attrs[l].format;
      vd.attributes[l].offset = 0;
      vd.attributes[l].bufferIndex = index;
      if (key->attrs[l].stride) {
        vd.layouts[index].stride = key->attrs[l].stride;
      } else {  // a disabled array reads the default (0, 0, 0, 1)
        vd.layouts[index].stride = 16;
        vd.layouts[index].stepFunction = MTLVertexStepFunctionConstant;
        vd.layouts[index].stepRate = 0;
      }
    }
    d.vertexDescriptor = vd;
    if (t->color) {
      MTLRenderPipelineColorAttachmentDescriptor *c = d.colorAttachments[0];
      c.pixelFormat = t->color.pixelFormat;
      c.writeMask = key->mask;
      c.blendingEnabled = key->blend;
      c.sourceRGBBlendFactor = c.sourceAlphaBlendFactor = blend_factor(mtl.blendSrc);
      c.destinationRGBBlendFactor = c.destinationAlphaBlendFactor = blend_factor(mtl.blendDst);
    }
    if (t->depth) d.depthAttachmentPixelFormat = t->depth.pixelFormat;
    if (t->stencil) d.stencilAttachmentPixelFormat = t->stencil.pixelFormat;
    NSError *error = nil;
    pso = [mtl.device newRenderPipelineStateWithDescriptor:d error:&error];
    if (!pso) {
      axLog("Metal: pipeline: %s", error.localizedDescription.UTF8String);
      return nil;
    }
    p->pipelines[data] = pso;
  }
  mtl.lastPipeline = pso;
  mtl.lastKey = *key;
  return pso;
}

static id<MTLDepthStencilState> depth_state_for(targets_t const *t)
{
  if (!t->depth && !t->stencil) return mtl.depthDisabled;
  struct {
    uint8_t test, func, write, stencil, sfunc, fail, zfail, zpass;
    uint32_t read, mask;
  } key = { 0 };
  key.test = mtl.depthTest && t->depth;
  key.func = key.test ? (uint8_t)compare_function(mtl.depthFunc) : MTLCompareFunctionAlways;
  key.write = key.test && mtl.depthMask;  // no depth is written with the test off
  key.stencil = mtl.stencilTest && t->stencil;
  if (key.stencil) {
    key.sfunc = (uint8_t)compare_function(mtl.stencilFunc);
    key.fail = (uint8_t)stencil_operation(mtl.stencilFail);
    key.zfail = (uint8_t)stencil_operation(mtl.stencilZFail);
    key.zpass = (uint8_t)stencil_operation(mtl.stencilZPass);
    key.read = mtl.stencilReadMask & 0xff;
    key.mask = mtl.stencilWriteMask & 0xff;
  }
  NSData *data = [NSData dataWithBytes:&key length:sizeof(key)];
  id<MTLDepthStencilState> state = mtl.depthStates[data];
  if (state) return state;
  MTLDepthStencilDescriptor *d = [MTLDepthStencilDescriptor new];
  d.depthCompareFunction = key.func;
  d.depthWriteEnabled = key.write;
  if (key.stencil) {
    MTLStencilDescriptor *s = [MTLStencilDescriptor new];
    s.stencilCompareFunction = key.sfunc;
    s.stencilFailureOperation = key.fail;
    s.depthFailureOperation = key.zfail;
    s.depthStencilPassOperation = key.zpass;
    s.readMask = key.read;
    s.writeMask = key.mask;
    d.frontFaceStencil = d.backFaceStencil = s;
  }
  return mtl.depthStates[data] = [mtl.device newDepthStencilStateWithDescriptor:d];
}

static void bind_samplers(OrcaProgram *p)
{
  for (uint32_t i = 0; i < p->refl->numUniforms; i++) {
    struct msl_uniform const *u = &p->refl->uniforms[i];
    if (u->texture[MSL_VERTEX] < 0 && u->texture[MSL_FRAGMENT] < 0) continue;
    int unit = p->units[i];
    BOOL cube = u->type == MTL_SAMPLER_CUBE;
    OrcaTexture *t = unit >= 0 && unit < MAX_TEXTURE_UNITS
                  ? (cube ? mtl.units[unit].boundCube : mtl.units[unit].bound2D) : nil;
    if (!t || !t->texture) t = fallback_texture(cube);
    use(t);
    id<MTLTexture> texture = sampled_texture(t);
    id<MTLSamplerState> sampler = sampler_for(t);
    if (u->texture[MSL_VERTEX] >= 0) {
      [mtl.encoder setVertexTexture:texture atIndex:u->texture[MSL_VERTEX]];
      [mtl.encoder setVertexSamplerState:sampler atIndex:u->sampler[MSL_VERTEX]];
    }
    if (u->texture[MSL_FRAGMENT] >= 0) {
      [mtl.encoder setFragmentTexture:texture atIndex:u->texture[MSL_FRAGMENT]];
      [mtl.encoder setFragmentSamplerState:sampler atIndex:u->sampler[MSL_FRAGMENT]];
    }
  }
}

// Encodes everything but the draw call itself; NO skips the draw.
static BOOL prepare_draw(void)
{
  OrcaProgram *p = mtl.program;
  if (!p || !p->linked || !ensure_device()) return set_error(MTL_INVALID_OPERATION), NO;
  if (mtl.cull && mtl.cullFace == MTL_FRONT_AND_BACK) return NO;
  if (!begin_pass()) return NO;
  targets_t t = framebuffer_targets(mtl.drawFramebuffer);

  pipeline_key_t key;
  memset(&key, 0, sizeof(key));
  key.program = p->serial;
  key.color = t.color ? (uint32_t)t.color.pixelFormat : 0;
  key.depth = t.depth ? (uint32_t)t.depth.pixelFormat : 0;
  key.stencil = t.stencil ? (uint32_t)t.stencil.pixelFormat : 0;
  key.blend = mtl.blend;
  key.src = mtl.blend ? (uint8_t)blend_factor(mtl.blendSrc) : 0;
  key.dst = mtl.blend ? (uint8_t)blend_factor(mtl.blendDst) : 0;
  key.mask = mtl.colorMask;
  OrcaVertexArray *va = mtl.vertexArray;
  for (uint32_t i = 0; i < p->refl->numAttributes; i++) {
    uint32_t l = p->refl->attributes[i].location;
    attrib_t const *a = &va->attribs[l];
    if (a->enabled && a->buffer) {
      key.attrs[l].format = (uint16_t)vertex_format(a->size, a->type, a->normalized);
      NSUInteger stride = source_stride(a);
      key.attrs[l].stride = (uint16_t)(stride % 4 ? (a->size * type_size(a->type) + 3) & ~3 : stride);
    } else {
      int columns;
      BOOL integer;
      int n = components(p->refl->attributes[i].type, &columns, &integer);
      static MTLVertexFormat const floats[] = { MTLVertexFormatFloat, MTLVertexFormatFloat, MTLVertexFormatFloat2,
                                                MTLVertexFormatFloat3, MTLVertexFormatFloat4 };
      key.attrs[l].format = (uint16_t)floats[MAX(0, MIN(n, 4))];
      key.attrs[l].stride = 0;
    }
  }
  id<MTLRenderPipelineState> pso = pipeline_for(p, &t, &key);
  if (!pso) return NO;
  [mtl.encoder setRenderPipelineState:pso];
  [mtl.encoder setDepthStencilState:depth_state_for(&t)];
  [mtl.encoder setStencilReferenceValue:(uint32_t)mtl.stencilRef];
  [mtl.encoder setCullMode:!mtl.cull ? MTLCullModeNone : mtl.cullFace == MTL_FRONT ? MTLCullModeFront : MTLCullModeBack];
  // Front faces are counter-clockwise; the Y flip of offscreen targets mirrors the winding.
  [mtl.encoder setFrontFacingWinding:t.flipped ? MTLWindingClockwise : MTLWindingCounterClockwise];

  id<MTLTexture> target = t.color ? t.color : t.depth;
  double height = target.height;
  MTLViewport vp = { mtl.viewport[0], t.flipped ? mtl.viewport[1] : height - mtl.viewport[1] - mtl.viewport[3],
                     mtl.viewport[2], mtl.viewport[3], 0, 1 };
  [mtl.encoder setViewport:vp];

  if (p->flipY >= 0) {
    struct msl_uniform const *u = &p->refl->uniforms[p->flipY];
    float flip = t.flipped ? -1 : 1;
    if (u->offset[MSL_VERTEX] >= 0) memcpy(p->data[MSL_VERTEX] + u->offset[MSL_VERTEX], &flip, 4);
  }
  for (int s = 0; s < MSL_STAGES; s++) {
    if (!p->refl->blockSize[s]) continue;
    if (s == MSL_VERTEX)
      [mtl.encoder setVertexBytes:p->data[s] length:p->refl->blockSize[s] atIndex:MSL_UNIFORM_BUFFER];
    else
      [mtl.encoder setFragmentBytes:p->data[s] length:p->refl->blockSize[s] atIndex:MSL_UNIFORM_BUFFER];
  }
  for (uint32_t i = 0; i < p->refl->numAttributes; i++) {
    uint32_t l = p->refl->attributes[i].location;
    attrib_t const *a = &va->attribs[l];
    NSUInteger index = MSL_VERTEX_BUFFER_BASE + l;
    if (a->enabled && a->buffer && a->buffer->buffer && source_stride(a) % 4)
      [mtl.encoder setVertexBuffer:padded_buffer(a) offset:0 atIndex:index];
    else if (a->enabled && a->buffer && a->buffer->buffer)
      [mtl.encoder setVertexBuffer:a->buffer->buffer offset:a->offset atIndex:index];
    else
      [mtl.encoder setVertexBuffer:mtl.constantAttribute offset:0 atIndex:index];
  }
  bind_samplers(p);
  return YES;
}

static MTLPrimitiveType primitive(mtlEnum mode, BOOL *ok)
{
  *ok = YES;
  switch (mode) {
    case MTL_POINTS: return MTLPrimitiveTypePoint;
    case MTL_LINES: return MTLPrimitiveTypeLine;
    case MTL_LINE_STRIP: return MTLPrimitiveTypeLineStrip;
    case MTL_TRIANGLES: return MTLPrimitiveTypeTriangle;
    case MTL_TRIANGLE_STRIP: return MTLPrimitiveTypeTriangleStrip;
  }
  *ok = NO;
  return MTLPrimitiveTypeTriangle;
}

void mtlDrawArrays(mtlEnum mode, int first, int count)
{
  BOOL ok;
  MTLPrimitiveType type = primitive(mode, &ok);
  if (!ok) return set_error(MTL_INVALID_ENUM);
  if (count <= 0 || !prepare_draw()) return;
  [mtl.encoder drawPrimitives:type vertexStart:first vertexCount:count];
}

void mtlDrawElements(mtlEnum mode, int count, mtlEnum type, void const *indices)
{
  BOOL ok;
  MTLPrimitiveType prim = primitive(mode, &ok);
  if (!ok || (type != MTL_UNSIGNED_INT && type != MTL_UNSIGNED_SHORT)) return set_error(MTL_INVALID_ENUM);
  OrcaBuffer *elements = mtl.vertexArray->elements;
  if (!elements || !elements->buffer) return set_error(MTL_INVALID_OPERATION);
  if (count <= 0 || !prepare_draw()) return;
  [mtl.encoder drawIndexedPrimitives:prim indexCount:count
                          indexType:type == MTL_UNSIGNED_INT ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16
                        indexBuffer:elements->buffer indexBufferOffset:(NSUInteger)(uintptr_t)indices];
}

#pragma mark - Submission

void mtlFlush(void)
{
  if (!mtl.device) return;
  flush_pending_clear();
  commit(NO);
}

void mtlFinish(void)
{
  if (!mtl.device) return;
  flush_pending_clear();
  commit(YES);
}

// iOS frames already run in the display link's run loop pool.
void mtlBeginFrame(void)
{
#if TARGET_OS_OSX
  if (!mtl.framePool) mtl.framePool = objc_autoreleasePoolPush();
#endif
}

void mtlPresent(void)
{
  if (mtl.device) {
    if (!mtl.drawFramebuffer) flush_pending_clear();
    end_pass();
#if TARGET_OS_IPHONE
    // iOS refuses GPU work from a background app.
    if (UIApplication.sharedApplication.applicationState == UIApplicationStateBackground) {
      mtl.drawable = nil;
      mtl.cb = nil;
    }
#endif
    if (mtl.drawable) {
      [command_buffer() presentDrawable:mtl.drawable];
      mtl.drawable = nil;
    }
    commit(NO);
  }
  if (mtl.framePool) objc_autoreleasePoolPop(mtl.framePool), mtl.framePool = NULL;
}
