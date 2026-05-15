#include <vector>
#include <algorithm>
#include <unordered_set>
#include <mutex>

#include "artHooking.h"
#include "../Utils/dexUtils.h"
#include "../liho.h"
#include "../global.h"
#include "../Utils/logUtils.h"

static std::unordered_set<void*> g_patched_methods;
static std::mutex g_patch_mtx;

static void* maybe_patch(void* method) {
    if (!method) return method;
    {
        std::lock_guard<std::mutex> lk(g_patch_mtx);
        if (!g_patched_methods.insert(method).second) return method;
    }

    std::string pretty = g_orig.PrettyMethod(method, 0);

    for (const auto& hook : get_registered_dex_hooks()) {
        if (pretty.find(hook.targetMethod) != std::string::npos) {
            JNIEnv* env = GetEnv();
            if (!env) return method;
            jclass newDexClass = (jclass)loadDexAndGetMethod(env, "/data/local/tmp", hook);
            jobject newMethodObj = getMethodObject(env, newDexClass, hook.newMethod.c_str(), hook.newSig.c_str());
            void* newArtMethod = hookARTMethod(env, newMethodObj);
            LOGV("[Patched] %s", pretty.c_str());
            return newArtMethod;
        }
    }
    return method;
}

static void trace_patch_frames(void* shadow_frame, void* callee) {
    std::vector<void*> frames;
    for (void* frame = shadow_frame; frame;
         frame = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(frame))) {
        void* method = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(frame) + sizeof(void*));
        if (method) frames.push_back(method);
    }
    std::reverse(frames.begin(), frames.end());

    std::vector<std::string> frame_names;
    frame_names.reserve(frames.size());
    for (void* f : frames)
        frame_names.push_back(g_orig.PrettyMethod(f, 0));

    std::string callee_pretty = callee ? g_orig.PrettyMethod(callee, 0) : "";

    bool do_log = should_log(callee_pretty);

    if (do_log) {
        LOGI("[DoCall] ┐");
        for (size_t i = 0; i < frame_names.size(); i++) {
            std::string indent(i * 2, ' ');
            LOGI("[DoCall] %s└─ %s", indent.c_str(), frame_names[i].c_str());
        }
        if (callee) {
            std::string indent(frame_names.size() * 2, ' ');
            LOGI("[DoCall] %s└─ %s  <callee>", indent.c_str(), callee_pretty.c_str());
        }
    }

    for (void* f : frames) maybe_patch(f);
    if (callee) maybe_patch(callee);
}

int hooked_doCall_indexed(int index, void* called_method, void* thread, void* shadow_frame, void* inst, int inst_data, void* result) {
    trace_patch_frames(shadow_frame, called_method);
    return g_orig.doCall[index](called_method, thread, shadow_frame, inst, inst_data, result);
}

void hooked_Invoke(void* thiz, void* self, uint32_t* args, uint32_t args_size, void* result, const char* shorty) {
    std::string pretty = g_orig.PrettyMethod(thiz, 0);

    if (should_log(pretty)) {
        LOGI("[Invoke] Unresolved/Reflection -> %s", pretty.c_str());
    }

    maybe_patch(thiz);

    g_orig.Invoke(thiz, self, args, args_size, result, shorty);
}

uint64_t hook_artQuickToInterpreterBridge(void* method, void* thread, void* sp) {
    std::string pretty = g_orig.PrettyMethod(method, 0);

    if (should_log(pretty)) {
        LOGI("[Quick2Interp] Interpreter -> %s", pretty.c_str());
    }

    void* effective_method = maybe_patch(method);

    return g_orig.artQuickToInterpreterBridge(effective_method, thread, sp);
}

bool hook_fastInterpToInterpInvoke(void* method) {
    return false;
}

int hooked_doCall_0(void* a, void* b, void* c, void* d, int e, void* f) { return hooked_doCall_indexed(0, a, b, c, d, e, f); }
int hooked_doCall_1(void* a, void* b, void* c, void* d, int e, void* f) { return hooked_doCall_indexed(1, a, b, c, d, e, f); }
int hooked_doCall_2(void* a, void* b, void* c, void* d, int e, void* f) { return hooked_doCall_indexed(2, a, b, c, d, e, f); }
int hooked_doCall_3(void* a, void* b, void* c, void* d, int e, void* f) { return hooked_doCall_indexed(3, a, b, c, d, e, f); }
