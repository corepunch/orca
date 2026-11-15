#ifndef ORCA_CONTACTS_NATIVE_H
#define ORCA_CONTACTS_NATIVE_H

#define CONTACTS_LIMIT 5000
#define CONTACTS_EMAIL_LIMIT 5
#define CONTACTS_TEXT 256

struct ContactsEntry {
  char name[CONTACTS_TEXT];
  char emails[CONTACTS_EMAIL_LIMIT][CONTACTS_TEXT];
  int count;
};

struct ContactsError { char const *code, *message; };

#if defined(ORCA_IOS)
/* Starts the permission prompt and background read; NULL with error on failure. */
void *Contacts_Begin(struct ContactsError *error);
/* 0 while pending, 1 with entries (owned by the request), -1 with error. */
int Contacts_Poll(void *request, struct ContactsEntry const **entries, int *count, struct ContactsError *error);
/* Drops the caller's reference; the read finishing later frees the rest. */
void Contacts_Release(void *request);
#endif

#endif
