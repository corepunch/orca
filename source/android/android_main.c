// Android entry point: NativeActivity's glue thread runs the same blocking
// Application loop as the desktop. Bundled files are unpacked from the APK to
// private storage once per build, since Lua and the engine read real paths.
#include <include/api.h>
#include <core/core.h>
#include <generated/UIKit/UIKit.h>
#include <android/asset_manager.h>
#include <android/configuration.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <jni.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libs/platform/android/ax_android.h>
#include "android_bridge.h"

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "orca", __VA_ARGS__)

int luaopen_orca(lua_State *L);

static struct android_app *android_app_state;

/* ── JNI ───────────────────────────────────────────────────────────────── */

static JNIEnv *
jni_env(void)
{
  if (!android_app_state) return NULL;
  JavaVM *vm = android_app_state->activity->vm;
  JNIEnv *env = NULL;
  if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) == JNI_OK) return env;
  return (*vm)->AttachCurrentThread(vm, &env, NULL) == JNI_OK ? env : NULL;
}

static jmethodID
jni_method(JNIEnv *env, char const *name, char const *signature)
{
  jclass type = (*env)->GetObjectClass(env, android_app_state->activity->clazz);
  jmethodID method = (*env)->GetMethodID(env, type, name, signature);
  (*env)->DeleteLocalRef(env, type);
  if (!method) (*env)->ExceptionClear(env);
  return method;
}

static int
jni_failed(JNIEnv *env)
{
  if (!(*env)->ExceptionCheck(env)) return 0;
  (*env)->ExceptionDescribe(env);
  (*env)->ExceptionClear(env);
  return 1;
}

int
Android_ShowKeyboard(int visible, int type)
{
  JNIEnv *env = jni_env();
  jmethodID method = env ? jni_method(env, "showKeyboard", "(ZI)V") : NULL;
  if (!method) return 0;
  (*env)->CallVoidMethod(env, android_app_state->activity->clazz, method, (jboolean)visible, (jint)type);
  return !jni_failed(env);
}

int
Android_OpenURL(char const *url)
{
  JNIEnv *env = jni_env();
  jmethodID method = env ? jni_method(env, "openUrl", "(Ljava/lang/String;)Z") : NULL;
  if (!method) return 0;
  jstring text = (*env)->NewStringUTF(env, url);
  jboolean ok = (*env)->CallBooleanMethod(env, android_app_state->activity->clazz, method, text);
  (*env)->DeleteLocalRef(env, text);
  return !jni_failed(env) && ok;
}

static jbyteArray
jni_bytes(JNIEnv *env, char const *data, size_t size)
{
  jbyteArray bytes = (*env)->NewByteArray(env, (jsize)size);
  if (bytes) (*env)->SetByteArrayRegion(env, bytes, 0, (jsize)size, (jbyte const *)data);
  return bytes;
}

int
Android_CredentialGet(char const *key, char **value, size_t *size, int *found)
{
  *value = NULL; *size = 0; *found = 0;
  JNIEnv *env = jni_env();
  jmethodID method = env ? jni_method(env, "credentialGet", "([B)[B") : NULL;
  if (!method) return 0;
  jbyteArray name = jni_bytes(env, key, strlen(key));
  jbyteArray result = (*env)->CallObjectMethod(env, android_app_state->activity->clazz, method, name);
  (*env)->DeleteLocalRef(env, name);
  if (jni_failed(env)) return -1;
  if (!result) return 1;
  jsize length = (*env)->GetArrayLength(env, result);
  *value = malloc(length ? (size_t)length : 1);
  if (*value) (*env)->GetByteArrayRegion(env, result, 0, length, (jbyte *)*value);
  (*env)->DeleteLocalRef(env, result);
  if (!*value) return -1;
  *size = (size_t)length; *found = 1;
  return 1;
}

int
Android_CredentialSet(char const *key, char const *value, size_t size)
{
  JNIEnv *env = jni_env();
  jmethodID method = env ? jni_method(env, "credentialSet", "([B[B)Z") : NULL;
  if (!method) return 0;
  jbyteArray name = jni_bytes(env, key, strlen(key)), data = jni_bytes(env, value, size);
  jboolean ok = (*env)->CallBooleanMethod(env, android_app_state->activity->clazz, method, name, data);
  (*env)->DeleteLocalRef(env, name);
  (*env)->DeleteLocalRef(env, data);
  return jni_failed(env) ? -1 : ok ? 1 : -1;
}

int
Android_CredentialDelete(char const *key)
{
  JNIEnv *env = jni_env();
  jmethodID method = env ? jni_method(env, "credentialDelete", "([B)Z") : NULL;
  if (!method) return 0;
  jbyteArray name = jni_bytes(env, key, strlen(key));
  jboolean ok = (*env)->CallBooleanMethod(env, android_app_state->activity->clazz, method, name);
  (*env)->DeleteLocalRef(env, name);
  return jni_failed(env) ? -1 : ok ? 1 : -1;
}

/* Java → native. These run on the UI thread and only copy data into queues. */

static void
post_text(char const *utf8)
{
  for (size_t i = 0, length = strlen(utf8); i < length;) {
    unsigned char c = (unsigned char)utf8[i];
    size_t n = c < 0x80 ? 1 : c < 0xe0 ? 2 : c < 0xf0 ? 3 : 4;
    if (n > length - i) n = length - i;
    void *chars = NULL;
    memcpy(&chars, utf8 + i, n < sizeof(chars) ? n : sizeof(chars));
    uint32_t key = n == 1 ? c : c & ((1u << (7 - n)) - 1);
    for (size_t j = 1; j < n; j++) key = (key << 6) | (utf8[i + j] & 0x3f);
    if (c == '\n' || c == '\r') { key = AX_KEY_ENTER; chars = NULL; }
    axAndroidPostMessage((struct AXmessage){ .message = kEventKeyDown, .wParam = key, .lParam = chars });
    axAndroidPostMessage((struct AXmessage){ .message = kEventKeyUp, .wParam = key });
    i += n;
  }
}

JNIEXPORT void JNICALL
Java_org_orca_OrcaActivity_nativeText(JNIEnv *env, jclass type, jstring text)
{
  (void)type;
  char const *utf8 = text ? (*env)->GetStringUTFChars(env, text, NULL) : NULL;
  if (!utf8) return;
  post_text(utf8);
  (*env)->ReleaseStringUTFChars(env, text, utf8);
}

JNIEXPORT void JNICALL
Java_org_orca_OrcaActivity_nativeKey(JNIEnv *env, jclass type, jint key)
{
  (void)env; (void)type;
  uint32_t code = key == 67 ? AX_KEY_BACKSPACE : key == 66 ? AX_KEY_ENTER : key == 112 ? AX_KEY_DEL : 0;
  if (!code) return;
  axAndroidPostMessage((struct AXmessage){ .message = kEventKeyDown, .wParam = code });
  axAndroidPostMessage((struct AXmessage){ .message = kEventKeyUp, .wParam = code });
}

JNIEXPORT void JNICALL
Java_org_orca_OrcaActivity_nativeUrl(JNIEnv *env, jclass type, jstring url)
{
  (void)type;
  char const *utf8 = url ? (*env)->GetStringUTFChars(env, url, NULL) : NULL;
  if (!utf8) return;
  Signin_AndroidURL(utf8);
  (*env)->ReleaseStringUTFChars(env, url, utf8);
}

JNIEXPORT void JNICALL
Java_org_orca_OrcaActivity_nativeInsets(JNIEnv *env, jclass type, jfloat left, jfloat top,
                                        jfloat right, jfloat bottom, jfloat keyboard)
{
  (void)env; (void)type;
  axAndroidSetInsets(left, top, right, bottom, keyboard);
}

/* ── soft keyboard ─────────────────────────────────────────────────────── */

void
ORCA_Android_SyncKeyboard(void)
{
  static int shown = -1, shownType = -1;
  struct Object *focus = core_GetFocus();
  struct Input *input = focus ? GetInput(focus) : NULL;
  int visible = input && input->Type != kInputTypeCheckbox && input->Type != kInputTypeRadio;
  // Java maps these to EditorInfo input types.
  int type = !input ? 0 : input->Type == kInputTypeNumber ? 1 : input->Type == kInputTypePhone ? 2
           : input->Type == kInputTypeEmail ? 3 : input->Type == kInputTypeUrl ? 4
           : input->Type == kInputTypeDecimal ? 5 : input->Type == kInputTypePassword ? 6 : 0;
  if (visible == shown && (!visible || type == shownType)) return;
  if (Android_ShowKeyboard(visible, type)) { shown = visible; shownType = type; }
}

/* ── bundled files ─────────────────────────────────────────────────────── */

static int
make_parents(char *path)
{
  for (char *slash = strchr(path + 1, '/'); slash; slash = strchr(slash + 1, '/')) {
    *slash = '\0';
    int failed = mkdir(path, 0700) && errno != EEXIST;
    *slash = '/';
    if (failed) return 0;
  }
  return 1;
}

static char *
asset_text(AAssetManager *assets, char const *name)
{
  AAsset *asset = AAssetManager_open(assets, name, AASSET_MODE_BUFFER);
  if (!asset) return NULL;
  off_t length = AAsset_getLength(asset);
  char *text = malloc((size_t)length + 1);
  if (text) { memcpy(text, AAsset_getBuffer(asset), (size_t)length); text[length] = '\0'; }
  AAsset_close(asset);
  return text;
}

/* orca-files.txt lists every bundled file; orca-version.txt changes per build. */
static int
unpack_assets(AAssetManager *assets, char const *root)
{
  char stamp[1024], current[256] = {0};
  snprintf(stamp, sizeof(stamp), "%s/.orca-version", root);
  char *version = asset_text(assets, "orca-version.txt");
  if (!version) return 0;
  FILE *f = fopen(stamp, "r");
  if (f) { if (!fgets(current, sizeof(current), f)) current[0] = '\0'; fclose(f); }
  if (!strcmp(current, version)) { free(version); return 1; }
  char *list = asset_text(assets, "orca-files.txt");
  int ok = list != NULL;
  for (char *line = list ? strtok(list, "\n") : NULL; ok && line; line = strtok(NULL, "\n")) {
    if (!*line || strstr(line, "..")) continue;
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", root, line);
    AAsset *asset = AAssetManager_open(assets, line, AASSET_MODE_STREAMING);
    FILE *out = asset && make_parents(path) ? fopen(path, "wb") : NULL;
    char buffer[16384];
    int n;
    while (out && (n = AAsset_read(asset, buffer, sizeof(buffer))) > 0)
      if (fwrite(buffer, 1, (size_t)n, out) != (size_t)n) ok = 0;
    if (!out) ok = 0;
    if (out && fclose(out)) ok = 0;
    if (asset) AAsset_close(asset);
  }
  free(list);
  if (ok && (f = fopen(stamp, "w"))) { fputs(version, f); fclose(f); }
  else LOG("Could not unpack the bundled application files");
  free(version);
  return ok;
}

/* ── stdio ─────────────────────────────────────────────────────────────── */

/* Android discards stdout and stderr; forward their lines (Lua print, error
 * reports) to logcat under the same tag. */
static void *
forward_stdio(void *fd)
{
  FILE *in = fdopen((int)(intptr_t)fd, "r");
  char line[1024];
  while (in && fgets(line, sizeof(line), in)) {
    line[strcspn(line, "\n")] = '\0';
    __android_log_write(ANDROID_LOG_INFO, "orca", line);
  }
  return NULL;
}

static void
redirect_stdio(void)
{
  static int redirected;
  int fds[2];
  pthread_t thread;
  if (redirected || pipe(fds)) return;
  setvbuf(stdout, NULL, _IOLBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
  dup2(fds[1], STDOUT_FILENO);
  dup2(fds[1], STDERR_FILENO);
  close(fds[1]);
  if (pthread_create(&thread, NULL, forward_stdio, (void *)(intptr_t)fds[0])) return;
  pthread_detach(thread);
  redirected = 1;
}

/* ── main ──────────────────────────────────────────────────────────────── */

static void
run(struct android_app *app)
{
  char const *root = app->activity->internalDataPath;
  if (!root || !unpack_assets(app->activity->assetManager, root)) return;
  char share[1024], project[1024], language[8] = {0};
  snprintf(share, sizeof(share), "%s/share", root);
  snprintf(project, sizeof(project), "%s/project", root);
  if (chdir(project)) { LOG("Cannot open the project at %s", project); return; }
  LOG("Starting %s", project);
  AConfiguration_getLanguage(app->config, language);
  if (language[0] && !getenv("ORCA_LANGUAGE")) setenv("ORCA_LANGUAGE", language, 0);
  lua_State *L = luaL_newstate();
  if (!L) return;
  *(void **)lua_getextraspace(L) = NULL;
  luaL_openlibs(L);
  lua_pushstring(L, root); lua_setglobal(L, "EXENAME");
  lua_pushstring(L, share); lua_setglobal(L, "SHAREDIR");
  lua_pushstring(L, root); lua_setglobal(L, "LIBDIR");
  lua_pushstring(L, root); lua_setglobal(L, "PLUGDIR");
  lua_pushstring(L, project); lua_setglobal(L, "PROJECTDIR");
  lua_pushstring(L, project); lua_setglobal(L, "DATADIR");
  luaL_requiref(L, "orca", luaopen_orca, 0);
  lua_pop(L, 1);
  char const *bootstrap =
    "package.path = SHAREDIR..'/?.lua;'..SHAREDIR..'/?/init.lua;'..package.path\n"
    "local Application = require 'orca.core.application'\n"
    "local Startup = require 'orca.core.startup'\n"
    "local app = assert(Startup.open(Application, DATADIR))\n"
    "return app:run()\n";
  if (luaL_dostring(L, bootstrap) != LUA_OK) LOG("%s", lua_tostring(L, -1));
  lua_close(L);
}

void
android_main(struct android_app *app)
{
  android_app_state = app;
  redirect_stdio();
  axAndroidSetApp(app);
  run(app);
  // The application returned (or failed to start): close the activity.
  ANativeActivity_finish(app->activity);
  while (!app->destroyRequested) {
    int events;
    struct android_poll_source *source = NULL;
    if (ALooper_pollOnce(-1, NULL, &events, (void **)&source) >= 0 && source) source->process(app, source);
  }
  JavaVM *vm = app->activity->vm;
  (*vm)->DetachCurrentThread(vm);
}
