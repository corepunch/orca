#ifndef ORCA_LOCATION_NATIVE_H
#define ORCA_LOCATION_NATIVE_H

#define LOCATION_TEXT 160

struct LocationResult {
  double lat, lon, accuracy, timestamp;
  int approximate, reverse; /* reverse: a geocoded name, else a fix */
  char name[LOCATION_TEXT], locality[LOCATION_TEXT], region[LOCATION_TEXT], country[LOCATION_TEXT], countryCode[8];
};

struct LocationError { char const *code, *message; };

#if defined(ORCA_IOS)
/* 1 when Core Location may be asked: the bundle carries the usage string. */
int Location_Supported(void);
/* not_determined, allowed, denied, restricted; *approximate for reduced accuracy. */
char const *Location_Status(int *approximate);
/* Start a one-shot fix or a reverse geocode; NULL with error on failure. */
void *Location_BeginFix(struct LocationError *error);
void *Location_BeginReverse(double lat, double lon, struct LocationError *error);
/* 0 while pending, 1 with the result (owned by the request), -1 with error. */
int Location_Poll(void *request, struct LocationResult const **result, struct LocationError *error);
/* Drops the caller's reference; a callback finishing later frees the rest. */
void Location_Release(void *request);
#endif

#endif
