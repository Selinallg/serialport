//
// Created by 86183 on 2020/7/6.
//

#ifndef ANDROID_MI_NOLO_HOME_MASTER_LOG_H
#define ANDROID_MI_NOLO_HOME_MASTER_LOG_H

#endif //ANDROID_MI_NOLO_HOME_MASTER_LOG_H

#include <stdio.h>
#include <android/log.h>
#include <errno.h>

#define  LOG_TAG    "glssIMU"

#define  LOG(...)  __android_log_print(ANDROID_LOG_DEBUG,  LOG_TAG, __VA_ARGS__ )
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,  LOG_TAG, __VA_ARGS__ )
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__ )
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__ )
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define  LOGEXP(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG_2, __VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_DEBUG,  LOG_TAG, __VA_ARGS__ )

//LOGV
#define LOG_TEXT_MAX_LENGTH        (1024)  //  单条日志大小
#define LOG_FILE_MAX_SIZE    (1024*1024*2*30) //  文件最大为2*30MB

enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4
};

#ifdef  __cplusplus


#endif