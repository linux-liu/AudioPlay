//
// Created by liuxin on 19-8-24.
//

#ifndef AUDIOPLAY_MYLOG_H
#define AUDIOPLAY_MYLOG_H

#include <android/log.h>
#define  IS_DEBUG 1
#define LOG_TAG "ffmpeg"
#define ALOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define ALOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define ALOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#endif //AUDIOPLAY_MYLOG_H
