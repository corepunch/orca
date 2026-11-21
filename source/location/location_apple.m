#include <TargetConditionals.h>
#include "location_native.h"

#if TARGET_OS_IOS && defined(ORCA_IOS)
#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#define LOCATION_TIMEOUT_SECONDS 20

// Shared by the Lua call and the Core Location callbacks, which all run on
// the main thread; whichever lets go last frees it, so an abandoned call
// never leaves a callback writing freed memory.
struct LocationRequest {
  _Atomic int references, done;
  struct LocationResult result;
  struct LocationError error;
};

static void location_unref(struct LocationRequest *request) {
  if (atomic_fetch_sub(&request->references, 1) == 1) free(request);
}

static void location_finish(struct LocationRequest *request, char const *code, char const *message) {
  if (code) request->error = (struct LocationError){ code, message };
  atomic_store(&request->done, 1);
  location_unref(request);
}

static struct LocationRequest *location_request(struct LocationError *error) {
  struct LocationRequest *request = calloc(1, sizeof(*request));
  if (!request) { *error = (struct LocationError){ "failed", "Your position could not be found." }; return NULL; }
  atomic_store(&request->references, 2);
  return request;
}

static void location_copy(char *target, size_t size, NSString *value) {
  target[0] = '\0';
  if (value.length) [value getCString:target maxLength:size encoding:NSUTF8StringEncoding];
}

int Location_Supported(void) {
  return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSLocationWhenInUseUsageDescription"] != nil;
}

static char const *location_status_name(CLAuthorizationStatus status) {
  switch (status) {
    case kCLAuthorizationStatusNotDetermined: return "not_determined";
    case kCLAuthorizationStatusDenied: return "denied";
    case kCLAuthorizationStatusRestricted: return "restricted";
    default: return "allowed";
  }
}

char const *Location_Status(int *approximate) {
  CLLocationManager *manager = [[CLLocationManager alloc] init];
  char const *status = location_status_name(manager.authorizationStatus);
  *approximate = manager.accuracyAuthorization == CLAccuracyAuthorizationReducedAccuracy;
  [manager release];
  return status;
}

// One fix: ask while in use if never asked, then a single requestLocation at
// about a kilometre. Nothing keeps running afterwards.
@interface OrcaLocationFix : NSObject <CLLocationManagerDelegate> {
@public
  struct LocationRequest *request;
  CLLocationManager *manager;
  BOOL asked, locating, ended;
}
@end

static NSMutableSet<OrcaLocationFix *> *location_active;

@implementation OrcaLocationFix
- (void)end:(char const *)code message:(char const *)message {
  if (ended) return;
  ended = YES;
  manager.delegate = nil;
  [manager stopUpdatingLocation];
  [manager release];
  manager = nil;
  location_finish(request, code, message);
  request = NULL;
  [[self retain] autorelease];
  [location_active removeObject:self];
}

- (void)decide {
  if (ended || locating) return;
  switch (manager.authorizationStatus) {
    case kCLAuthorizationStatusNotDetermined:
      if (!asked) { asked = YES; [manager requestWhenInUseAuthorization]; }
      return;
    case kCLAuthorizationStatusDenied:
    case kCLAuthorizationStatusRestricted:
      [self end:"permission" message:"Location is off for this app. You can turn it on in Settings, or search for a place."];
      return;
    default:
      locating = YES;
      [manager requestLocation];
  }
}

- (void)start {
  manager = [[CLLocationManager alloc] init];
  manager.delegate = self;
  manager.desiredAccuracy = kCLLocationAccuracyKilometer;
  [self decide];
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)LOCATION_TIMEOUT_SECONDS * NSEC_PER_SEC),
                 dispatch_get_main_queue(), ^{
    [self end:"timeout" message:"Your position could not be found in time. Try again outdoors, or search for a place."];
  });
}

- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)changed {
  (void)changed;
  [self decide];
}

- (void)locationManager:(CLLocationManager *)changed didUpdateLocations:(NSArray<CLLocation *> *)locations {
  CLLocation *location = locations.lastObject;
  if (!location || ended) return;
  struct LocationResult *result = &request->result;
  result->lat = location.coordinate.latitude;
  result->lon = location.coordinate.longitude;
  result->accuracy = location.horizontalAccuracy;
  result->timestamp = location.timestamp.timeIntervalSince1970;
  result->approximate = changed.accuracyAuthorization == CLAccuracyAuthorizationReducedAccuracy;
  [self end:NULL message:NULL];
}

- (void)locationManager:(CLLocationManager *)changed didFailWithError:(NSError *)error {
  (void)changed;
  if (error.code == kCLErrorDenied)
    [self end:"permission" message:"Location is off for this app. You can turn it on in Settings, or search for a place."];
  else
    [self end:"failed" message:"Your position could not be found. Search for a place instead."];
}
@end

void *Location_BeginFix(struct LocationError *error) {
  struct LocationRequest *request = location_request(error);
  if (!request) return NULL;
  if (!location_active) location_active = [[NSMutableSet alloc] init];
  OrcaLocationFix *fix = [[OrcaLocationFix alloc] init];
  fix->request = request;
  [location_active addObject:fix];
  [fix release];
  [fix start];
  return request;
}

void *Location_BeginReverse(double lat, double lon, struct LocationError *error) {
  struct LocationRequest *request = location_request(error);
  if (!request) return NULL;
  request->result.reverse = 1;
  request->result.lat = lat;
  request->result.lon = lon;
  CLGeocoder *geocoder = [[CLGeocoder alloc] init];
  CLLocation *location = [[CLLocation alloc] initWithLatitude:lat longitude:lon];
  [geocoder reverseGeocodeLocation:location completionHandler:^(NSArray<CLPlacemark *> *marks, NSError *failure) {
    CLPlacemark *mark = marks.firstObject;
    if (failure || !mark) {
      BOOL offline = failure.code == kCLErrorNetwork;
      location_finish(request, offline ? "network" : "failed",
                      offline ? "No connection to name this place." : "This place could not be named.");
    } else {
      struct LocationResult *result = &request->result;
      location_copy(result->name, sizeof(result->name), mark.locality ?: mark.subAdministrativeArea ?: mark.name);
      location_copy(result->locality, sizeof(result->locality), mark.locality);
      location_copy(result->region, sizeof(result->region), mark.administrativeArea);
      location_copy(result->country, sizeof(result->country), mark.country);
      location_copy(result->countryCode, sizeof(result->countryCode), mark.ISOcountryCode);
      location_finish(request, NULL, NULL);
    }
    [geocoder release];
  }];
  [location release];
  return request;
}

int Location_Poll(void *state, struct LocationResult const **result, struct LocationError *error) {
  struct LocationRequest *request = state;
  if (!atomic_load(&request->done)) return 0;
  if (request->error.code) { *error = request->error; return -1; }
  *result = &request->result;
  return 1;
}

void Location_Release(void *request) { location_unref(request); }
#endif
