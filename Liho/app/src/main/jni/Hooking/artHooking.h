#pragma once

#include <jni.h>
#include <string>

using doCall_fn = int(*)(void*, void*, void*, void*, int, void*);
using PrettyMethod_fn = std::string(*)(void*, bool);
using Quick2Interp_fn = uint64_t(*)(void*, void*, void*);
using Invoke_fn = void(*)(void*, void*, uint32_t*, uint32_t, void*, const char*);
using FastInterpToInterpInvoke_fn = bool(*)(void*);

struct ArtOrigs {
    doCall_fn       doCall[4]                  = {};
    PrettyMethod_fn PrettyMethod               = nullptr;
    Quick2Interp_fn artQuickToInterpreterBridge = nullptr;
    Invoke_fn       Invoke                     = nullptr;
    FastInterpToInterpInvoke_fn fastInterpToInterpInvoke = nullptr;
};

extern ArtOrigs g_orig;

int hooked_doCall_0(void* a, void* b, void* c, void* d, int e, void* f);
int hooked_doCall_1(void* a, void* b, void* c, void* d, int e, void* f);
int hooked_doCall_2(void* a, void* b, void* c, void* d, int e, void* f);
int hooked_doCall_3(void* a, void* b, void* c, void* d, int e, void* f);
void hooked_Invoke(void* thiz, void* self, uint32_t* args, uint32_t args_size, void* result, const char* shorty);
uint64_t hook_artQuickToInterpreterBridge(void* method, void* thread, void* sp);
bool hook_fastInterpToInterpInvoke(void* method);
