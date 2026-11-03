#ifndef ORCA_ANDROID_BRIDGE_H
#define ORCA_ANDROID_BRIDGE_H
#include <stddef.h>

/* Calls into org.orca.OrcaActivity from the native thread. Each returns 0 when
 * the activity or method is unavailable. */
int Android_ShowKeyboard(int visible, int type);
int Android_OpenURL(char const *url);
/* Keystore-encrypted private storage. Get returns a malloc'd copy, or NULL
 * with *found = 0 when absent; -1 on failure. */
int Android_CredentialGet(char const *key, char **value, size_t *size, int *found);
int Android_CredentialSet(char const *key, char const *value, size_t size);
int Android_CredentialDelete(char const *key);

/* Called by queue.c before it waits, so the soft keyboard follows focus. */
void ORCA_Android_SyncKeyboard(void);

/* Implemented by the sign-in module: links the activity received. */
void Signin_AndroidURL(char const *url);
#endif
