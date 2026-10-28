#ifndef ORCA_MEDIA_NATIVE_H
#define ORCA_MEDIA_NATIVE_H

#include <stddef.h>
#include <stdint.h>

#define MEDIA_INPUT_LIMIT (32u * 1024u * 1024u)
#define MEDIA_PHOTO_LIMIT 250000u
#define MEDIA_PHOTO_PIXELS 1500000u
#define MEDIA_PHOTO_EDGE 1600u

struct MediaPhoto {
  unsigned char *jpeg, *thumbnail;
  size_t byte_count, thumbnail_byte_count;
  uint32_t width, height, thumbnail_width, thumbnail_height;
  double quality;
};

struct MediaError { char const *code, *message; };

int Media_ProcessPhoto(void const *bytes, size_t size, struct MediaPhoto *photo, struct MediaError *error);
int Media_PickPhoto(struct MediaPhoto *photo, struct MediaError *error);

#if defined(ORCA_IOS)
void *Media_BeginPickPhoto(struct MediaError *error);
int Media_PollPickedPhoto(void *request, struct MediaPhoto *photo, struct MediaError *error);
void Media_ReleasePhotoPicker(void *request);
#endif

#endif
