#import <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <math.h>
#include <include/api.h>
#include <libs/platform/platform.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int luaopen_orca_media(lua_State *L);

static int checks, failures, picker_calls;

static void check(int condition, char const *message) {
  checks++;
  if (!condition) { fprintf(stderr, "FAIL: %s\n", message); failures++; }
}

// Keep cancellation deterministic and avoid opening a dialog in an automated test.
bool_t axGetOpenFileName(AXopenfilename const *picker) {
  (void)picker;
  picker_calls++;
  return FALSE;
}

static CFDataRef fixture(size_t width, size_t height, int orientation, int alpha, int noise) {
  CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  CGContextRef context = CGBitmapContextCreate(NULL, width, height, 8, width * 4, space,
      kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
  CGColorSpaceRelease(space);
  if (!context) return NULL;
  unsigned char *pixels = CGBitmapContextGetData(context);
  uint32_t seed = 1234567;
  for (size_t y = 0; y < height; y++) for (size_t x = 0; x < width; x++) {
    size_t p = (y * width + x) * 4;
    if (noise) {
      seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
      pixels[p] = seed; pixels[p + 1] = seed >> 8; pixels[p + 2] = seed >> 16;
    } else {
      pixels[p] = x < width / 2 ? 240 : 15;
      pixels[p + 1] = 20;
      pixels[p + 2] = x < width / 2 ? 15 : 240;
    }
    pixels[p + 3] = 255;
    if (alpha && x < width / 4) memset(pixels + p, 0, 4);
  }
  CGImageRef image = CGBitmapContextCreateImage(context);
  CFMutableDataRef data = CFDataCreateMutable(kCFAllocatorDefault, 0);
  CGImageDestinationRef output = CGImageDestinationCreateWithData(data,
      alpha ? CFSTR("public.png") : CFSTR("public.jpeg"), 1, NULL);
  CGImageDestinationAddImage(output, image, (CFDictionaryRef)@{
    (id)kCGImageDestinationLossyCompressionQuality: @1.0,
    (id)kCGImagePropertyOrientation: @(orientation),
    (id)kCGImagePropertyGPSDictionary: @{
      (id)kCGImagePropertyGPSLatitude: @52.5, (id)kCGImagePropertyGPSLatitudeRef: @"N",
      (id)kCGImagePropertyGPSLongitude: @13.4, (id)kCGImagePropertyGPSLongitudeRef: @"E",
    },
    (id)kCGImagePropertyTIFFDictionary: @{ (id)kCGImagePropertyTIFFMake: @"Private fixture camera" },
    (id)kCGImagePropertyExifDictionary: @{ (id)kCGImagePropertyExifUserComment: @"Private source comment" },
  });
  int success = CGImageDestinationFinalize(output);
  CFRelease(output); CGImageRelease(image); CGContextRelease(context);
  if (!success) { CFRelease(data); return NULL; }
  return data;
}

static int process(lua_State *L, CFDataRef data) {
  lua_getglobal(L, "media"); lua_getfield(L, -1, "processPhoto"); lua_remove(L, -2);
  lua_pushlstring(L, (char const *)CFDataGetBytePtr(data), CFDataGetLength(data));
  int status = lua_pcall(L, 1, 2, 0);
  if (status != LUA_OK) fprintf(stderr, "%s\n", lua_tostring(L, -1));
  return status == LUA_OK && lua_istable(L, -2) && lua_isnil(L, -1);
}

static CFDataRef result_data(lua_State *L, char const *field) {
  lua_getfield(L, -2, field);
  size_t size;
  char const *bytes = lua_tolstring(L, -1, &size);
  CFDataRef data = bytes ? CFDataCreate(kCFAllocatorDefault, (UInt8 const *)bytes, size) : NULL;
  lua_pop(L, 1);
  return data;
}

static lua_Integer result_integer(lua_State *L, char const *field) {
  lua_getfield(L, -2, field);
  lua_Integer value = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return value;
}

static CGImageRef inspect_jpeg(CFDataRef data, size_t width, size_t height) {
  check(data && CFDataGetLength(data) > 4, "Encoder returns actual JPEG bytes");
  if (!data || CFDataGetLength(data) <= 4) return NULL;
  UInt8 const *bytes = CFDataGetBytePtr(data);
  size_t size = CFDataGetLength(data);
  check(bytes[0] == 0xff && bytes[1] == 0xd8 && bytes[size - 2] == 0xff && bytes[size - 1] == 0xd9,
      "Encoded bytes have JPEG SOI and EOI markers");
  CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
  check(source && CFEqual(CGImageSourceGetType(source), CFSTR("public.jpeg")), "Output decodes as JPEG");
  if (!source) return NULL;
  CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
  check(image && CGImageGetWidth(image) == width && CGImageGetHeight(image) == height,
      "Encoded dimensions match the public result");
  NSDictionary *properties = (NSDictionary *)CGImageSourceCopyPropertiesAtIndex(source, 0, NULL);
  check(!properties[(id)kCGImagePropertyGPSDictionary], "Source GPS metadata is omitted");
  check(!properties[(id)kCGImagePropertyTIFFDictionary][(id)kCGImagePropertyTIFFMake], "Source camera metadata is omitted");
  check(!properties[(id)kCGImagePropertyExifDictionary][(id)kCGImagePropertyExifUserComment], "Source private EXIF is omitted");
  NSNumber *orientation = properties[(id)kCGImagePropertyOrientation];
  check(!orientation || orientation.intValue == 1, "Output pixels need no EXIF orientation transform");
  NSString *profile = properties[(id)kCGImagePropertyProfileName];
  check(profile && [profile rangeOfString:@"sRGB" options:NSCaseInsensitiveSearch].location != NSNotFound,
      "Output has an sRGB profile");
  [properties release]; CFRelease(source);
  return image;
}

static void test_oriented_photo(lua_State *L) {
  CFDataRef input = fixture(800, 400, 6, 0, 0);
  check(input != NULL, "Create the local oriented fixture");
  if (!input) return;
  CGImageSourceRef source = CGImageSourceCreateWithData(input, NULL);
  NSDictionary *properties = (NSDictionary *)CGImageSourceCopyPropertiesAtIndex(source, 0, NULL);
  check(properties[(id)kCGImagePropertyGPSDictionary] != nil && [properties[(id)kCGImagePropertyOrientation] intValue] == 6,
      "The input fixture really contains GPS and a 90-degree EXIF transform");
  [properties release]; CFRelease(source);
  int success = process(L, input);
  check(success, "Process a real JPEG through the generated Lua API");
  CFRelease(input);
  if (success) {
    check(result_integer(L, "width") == 400 && result_integer(L, "height") == 800,
        "Processing applies the input EXIF rotation to pixels");
    CFDataRef jpeg = result_data(L, "jpeg");
    CGImageRef image = inspect_jpeg(jpeg, 400, 800);
    check(result_integer(L, "byte_count") == CFDataGetLength(jpeg), "Byte count reports encoded bytes exactly");
    if (image) {
      CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
      unsigned char pixels[8] = {0};
      CGContextRef context = CGBitmapContextCreate(pixels, 1, 2, 8, 4, space,
          kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
      CGContextDrawImage(context, CGRectMake(0, 0, 1, 2), image);
      check(abs(pixels[0] - pixels[4]) > 150 && abs(pixels[2] - pixels[6]) > 150,
          "The EXIF transform rotates the left/right source colors into top/bottom pixels");
      CGContextRelease(context); CGColorSpaceRelease(space); CGImageRelease(image);
    }
    CFRelease(jpeg);
    CFDataRef thumbnail = result_data(L, "thumbnail");
    check(result_integer(L, "thumbnail_width") == 128 && result_integer(L, "thumbnail_height") == 256,
        "Thumbnail keeps the oriented aspect ratio at a 256-pixel longest edge");
    image = inspect_jpeg(thumbnail, 128, 256);
    if (image) CGImageRelease(image);
    CFRelease(thumbnail);
  }
  lua_settop(L, 0);
}

static void test_large_photo(lua_State *L) {
  CFDataRef input = fixture(2400, 1800, 1, 0, 1);
  check(input && process(L, input), "Process a large high-entropy local photo");
  if (input) CFRelease(input);
  if (lua_istable(L, -2)) {
    size_t w = result_integer(L, "width"), h = result_integer(L, "height");
    check(w > 0 && h > 0 && w <= 1600 && h <= 1600, "Downsample while decoding to bounded output dimensions");
    check(fabs((double)w / h - 4.0 / 3) < .01, "Downsampling preserves the photo aspect ratio");
    CFDataRef jpeg = result_data(L, "jpeg");
    check(jpeg && CFDataGetLength(jpeg) <= 1000000, "Actual high-entropy JPEG fits the hard byte limit");
    check(jpeg && CFDataGetLength(jpeg) <= 500000, "Adaptive encoding reaches the preferred budget for the noisy fixture");
    CGImageRef image = inspect_jpeg(jpeg, w, h);
    if (image) CGImageRelease(image);
    CFRelease(jpeg);
    lua_getfield(L, -2, "quality");
    check(lua_tonumber(L, -1) >= .6 && lua_tonumber(L, -1) <= .8, "Adaptive encoding observes the quality floor");
    lua_pop(L, 1);
  }
  lua_settop(L, 0);
}

static void test_alpha_and_small_photo(lua_State *L) {
  CFDataRef input = fixture(32, 24, 1, 1, 0);
  check(input && process(L, input), "Process a transparent local PNG");
  if (input) CFRelease(input);
  if (lua_istable(L, -2)) {
    CFDataRef jpeg = result_data(L, "jpeg");
    CGImageRef image = inspect_jpeg(jpeg, 32, 24);
    check(result_integer(L, "width") == 32 && result_integer(L, "height") == 24,
        "Small source images are never enlarged");
    check(result_integer(L, "thumbnail_width") == 32 && result_integer(L, "thumbnail_height") == 24,
        "Small thumbnails are never enlarged");
    if (image) {
      CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
      unsigned char pixel[4] = {0};
      CGContextRef context = CGBitmapContextCreate(pixel, 1, 1, 8, 4, space,
          kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
      CGContextDrawImage(context, CGRectMake(-2, -10, 32, 24), image);
      check(pixel[0] >= 230 && pixel[1] >= 230 && pixel[2] >= 230 && pixel[3] == 255,
          "JPEG transparency is flattened onto opaque neutral light gray");
      CGContextRelease(context); CGColorSpaceRelease(space); CGImageRelease(image);
    }
    CFRelease(jpeg);
  }
  lua_settop(L, 0);
}

int main(void) {
  @autoreleasepool {
    lua_State *L = luaL_newstate(); luaL_openlibs(L);
    luaL_requiref(L, "orca.media", luaopen_orca_media, 1); lua_setglobal(L, "media");
    test_oriented_photo(L);
    test_large_photo(L);
    test_alpha_and_small_photo(L);
    int status = luaL_dostring(L,
        "local p,e=media.processPhoto('not an image'); assert(p==nil and e.code=='invalid_image')\n"
        "p,e=media.processPhoto(''); assert(p==nil and e.code=='invalid_image')\n"
        "p,e=media.processPhoto(string.rep('x',32*1024*1024+1)); assert(p==nil and e.code=='input_too_large')\n"
        "p,e=media.pickPhoto(); assert(p==nil and e.code=='cancelled')\n");
    if (status != LUA_OK) fprintf(stderr, "%s\n", lua_tostring(L, -1));
    check(status == LUA_OK, "Bad input, oversize input and picker cancellation return structured errors");
    check(picker_calls == 1, "Cancellation uses the existing native file-dialog bridge");
    lua_close(L);
    printf("Native photo processing: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
  }
}
