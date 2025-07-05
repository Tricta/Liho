#include "zygisk.hpp"
#include "liho.h"
#include "global.h"
#include "logUtils.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;

typedef int64_t (*NativeFunc)(char*);
NativeFunc orig_nativeFunc = nullptr;

class Liho : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        const char *packageNameChars = env->GetStringUTFChars(args->nice_name, nullptr);
        const char* appDataDirChars  = env->GetStringUTFChars(args->app_data_dir, nullptr);

        set_apk_name("com.example.dummy3");

        if (strcmp(packageNameChars, APK_NAME) == 0) {
            LOGI("find process: %s", APK_NAME);
            if (!g_vm) {
                this->env->GetJavaVM(&g_vm);
            }

            register_dex_hook("com.example.dummy3.MainActivity.isRoot", "com.example.dummy3.mydex", "isRoot", "()Ljava/lang/Boolean;","mydex.dex");
            register_native_hook("libdummy3.so", "Java_com_example_dummy3_MainActivity_stringFromNativeCode", (void*)hooked_nativeFunc, (void**)&orig_nativeFunc);

            initialize_hooking_framework();
        }

        env->ReleaseStringUTFChars(args->nice_name, packageNameChars);
        env->ReleaseStringUTFChars(args->app_data_dir, appDataDirChars);
    }

private:
    Api *api;
    JNIEnv *env;

    static int64_t hooked_nativeFunc(char* arg1) {
        JNIEnv* env = GetEnv();
        if (env == nullptr) return (int64_t)orig_nativeFunc(arg1);

        jstring hackedStr = env->NewStringUTF("Hacked");
        return reinterpret_cast<int64_t>(hackedStr);
    }
};

REGISTER_ZYGISK_MODULE(Liho)