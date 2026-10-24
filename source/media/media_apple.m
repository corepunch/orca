#import <Foundation/Foundation.h>
#include <TargetConditionals.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <libs/platform/platform.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "media_native.h"

static int Media_Fail(struct MediaError *error, char const *code, char const *message) {
  *error = (struct MediaError){ code, message };
  return 0;
}

static CGImageRef Media_Flatten(CGImageRef source, size_t edge) {
  size_t w = CGImageGetWidth(source), h = CGImageGetHeight(source);
  double scale = fmin(1, (double)edge / fmax(w, h));
  w = fmax(1, floor(w * scale)); h = fmax(1, floor(h * scale));
  CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  if (!space) return NULL;
  CGContextRef context = CGBitmapContextCreate(NULL, w, h, 8, w * 4, space,
      kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
  CGColorSpaceRelease(space);
  if (!context) return NULL;
  CGContextSetRGBFillColor(context, 245.0 / 255, 245.0 / 255, 245.0 / 255, 1);
  CGContextFillRect(context, CGRectMake(0, 0, w, h));
  CGContextSetInterpolationQuality(context, kCGInterpolationHigh);
  CGContextDrawImage(context, CGRectMake(0, 0, w, h), source);
  CGImageRef image = CGBitmapContextCreateImage(context);
  CGContextRelease(context);
  return image;
}

static CFDataRef Media_Jpeg(CGImageRef image, double quality) {
  CFMutableDataRef data = CFDataCreateMutable(kCFAllocatorDefault, 0);
  if (!data) return NULL;
  CGImageDestinationRef destination = CGImageDestinationCreateWithData(data, CFSTR("public.jpeg"), 1, NULL);
  if (!destination) { CFRelease(data); return NULL; }
  // A fresh destination receives pixels and an encoder option only, never source EXIF/GPS.
  CGImageDestinationAddImage(destination, image, (CFDictionaryRef)@{
    (id)kCGImageDestinationLossyCompressionQuality: @(quality),
  });
  bool success = CGImageDestinationFinalize(destination);
  CFRelease(destination);
  if (!success) { CFRelease(data); return NULL; }
  return data;
}

static CFDataRef Media_Compress(CGImageRef image, struct MediaPhoto *photo) {
  static double const qualities[] = { .8, .7, .6 };
  size_t longest = MAX(CGImageGetWidth(image), CGImageGetHeight(image));
  size_t minimum = MIN(longest, 800);
  CFDataRef best = NULL;
  for (int step = 0; step < 5; step++) {
    size_t edge = MAX(minimum, floor(longest * pow(.8, step)));
    CGImageRef resized = Media_Flatten(image, edge);
    if (!resized) break;
    for (size_t i = 0; i < sizeof(qualities) / sizeof(qualities[0]); i++) {
      CFDataRef data = Media_Jpeg(resized, qualities[i]);
      if (!data) continue;
      size_t size = CFDataGetLength(data);
      if (size <= MEDIA_PHOTO_LIMIT && (!best || size <= 500000)) {
        if (best) CFRelease(best);
        best = data;
        photo->quality = qualities[i];
        photo->width = (uint32_t)CGImageGetWidth(resized);
        photo->height = (uint32_t)CGImageGetHeight(resized);
        if (size <= 500000) { CGImageRelease(resized); return best; }
      } else CFRelease(data);
    }
    CGImageRelease(resized);
    if (edge == minimum) break;
  }
  return best;
}

int Media_ProcessPhoto(void const *bytes, size_t size, struct MediaPhoto *photo, struct MediaError *error) {
  @autoreleasepool {
    if (!bytes || !size) return Media_Fail(error, "invalid_image", "Choose a readable still image.");
    if (size > MEDIA_INPUT_LIMIT) return Media_Fail(error, "input_too_large", "The source photo exceeds 32 MiB.");
    CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, bytes, size, kCFAllocatorNull);
    if (!data) return Media_Fail(error, "memory", "There is not enough memory to prepare this photo.");
    CGImageSourceRef source = CGImageSourceCreateWithData(data,
        (CFDictionaryRef)@{ (id)kCGImageSourceShouldCache: @NO });
    CFRelease(data);
    if (!source) return Media_Fail(error, "invalid_image", "This file is not a supported still image.");
    size_t count = CGImageSourceGetCount(source);
    if (!count) {
      CFRelease(source);
      return Media_Fail(error, "invalid_image", "This file is not a readable still image.");
    }
    if (count != 1) {
      CFRelease(source);
      return Media_Fail(error, "unsupported_image", "Choose one still image; animated and multi-image files are not supported.");
    }
    CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(source, 0, NULL);
    size_t w = 0, h = 0;
    if (properties) {
      CFNumberRef width = CFDictionaryGetValue(properties, kCGImagePropertyPixelWidth);
      CFNumberRef height = CFDictionaryGetValue(properties, kCGImagePropertyPixelHeight);
      if (width) CFNumberGetValue(width, kCFNumberLongLongType, &w);
      if (height) CFNumberGetValue(height, kCFNumberLongLongType, &h);
      CFRelease(properties);
    }
    if (!w || !h || w > 32768 || h > 32768 || (uint64_t)w * h > 100000000) {
      CFRelease(source);
      return Media_Fail(error, "dimensions", "Choose a photo within 100 megapixels and 32768 pixels per edge.");
    }
    CGImageRef decoded = CGImageSourceCreateThumbnailAtIndex(source, 0, (CFDictionaryRef)@{
      (id)kCGImageSourceCreateThumbnailFromImageAlways: @YES,
      (id)kCGImageSourceCreateThumbnailWithTransform: @YES,
      (id)kCGImageSourceShouldCacheImmediately: @YES,
      (id)kCGImageSourceThumbnailMaxPixelSize: @(MIN(MAX(w, h), 1600)),
    });
    CFRelease(source);
    if (!decoded) return Media_Fail(error, "invalid_image", "The photo could not be decoded.");
    CGImageRef flattened = Media_Flatten(decoded, 1600);
    CGImageRelease(decoded);
    if (!flattened) return Media_Fail(error, "memory", "There is not enough memory to prepare this photo.");
    CFDataRef jpeg = Media_Compress(flattened, photo);
    CGImageRef small = Media_Flatten(flattened, 256);
    CGImageRelease(flattened);
    CFDataRef thumbnail = small ? Media_Jpeg(small, .8) : NULL;
    if (small) {
      photo->thumbnail_width = (uint32_t)CGImageGetWidth(small);
      photo->thumbnail_height = (uint32_t)CGImageGetHeight(small);
      CGImageRelease(small);
    }
    if (!jpeg || !thumbnail || CFDataGetLength(thumbnail) > MEDIA_PHOTO_LIMIT) {
      if (jpeg) CFRelease(jpeg);
      if (thumbnail) CFRelease(thumbnail);
      return Media_Fail(error, "encode", "This photo could not fit the JPEG upload limit without reducing quality further.");
    }
    photo->byte_count = CFDataGetLength(jpeg);
    photo->thumbnail_byte_count = CFDataGetLength(thumbnail);
    photo->jpeg = malloc(photo->byte_count);
    photo->thumbnail = malloc(photo->thumbnail_byte_count);
    if (photo->jpeg && photo->thumbnail) {
      memcpy(photo->jpeg, CFDataGetBytePtr(jpeg), photo->byte_count);
      memcpy(photo->thumbnail, CFDataGetBytePtr(thumbnail), photo->thumbnail_byte_count);
    }
    CFRelease(jpeg); CFRelease(thumbnail);
    if (!photo->jpeg || !photo->thumbnail) {
      free(photo->jpeg); free(photo->thumbnail);
      memset(photo, 0, sizeof(*photo));
      return Media_Fail(error, "memory", "There is not enough memory to prepare this photo.");
    }
    return 1;
  }
}

int Media_PickPhoto(struct MediaPhoto *photo, struct MediaError *error) {
#if TARGET_OS_OSX
  char path[4096] = {0};
  AXopenfilename picker = { .lpstrFile = path, .nMaxFile = sizeof(path),
    .lpstrTitle = "Choose a photo", .Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST };
  if (!axGetOpenFileName(&picker)) return Media_Fail(error, "cancelled", "Photo selection was cancelled.");
  struct stat info;
  if (stat(path, &info) || !S_ISREG(info.st_mode) || info.st_size <= 0)
    return Media_Fail(error, "read", "The selected photo could not be read.");
  if ((uint64_t)info.st_size > MEDIA_INPUT_LIMIT)
    return Media_Fail(error, "input_too_large", "The source photo exceeds 32 MiB.");
  size_t capacity = (size_t)info.st_size;
  unsigned char *bytes = malloc(capacity + 1);
  FILE *file = fopen(path, "rb");
  if (!bytes || !file) {
    free(bytes); if (file) fclose(file);
    return Media_Fail(error, "read", "The selected photo could not be read.");
  }
  size_t size = fread(bytes, 1, capacity + 1, file);
  int failed = ferror(file);
  fclose(file);
  int result;
  if (failed || size != capacity) result = Media_Fail(error, "read", "The selected photo changed or could not be read.");
  else result = Media_ProcessPhoto(bytes, size, photo, error);
  free(bytes);
  return result;
#else
  (void)photo;
  return Media_Fail(error, "unsupported", "Native photo picking is currently available on macOS.");
#endif
}
