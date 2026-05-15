#include "dexUtils.h"
#include "../liho.h"
#include "logUtils.h"

jobject loadDexAndGetMethod(JNIEnv* env, const char* workingDir, const DexHookTarget& hook) {
    LOGV("loading Dex and get new method...");
    char dexFullPathStr[PATH_MAX];
    snprintf(dexFullPathStr, sizeof(dexFullPathStr), "%s/%s", workingDir, hook.dexFile.c_str());

    jclass dexLoaderClass = env->FindClass("dalvik/system/DexClassLoader");
    jmethodID dexLoaderInit = env->GetMethodID(dexLoaderClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");

    jclass contextClass = env->FindClass("android/app/ActivityThread");
    jmethodID currentApplication = env->GetStaticMethodID(contextClass, "currentApplication", "()Landroid/app/Application;");
    jobject app = env->CallStaticObjectMethod(contextClass, currentApplication);

    jclass appClass = env->GetObjectClass(app);
    jmethodID getClassLoader = env->GetMethodID(appClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject appClassLoader = env->CallObjectMethod(app, getClassLoader);

    jstring dexPathStr = env->NewStringUTF(dexFullPathStr);

    char cacheFullPathStr[PATH_MAX];
    snprintf(cacheFullPathStr, sizeof(cacheFullPathStr), "%s/code_cache", workingDir);
    jstring odexDirStr = env->NewStringUTF(cacheFullPathStr);

    jobject dexClassLoader = env->NewObject(dexLoaderClass, dexLoaderInit, dexPathStr, odexDirStr, NULL, appClassLoader);

    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    jmethodID loadClassMethod = env->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring className = env->NewStringUTF(hook.newClass.c_str());
    jobject loadedClass = env->CallObjectMethod(dexClassLoader, loadClassMethod, className);

    return (jclass)loadedClass;
}

std::vector<jclass> parseParamTypes(JNIEnv* env, const std::string& sig) {
    std::vector<jclass> params;
    size_t i = 1;

    while (sig[i] != ')') {
        if (sig[i] == 'L') {
            size_t end = sig.find(';', i);
            std::string cls = sig.substr(i + 1, end - i - 1);

            jclass c = env->FindClass(cls.c_str());
            params.push_back(c);

            i = end + 1;
        }
        else if (sig[i] == '[') {
            size_t start = i;

            while (sig[i] == '[')
                i++;

            if (sig[i] == 'L') {
                size_t end = sig.find(';', i);
                i = end + 1;
            } else {
                i++;
            }

            std::string arraySig = sig.substr(start, i - start);

            jclass c = env->FindClass(arraySig.c_str());

            params.push_back(c);
        }
        else {
            const char* wrapper = nullptr;

            switch (sig[i]) {
                case 'I': wrapper = "java/lang/Integer"; break;
                case 'Z': wrapper = "java/lang/Boolean"; break;
                case 'J': wrapper = "java/lang/Long"; break;
                case 'F': wrapper = "java/lang/Float"; break;
                case 'D': wrapper = "java/lang/Double"; break;
                case 'B': wrapper = "java/lang/Byte"; break;
                case 'C': wrapper = "java/lang/Character"; break;
                case 'S': wrapper = "java/lang/Short"; break;
                case 'V': wrapper = "java/lang/Void"; break;
                default:  wrapper = nullptr; break;
            }

            jclass primitiveClass = nullptr;

            if (wrapper) {
                jclass wrapperClass = env->FindClass(wrapper);
                jfieldID typeField = env->GetStaticFieldID(wrapperClass, "TYPE", "Ljava/lang/Class;");
                primitiveClass = (jclass)env->GetStaticObjectField(wrapperClass, typeField);
            }

            params.push_back(primitiveClass);

            i++;
        }
    }

    return params;
}

jobject getMethodObject(JNIEnv* env, jclass clazz, const char* methodName, const char* methodSig) {
    jclass classClass = env->FindClass("java/lang/Class");
    jmethodID getMethod = env->GetMethodID(classClass, "getDeclaredMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;");

    jstring nameStr = env->NewStringUTF(methodName);

    std::vector<jclass> paramTypes = parseParamTypes(env, methodSig);
    jobjectArray params = env->NewObjectArray(paramTypes.size(), classClass, nullptr);

    for (size_t i = 0; i < paramTypes.size(); i++) {
        env->SetObjectArrayElement(params, i, paramTypes[i]);
    }

    jobject method = env->CallObjectMethod(clazz, getMethod, nameStr, params);

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return nullptr;
    }

    return method;
}

void* hookARTMethod(JNIEnv* env, jobject targetObject) {
    jclass executableClass = env->FindClass("java/lang/reflect/Executable");
    jfieldID artMethodID = env->GetFieldID(executableClass, "artMethod", "J");
    return (void*) env->GetLongField(targetObject, artMethodID);
}
