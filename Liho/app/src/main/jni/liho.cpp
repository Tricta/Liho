#include "liho.h"

#include "global.h"
#include "artMethodHooking.h"
#include "libartHook.h"
#include "dobby/dobby.h"
#include "logUtils.h"
#include "global.h"

const char* APK_NAME = nullptr;
static std::vector<DexHookTarget> registeredDexHooks;
static std::vector<NativeHookTarget> registeredNativeHooks;

void register_dex_hook(const char* targetMethod, const char* newClass, const char* newMethod, const char* newSig, const char* dexFile) {
    registeredDexHooks.push_back(DexHookTarget{targetMethod,newClass,newMethod,newSig,dexFile});
}

const std::vector<DexHookTarget>& get_registered_dex_hooks() {
    return registeredDexHooks;
}

void register_native_hook(const char* libName, const char* symbolName, void* hookFunc, void** origFuncPtr) {
    registeredNativeHooks.push_back(NativeHookTarget{libName, symbolName, hookFunc, origFuncPtr});
}

const std::vector<NativeHookTarget>& get_registered_native_hooks() {
    return registeredNativeHooks;
}

void set_apk_name(const char* name) {
    APK_NAME = name;
}

void initialize_hooking_framework() {
    if (!registeredDexHooks.empty()) {
        unsigned long int PrettyMethodAddr;

        if (find_name("_ZN3art9ArtMethod12PrettyMethodEPS0_b", "libart.so", &PrettyMethodAddr) <
            0) {
            LOGI("can't find: _ZN3art9ArtMethod12PrettyMethodEPS0_b");
            return;
        }

        if (DobbyHook((void *) PrettyMethodAddr, (void *) PrettyMethod,
                      (void **) &orig_PrettyMethod) != 0) {
            LOGE("PrettyMethod hook failed.");
        }

        char *doCallSymbols[4] = {
                "_ZN3art11interpreter6DoCallILb0ELb0EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE",
                "_ZN3art11interpreter6DoCallILb0ELb1EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE",
                "_ZN3art11interpreter6DoCallILb1ELb0EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE",
                "_ZN3art11interpreter6DoCallILb1ELb1EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE"
        };

        void *trampolines[] = {(void *) hooked_doCall_0, (void *) hooked_doCall_1,
                               (void *) hooked_doCall_2, (void *) hooked_doCall_3};

        for (int i = 0; i < 4; ++i) {
            unsigned long int doCallAddr;
            if (find_name(doCallSymbols[i], "libart.so", &doCallAddr) < 0) {
                LOGI("can't find: %s", doCallSymbols[i]);
            }

            if (DobbyHook((void *) doCallAddr, trampolines[i], (void **) &orig_doCall[i]) != 0) {
                LOGE("doCall hook failed for: %s", doCallSymbols[i]);
            }
        }
    }

    if (!registeredNativeHooks.empty()) {
        void *sym = DobbySymbolResolver(nullptr, "android_dlopen_ext");
        if (sym) {
            if (DobbyHook(sym, (void *) hooked_android_dlopen_ext,(void **) &orig_android_dlopen_ext) != 0) {
                LOGE("Failed to hook android_dlopen_ext");
            }
        } else {
            LOGE("Failed to resolve android_dlopen_ext symbol");
        }
    }
}