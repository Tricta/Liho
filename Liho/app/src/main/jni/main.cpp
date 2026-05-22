#include "zygisk.hpp"
#include "liho.h"
#include "global.h"
#include "Utils/logUtils.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;

int64_t (*orig_isAdbEnabled)() = nullptr;
void (*orig_jwdpDetect)() = nullptr;

class Liho : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        const char *packageNameChars = env->GetStringUTFChars(args->nice_name, nullptr);

        set_apk_name("com.example.hookingdemo");
        set_debug_enabled(true);
        set_log_filter("com.example.hookingdemo");

        if (strcmp(packageNameChars, APK_NAME) == 0) {
            LOGI("find process: %s", APK_NAME);
            if (!g_vm) {
                this->env->GetJavaVM(&g_vm);
            }

            register_dex_hook("com.example.hookingdemo.MainActivity.isDebuggable", 
                "com.example.hookingdemo.Bypass", "isDebuggable", "(Landroid/content/Context;)Z","Bypass.dex");
            
            register_dex_hook("com.example.hookingdemo.MainActivity.isEmulator", 
                "com.example.hookingdemo.Bypass", "isEmulator", "()Z","Bypass.dex");

            register_dex_hook("com.example.hookingdemo.MainActivity.dynamicText", 
                "com.example.hookingdemo.Bypass", "dynamicText", "()Ljava/lang/String;","Bypass.dex");

            set_dlopen_hook_method(DlopenHookMethod::INLINE);

            register_native_hook("libhookingdemo.so", "Java_com_example_hookingdemo_MainActivity_isAdbEnabled", 
                (void*)hooked_isAdbEnabled, (void**)&orig_isAdbEnabled);

            register_native_hook("libhookingdemo.so", "Java_com_example_hookingdemo_MainActivity_jwdpDetect", 
                (void*)hooked_jwdpDetect, (void**)&orig_jwdpDetect);

            initialize_hooking_framework();
        }

        env->ReleaseStringUTFChars(args->nice_name, packageNameChars);
    }

private:
    Api *api;
    JNIEnv *env;

    static int64_t hooked_isAdbEnabled() {
        LOGI("Bypassing isAdbEnabled...");
        return false;
    }

    static void hooked_jwdpDetect() {
        LOGI("Bypassing jwdpDetect...");
        return;
    }
};

REGISTER_ZYGISK_MODULE(Liho)