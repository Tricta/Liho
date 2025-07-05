#include "nativeHooking.h"

#include "global.h"
#include "logUtils.h"
#include "liho.h"

void *(*orig_android_dlopen_ext)(const char *_Nullable __filename, int __flags, const android_dlextinfo *_Nullable __info, const void* caller_addr);

void* hooked_android_dlopen_ext(const char* __filename, int __flags, const android_dlextinfo* __info, const void* caller_addr) {
    void* handle = orig_android_dlopen_ext(__filename, __flags, __info, caller_addr);

    for (const auto& hook : get_registered_native_hooks()) {
        if (strstr(__filename, hook.libName)) {
            void* sym = dlsym(handle, hook.symbolName);
            if (sym) {
                if (DobbyHook(sym, hook.hookFunc, hook.origFuncPtr) == 0) {
                    LOGV("Hooked %s in %s", hook.symbolName, hook.libName);
                } else {
                    LOGE("Failed to hook %s", hook.symbolName);
                }
            } else {
                LOGE("Symbol not found: %s in %s", hook.symbolName, hook.libName);
            }
        }
    }

    return handle;
}