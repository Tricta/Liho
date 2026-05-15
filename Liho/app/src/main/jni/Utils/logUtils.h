#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <android/log.h>
#include <string>

#define LOG_TAG "liho"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern bool DEBUG;
extern std::string LOGFILTER;

inline bool should_log(const std::string& pretty) {
    if (!DEBUG) return false;
    if (LOGFILTER.empty()) return true;
    return pretty.find(LOGFILTER) != std::string::npos;
}

#endif
