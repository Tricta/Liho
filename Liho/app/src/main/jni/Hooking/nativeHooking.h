#include <android/dlext.h>
#include <cstring>
#include <dlfcn.h>
#include "../dobby/dobby.h"

extern void *(*orig_android_dlopen_ext)(const char *_Nullable __filename, int __flags, const android_dlextinfo *_Nullable __info, const void* caller_addr);
void* hooked_android_dlopen_ext(const char* __filename, int __flags, const android_dlextinfo* __info, const void* caller_addr);
