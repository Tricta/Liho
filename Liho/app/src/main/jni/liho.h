#pragma once

#include <vector>

#include "Hooking/nativeHooking.h"

struct NativeHookTarget {
    const char* libName;
    const char* symbolName;
    void* hookFunc;
    void** origFuncPtr;
};

struct DexHookTarget {
    std::string targetMethod;
    std::string newClass;
    std::string newMethod;
    std::string newSig;
    std::string dexFile;
};

enum class DlopenHookMethod { INLINE, PLT };

extern const char* APK_NAME;
extern bool DEBUG;
extern std::string LOGFILTER;

void register_native_hook(const char* libName, const char* symbolName, void* hookFunc, void** origFuncPtr);
const std::vector<NativeHookTarget>& get_registered_native_hooks();

void register_dex_hook(const char* targetMethod, const char* newClass, const char* newMethod, const char* newSig, const char* dexFile);
const std::vector<DexHookTarget>& get_registered_dex_hooks();

void initialize_hooking_framework();
void set_apk_name(const char* name);
void set_debug_enabled(bool enabled);
void set_log_filter(const std::string& filter);
void set_dlopen_hook_method(DlopenHookMethod method);
DlopenHookMethod get_dlopen_hook_method();