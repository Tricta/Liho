#include "liho.h"

#include "global.h"
#include "Hooking/artHooking.h"
#include "dobby.h"
#include "xhook.h"
#include "Utils/logUtils.h"
#include <cstdio>
#include <cstring>

const char* APK_NAME = nullptr;
bool DEBUG = false;
std::string LOGFILTER;

static std::vector<DexHookTarget> registeredDexHooks;
static std::vector<NativeHookTarget> registeredNativeHooks;

static DlopenHookMethod g_dlopen_hook_method = DlopenHookMethod::INLINE;

ArtOrigs g_orig;

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

void set_dlopen_hook_method(DlopenHookMethod method) {
    g_dlopen_hook_method = method;
}

void set_apk_name(const char* name) {
    APK_NAME = name;
}

void set_debug_enabled(bool enabled) {
    DEBUG = enabled;
}

void set_log_filter(const std::string& filter) {
    LOGFILTER = filter;
}

static std::string find_lib_path(const char* libname) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return "";
    char line[512], path[256];
    while (fgets(line, sizeof(line), fp)) {
        path[0] = '\0';
        if (sscanf(line, "%*lx-%*lx %*s %*s %*s %*s %255s", path) < 1) continue;
        const char* p = strrchr(path, '/');
        if (p && strstr(p + 1, libname)) {
            fclose(fp);
            return path;
        }
    }
    fclose(fp);
    return "";
}

void initialize_hooking_framework() {
    std::string libart_path = find_lib_path("libart.so");
    if (libart_path.empty()) {
        LOGE("libart.so not found in /proc/self/maps");
        return;
    }

    auto sym = [&](const char *name) -> void * {
        void *s = DobbySymbolResolver(libart_path.c_str(), name);
        if (!s) LOGE("cannot find symbol %s", name);
        return s;
    };

    void *prettyMethodSym = sym("_ZN3art9ArtMethod12PrettyMethodEPS0_b");
    if (!prettyMethodSym) return;
    g_orig.PrettyMethod = (PrettyMethod_fn)prettyMethodSym;

    void *invokeSym = sym("_ZN3art9ArtMethod6InvokeEPNS_6ThreadEPjjPNS_6JValueEPKc");
    if (!invokeSym) return;
    if (DobbyHook(invokeSym, (void *)hooked_Invoke, (void **)&g_orig.Invoke) != 0)
        LOGE("Invoke hook failed.");

    void *quickToInterpSym = sym("artQuickToInterpreterBridge");
    if (!quickToInterpSym) return;
    if (DobbyHook(quickToInterpSym, (void *)hook_artQuickToInterpreterBridge, (void **)&g_orig.artQuickToInterpreterBridge) != 0)
        LOGE("quickToInterpreterAddr hook failed.");

    void *fastInterpToInterpInvokeSym = sym("_ZN3art11interpreter37UseFastInterpreterToInterpreterInvokeEPNS_9ArtMethodE");
    if (!fastInterpToInterpInvokeSym) return;
    if (DobbyHook(fastInterpToInterpInvokeSym, (void *)hook_fastInterpToInterpInvoke, (void **)&g_orig.fastInterpToInterpInvoke) != 0)
        LOGE("fastInterpToInterpInvoke hook failed.");

    const char *doCallSymbols[4] = {
        "_ZN3art11interpreter6DoCallILb0ELb0EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE",
        "_ZN3art11interpreter6DoCallILb0ELb1EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE",
        "_ZN3art11interpreter6DoCallILb1ELb0EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE",
        "_ZN3art11interpreter6DoCallILb1ELb1EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE"
    };
    void *trampolines[4] = {
        (void *)hooked_doCall_0, (void *)hooked_doCall_1,
        (void *)hooked_doCall_2, (void *)hooked_doCall_3
    };

    for (int i = 0; i < 4; ++i) {
        void *doCallSym = sym(doCallSymbols[i]);
        if (!doCallSym) continue;
        if (DobbyHook(doCallSym, trampolines[i], (void **)&g_orig.doCall[i]) != 0)
            LOGE("doCall hook failed for: %s", doCallSymbols[i]);
    }

    if (!registeredNativeHooks.empty()) {
        if (g_dlopen_hook_method == DlopenHookMethod::PLT) {
            xhook_register(".*\\.so$", "android_dlopen_ext",
                    (void *)hooked_android_dlopen_ext, (void **)&orig_android_dlopen_ext);
            if (xhook_refresh(0) != 0)
                LOGE("Failed to PLT hook android_dlopen_ext");
        } else {
            void *dlopenSym = DobbySymbolResolver(nullptr, "android_dlopen_ext");
            if (dlopenSym) {
                if (DobbyHook(dlopenSym, (void *)hooked_android_dlopen_ext, (void **)&orig_android_dlopen_ext) != 0)
                    LOGE("Failed to inline hook android_dlopen_ext");
            } else {
                LOGE("Failed to resolve android_dlopen_ext symbol");
            }
        }
    }
}