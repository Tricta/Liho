#include "nativeHooking.h"

#include "../global.h"
#include "../Utils/logUtils.h"
#include "../liho.h"
#include "xhook.h"

void *(*orig_android_dlopen_ext)(const char *_Nullable __filename, int __flags, const android_dlextinfo *_Nullable __info, const void* caller_addr);

void* hooked_android_dlopen_ext(const char* __filename, int __flags, const android_dlextinfo* __info, const void* caller_addr) {
    void* handle = orig_android_dlopen_ext(__filename, __flags, __info, caller_addr);

    if (!__filename || !handle) return handle;

    for (const auto& hook : get_registered_native_hooks()) {
        if (!strstr(__filename, hook.libName)) continue;

        if (get_dlopen_hook_method() == DlopenHookMethod::PLT) {
            xhook_register(__filename, hook.symbolName, hook.hookFunc, hook.origFuncPtr);
            if (xhook_refresh(0) != 0)
                LOGE("PLT hook failed: %s in %s", hook.symbolName, hook.libName);
            else
                LOGV("PLT hooked %s in %s", hook.symbolName, hook.libName);
        } else {
            void* sym = dlsym(handle, hook.symbolName);
            if (sym) {
                if (DobbyHook(sym, hook.hookFunc, hook.origFuncPtr) == 0)
                    LOGV("Hooked %s in %s", hook.symbolName, hook.libName);
                else
                    LOGE("Failed to hook %s", hook.symbolName);
            } else {
                LOGE("Symbol not found: %s in %s", hook.symbolName, hook.libName);
            }
        }
    }

    return handle;
}
