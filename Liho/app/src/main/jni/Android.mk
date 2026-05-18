LOCAL_PATH := $(call my-dir)

DOBBY_SRC   := $(LOCAL_PATH)/dobby
DOBBY_BUILD := $(DOBBY_SRC)/.build/$(TARGET_ARCH_ABI)

$(shell mkdir -p $(DOBBY_BUILD))
$(shell cmake \
    -S $(DOBBY_SRC) \
    -B $(DOBBY_BUILD) \
    -DCMAKE_TOOLCHAIN_FILE=$(NDK_ROOT)/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$(TARGET_ARCH_ABI) \
    -DANDROID_PLATFORM=android-29 \
    -DCMAKE_BUILD_TYPE=Release \
    -DDOBBY_DEBUG=OFF \
    -DDOBBY_GENERATE_SHARED=OFF \
    "-DPlugin.GlobalOffsetTableHook=ON" \
    > $(DOBBY_BUILD)/cmake_configure.log 2>&1)
$(shell cmake --build $(DOBBY_BUILD) --target dobby \
    > $(DOBBY_BUILD)/cmake_build.log 2>&1)


include $(CLEAR_VARS)

LOCAL_MODULE            := local_dobby
LOCAL_SRC_FILES         := dobby/.build/$(TARGET_ARCH_ABI)/libdobby.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/dobby/include
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE     := xhook
LOCAL_CFLAGS     := "-DPAGE_SIZE=4096" "-DPAGE_MASK=(~(PAGE_SIZE-1))"
LOCAL_SRC_FILES  := \
    xhook/libxhook/jni/xhook.c \
    xhook/libxhook/jni/xh_core.c \
    xhook/libxhook/jni/xh_elf.c \
    xhook/libxhook/jni/xh_log.c \
    xhook/libxhook/jni/xh_util.c \
    xhook/libxhook/jni/xh_version.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/xhook/libxhook/jni
LOCAL_LDLIBS     := -llog
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE           := liho
LOCAL_SRC_FILES        := \
    liho.cpp \
    Hooking/nativeHooking.cpp \
    global.cpp \
    Hooking/artHooking.cpp \
    Utils/dexUtils.cpp \
    main.cpp

LOCAL_C_INCLUDES       := $(LOCAL_PATH)/dobby/include $(LOCAL_PATH)/xhook/libxhook/jni $(LOCAL_PATH)
LOCAL_STATIC_LIBRARIES := local_dobby xhook
LOCAL_LDLIBS           := -llog

include $(BUILD_SHARED_LIBRARY)
