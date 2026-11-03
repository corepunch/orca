#ifndef ORCA_SIGNIN_NATIVE_H
#define ORCA_SIGNIN_NATIVE_H

struct SigninEvent {
  char type[12], code[32], message[256], url[4096];
};

/* Copies platform callback data into a bounded queue; no Lua state crosses threads. */
void Signin_Enqueue(struct SigninEvent const *event);
void Signin_EnqueueURL(char const *type, char const *url);
void Signin_EnqueueError(char const *code, char const *message);

#if defined(__APPLE__)
int Signin_AppleOpen(char const *url, char const *scheme);
/* Receives an NSURL the system opened the application with. */
void Signin_AppleOpenURL(void *url);
#endif

#endif
