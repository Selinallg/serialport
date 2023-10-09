//
// Created by franks on 2022/6/29.
//

#ifndef NATIVEDEMO_IMUDATA_H
#define NATIVEDEMO_IMUDATA_H


#include <functional>
#include <iostream>

#ifdef WIN32
#define NOLO_DECL_EXPORT __declspec(dllexport)
#define NOLO_DECL_IMPORT __declspec(dllimport)
#define NOLO_DECL_HIDDEN
#else
#define NOLO_DECL_EXPORT __attribute__((visibility("default")))
#define NOLO_DECL_IMPORT __attribute__((visibility("default")))
#define NOLO_DECL_HIDDEN __attribute__((visibility("hidden")))
#endif

#ifdef NOLO_EXPORTS
#define NOLO_API NOLO_DECL_EXPORT
#else
#define NOLO_API NOLO_DECL_IMPORT
#endif

namespace nolo {
    typedef struct NOLO_API ImuData {
        uint64_t _gyroTimestamp = 0;
        int16_t _size = 0;
        int16_t _cmd = 0;
        // 0 -- x 1 -- y 2 -- z
        int16_t _gyro[3] = {0};
        int16_t _accel[3] = {0};
        double _cGyro[3] = {0};
        double _cAccel[3] = {0};
    } ImuData;

    typedef int (*ImuCallBack) (ImuData& data);
    void setImuDataCB(ImuCallBack cb);
}

#endif //NATIVEDEMO_IMUDATA_H
