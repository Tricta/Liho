#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include "../liho.h"

jobject loadDexAndGetMethod(JNIEnv* env, const char* workingDir, const DexHookTarget& hook);
std::vector<jclass> parseParamTypes(JNIEnv* env, const std::string& sig);
jobject getMethodObject(JNIEnv* env, jclass clazz, const char* methodName, const char* methodSig);
void* hookARTMethod(JNIEnv* env, jobject targetObject);
