LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := local_dobby
LOCAL_SRC_FILES := dobby/libs/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := liho
LOCAL_SRC_FILES := liho.cpp nativeHooking.cpp libartHook.cpp global.cpp artMethodHooking.cpp main.cpp
LOCAL_C_INCLUDES := dobby/libs
LOCAL_STATIC_LIBRARIES := local_dobby
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)