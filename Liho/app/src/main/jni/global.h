#ifndef OPERATOR_GLOBAL_H
#define OPERATOR_GLOBAL_H

#include <jni.h>
#include "Utils/logUtils.h"

extern JavaVM* g_vm;
JNIEnv* GetEnv();

#endif