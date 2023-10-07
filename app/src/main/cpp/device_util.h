//
// Created by Administrator on 2022/9/7.
//

#ifndef NOLO_DEVICE_UTIL_H
#define NOLO_DEVICE_UTIL_H

#include <sched.h>
#include <asm-generic/unistd.h>
#include <pthread.h>
#include <sys/system_properties.h>
#include <unistd.h>
//#include <KinBase/KinLog.hpp>
#include <assert.h>
#include <string>
#include <stdio.h>
//#include <fmt/format.h>

class device_util{
public:
    device_util();

    ~device_util();

    static int get_thread_policy( pthread_attr_t &attr );

    static void show_thread_priority( pthread_attr_t &attr, int policy );

    static int get_thread_priority( pthread_attr_t &attr );

    static int set_thread_priority( pthread_attr_t &attr );

    static void set_thread_policy( pthread_attr_t &attr,  int policy );

    static void set_cur_thread_affinity(int id);

    static void set_thread_priority_self();

    static void set_self_thread_priority();

    static int32_t SystemProp_get(const char *key, int32_t default_value);

    static void SystemProp_set(const char *key, int32_t value);

    static float SystemProp_get_float(const char *key, float default_value);

    static void SystemProp_set_float(const char *key, float value);

    static pid_t getProcessPidByName(const char *proc_name);

    static void set_affinity_mask(uint16_t affinity_mask);

    static int64_t get_timestamp_ms();
};



#endif //NOLO_DEVICE_UTIL_H
