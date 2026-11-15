#include <TargetConditionals.h>
#include "contacts_native.h"

#if TARGET_OS_IOS && defined(ORCA_IOS)
#import <Foundation/Foundation.h>
#import <Contacts/Contacts.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

// Shared by the Lua call and the background read; whichever finishes last
// frees it, so an abandoned call never leaves the read writing freed memory.
struct ContactsRequest {
  _Atomic int references, done;
  struct ContactsEntry *entries;
  int count;
  struct ContactsError error;
};

static void contacts_unref(struct ContactsRequest *request) {
  if (atomic_fetch_sub(&request->references, 1) != 1) return;
  free(request->entries);
  free(request);
}

static void contacts_copy(char *target, NSString *value) {
  target[0] = '\0';
  if (value.length) [value getCString:target maxLength:CONTACTS_TEXT encoding:NSUTF8StringEncoding];
}

static void contacts_finish(struct ContactsRequest *request, char const *code, char const *message) {
  if (code) request->error = (struct ContactsError){ code, message };
  atomic_store(&request->done, 1);
  contacts_unref(request);
}

static void contacts_read(struct ContactsRequest *request, CNContactStore *store) {
  @autoreleasepool {
    NSArray *keys = @[[CNContactFormatter descriptorForRequiredKeysForStyle:CNContactFormatterStyleFullName],
                      CNContactEmailAddressesKey];
    CNContactFetchRequest *fetch = [[[CNContactFetchRequest alloc] initWithKeysToFetch:keys] autorelease];
    request->entries = calloc(CONTACTS_LIMIT, sizeof(*request->entries));
    if (!request->entries) { contacts_finish(request, "failed", "Contacts could not be read."); return; }
    NSError *error = nil;
    BOOL ok = [store enumerateContactsWithFetchRequest:fetch error:&error usingBlock:^(CNContact *contact, BOOL *stop) {
      if (!contact.emailAddresses.count) return;
      struct ContactsEntry *entry = &request->entries[request->count];
      contacts_copy(entry->name, [CNContactFormatter stringFromContact:contact style:CNContactFormatterStyleFullName] ?: @"");
      for (CNLabeledValue<NSString *> *email in contact.emailAddresses) {
        if (entry->count == CONTACTS_EMAIL_LIMIT) break;
        contacts_copy(entry->emails[entry->count], email.value);
        if (entry->emails[entry->count][0]) entry->count++;
      }
      if (entry->count && ++request->count == CONTACTS_LIMIT) *stop = YES;
    }];
    contacts_finish(request, ok ? NULL : "failed", "Contacts could not be read.");
  }
}

void *Contacts_Begin(struct ContactsError *error) {
  struct ContactsRequest *request = calloc(1, sizeof(*request));
  if (!request) { *error = (struct ContactsError){ "failed", "Contacts could not be read." }; return NULL; }
  atomic_store(&request->references, 2);
  CNContactStore *store = [[CNContactStore alloc] init];
  [store requestAccessForEntityType:CNEntityTypeContacts completionHandler:^(BOOL granted, NSError *failure) {
    (void)failure;
    if (!granted) {
      contacts_finish(request, "permission", "Allow access to contacts in Settings to find friends from them.");
    } else {
      dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{ contacts_read(request, store); });
    }
    [store release];
  }];
  return request;
}

int Contacts_Poll(void *state, struct ContactsEntry const **entries, int *count, struct ContactsError *error) {
  struct ContactsRequest *request = state;
  if (!atomic_load(&request->done)) return 0;
  if (request->error.code) { *error = request->error; return -1; }
  *entries = request->entries;
  *count = request->count;
  return 1;
}

void Contacts_Release(void *request) { contacts_unref(request); }
#endif
