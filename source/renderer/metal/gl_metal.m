// The OpenGL ES 3.0 subset used by the ORCA renderer, implemented on Metal.
// See gl_metal.h and docs/METAL.md. Compiled with ARC.
//
// Model:
// - GL objects are Objective-C objects in per-kind name tables.
// - A render pass starts lazily at the first draw on the bound framebuffer.
//   glClear before that becomes the pass's load action; glClear inside an open
//   pass ends it and the next draw starts a cleared pass.
// - Offscreen framebuffers are rendered with the vertex stage's Y flip, so
//   their textures keep OpenGL's bottom-up rows: textures uploaded by the
//   engine and textures it renders to then sample the same way. The default
//   framebuffer is the CAMetalLayer's drawable and is rendered unflipped.
// - Uniform values live in each program's per-stage block and are copied
//   into the command stream with every draw (setVertexBytes).
// - An upload to a texture the GPU may still read is encoded as a blit in
//   the command stream, so draws before and after it see OpenGL's order.

#if !__has_feature(objc_arc)
#error gl_metal.m must be compiled with -fobjc-arc
#endif

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <TargetConditionals.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libs/platform/platform.h>
#include "gl_metal.h"
#include "glsl_msl.h"

#define MAX_TEXTURE_UNITS 16
#define MAX_VERTEX_ATTRIBS GLSL_MSL_MAX_ATTRIBUTES
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

@interface GLMTexture : NSObject {
@public
  GLenum target;                  // 0 until first bound
  id<MTLTexture> texture;
  id<MTLTexture> swizzled;        // view for a non-identity swizzle
  GLint width, height, levels;
  GLenum internalFormat;
  GLenum minFilter, magFilter, wrapS, wrapT, wrapR;
  GLint maxLevel;
  GLenum swizzle[4];
  id<MTLSamplerState> sampler;
  uint64_t lastUse;               // command buffer serial that last read or wrote it
  BOOL renderbuffer;
}
@end
@implementation GLMTexture
- (instancetype)init
{
  if ((self = [super init])) {
    minFilter = GL_NEAREST_MIPMAP_LINEAR;
    magFilter = GL_LINEAR;
    wrapS = wrapT = wrapR = GL_REPEAT;
    maxLevel = 1000;
    swizzle[0] = GL_RED, swizzle[1] = GL_GREEN, swizzle[2] = GL_BLUE, swizzle[3] = GL_ALPHA;
  }
  return self;
}
@end

@interface GLMBuffer : NSObject {
@public
  id<MTLBuffer> buffer;
  NSUInteger size;
  // Metal needs vertex strides in multiples of 4: a copy of the contents with
  // padded elements, for the source stride, offset and element size it was made for.
  id<MTLBuffer> padded;
  NSUInteger paddedStride, paddedOffset, paddedElement;
}
@end
@implementation GLMBuffer
@end

typedef struct
{
  GLMBuffer *__unsafe_unretained buffer;  // owned by GLMVertexArray.retained
  GLint size;
  GLenum type;
  GLboolean normalized;
  GLsizei stride;
  uintptr_t offset;
  BOOL enabled;
} attrib_t;

@interface GLMVertexArray : NSObject {
@public
  attrib_t attribs[MAX_VERTEX_ATTRIBS];
  NSMutableArray<GLMBuffer *> *retained;
  GLMBuffer *elements;
}
@end
@implementation GLMVertexArray
- (instancetype)init
{
  if ((self = [super init])) retained = [NSMutableArray new];
  return self;
}
@end

@interface GLMFramebuffer : NSObject {
@public
  GLMTexture *color;
  GLMTexture *depth;     // depth, or depth and stencil
  GLMTexture *stencil;
}
@end
@implementation GLMFramebuffer
@end

@interface GLMShader : NSObject {
@public
  GLenum type;
  NSString *source;
}
@end
@implementation GLMShader
@end

@interface GLMProgram : NSObject {
@public
  GLMShader *vs, *fs;
  BOOL linked;
  NSString *log;
  struct glsl_msl_program const *preset_refl;  // set before glLinkProgram; not freed
  struct glsl_msl_program const *refl;         // points to preset_refl after link
  id<MTLFunction> functions[GLSL_MSL_STAGES];
  uint8_t *data[GLSL_MSL_STAGES];
  GLint units[GLSL_MSL_MAX_UNIFORMS];
  int32_t flipY;                         // uniform index of GLSL_MSL_FLIP_Y
  uint32_t serial;
  NSMutableDictionary<NSData *, id<MTLRenderPipelineState>> *pipelines;
}
@end
@implementation GLMProgram
- (void)reset
{
  refl = NULL;
  for (int s = 0; s < GLSL_MSL_STAGES; s++) free(data[s]), data[s] = NULL, functions[s] = nil;
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
  GLenum target;
  GLMTexture *__unsafe_unretained bound2D, *__unsafe_unretained boundCube;
} unit_t;

static struct
{
  id<MTLDevice> device;
  id<MTLCommandQueue> queue;
  id<MTLCommandBuffer> cb;
  id<MTLRenderCommandEncoder> encoder;
  GLMFramebuffer *encoderFramebuffer;   // nil: the default framebuffer
  BOOL encoderOpen, encoderDefault;
  uint64_t serial;
  _Atomic uint64_t completed;

  CAMetalLayer *layer;
  id<CAMetalDrawable> drawable;
  id<MTLTexture> offscreen;             // default framebuffer without a layer
  id<MTLTexture> defaultDepth;
  BOOL srgb;

  NSMutableDictionary<NSNumber *, id> *objects[7];
  GLuint nextName;

  GLMBuffer *arrayBuffer, *unpackBuffer;
  GLMVertexArray *vertexArray, *defaultVertexArray;
  GLMFramebuffer *drawFramebuffer, *readFramebuffer;
  GLMTexture *renderbuffer;
  GLMProgram *program;
  GLuint drawName, readName, renderbufferName, programName, vertexArrayName;
  unit_t units[MAX_TEXTURE_UNITS];
  GLuint activeUnit;
  GLint unpackAlignment;

  BOOL blend, depthTest, stencilTest, cull;
  GLenum blendSrc, blendDst;
  uint8_t colorMask;
  GLenum cullFace, depthFunc;
  BOOL depthMask;
  GLenum stencilFunc, stencilFail, stencilZFail, stencilZPass;
  GLint stencilRef;
  GLuint stencilReadMask, stencilWriteMask;
  GLint viewport[4];

  GLfloat clearColor[4];
  GLfloat clearDepth;
  GLint clearStencil;
  GLbitfield pendingClear;              // for drawFramebuffer, before its pass starts
  GLfloat pendingColor[4];
  GLfloat pendingDepth;
  GLint pendingStencil;

  GLenum error;
  id<MTLRenderPipelineState> lastPipeline;
  NSMutableDictionary<NSData *, id<MTLDepthStencilState>> *depthStates;
  NSMutableDictionary<NSNumber *, id<MTLSamplerState>> *samplers;
  NSMutableDictionary<NSNumber *, id<MTLRenderPipelineState>> *blitPipelines;
  id<MTLLibrary> blitLibrary;
  id<MTLDepthStencilState> depthAlways, depthDisabled;
  id<MTLBuffer> constantAttribute;
  GLMTexture *blackTexture, *blackCube;
  void *framePool;
  pipeline_key_t lastKey;
} gl;

enum { OBJ_TEXTURE, OBJ_BUFFER, OBJ_FRAMEBUFFER, OBJ_RENDERBUFFER, OBJ_VERTEXARRAY, OBJ_SHADER, OBJ_PROGRAM };

static void set_error(GLenum error) { if (!gl.error) gl.error = error; }

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
  if (layer == gl.layer) return layer;
  gl.layer = layer;
  gl.drawable = nil;
  if (layer) {
    layer.device = gl.device;
    layer.framebufferOnly = NO;  // the drawable is read by backdrop filters
    layer.pixelFormat = gl.srgb ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
  }
  return layer;
}

static BOOL ensure_device(void)
{
  if (gl.device) return YES;
  CAMetalLayer *layer = (__bridge CAMetalLayer *)axGetMetalLayer();
  gl.device = layer.device ? layer.device : MTLCreateSystemDefaultDevice();
  if (!gl.device) return log_once("no Metal device"), NO;
  gl.queue = [gl.device newCommandQueue];
  current_layer();
  for (int i = 0; i < 7; i++) gl.objects[i] = [NSMutableDictionary new];
  gl.depthStates = [NSMutableDictionary new];
  gl.samplers = [NSMutableDictionary new];
  gl.blitPipelines = [NSMutableDictionary new];
  gl.defaultVertexArray = gl.vertexArray = [GLMVertexArray new];
  gl.unpackAlignment = 4;
  gl.blendSrc = GL_ONE, gl.blendDst = GL_ZERO;
  gl.colorMask = 0xf;
  gl.cullFace = GL_BACK;
  gl.depthFunc = GL_LESS;
  gl.depthMask = YES;
  gl.stencilFunc = GL_ALWAYS;
  gl.stencilFail = gl.stencilZFail = gl.stencilZPass = GL_KEEP;
  gl.stencilReadMask = gl.stencilWriteMask = 0xffffffff;
  gl.clearDepth = 1;
  float constant[4] = { 0, 0, 0, 1 };
  gl.constantAttribute = [gl.device newBufferWithBytes:constant length:sizeof(constant)
                                               options:MTLResourceStorageModeShared];
  MTLDepthStencilDescriptor *ds = [MTLDepthStencilDescriptor new];
  ds.depthCompareFunction = MTLCompareFunctionAlways;
  ds.depthWriteEnabled = YES;
  gl.depthAlways = [gl.device newDepthStencilStateWithDescriptor:ds];
  ds.depthWriteEnabled = NO;
  gl.depthDisabled = [gl.device newDepthStencilStateWithDescriptor:ds];
  return YES;
}

static id object(int kind, GLuint name) { return name ? gl.objects[kind][@(name)] : nil; }

static void gen(int kind, GLsizei n, GLuint *names, Class cls)
{
  if (!ensure_device() || n < 0) return set_error(GL_INVALID_VALUE);
  for (GLsizei i = 0; i < n; i++) {
    names[i] = ++gl.nextName;
    gl.objects[kind][@(names[i])] = cls ? [cls new] : [NSNull null];
  }
}

static void forget(int kind, GLsizei n, GLuint const *names)
{
  for (GLsizei i = 0; names && i < n; i++) if (names[i]) [gl.objects[kind] removeObjectForKey:@(names[i])];
}

#pragma mark - Command buffers and passes

static id<MTLCommandBuffer> command_buffer(void)
{
  if (gl.cb) return gl.cb;
  gl.cb = [gl.queue commandBuffer];
  uint64_t serial = ++gl.serial;
  [gl.cb addCompletedHandler:^(id<MTLCommandBuffer> cb) {
    (void)cb;
    uint64_t done = atomic_load(&gl.completed);
    while (done < serial && !atomic_compare_exchange_weak(&gl.completed, &done, serial)) {}
  }];
  return gl.cb;
}

static void end_pass(void)
{
  if (!gl.encoderOpen) return;
  [gl.encoder endEncoding];
  gl.encoder = nil;
  gl.encoderFramebuffer = nil;
  gl.encoderOpen = NO;
  gl.lastPipeline = nil;
}

static void commit(BOOL wait)
{
  end_pass();
  if (!gl.cb) return;
  id<MTLCommandBuffer> cb = gl.cb;
  gl.cb = nil;
  [cb commit];
  if (wait) [cb waitUntilCompleted];
}

static void use(GLMTexture *t) { if (t) t->lastUse = gl.serial; }

// The texture the default framebuffer draws into this frame.
static id<MTLTexture> default_color(void)
{
  CAMetalLayer *layer = current_layer();
  if (layer) {
    if (!gl.drawable) {
      CGSize size = layer.drawableSize;
      if (size.width < 1 || size.height < 1) return nil;
      gl.drawable = [layer nextDrawable];
    }
    return gl.drawable.texture;
  }
  struct AXsize size;
  axGetSize(&size);
  NSUInteger w = MAX(1, (NSUInteger)(size.width * axGetScaling()));
  NSUInteger h = MAX(1, (NSUInteger)(size.height * axGetScaling()));
  if (!gl.offscreen || gl.offscreen.width != w || gl.offscreen.height != h) {
    MTLPixelFormat format = gl.srgb ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm;
    MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                                                  width:w height:h mipmapped:NO];
    d.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    d.storageMode = MTLStorageModePrivate;
    gl.offscreen = [gl.device newTextureWithDescriptor:d];
  }
  return gl.offscreen;
}

static id<MTLTexture> default_depth(id<MTLTexture> color)
{
  if (!color) return nil;
  if (!gl.defaultDepth || gl.defaultDepth.width != color.width || gl.defaultDepth.height != color.height) {
    MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
                                                                                  width:color.width
                                                                                 height:color.height
                                                                              mipmapped:NO];
    d.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    d.storageMode = MTLStorageModePrivate;
    gl.defaultDepth = [gl.device newTextureWithDescriptor:d];
  }
  return gl.defaultDepth;
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
  GLMTexture *__unsafe_unretained colorObject, *__unsafe_unretained depthObject;
  BOOL flipped;   // offscreen: rows in OpenGL order
} targets_t;

static targets_t framebuffer_targets(GLMFramebuffer *fb)
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
  if (gl.encoderOpen && gl.encoderFramebuffer == gl.drawFramebuffer &&
      gl.encoderDefault == !gl.drawFramebuffer)
    return YES;
  end_pass();
  targets_t t = framebuffer_targets(gl.drawFramebuffer);
  if (!t.color && !t.depth) return NO;
  MTLRenderPassDescriptor *rp = [MTLRenderPassDescriptor renderPassDescriptor];
  if (t.color) {
    rp.colorAttachments[0].texture = t.color;
    rp.colorAttachments[0].storeAction = MTLStoreActionStore;
    if (gl.pendingClear & GL_COLOR_BUFFER_BIT) {
      rp.colorAttachments[0].loadAction = MTLLoadActionClear;
      rp.colorAttachments[0].clearColor = MTLClearColorMake(gl.pendingColor[0], gl.pendingColor[1],
                                                            gl.pendingColor[2], gl.pendingColor[3]);
    } else {
      rp.colorAttachments[0].loadAction = MTLLoadActionLoad;
    }
  }
  if (t.depth) {
    rp.depthAttachment.texture = t.depth;
    rp.depthAttachment.storeAction = MTLStoreActionStore;
    rp.depthAttachment.loadAction = gl.pendingClear & GL_DEPTH_BUFFER_BIT ? MTLLoadActionClear : MTLLoadActionLoad;
    rp.depthAttachment.clearDepth = gl.pendingDepth;
  }
  if (t.stencil) {
    rp.stencilAttachment.texture = t.stencil;
    rp.stencilAttachment.storeAction = MTLStoreActionStore;
    rp.stencilAttachment.loadAction = gl.pendingClear & GL_STENCIL_BUFFER_BIT ? MTLLoadActionClear : MTLLoadActionLoad;
    rp.stencilAttachment.clearStencil = (uint32_t)gl.pendingStencil;
  }
  gl.pendingClear = 0;
  gl.encoder = [command_buffer() renderCommandEncoderWithDescriptor:rp];
  if (!gl.encoder) return NO;
  gl.encoderOpen = YES;
  gl.encoderFramebuffer = gl.drawFramebuffer;
  gl.encoderDefault = !gl.drawFramebuffer;
  use(t.colorObject);
  use(t.depthObject);
  return YES;
}

// A clear recorded for a framebuffer that is no longer bound must still happen.
static void flush_pending_clear(void)
{
  if (gl.pendingClear && begin_pass()) end_pass();
  gl.pendingClear = 0;
}

#pragma mark - Pixel formats and uploads

static int is_compressed(GLenum f) { return (f >= 0x93B0 && f <= 0x93DD) || (f >= 0x83F0 && f <= 0x83F3); }

static MTLPixelFormat pixel_format(GLenum internal, GLenum format)
{
  switch (internal) {
    case GL_RGBA: case GL_RGBA8: case GL_RGB: case GL_RGB8:
      return format == GL_BGRA ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatRGBA8Unorm;
    case GL_SRGB8_ALPHA8: case GL_SRGB8: case GL_SRGB:
      return format == GL_BGRA ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm_sRGB;
    case GL_RED: case GL_R8: case GL_ALPHA: case GL_LUMINANCE: return MTLPixelFormatR8Unorm;
    case GL_DEPTH_COMPONENT: case GL_DEPTH_COMPONENT24: case GL_DEPTH_COMPONENT32F: return MTLPixelFormatDepth32Float;
    case GL_DEPTH_COMPONENT16: return MTLPixelFormatDepth16Unorm;
    case GL_DEPTH_STENCIL: case GL_DEPTH24_STENCIL8: return MTLPixelFormatDepth32Float_Stencil8;
  }
  if (internal >= 0x93B0 && internal <= 0x93DD) {
    if (@available(macOS 11.0, iOS 13.0, *)) {
      if (![gl.device supportsFamily:MTLGPUFamilyApple2]) return MTLPixelFormatInvalid;
      if (internal == 0x93B0) return MTLPixelFormatASTC_4x4_LDR;
      if (internal == 0x93D0) return MTLPixelFormatASTC_4x4_sRGB;
    }
    return MTLPixelFormatInvalid;
  }
  if (internal >= 0x83F0 && internal <= 0x83F3) {
    if (@available(macOS 11.0, iOS 16.4, *)) {
      if (!gl.device.supportsBCTextureCompression) return MTLPixelFormatInvalid;
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

static int source_bpp(GLenum format, GLenum type)
{
  if (type == GL_UNSIGNED_INT_8_8_8_8_REV) return 4;
  switch (format) {
    case GL_RED: case GL_ALPHA: case GL_LUMINANCE: return 1;
    case GL_RGB: return 3;
    default: return 4;
  }
}

static NSUInteger target_slice(GLenum target)
{
  return target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
       ? target - GL_TEXTURE_CUBE_MAP_POSITIVE_X : 0;
}

static GLenum base_target(GLenum target)
{
  return target_slice(target) || target == GL_TEXTURE_CUBE_MAP_POSITIVE_X ? GL_TEXTURE_CUBE_MAP : target;
}

static GLMTexture *bound_texture(GLenum target)
{
  unit_t *u = &gl.units[gl.activeUnit];
  return base_target(target) == GL_TEXTURE_CUBE_MAP ? u->boundCube : u->bound2D;
}

// Mipmapped loaders (DDS, PVR) set GL_TEXTURE_MAX_LEVEL before uploading;
// other textures get one level, whatever the default minification filter.
static BOOL wants_mipmaps(GLMTexture *t)
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
static BOOL allocate(GLMTexture *t, GLenum target, MTLPixelFormat format, GLint w, GLint h, GLenum internal)
{
  BOOL cube = base_target(target) == GL_TEXTURE_CUBE_MAP;
  NSUInteger levels = wants_mipmaps(t) ? MIN(full_chain(w, h), (NSUInteger)t->maxLevel + 1) : 1;
  if (t->texture && t->texture.pixelFormat == format && t->width == w && t->height == h &&
      (t->texture.textureType == MTLTextureTypeCube) == cube && t->texture.mipmapLevelCount >= levels &&
      t->lastUse <= atomic_load(&gl.completed))
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
  if (@available(macOS 10.15, *)) if (gl.device.hasUnifiedMemory) d.storageMode = MTLStorageModeShared;
#endif
  if (is_depth_format(format)) d.storageMode = MTLStorageModePrivate;
  id<MTLTexture> texture = [gl.device newTextureWithDescriptor:d];
  if (!texture) return set_error(GL_OUT_OF_MEMORY), NO;
  t->texture = texture;
  t->swizzled = nil;
  t->sampler = nil;
  t->width = w, t->height = h;
  t->levels = (GLint)levels;
  t->internalFormat = internal;
  t->lastUse = 0;
  return YES;
}

// Adds mip levels to a texture whose loader uploads more than it allocated.
static BOOL grow_levels(GLMTexture *t, GLint level)
{
  if (level < (GLint)t->texture.mipmapLevelCount) return YES;
  MTLTextureDescriptor *d = [MTLTextureDescriptor new];
  d.textureType = t->texture.textureType;
  d.pixelFormat = t->texture.pixelFormat;
  d.width = t->texture.width, d.height = t->texture.height;
  d.mipmapLevelCount = MIN(full_chain(d.width, d.height), (NSUInteger)level + 1);
  d.usage = t->texture.usage;
  d.storageMode = t->texture.storageMode;
  id<MTLTexture> grown = [gl.device newTextureWithDescriptor:d];
  if (!grown || level >= (GLint)grown.mipmapLevelCount) return set_error(GL_INVALID_VALUE), NO;
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
  t->levels = (GLint)grown.mipmapLevelCount;
  t->lastUse = gl.serial;
  return YES;
}

// Writes rows into a texture region in OpenGL order (first row = row 0). A
// texture the GPU may still read is updated through a blit in the stream.
static void write_region(GLMTexture *t, NSUInteger slice, GLint level, GLint x, GLint y, GLsizei w, GLsizei h,
                         void const *bytes, NSUInteger rowBytes, NSUInteger imageBytes)
{
  MTLRegion region = MTLRegionMake2D(x, y, w, h);
  BOOL busy = t->lastUse > atomic_load(&gl.completed) || t->texture.storageMode == MTLStorageModePrivate;
  if (!busy) {
    [t->texture replaceRegion:region mipmapLevel:level slice:slice withBytes:bytes
                  bytesPerRow:rowBytes bytesPerImage:0];
    return;
  }
  id<MTLBuffer> staging = [gl.device newBufferWithBytes:bytes length:MAX(imageBytes, 1)
                                                options:MTLResourceStorageModeShared];
  end_pass();
  id<MTLBlitCommandEncoder> blit = [command_buffer() blitCommandEncoder];
  [blit copyFromBuffer:staging sourceOffset:0 sourceBytesPerRow:rowBytes sourceBytesPerImage:0
            sourceSize:MTLSizeMake(w, h, 1) toTexture:t->texture destinationSlice:slice
      destinationLevel:level destinationOrigin:MTLOriginMake(x, y, 0)];
  [blit endEncoding];
  t->lastUse = gl.serial;
}

// Converts client pixels to the texture's layout: RGB gains alpha and rows
// lose their unpack padding. Returns a malloc'd buffer the caller frees, or
// NULL when `pixels` can be used as is (then *rowBytes is set).
static void *convert_pixels(void const *pixels, GLsizei w, GLsizei h, GLenum format, GLenum type,
                            NSUInteger *rowBytes)
{
  int bpp = source_bpp(format, type);
  NSUInteger stride = ((NSUInteger)w * bpp + gl.unpackAlignment - 1) / gl.unpackAlignment * gl.unpackAlignment;
  if (format != GL_RGB) return *rowBytes = stride, NULL;
  uint8_t *out = malloc((size_t)w * h * 4);
  if (!out) return *rowBytes = 0, NULL;
  for (GLsizei r = 0; r < h; r++) {
    uint8_t const *src = (uint8_t const *)pixels + r * stride;
    uint8_t *dst = out + (size_t)r * w * 4;
    for (GLsizei c = 0; c < w; c++)
      dst[c * 4] = src[c * 3], dst[c * 4 + 1] = src[c * 3 + 1], dst[c * 4 + 2] = src[c * 3 + 2], dst[c * 4 + 3] = 255;
  }
  *rowBytes = (NSUInteger)w * 4;
  return out;
}

static void const *unpack_source(void const *pixels)
{
  if (!gl.unpackBuffer) return pixels;
  if (!gl.unpackBuffer->buffer) return NULL;
  return (uint8_t const *)gl.unpackBuffer->buffer.contents + (uintptr_t)pixels;
}

#pragma mark - Buffers

void glGenBuffers(GLsizei n, GLuint *buffers) { gen(OBJ_BUFFER, n, buffers, GLMBuffer.class); }

void glDeleteBuffers(GLsizei n, GLuint const *buffers)
{
  for (GLsizei i = 0; buffers && i < n; i++) {
    GLMBuffer *b = object(OBJ_BUFFER, buffers[i]);
    if (b && gl.arrayBuffer == b) gl.arrayBuffer = nil;
    if (b && gl.unpackBuffer == b) gl.unpackBuffer = nil;
    if (b && gl.vertexArray->elements == b) gl.vertexArray->elements = nil;
  }
  forget(OBJ_BUFFER, n, buffers);
}

static GLMBuffer *__strong *buffer_binding(GLenum target)
{
  switch (target) {
    case GL_ARRAY_BUFFER: return &gl.arrayBuffer;
    case GL_PIXEL_UNPACK_BUFFER: return &gl.unpackBuffer;
    case GL_ELEMENT_ARRAY_BUFFER: return gl.vertexArray ? &gl.vertexArray->elements : NULL;
  }
  return NULL;
}

void glBindBuffer(GLenum target, GLuint buffer)
{
  GLMBuffer *__strong *binding = buffer_binding(target);
  if (!binding) return set_error(GL_INVALID_ENUM);
  *binding = object(OBJ_BUFFER, buffer);
}

// New storage every time, as OpenGL orphans it: draws already encoded keep
// reading the previous contents.
void glBufferData(GLenum target, GLsizeiptr size, void const *data, GLenum usage)
{
  (void)usage;
  GLMBuffer *__strong *binding = buffer_binding(target);
  GLMBuffer *b = binding ? *binding : nil;
  if (!b) return set_error(GL_INVALID_OPERATION);
  b->size = (NSUInteger)MAX(size, 0);
  b->padded = nil;
  b->buffer = !size ? nil
            : data ? [gl.device newBufferWithBytes:data length:size options:MTLResourceStorageModeShared]
                   : [gl.device newBufferWithLength:size options:MTLResourceStorageModeShared];
  if (size && !b->buffer) set_error(GL_OUT_OF_MEMORY);
}

void *glMapBuffer(GLenum target, GLenum access)
{
  (void)access;
  GLMBuffer *__strong *binding = buffer_binding(target);
  GLMBuffer *b = binding ? *binding : nil;
  return b && b->buffer ? b->buffer.contents : NULL;
}

GLboolean glUnmapBuffer(GLenum target) { (void)target; return GL_TRUE; }

#pragma mark - Vertex arrays

void glGenVertexArrays(GLsizei n, GLuint *arrays) { gen(OBJ_VERTEXARRAY, n, arrays, GLMVertexArray.class); }

void glDeleteVertexArrays(GLsizei n, GLuint const *arrays)
{
  for (GLsizei i = 0; arrays && i < n; i++)
    if (arrays[i] && arrays[i] == gl.vertexArrayName) gl.vertexArray = gl.defaultVertexArray, gl.vertexArrayName = 0;
  forget(OBJ_VERTEXARRAY, n, arrays);
}

void glBindVertexArray(GLuint array)
{
  GLMVertexArray *va = array ? object(OBJ_VERTEXARRAY, array) : gl.defaultVertexArray;
  if (!va) return set_error(GL_INVALID_OPERATION);
  gl.vertexArray = va;
  gl.vertexArrayName = array;
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                           GLsizei stride, void const *pointer)
{
  if (index >= MAX_VERTEX_ATTRIBS || size < 1 || size > 4) return set_error(GL_INVALID_VALUE);
  GLMVertexArray *va = gl.vertexArray;
  if (gl.arrayBuffer && ![va->retained containsObject:gl.arrayBuffer]) [va->retained addObject:gl.arrayBuffer];
  attrib_t *a = &va->attribs[index];
  a->buffer = gl.arrayBuffer;
  a->size = size, a->type = type, a->normalized = normalized;
  a->stride = stride;
  a->offset = (uintptr_t)pointer;
}

void glEnableVertexAttribArray(GLuint index)
{
  if (index >= MAX_VERTEX_ATTRIBS) return set_error(GL_INVALID_VALUE);
  gl.vertexArray->attribs[index].enabled = YES;
}

#pragma mark - Textures

void glGenTextures(GLsizei n, GLuint *textures) { gen(OBJ_TEXTURE, n, textures, GLMTexture.class); }

void glDeleteTextures(GLsizei n, GLuint const *textures)
{
  for (GLsizei i = 0; textures && i < n; i++) {
    GLMTexture *t = object(OBJ_TEXTURE, textures[i]);
    for (int u = 0; t && u < MAX_TEXTURE_UNITS; u++) {
      if (gl.units[u].bound2D == t) gl.units[u].bound2D = nil;
      if (gl.units[u].boundCube == t) gl.units[u].boundCube = nil;
    }
  }
  forget(OBJ_TEXTURE, n, textures);
}

void glActiveTexture(GLenum texture)
{
  if (texture < GL_TEXTURE0 || texture >= GL_TEXTURE0 + MAX_TEXTURE_UNITS) return set_error(GL_INVALID_ENUM);
  gl.activeUnit = texture - GL_TEXTURE0;
}

void glBindTexture(GLenum target, GLuint texture)
{
  GLMTexture *t = object(OBJ_TEXTURE, texture);
  if (texture && !t) return set_error(GL_INVALID_OPERATION);
  if (t && !t->target) t->target = target;
  unit_t *u = &gl.units[gl.activeUnit];
  if (target == GL_TEXTURE_CUBE_MAP) u->boundCube = t;
  else if (target == GL_TEXTURE_2D || target == GL_TEXTURE_RECTANGLE) u->bound2D = t;
  else set_error(GL_INVALID_ENUM);
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
  GLMTexture *t = bound_texture(target);
  if (!t) return set_error(GL_INVALID_OPERATION);
  switch (pname) {
    case GL_TEXTURE_MIN_FILTER: t->minFilter = param; break;
    case GL_TEXTURE_MAG_FILTER: t->magFilter = param; break;
    case GL_TEXTURE_WRAP_S: t->wrapS = param; break;
    case GL_TEXTURE_WRAP_T: t->wrapT = param; break;
    case GL_TEXTURE_WRAP_R: t->wrapR = param; break;
    case GL_TEXTURE_MAX_LEVEL: t->maxLevel = param; return;
    case GL_TEXTURE_BASE_LEVEL: return;
    case GL_TEXTURE_SWIZZLE_R: case GL_TEXTURE_SWIZZLE_G:
    case GL_TEXTURE_SWIZZLE_B: case GL_TEXTURE_SWIZZLE_A:
      t->swizzle[pname - GL_TEXTURE_SWIZZLE_R] = param;
      t->swizzled = nil;
      return;
    default: return set_error(GL_INVALID_ENUM);
  }
  t->sampler = nil;
}

void glPixelStorei(GLenum pname, GLint param)
{
  if (pname == GL_UNPACK_ALIGNMENT && (param == 1 || param == 2 || param == 4 || param == 8))
    gl.unpackAlignment = param;
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                  GLint border, GLenum format, GLenum type, void const *pixels)
{
  (void)border;
  GLMTexture *t = bound_texture(target);
  if (!t || !ensure_device()) return set_error(GL_INVALID_OPERATION);
  if (width < 1 || height < 1) return;
  MTLPixelFormat pf = pixel_format(internalformat, format);
  if (pf == MTLPixelFormatInvalid) return log_once("unsupported texture format"), set_error(GL_INVALID_ENUM);
  if (level == 0) {
    if (!allocate(t, target, pf, width, height, internalformat)) return;
  } else if (!t->texture || !grow_levels(t, level)) {
    return;
  }
  void const *source = unpack_source(pixels);
  if (!source || is_depth_format(pf)) return;
  NSUInteger rowBytes;
  void *converted = convert_pixels(source, width, height, format, type, &rowBytes);
  if (!rowBytes) return set_error(GL_OUT_OF_MEMORY);
  write_region(t, target_slice(target), level, 0, 0, width, height, converted ? converted : source,
               rowBytes, rowBytes * height);
  free(converted);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                     GLsizei height, GLenum format, GLenum type, void const *pixels)
{
  GLMTexture *t = bound_texture(target);
  if (!t || !t->texture) return set_error(GL_INVALID_OPERATION);
  if (width < 1 || height < 1) return;
  if (xoffset + width > (GLint)(t->texture.width >> level) || yoffset + height > (GLint)(t->texture.height >> level))
    return set_error(GL_INVALID_VALUE);
  void const *source = unpack_source(pixels);
  if (!source) return;
  NSUInteger rowBytes;
  void *converted = convert_pixels(source, width, height, format, type, &rowBytes);
  if (!rowBytes) return set_error(GL_OUT_OF_MEMORY);
  write_region(t, target_slice(target), level, xoffset, yoffset, width, height,
               converted ? converted : source, rowBytes, rowBytes * height);
  free(converted);
}

void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width,
                            GLsizei height, GLint border, GLsizei imageSize, void const *data)
{
  (void)border;
  GLMTexture *t = bound_texture(target);
  if (!t || !ensure_device()) return set_error(GL_INVALID_OPERATION);
  MTLPixelFormat pf = pixel_format(internalformat, 0);
  if (pf == MTLPixelFormatInvalid) return log_once("unsupported compressed texture format"), set_error(GL_INVALID_ENUM);
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

int GLMTL_TexImageIOSurface(GLenum target, void *iosurface, GLsizei width, GLsizei height)
{
  GLMTexture *t = bound_texture(target);
  if (!t || !iosurface || !ensure_device()) return -1;
  MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                width:width height:height
                                                                            mipmapped:NO];
  d.usage = MTLTextureUsageShaderRead;
  id<MTLTexture> texture = [gl.device newTextureWithDescriptor:d iosurface:(IOSurfaceRef)iosurface plane:0];
  if (!texture) return -1;
  t->texture = texture;
  t->swizzled = nil;
  t->width = width, t->height = height, t->levels = 1;
  t->internalFormat = GL_RGBA;
  return 0;
}

static MTLSamplerAddressMode address_mode(GLenum wrap)
{
  switch (wrap) {
    case GL_CLAMP_TO_EDGE: return MTLSamplerAddressModeClampToEdge;
    case GL_MIRRORED_REPEAT: return MTLSamplerAddressModeMirrorRepeat;
    default: return MTLSamplerAddressModeRepeat;
  }
}

static id<MTLSamplerState> sampler_for(GLMTexture *t)
{
  if (t->sampler) return t->sampler;
  BOOL mips = t->texture && t->texture.mipmapLevelCount > 1;
  GLenum min = t->minFilter;
  MTLSamplerMinMagFilter minFilter =
    (min == GL_NEAREST || min == GL_NEAREST_MIPMAP_NEAREST || min == GL_NEAREST_MIPMAP_LINEAR)
      ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
  MTLSamplerMipFilter mipFilter = !mips || min == GL_NEAREST || min == GL_LINEAR ? MTLSamplerMipFilterNotMipmapped
                                : (min == GL_NEAREST_MIPMAP_LINEAR || min == GL_LINEAR_MIPMAP_LINEAR)
                                  ? MTLSamplerMipFilterLinear : MTLSamplerMipFilterNearest;
  MTLSamplerMinMagFilter magFilter = t->magFilter == GL_NEAREST ? MTLSamplerMinMagFilterNearest
                                                                : MTLSamplerMinMagFilterLinear;
  NSNumber *key = @((uint32_t)minFilter | (uint32_t)magFilter << 2 | (uint32_t)mipFilter << 4 |
                    (uint32_t)address_mode(t->wrapS) << 8 | (uint32_t)address_mode(t->wrapT) << 12 |
                    (uint32_t)address_mode(t->wrapR) << 16);
  id<MTLSamplerState> s = gl.samplers[key];
  if (!s) {
    MTLSamplerDescriptor *d = [MTLSamplerDescriptor new];
    d.minFilter = minFilter;
    d.magFilter = magFilter;
    d.mipFilter = mipFilter;
    d.sAddressMode = address_mode(t->wrapS);
    d.tAddressMode = address_mode(t->wrapT);
    d.rAddressMode = address_mode(t->wrapR);
    s = [gl.device newSamplerStateWithDescriptor:d];
    gl.samplers[key] = s;
  }
  return t->sampler = s;
}

static MTLTextureSwizzle swizzle_channel(GLenum s)
{
  switch (s) {
    case GL_RED: return MTLTextureSwizzleRed;
    case GL_GREEN: return MTLTextureSwizzleGreen;
    case GL_BLUE: return MTLTextureSwizzleBlue;
    case GL_ALPHA: return MTLTextureSwizzleAlpha;
    case GL_ONE: return MTLTextureSwizzleOne;
    default: return MTLTextureSwizzleZero;
  }
}

static id<MTLTexture> sampled_texture(GLMTexture *t)
{
  if (t->swizzle[0] == GL_RED && t->swizzle[1] == GL_GREEN && t->swizzle[2] == GL_BLUE && t->swizzle[3] == GL_ALPHA)
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

static GLMTexture *fallback_texture(BOOL cube)
{
  GLMTexture *__strong *slot = cube ? &gl.blackCube : &gl.blackTexture;
  if (*slot) return *slot;
  GLMTexture *t = [GLMTexture new];
  uint32_t black = 0xff000000;
  MTLTextureDescriptor *d = cube
    ? [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm size:1 mipmapped:NO]
    : [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:1 height:1 mipmapped:NO];
  d.usage = MTLTextureUsageShaderRead;
  t->texture = [gl.device newTextureWithDescriptor:d];
  for (NSUInteger s = 0; s < (cube ? 6u : 1u); s++)
    [t->texture replaceRegion:MTLRegionMake2D(0, 0, 1, 1) mipmapLevel:0 slice:s withBytes:&black
                  bytesPerRow:4 bytesPerImage:4];
  t->minFilter = GL_NEAREST;
  t->width = t->height = t->levels = 1;
  return *slot = t;
}

#pragma mark - Framebuffers

void glGenFramebuffers(GLsizei n, GLuint *framebuffers) { gen(OBJ_FRAMEBUFFER, n, framebuffers, GLMFramebuffer.class); }

void glDeleteFramebuffers(GLsizei n, GLuint const *framebuffers)
{
  for (GLsizei i = 0; framebuffers && i < n; i++) {
    if (!framebuffers[i]) continue;
    if (framebuffers[i] == gl.drawName) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    if (framebuffers[i] == gl.readName) glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }
  forget(OBJ_FRAMEBUFFER, n, framebuffers);
}

void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
  if (!ensure_device()) return;
  GLMFramebuffer *fb = object(OBJ_FRAMEBUFFER, framebuffer);
  if (framebuffer && !fb) return set_error(GL_INVALID_OPERATION);
  if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER) {
    if (framebuffer != gl.drawName) flush_pending_clear();
    gl.drawFramebuffer = fb;
    gl.drawName = framebuffer;
  }
  if (target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER) {
    gl.readFramebuffer = fb;
    gl.readName = framebuffer;
  }
}

static GLMFramebuffer *framebuffer_for(GLenum target)
{
  return target == GL_READ_FRAMEBUFFER ? gl.readFramebuffer : gl.drawFramebuffer;
}

static void attach(GLenum target, GLenum attachment, GLMTexture *t)
{
  GLMFramebuffer *fb = framebuffer_for(target);
  if (!fb) return set_error(GL_INVALID_OPERATION);
  if (gl.encoderOpen && gl.encoderFramebuffer == fb) end_pass();
  switch (attachment) {
    case GL_COLOR_ATTACHMENT0: fb->color = t; break;
    case GL_DEPTH_ATTACHMENT: fb->depth = t; break;
    case GL_STENCIL_ATTACHMENT: fb->stencil = t; break;
    case GL_DEPTH_STENCIL_ATTACHMENT: fb->depth = t, fb->stencil = nil; break;
    default: set_error(GL_INVALID_ENUM);
  }
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
  (void)textarget, (void)level;
  attach(target, attachment, object(OBJ_TEXTURE, texture));
}

void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
  (void)renderbuffertarget;
  attach(target, attachment, object(OBJ_RENDERBUFFER, renderbuffer));
}

GLenum glCheckFramebufferStatus(GLenum target)
{
  GLMFramebuffer *fb = framebuffer_for(target);
  if (!fb) return GL_FRAMEBUFFER_COMPLETE;
  if (!fb->color && !fb->depth) return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
  if ((fb->color && !fb->color->texture) || (fb->depth && !fb->depth->texture))
    return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
  return GL_FRAMEBUFFER_COMPLETE;
}

void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
  targets_t t = framebuffer_targets(framebuffer_for(target));
  if (!params) return;
  *params = 0;
  BOOL depth = attachment == GL_DEPTH || attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT;
  if (pname == GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE && depth && t.depth) *params = 24;
  if (pname == GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING && !depth && t.color) {
    MTLPixelFormat f = t.color.pixelFormat;
    *params = f == MTLPixelFormatRGBA8Unorm_sRGB || f == MTLPixelFormatBGRA8Unorm_sRGB ? GL_SRGB : GL_LINEAR;
  }
}

void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
  gen(OBJ_RENDERBUFFER, n, renderbuffers, GLMTexture.class);
  for (GLsizei i = 0; i < n; i++) ((GLMTexture *)object(OBJ_RENDERBUFFER, renderbuffers[i]))->renderbuffer = YES;
}

void glDeleteRenderbuffers(GLsizei n, GLuint const *renderbuffers)
{
  for (GLsizei i = 0; renderbuffers && i < n; i++)
    if (renderbuffers[i] && renderbuffers[i] == gl.renderbufferName) gl.renderbuffer = nil, gl.renderbufferName = 0;
  forget(OBJ_RENDERBUFFER, n, renderbuffers);
}

void glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
  (void)target;
  gl.renderbuffer = object(OBJ_RENDERBUFFER, renderbuffer);
  gl.renderbufferName = renderbuffer;
}

void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
  (void)target;
  if (!gl.renderbuffer) return set_error(GL_INVALID_OPERATION);
  MTLPixelFormat pf = pixel_format(internalformat, 0);
  if (pf == MTLPixelFormatInvalid) return set_error(GL_INVALID_ENUM);
  gl.renderbuffer->maxLevel = 0;
  allocate(gl.renderbuffer, GL_TEXTURE_2D, pf, width, height, internalformat);
}

#pragma mark - Blits

// A full-screen-quad copy used by glBlitFramebuffer and glCopyTexSubImage2D:
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
  id<MTLRenderPipelineState> pso = gl.blitPipelines[key];
  if (pso) return pso;
  NSError *error = nil;
  if (!gl.blitLibrary) gl.blitLibrary = [gl.device newLibraryWithSource:kBlitSource options:nil error:&error];
  if (!gl.blitLibrary) return axLog("Metal: blit shaders: %s", error.localizedDescription.UTF8String), nil;
  MTLRenderPipelineDescriptor *d = [MTLRenderPipelineDescriptor new];
  d.vertexFunction = [gl.blitLibrary newFunctionWithName:@"orca_blit_vertex"];
  d.fragmentFunction = [gl.blitLibrary newFunctionWithName:depth ? @"orca_blit_depth" : @"orca_blit_color"];
  if (depth) {
    d.depthAttachmentPixelFormat = format;
    if (has_stencil(format)) d.stencilAttachmentPixelFormat = format;
  } else {
    d.colorAttachments[0].pixelFormat = format;
  }
  pso = [gl.device newRenderPipelineStateWithDescriptor:d error:&error];
  if (!pso) return axLog("Metal: blit pipeline: %s", error.localizedDescription.UTF8String), nil;
  return gl.blitPipelines[key] = pso;
}

// Draws the OpenGL window rectangle (sx0, sy0)-(sx1, sy1) of `src` over
// (dx0, dy0)-(dx1, dy1) of `dst`. Both rectangles are in OpenGL's bottom-up
// coordinates; `flipped` says which textures store rows in OpenGL order.
static void draw_quad(id<MTLTexture> src, BOOL srcFlipped, id<MTLTexture> dst, BOOL dstFlipped, BOOL depth,
                      GLint sx0, GLint sy0, GLint sx1, GLint sy1,
                      GLint dx0, GLint dy0, GLint dx1, GLint dy1, BOOL linear)
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
  // Texture v of an OpenGL row coordinate, and the destination's NDC y.
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
  if (depth) [enc setDepthStencilState:gl.depthAlways];
  [enc setVertexBytes:quad length:sizeof(quad) atIndex:0];
  [enc setFragmentTexture:src atIndex:0];
  MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
  sd.minFilter = sd.magFilter = linear && !depth ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
  sd.sAddressMode = sd.tAddressMode = MTLSamplerAddressModeClampToEdge;
  NSNumber *key = @(linear && !depth ? 0x80000001u : 0x80000000u);
  if (!gl.samplers[key]) gl.samplers[key] = [gl.device newSamplerStateWithDescriptor:sd];
  [enc setFragmentSamplerState:gl.samplers[key] atIndex:0];
  [enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
  [enc endEncoding];
}

void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0,
                       GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
  if (!(mask & GL_COLOR_BUFFER_BIT)) return;
  flush_pending_clear();
  targets_t src = framebuffer_targets(gl.readFramebuffer);
  targets_t dst = framebuffer_targets(gl.drawFramebuffer);
  if (!src.color || !dst.color) return set_error(GL_INVALID_OPERATION);
  draw_quad(src.color, src.flipped, dst.color, dst.flipped, NO, srcX0, srcY0, srcX1, srcY1,
            dstX0, dstY0, dstX1, dstY1, filter == GL_LINEAR);
  use(src.colorObject);
  use(dst.colorObject);
}

void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
                         GLsizei width, GLsizei height)
{
  (void)level;
  GLMTexture *t = bound_texture(target);
  if (!t || !t->texture) return set_error(GL_INVALID_OPERATION);
  flush_pending_clear();
  BOOL depth = is_depth_format(t->texture.pixelFormat);
  targets_t src = framebuffer_targets(gl.readFramebuffer);
  id<MTLTexture> from = depth ? src.depth : src.color;
  if (!from) return set_error(GL_INVALID_OPERATION);
  draw_quad(from, src.flipped, t->texture, YES, depth, x, y, x + width, y + height,
            xoffset, yoffset, xoffset + width, yoffset + height, NO);
  use(t);
  use(src.colorObject);
  use(src.depthObject);
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
  if (format != GL_RGBA || type != GL_UNSIGNED_BYTE || !pixels) return set_error(GL_INVALID_ENUM);
  flush_pending_clear();
  targets_t src = framebuffer_targets(gl.readFramebuffer);
  if (!src.color || width < 1 || height < 1) return set_error(GL_INVALID_OPERATION);
  // Metal rejects a copy outside the texture; OpenGL leaves those pixels undefined.
  if (x < 0 || y < 0 || x + width > (GLint)src.color.width || y + height > (GLint)src.color.height)
    return set_error(GL_INVALID_VALUE);
  GLint top = src.flipped ? y : (GLint)src.color.height - (y + height);
  NSUInteger rowBytes = (NSUInteger)width * 4;
  id<MTLBuffer> buffer = [gl.device newBufferWithLength:rowBytes * height options:MTLResourceStorageModeShared];
  end_pass();
  id<MTLBlitCommandEncoder> blit = [command_buffer() blitCommandEncoder];
  [blit copyFromTexture:src.color sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(x, top, 0)
             sourceSize:MTLSizeMake(width, height, 1) toBuffer:buffer destinationOffset:0
 destinationBytesPerRow:rowBytes destinationBytesPerImage:0];
  [blit endEncoding];
  commit(YES);
  BOOL bgra = src.color.pixelFormat == MTLPixelFormatBGRA8Unorm || src.color.pixelFormat == MTLPixelFormatBGRA8Unorm_sRGB;
  for (GLsizei r = 0; r < height; r++) {
    uint8_t const *row = (uint8_t const *)buffer.contents + rowBytes * (src.flipped ? r : height - 1 - r);
    uint8_t *out = (uint8_t *)pixels + rowBytes * r;
    memcpy(out, row, rowBytes);
    for (GLsizei c = 0; bgra && c < width; c++) {
      uint8_t b = out[c * 4];
      out[c * 4] = out[c * 4 + 2], out[c * 4 + 2] = b;
    }
  }
}

#pragma mark - Shaders and programs

GLuint glCreateShader(GLenum type)
{
  GLuint name;
  gen(OBJ_SHADER, 1, &name, GLMShader.class);
  GLMShader *s = object(OBJ_SHADER, name);
  if (!s) return 0;
  s->type = type;
  return name;
}

void glDeleteShader(GLuint shader) { forget(OBJ_SHADER, 1, &shader); }

void glShaderSource(GLuint shader, GLsizei count, GLchar const *const *string, GLint const *length)
{
  GLMShader *s = object(OBJ_SHADER, shader);
  if (!s) return set_error(GL_INVALID_VALUE);
  NSMutableString *text = [NSMutableString new];
  for (GLsizei i = 0; i < count; i++) {
    NSString *part = length && length[i] >= 0
      ? [[NSString alloc] initWithBytes:string[i] length:length[i] encoding:NSUTF8StringEncoding]
      : [NSString stringWithUTF8String:string[i]];
    if (part) [text appendString:part];
  }
  s->source = text;
}

// Translation needs both stages, so it happens when the program links.
void glCompileShader(GLuint shader) { (void)shader; }

void glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
  GLMShader *s = object(OBJ_SHADER, shader);
  if (!params) return;
  if (pname == GL_COMPILE_STATUS) *params = s && s->source ? GL_TRUE : GL_FALSE;
  else if (pname == GL_INFO_LOG_LENGTH) *params = 0;
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
  (void)shader;
  if (length) *length = 0;
  if (infoLog && bufSize > 0) *infoLog = 0;
}

GLuint glCreateProgram(void)
{
  GLuint name;
  gen(OBJ_PROGRAM, 1, &name, GLMProgram.class);
  return object(OBJ_PROGRAM, name) ? name : 0;
}

void glDeleteProgram(GLuint program)
{
  if (program && program == gl.programName) gl.program = nil, gl.programName = 0;
  forget(OBJ_PROGRAM, 1, &program);
}

void glAttachShader(GLuint program, GLuint shader)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  GLMShader *s = object(OBJ_SHADER, shader);
  if (!p || !s) return set_error(GL_INVALID_VALUE);
  if (s->type == GL_VERTEX_SHADER) p->vs = s;
  else p->fs = s;
}

void glDetachShader(GLuint program, GLuint shader)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  GLMShader *s = object(OBJ_SHADER, shader);
  if (p && p->vs == s) p->vs = nil;
  if (p && p->fs == s) p->fs = nil;
}

static id<MTLFunction> compile_stage(GLMProgram *p, int stage, NSMutableString *log)
{
  NSError *error = nil;
  MTLCompileOptions *options = [MTLCompileOptions new];
  options.languageVersion = MTLLanguageVersion2_1;
  id<MTLLibrary> library = [gl.device newLibraryWithSource:@(p->refl->source[stage]) options:options error:&error];
  if (!library) {
    [log appendFormat:@"%s: %@\n", stage == GLSL_MSL_VERTEX ? "vertex" : "fragment", error.localizedDescription];
    return nil;
  }
  return [library newFunctionWithName:@(p->refl->entry[stage])];
}

void glSetMSLReflection(GLuint program, struct glsl_msl_program const *refl)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  if (p) p->preset_refl = refl;
}

void glLinkProgram(GLuint program)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  if (!p || !ensure_device()) return set_error(GL_INVALID_VALUE);
  [p reset];
  if (!p->pipelines) p->pipelines = [NSMutableDictionary new];
  static uint32_t serial;
  p->serial = ++serial;
  NSMutableString *log = [NSMutableString string];
  if (!p->preset_refl) {
    p->log = @"no MSL reflection: use glSetMSLReflection before glLinkProgram, or add lang=\"msl\" to the Shader XML";
    return;
  }
  p->refl = p->preset_refl;
  int ok = YES;
  for (int s = 0; ok && s < GLSL_MSL_STAGES; s++) {
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
    if (!strcmp(p->refl->uniforms[i].name, GLSL_MSL_FLIP_Y)) p->flipY = (int32_t)i;
  }
  p->log = log;
  p->linked = ok;
  if (!ok) [p reset], p->log = log;
}

void glUseProgram(GLuint program)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  if (program && (!p || !p->linked)) return set_error(GL_INVALID_OPERATION);
  gl.program = p;
  gl.programName = program;
}

static BOOL hidden(struct glsl_msl_uniform const *u) { return !strcmp(u->name, GLSL_MSL_FLIP_Y); }

void glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  if (!p || !params) return set_error(GL_INVALID_VALUE);
  switch (pname) {
    case GL_LINK_STATUS: *params = p->linked; break;
    case GL_INFO_LOG_LENGTH: *params = p->log.length ? (GLint)strlen(p->log.UTF8String) + 1 : 0; break;
    case GL_ACTIVE_UNIFORMS:
      *params = 0;
      for (uint32_t i = 0; p->refl && i < p->refl->numUniforms; i++) *params += !hidden(&p->refl->uniforms[i]);
      break;
    case GL_ACTIVE_ATTRIBUTES: *params = p->refl ? (GLint)p->refl->numAttributes : 0; break;
    default: set_error(GL_INVALID_ENUM);
  }
}

void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  char const *text = p && p->log ? p->log.UTF8String : "";
  GLsizei n = bufSize > 0 ? (GLsizei)MIN(strlen(text), (size_t)bufSize - 1) : 0;
  if (infoLog && bufSize > 0) memcpy(infoLog, text, n), infoLog[n] = 0;
  if (length) *length = n;
}

void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size,
                        GLenum *type, GLchar *name)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  for (uint32_t i = 0, visible = 0; p && p->refl && i < p->refl->numUniforms; i++) {
    struct glsl_msl_uniform const *u = &p->refl->uniforms[i];
    if (hidden(u) || visible++ != index) continue;
    char text[GLSL_MSL_NAME + 4];
    snprintf(text, sizeof(text), u->arraySize > 1 ? "%s[0]" : "%s", u->name);
    GLsizei n = bufSize > 0 ? (GLsizei)MIN(strlen(text), (size_t)bufSize - 1) : 0;
    if (name && bufSize > 0) memcpy(name, text, n), name[n] = 0;
    if (length) *length = n;
    if (size) *size = (GLint)u->arraySize;
    if (type) *type = u->gltype;
    return;
  }
  set_error(GL_INVALID_VALUE);
}

// A location is the uniform's index with the array element above bit 8.
GLint glGetUniformLocation(GLuint program, GLchar const *name)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  if (!p || !p->refl || !name) return -1;
  char base[GLSL_MSL_NAME];
  int element = 0;
  snprintf(base, sizeof(base), "%s", name);
  char *bracket = strchr(base, '[');
  if (bracket) element = atoi(bracket + 1), *bracket = 0;
  for (uint32_t i = 0; i < p->refl->numUniforms; i++) {
    struct glsl_msl_uniform const *u = &p->refl->uniforms[i];
    if (hidden(u) || strcmp(u->name, base)) continue;
    return element < 0 || (uint32_t)element >= u->arraySize ? -1 : (GLint)(i | (uint32_t)element << 8);
  }
  return -1;
}

GLint glGetAttribLocation(GLuint program, GLchar const *name)
{
  GLMProgram *p = object(OBJ_PROGRAM, program);
  for (uint32_t i = 0; p && p->refl && name && i < p->refl->numAttributes; i++)
    if (!strcmp(p->refl->attributes[i].name, name)) return (GLint)p->refl->attributes[i].location;
  return -1;
}

static int components(GLenum type, int *columns, BOOL *integer)
{
  *columns = 1;
  *integer = NO;
  switch (type) {
    case GL_FLOAT: return 1;
    case GL_FLOAT_VEC2: return 2;
    case GL_FLOAT_VEC3: return 3;
    case GL_FLOAT_VEC4: return 4;
    case GL_INT: case GL_BOOL: case GL_UNSIGNED_INT: *integer = YES; return 1;
    case GL_INT_VEC2: case GL_BOOL_VEC2: case GL_UNSIGNED_INT_VEC2: *integer = YES; return 2;
    case GL_INT_VEC3: case GL_BOOL_VEC3: case GL_UNSIGNED_INT_VEC3: *integer = YES; return 3;
    case GL_INT_VEC4: case GL_BOOL_VEC4: case GL_UNSIGNED_INT_VEC4: *integer = YES; return 4;
    case GL_FLOAT_MAT2: *columns = 2; return 2;
    case GL_FLOAT_MAT3: *columns = 3; return 3;
    case GL_FLOAT_MAT4: *columns = 4; return 4;
  }
  return 0;
}

static struct glsl_msl_uniform const *uniform_at(GLint location, uint32_t *index, uint32_t *element)
{
  GLMProgram *p = gl.program;
  if (location < 0 || !p || !p->refl) return NULL;
  *index = (uint32_t)location & 0xff;
  *element = (uint32_t)location >> 8;
  if (*index >= p->refl->numUniforms) return set_error(GL_INVALID_OPERATION), NULL;
  return &p->refl->uniforms[*index];
}

// Writes `count` elements of `n` values each; values are floats or ints as
// the caller passes them and are converted to the uniform's type.
static void write_uniform(GLint location, GLsizei count, int n, float const *f, GLint const *i)
{
  uint32_t index, element;
  struct glsl_msl_uniform const *u = uniform_at(location, &index, &element);
  if (!u) return;
  if (u->gltype == GL_SAMPLER_2D || u->gltype == GL_SAMPLER_CUBE || u->gltype == GL_SAMPLER_3D ||
      u->gltype == GL_SAMPLER_2D_SHADOW) {
    gl.program->units[index] = i ? i[0] : (GLint)f[0];
    return;
  }
  int columns;
  BOOL integer;
  int size = components(u->gltype, &columns, &integer);
  if (!size || columns > 1) return set_error(GL_INVALID_OPERATION);
  for (int s = 0; s < GLSL_MSL_STAGES; s++) {
    if (u->offset[s] < 0) continue;
    for (GLsizei e = 0; e < count && element + e < u->arraySize; e++) {
      uint32_t at = (uint32_t)u->offset[s] + (element + e) * u->arrayStride[s];
      if (at + size * 4 > gl.program->refl->blockSize[s]) break;
      uint8_t *dst = gl.program->data[s] + at;
      for (int c = 0; c < size && c < n; c++) {
        int k = e * n + c;
        if (integer) {
          int32_t v = i ? i[k] : (int32_t)f[k];
          if (u->gltype >= GL_BOOL && u->gltype <= GL_BOOL_VEC4) v = v != 0;
          memcpy(dst + c * 4, &v, 4);
        } else {
          float v = f ? f[k] : (float)i[k];
          memcpy(dst + c * 4, &v, 4);
        }
      }
    }
  }
}

static void write_matrix(GLint location, GLsizei count, int n, GLboolean transpose, GLfloat const *value)
{
  uint32_t index, element;
  struct glsl_msl_uniform const *u = uniform_at(location, &index, &element);
  if (!u) return;
  int columns;
  BOOL integer;
  if (components(u->gltype, &columns, &integer) != n || columns != n) return set_error(GL_INVALID_OPERATION);
  for (int s = 0; s < GLSL_MSL_STAGES; s++) {
    if (u->offset[s] < 0) continue;
    for (GLsizei e = 0; e < count && element + e < u->arraySize; e++) {
      uint32_t at = (uint32_t)u->offset[s] + (element + e) * u->arrayStride[s];
      if (at + (n - 1) * u->matrixStride[s] + n * 4 > gl.program->refl->blockSize[s]) break;
      float const *m = value + e * n * n;
      for (int c = 0; c < n; c++) {
        float *col = (float *)(gl.program->data[s] + at + c * u->matrixStride[s]);
        for (int r = 0; r < n; r++) col[r] = transpose ? m[r * n + c] : m[c * n + r];
      }
    }
  }
}

void glUniform1f(GLint location, GLfloat v0) { write_uniform(location, 1, 1, &v0, NULL); }
void glUniform2f(GLint location, GLfloat v0, GLfloat v1) { GLfloat v[] = { v0, v1 }; write_uniform(location, 1, 2, v, NULL); }
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { GLfloat v[] = { v0, v1, v2 }; write_uniform(location, 1, 3, v, NULL); }
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { GLfloat v[] = { v0, v1, v2, v3 }; write_uniform(location, 1, 4, v, NULL); }
void glUniform1i(GLint location, GLint v0) { write_uniform(location, 1, 1, NULL, &v0); }
void glUniform2fv(GLint location, GLsizei count, GLfloat const *value) { write_uniform(location, count, 2, value, NULL); }
void glUniform3fv(GLint location, GLsizei count, GLfloat const *value) { write_uniform(location, count, 3, value, NULL); }
void glUniform4fv(GLint location, GLsizei count, GLfloat const *value) { write_uniform(location, count, 4, value, NULL); }
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, GLfloat const *value) { write_matrix(location, count, 3, transpose, value); }
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, GLfloat const *value) { write_matrix(location, count, 4, transpose, value); }

#pragma mark - Fixed-function state

void glEnable(GLenum cap)
{
  switch (cap) {
    case GL_BLEND: gl.blend = YES; break;
    case GL_DEPTH_TEST: gl.depthTest = YES; break;
    case GL_STENCIL_TEST: gl.stencilTest = YES; break;
    case GL_CULL_FACE: gl.cull = YES; break;
    case GL_FRAMEBUFFER_SRGB:
      gl.srgb = YES;
      if (ensure_device() && current_layer()) gl.layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
      break;
  }
}

void glDisable(GLenum cap)
{
  switch (cap) {
    case GL_BLEND: gl.blend = NO; break;
    case GL_DEPTH_TEST: gl.depthTest = NO; break;
    case GL_STENCIL_TEST: gl.stencilTest = NO; break;
    case GL_CULL_FACE: gl.cull = NO; break;
    case GL_FRAMEBUFFER_SRGB:
      gl.srgb = NO;
      if (gl.layer) gl.layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
      break;
  }
}

void glBlendFunc(GLenum sfactor, GLenum dfactor) { gl.blendSrc = sfactor, gl.blendDst = dfactor; }

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
  gl.colorMask = (red ? MTLColorWriteMaskRed : 0) | (green ? MTLColorWriteMaskGreen : 0) |
                 (blue ? MTLColorWriteMaskBlue : 0) | (alpha ? MTLColorWriteMaskAlpha : 0);
}

void glCullFace(GLenum mode) { gl.cullFace = mode; }
void glDepthFunc(GLenum func) { gl.depthFunc = func; }
void glDepthMask(GLboolean flag) { gl.depthMask = flag; }
void glStencilFunc(GLenum func, GLint ref, GLuint mask) { gl.stencilFunc = func, gl.stencilRef = ref, gl.stencilReadMask = mask; }
void glStencilMask(GLuint mask) { gl.stencilWriteMask = mask; }
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) { gl.stencilFail = fail, gl.stencilZFail = zfail, gl.stencilZPass = zpass; }
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
  gl.viewport[0] = x, gl.viewport[1] = y, gl.viewport[2] = width, gl.viewport[3] = height;
}
// The renderer never enables GL_SCISSOR_TEST, so the rectangle has no effect.
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) { (void)x, (void)y, (void)width, (void)height; }

void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  gl.clearColor[0] = r, gl.clearColor[1] = g, gl.clearColor[2] = b, gl.clearColor[3] = a;
}
void glClearDepthf(GLfloat d) { gl.clearDepth = d; }
void glClearStencil(GLint s) { gl.clearStencil = s; }

// Clears the whole attachment: the renderer never enables the scissor test.
void glClear(GLbitfield mask)
{
  if (!ensure_device()) return;
  if (gl.encoderOpen && gl.encoderFramebuffer == gl.drawFramebuffer && gl.encoderDefault == !gl.drawFramebuffer)
    end_pass();
  gl.pendingClear |= mask & (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  if (mask & GL_COLOR_BUFFER_BIT) memcpy(gl.pendingColor, gl.clearColor, sizeof(gl.pendingColor));
  if (mask & GL_DEPTH_BUFFER_BIT) gl.pendingDepth = gl.clearDepth;
  if (mask & GL_STENCIL_BUFFER_BIT) gl.pendingStencil = gl.clearStencil;
}

void glGetIntegerv(GLenum pname, GLint *data)
{
  if (!data) return;
  switch (pname) {
    case GL_VIEWPORT: memcpy(data, gl.viewport, sizeof(gl.viewport)); return;
    case GL_FRAMEBUFFER_BINDING: *data = (GLint)gl.drawName; return;
    case GL_READ_FRAMEBUFFER_BINDING: *data = (GLint)gl.readName; return;
    case GL_RENDERBUFFER_BINDING: *data = (GLint)gl.renderbufferName; return;
    case GL_SAMPLES: *data = 0; return;
    case GL_NUM_EXTENSIONS: *data = 0; return;
    case GL_ACTIVE_TEXTURE: *data = GL_TEXTURE0 + gl.activeUnit; return;
    case GL_UNPACK_ALIGNMENT: *data = gl.unpackAlignment; return;
    case GL_TEXTURE_BINDING_2D: {
      GLMTexture *t = gl.units[gl.activeUnit].bound2D;
      *data = 0;
      for (NSNumber *name in gl.objects[OBJ_TEXTURE])
        if (gl.objects[OBJ_TEXTURE][name] == t) *data = name.intValue;
      return;
    }
    case GL_MAX_TEXTURE_SIZE:
      *data = 8192;
      if (ensure_device()) {
#if TARGET_OS_OSX
        *data = 16384;
#else
        if (@available(iOS 13.0, *)) if ([gl.device supportsFamily:MTLGPUFamilyApple3]) *data = 16384;
#endif
      }
      return;
  }
  *data = 0;
  set_error(GL_INVALID_ENUM);
}

GLubyte const *glGetString(GLenum name)
{
  switch (name) {
    case GL_VERSION: return (GLubyte const *)"OpenGL ES 3.0 (ORCA on Metal)";
    case GL_VENDOR: return (GLubyte const *)"ORCA";
    case GL_RENDERER: return (GLubyte const *)"Metal";
    case GL_EXTENSIONS: return (GLubyte const *)"";
  }
  return NULL;
}

GLubyte const *glGetStringi(GLenum name, GLuint index) { (void)name, (void)index; return NULL; }

GLenum glGetError(void)
{
  GLenum error = gl.error;
  gl.error = GL_NO_ERROR;
  return error;
}

#pragma mark - Drawing

static MTLCompareFunction compare_function(GLenum f)
{
  switch (f) {
    case GL_NEVER: return MTLCompareFunctionNever;
    case GL_LESS: return MTLCompareFunctionLess;
    case GL_EQUAL: return MTLCompareFunctionEqual;
    case GL_LEQUAL: return MTLCompareFunctionLessEqual;
    case GL_GREATER: return MTLCompareFunctionGreater;
    case GL_NOTEQUAL: return MTLCompareFunctionNotEqual;
    case GL_GEQUAL: return MTLCompareFunctionGreaterEqual;
    default: return MTLCompareFunctionAlways;
  }
}

static MTLStencilOperation stencil_operation(GLenum op)
{
  switch (op) {
    case GL_ZERO: return MTLStencilOperationZero;
    case GL_REPLACE: return MTLStencilOperationReplace;
    case GL_INCR: return MTLStencilOperationIncrementClamp;
    case GL_DECR: return MTLStencilOperationDecrementClamp;
    case GL_INVERT: return MTLStencilOperationInvert;
    case GL_INCR_WRAP: return MTLStencilOperationIncrementWrap;
    case GL_DECR_WRAP: return MTLStencilOperationDecrementWrap;
    default: return MTLStencilOperationKeep;
  }
}

static MTLBlendFactor blend_factor(GLenum f)
{
  switch (f) {
    case GL_ZERO: return MTLBlendFactorZero;
    case GL_ONE: return MTLBlendFactorOne;
    case GL_SRC_COLOR: return MTLBlendFactorSourceColor;
    case GL_ONE_MINUS_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
    case GL_SRC_ALPHA: return MTLBlendFactorSourceAlpha;
    case GL_ONE_MINUS_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
    case GL_DST_ALPHA: return MTLBlendFactorDestinationAlpha;
    case GL_ONE_MINUS_DST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
    case GL_DST_COLOR: return MTLBlendFactorDestinationColor;
    case GL_ONE_MINUS_DST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
    default: return MTLBlendFactorOne;
  }
}

static MTLVertexFormat vertex_format(GLint size, GLenum type, GLboolean normalized)
{
  static MTLVertexFormat const floats[] = { MTLVertexFormatFloat, MTLVertexFormatFloat2, MTLVertexFormatFloat3, MTLVertexFormatFloat4 };
  static MTLVertexFormat const halves[] = { MTLVertexFormatHalf, MTLVertexFormatHalf2, MTLVertexFormatHalf3, MTLVertexFormatHalf4 };
  static MTLVertexFormat const ubytesN[] = { MTLVertexFormatUCharNormalized, MTLVertexFormatUChar2Normalized, MTLVertexFormatUChar3Normalized, MTLVertexFormatUChar4Normalized };
  static MTLVertexFormat const bytesN[] = { MTLVertexFormatCharNormalized, MTLVertexFormatChar2Normalized, MTLVertexFormatChar3Normalized, MTLVertexFormatChar4Normalized };
  switch (type) {
    case GL_FLOAT: return floats[size - 1];
    case GL_HALF_FLOAT: return halves[size - 1];
    case GL_UNSIGNED_BYTE:
      // Metal converts only normalized integers to a float input.
      if (!normalized) log_once("unnormalized byte attribute read as normalized");
      return ubytesN[size - 1];
    case GL_BYTE:
      if (!normalized) log_once("unnormalized byte attribute read as normalized");
      return bytesN[size - 1];
  }
  return MTLVertexFormatInvalid;
}

static GLsizei type_size(GLenum type)
{
  return type == GL_FLOAT ? 4 : type == GL_HALF_FLOAT ? 2 : 1;
}

static NSUInteger source_stride(attrib_t const *a)
{
  return a->stride ? (NSUInteger)a->stride : (NSUInteger)(a->size * type_size(a->type));
}

// Tightly packed 3-byte or 6-byte elements (UINT8 or FLOAT16 vectors from a
// mesh file) are copied once into elements padded to a multiple of 4 bytes.
static id<MTLBuffer> padded_buffer(attrib_t const *a)
{
  GLMBuffer *b = a->buffer;
  NSUInteger stride = source_stride(a), element = (NSUInteger)(a->size * type_size(a->type));
  if (b->padded && b->paddedStride == stride && b->paddedOffset == a->offset && b->paddedElement == element)
    return b->padded;
  NSUInteger count = b->size > a->offset ? (b->size - a->offset + stride - 1) / stride : 0;
  NSUInteger padded = (element + 3) & ~(NSUInteger)3;
  b->padded = [gl.device newBufferWithLength:MAX(count * padded, 4) options:MTLResourceStorageModeShared];
  uint8_t const *src = (uint8_t const *)b->buffer.contents + a->offset;
  uint8_t *dst = b->padded.contents;
  for (NSUInteger i = 0; i < count; i++)
    memcpy(dst + i * padded, src + i * stride, MIN(element, b->size - a->offset - i * stride));
  b->paddedStride = stride, b->paddedOffset = a->offset, b->paddedElement = element;
  return b->padded;
}

static id<MTLRenderPipelineState> pipeline_for(GLMProgram *p, targets_t const *t, pipeline_key_t *key)
{
  if (gl.lastPipeline && !memcmp(&gl.lastKey, key, sizeof(*key))) return gl.lastPipeline;
  NSData *data = [NSData dataWithBytes:key length:sizeof(*key)];
  id<MTLRenderPipelineState> pso = p->pipelines[data];
  if (!pso) {
    MTLRenderPipelineDescriptor *d = [MTLRenderPipelineDescriptor new];
    d.vertexFunction = p->functions[GLSL_MSL_VERTEX];
    d.fragmentFunction = p->functions[GLSL_MSL_FRAGMENT];
    MTLVertexDescriptor *vd = [MTLVertexDescriptor vertexDescriptor];
    for (uint32_t i = 0; i < p->refl->numAttributes; i++) {
      uint32_t l = p->refl->attributes[i].location;
      NSUInteger index = GLSL_MSL_VERTEX_BUFFER_BASE + l;
      vd.attributes[l].format = key->attrs[l].format;
      vd.attributes[l].offset = 0;
      vd.attributes[l].bufferIndex = index;
      if (key->attrs[l].stride) {
        vd.layouts[index].stride = key->attrs[l].stride;
      } else {  // a disabled array reads OpenGL's default (0, 0, 0, 1)
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
      c.sourceRGBBlendFactor = c.sourceAlphaBlendFactor = blend_factor(gl.blendSrc);
      c.destinationRGBBlendFactor = c.destinationAlphaBlendFactor = blend_factor(gl.blendDst);
    }
    if (t->depth) d.depthAttachmentPixelFormat = t->depth.pixelFormat;
    if (t->stencil) d.stencilAttachmentPixelFormat = t->stencil.pixelFormat;
    NSError *error = nil;
    pso = [gl.device newRenderPipelineStateWithDescriptor:d error:&error];
    if (!pso) {
      axLog("Metal: pipeline: %s", error.localizedDescription.UTF8String);
      return nil;
    }
    p->pipelines[data] = pso;
  }
  gl.lastPipeline = pso;
  gl.lastKey = *key;
  return pso;
}

static id<MTLDepthStencilState> depth_state_for(targets_t const *t)
{
  if (!t->depth && !t->stencil) return gl.depthDisabled;
  struct {
    uint8_t test, func, write, stencil, sfunc, fail, zfail, zpass;
    uint32_t read, mask;
  } key = { 0 };
  key.test = gl.depthTest && t->depth;
  key.func = key.test ? (uint8_t)compare_function(gl.depthFunc) : MTLCompareFunctionAlways;
  key.write = key.test && gl.depthMask;  // OpenGL writes no depth with the test off
  key.stencil = gl.stencilTest && t->stencil;
  if (key.stencil) {
    key.sfunc = (uint8_t)compare_function(gl.stencilFunc);
    key.fail = (uint8_t)stencil_operation(gl.stencilFail);
    key.zfail = (uint8_t)stencil_operation(gl.stencilZFail);
    key.zpass = (uint8_t)stencil_operation(gl.stencilZPass);
    key.read = gl.stencilReadMask & 0xff;
    key.mask = gl.stencilWriteMask & 0xff;
  }
  NSData *data = [NSData dataWithBytes:&key length:sizeof(key)];
  id<MTLDepthStencilState> state = gl.depthStates[data];
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
  return gl.depthStates[data] = [gl.device newDepthStencilStateWithDescriptor:d];
}

static void bind_samplers(GLMProgram *p)
{
  for (uint32_t i = 0; i < p->refl->numUniforms; i++) {
    struct glsl_msl_uniform const *u = &p->refl->uniforms[i];
    if (u->texture[GLSL_MSL_VERTEX] < 0 && u->texture[GLSL_MSL_FRAGMENT] < 0) continue;
    GLint unit = p->units[i];
    BOOL cube = u->gltype == GL_SAMPLER_CUBE;
    GLMTexture *t = unit >= 0 && unit < MAX_TEXTURE_UNITS
                  ? (cube ? gl.units[unit].boundCube : gl.units[unit].bound2D) : nil;
    if (!t || !t->texture) t = fallback_texture(cube);
    use(t);
    id<MTLTexture> texture = sampled_texture(t);
    id<MTLSamplerState> sampler = sampler_for(t);
    if (u->texture[GLSL_MSL_VERTEX] >= 0) {
      [gl.encoder setVertexTexture:texture atIndex:u->texture[GLSL_MSL_VERTEX]];
      [gl.encoder setVertexSamplerState:sampler atIndex:u->sampler[GLSL_MSL_VERTEX]];
    }
    if (u->texture[GLSL_MSL_FRAGMENT] >= 0) {
      [gl.encoder setFragmentTexture:texture atIndex:u->texture[GLSL_MSL_FRAGMENT]];
      [gl.encoder setFragmentSamplerState:sampler atIndex:u->sampler[GLSL_MSL_FRAGMENT]];
    }
  }
}

// Encodes everything but the draw call itself; NO skips the draw.
static BOOL prepare_draw(void)
{
  GLMProgram *p = gl.program;
  if (!p || !p->linked || !ensure_device()) return set_error(GL_INVALID_OPERATION), NO;
  if (gl.cull && gl.cullFace == GL_FRONT_AND_BACK) return NO;
  if (!begin_pass()) return NO;
  targets_t t = framebuffer_targets(gl.drawFramebuffer);

  pipeline_key_t key;
  memset(&key, 0, sizeof(key));
  key.program = p->serial;
  key.color = t.color ? (uint32_t)t.color.pixelFormat : 0;
  key.depth = t.depth ? (uint32_t)t.depth.pixelFormat : 0;
  key.stencil = t.stencil ? (uint32_t)t.stencil.pixelFormat : 0;
  key.blend = gl.blend;
  key.src = gl.blend ? (uint8_t)blend_factor(gl.blendSrc) : 0;
  key.dst = gl.blend ? (uint8_t)blend_factor(gl.blendDst) : 0;
  key.mask = gl.colorMask;
  GLMVertexArray *va = gl.vertexArray;
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
      int n = components(p->refl->attributes[i].gltype, &columns, &integer);
      static MTLVertexFormat const floats[] = { MTLVertexFormatFloat, MTLVertexFormatFloat, MTLVertexFormatFloat2,
                                                MTLVertexFormatFloat3, MTLVertexFormatFloat4 };
      key.attrs[l].format = (uint16_t)floats[MAX(0, MIN(n, 4))];
      key.attrs[l].stride = 0;
    }
  }
  id<MTLRenderPipelineState> pso = pipeline_for(p, &t, &key);
  if (!pso) return NO;
  [gl.encoder setRenderPipelineState:pso];
  [gl.encoder setDepthStencilState:depth_state_for(&t)];
  [gl.encoder setStencilReferenceValue:(uint32_t)gl.stencilRef];
  [gl.encoder setCullMode:!gl.cull ? MTLCullModeNone : gl.cullFace == GL_FRONT ? MTLCullModeFront : MTLCullModeBack];
  // OpenGL's default counter-clockwise front faces; the Y flip of offscreen
  // targets mirrors the winding.
  [gl.encoder setFrontFacingWinding:t.flipped ? MTLWindingClockwise : MTLWindingCounterClockwise];

  id<MTLTexture> target = t.color ? t.color : t.depth;
  double height = target.height;
  MTLViewport vp = { gl.viewport[0], t.flipped ? gl.viewport[1] : height - gl.viewport[1] - gl.viewport[3],
                     gl.viewport[2], gl.viewport[3], 0, 1 };
  [gl.encoder setViewport:vp];

  if (p->flipY >= 0) {
    struct glsl_msl_uniform const *u = &p->refl->uniforms[p->flipY];
    float flip = t.flipped ? -1 : 1;
    if (u->offset[GLSL_MSL_VERTEX] >= 0) memcpy(p->data[GLSL_MSL_VERTEX] + u->offset[GLSL_MSL_VERTEX], &flip, 4);
  }
  for (int s = 0; s < GLSL_MSL_STAGES; s++) {
    if (!p->refl->blockSize[s]) continue;
    if (s == GLSL_MSL_VERTEX)
      [gl.encoder setVertexBytes:p->data[s] length:p->refl->blockSize[s] atIndex:GLSL_MSL_UNIFORM_BUFFER];
    else
      [gl.encoder setFragmentBytes:p->data[s] length:p->refl->blockSize[s] atIndex:GLSL_MSL_UNIFORM_BUFFER];
  }
  for (uint32_t i = 0; i < p->refl->numAttributes; i++) {
    uint32_t l = p->refl->attributes[i].location;
    attrib_t const *a = &va->attribs[l];
    NSUInteger index = GLSL_MSL_VERTEX_BUFFER_BASE + l;
    if (a->enabled && a->buffer && a->buffer->buffer && source_stride(a) % 4)
      [gl.encoder setVertexBuffer:padded_buffer(a) offset:0 atIndex:index];
    else if (a->enabled && a->buffer && a->buffer->buffer)
      [gl.encoder setVertexBuffer:a->buffer->buffer offset:a->offset atIndex:index];
    else
      [gl.encoder setVertexBuffer:gl.constantAttribute offset:0 atIndex:index];
  }
  bind_samplers(p);
  return YES;
}

static MTLPrimitiveType primitive(GLenum mode, BOOL *ok)
{
  *ok = YES;
  switch (mode) {
    case GL_POINTS: return MTLPrimitiveTypePoint;
    case GL_LINES: return MTLPrimitiveTypeLine;
    case GL_LINE_STRIP: return MTLPrimitiveTypeLineStrip;
    case GL_TRIANGLES: return MTLPrimitiveTypeTriangle;
    case GL_TRIANGLE_STRIP: return MTLPrimitiveTypeTriangleStrip;
  }
  *ok = NO;
  return MTLPrimitiveTypeTriangle;
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
  BOOL ok;
  MTLPrimitiveType type = primitive(mode, &ok);
  if (!ok) return set_error(GL_INVALID_ENUM);
  if (count <= 0 || !prepare_draw()) return;
  [gl.encoder drawPrimitives:type vertexStart:first vertexCount:count];
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, void const *indices)
{
  BOOL ok;
  MTLPrimitiveType prim = primitive(mode, &ok);
  if (!ok || (type != GL_UNSIGNED_INT && type != GL_UNSIGNED_SHORT)) return set_error(GL_INVALID_ENUM);
  GLMBuffer *elements = gl.vertexArray->elements;
  if (!elements || !elements->buffer) return set_error(GL_INVALID_OPERATION);
  if (count <= 0 || !prepare_draw()) return;
  [gl.encoder drawIndexedPrimitives:prim indexCount:count
                          indexType:type == GL_UNSIGNED_INT ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16
                        indexBuffer:elements->buffer indexBufferOffset:(NSUInteger)(uintptr_t)indices];
}

#pragma mark - Submission

void glFlush(void)
{
  if (!gl.device) return;
  flush_pending_clear();
  commit(NO);
}

void glFinish(void)
{
  if (!gl.device) return;
  flush_pending_clear();
  commit(YES);
}

// iOS frames already run in the display link's run loop pool.
void GLMTL_BeginFrame(void)
{
#if TARGET_OS_OSX
  if (!gl.framePool) gl.framePool = objc_autoreleasePoolPush();
#endif
}

void GLMTL_Present(void)
{
  if (gl.device) {
    if (!gl.drawFramebuffer) flush_pending_clear();
    end_pass();
#if TARGET_OS_IPHONE
    // iOS refuses GPU work from a background app, as OpenGL ES presents were.
    if (UIApplication.sharedApplication.applicationState == UIApplicationStateBackground) {
      gl.drawable = nil;
      gl.cb = nil;
    }
#endif
    if (gl.drawable) {
      [command_buffer() presentDrawable:gl.drawable];
      gl.drawable = nil;
    }
    commit(NO);
  }
  if (gl.framePool) objc_autoreleasePoolPop(gl.framePool), gl.framePool = NULL;
}
