/*
 * @Copyright: nolo
 * @Author: qujunyang
 * @Date: 2022-05-04 19:49:47
 * @LastEditTime: 2022-05-04 19:58:43
 * @LastEditors: qujunyang
 * @FilePath: \NOLOSensor\include\nolo\type.h
 * @Description:
 */
#ifndef NOLO_TYPE_H
#define NOLO_TYPE_H

#include <functional>
#include <iostream>
#include <jni.h>

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

    uint8_t compute_easy_crc(const uint8_t *buf , uint16_t length);
    bool checkCrc(const uint8_t *buf,int len);
    enum class STATUS_TYPE : int32_t {
        OPEN_SUCCESS,
        OPEN_FAILED,
        HAVE_OPENED_DEVICE,
        OPEN_CAMERA_DEVICE_FAILED,
        START_SUCCESS,
        STOP_SUCCESS,
        DEVICE_IS_CLOSED,
        INIT_HID_FAILED,
        OPEN_HID_FAILED,
        CLAIM_HID_FAILED,
        CLAIM_HID_SUCCESS,
        REQUIRE_HID_DATA_ERROR,
        OPEN_HID_SUCCESS,
        HAVE_CAPTURED_DATA,
        START_IMU_STREAM_FAILED,
        START_CAMERA_STREAM_FAILED,
        START_FAILED,
        STOP_FAILED,
        UNSUPPORTED_FPS,
    };

    enum class NOLO_DEVICE_TYPE : int32_t {
        OV580 = 0,
        OV680,
        TAI_SHAN,
        TAI_SHAN_NEW,
        NONE_DEVICE,
    };
    enum class NOLO_DATA_TYPE : int32_t {
        VSYNC = 0,
        NONE,
    };

    enum class OP_TYPE : int32_t {
        ALL,
        CAMERA,
        IMU,
    };

    enum class RECTIFY_TYPE : int32_t {
        NONE,
        RECTIFY_IMU,
        RECTIFY_CAMERA,
        RECTIFY_ALL,
    };

    enum class NOLO_FPS : int32_t {
        NONE = 0,
        FPS_30 = 30,
        FPS_50 = 50,
        FPS_60 = 60,
    };

    enum class IMU_TYPE : int32_t {
        HEAD = 0,
        LEFT_HAND = 1,
        RIGHT_HAND = 2,
        LEFT_RIGHT_HAND = 3,
        ALL = 4,
    };

// imu parameters
    typedef struct NOLO_API IMUIntrinsics {
        double _Ta[3][3] = {{0}};
        double _Ka[3][3] = {{0}};
        double _Ba[3] = {0};
        double _Tg[3][3] = {{0}};
        double _Kg[3][3] = {{0}};
        double _Bg[3] = {0};

        double Ka_data[3]={0};
        double Ba_data[3]={0};
        double Kg_data[3]={0};
        double Bg_data[3]={0};
        IMUIntrinsics();
        ~IMUIntrinsics(){
        }
        IMUIntrinsics(const uint8_t *data, const std::size_t &size,const uint8_t start,
                const NOLO_DEVICE_TYPE &type);
        friend std::ostream &operator<<(std::ostream &os,
                                        const IMUIntrinsics &intrinsics) {
            os << "Ta: [ ";
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Ta[0][i] << ", ";
            }
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Ta[1][i] << ", ";
            }
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Ta[2][i] << ", ";
            }
            os << intrinsics._Ta[2][2] << " ]";

            os << ", Ka: [ ";
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Ka[0][i] << ", ";
            }
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Ka[1][i] << ", ";
            }
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Ka[2][i] << ", ";
            }
            os << intrinsics._Ka[2][2] << " ]";

            os << ", Ba: [ ";
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Ba[i] << ", ";
            }
            os << intrinsics._Ba[2] << " ]" << std::endl;

            os << "Tg: [ ";
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Tg[0][i] << ", ";
            }
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Tg[1][i] << ", ";
            }
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Tg[2][i] << ", ";
            }
            os << intrinsics._Tg[2][2] << " ]";

            os << ", Kg: [ ";
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Kg[0][i] << ", ";
            }
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Kg[1][i] << ", ";
            }
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Kg[2][i] << ", ";
            }
            os << intrinsics._Kg[2][2] << " ]";

            os << ", Ba: [ ";
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Bg[i] << ", ";
            }
            os << intrinsics._Bg[2] << " ]" << std::endl;
            return os;
        }
    } IMUIntrinsics;

// imu data packet
    typedef struct NOLO_API IMUData {
        uint64_t _accelTimestamp = 0;
        uint64_t _gyroTimestamp = 0;
        int16_t _size = 0;
        int16_t _cmd = 0;
        // 0 -- x 1 -- y 2 -- z
        int16_t _gyro[3] = {0};
        int16_t _accel[3] = {0};
        double _cGyro[3] = {0};
        double _cAccel[3] = {0};

        double rotation[4] = {0};

        bool _isCalib = false;
        // 0 头部 IMU 数据  1 左手 IMU 数据  2 右手 IMU 数据
        int32_t _type = 0;
        bool _isValid = false;
        int nrf;

        IMUData();

        IMUData(const uint8_t *data, const std::size_t &size,
                const NOLO_DEVICE_TYPE &type);

        IMUData(const uint8_t *data, const std::size_t &size,
                const NOLO_DEVICE_TYPE &type, const IMUIntrinsics &intrinsics);

        void OV580IMU(const uint8_t *data);

        bool isOV680(const uint8_t *data, const std::size_t &size);

        void OV680IMU(const uint8_t *data, const std::size_t &size);

        void tai_shan_imu(const uint8_t *data, const std::size_t &size,const NOLO_DEVICE_TYPE &type);

        ~IMUData(){
        }
        friend std::ostream &operator<<(std::ostream &os, const IMUData &data) {
            os << "imu type: " << data._type << std::endl;
            os << "gyro [ " << data._gyroTimestamp << ", " << data._gyro[0] << ", "
               << data._gyro[1] << ", " << data._gyro[2] << " ]" << std::endl;
            os << "accel [ " << data._accelTimestamp << ", " << data._accel[0] << ", "
               << data._accel[1] << ", " << data._accel[2] << " ]" << std::endl;

            if (data._isCalib) {
                os << "calibrated gyro [ " << data._gyroTimestamp << ", "
                   << data._cGyro[0] << ", " << data._cGyro[1] << ", " << data._cGyro[2]
                   << " ]" << std::endl;
                os << "calibrated accel [ " << data._accelTimestamp << ", "
                   << data._cAccel[0] << ", " << data._cAccel[1] << ", "
                   << data._cAccel[2] << " ]" << std::endl;
            }

            return os;
        }
    } IMUData;

// nsync data packet
    typedef struct NOLO_API NSyncData {
        int16_t _size = 0;
        int16_t _cmd = 0;
        uint64_t _syncTimestamp = 0;
        uint32_t _syncTimestamp32 = 0;

        NSyncData();

        NSyncData(const uint8_t *data, const std::size_t &size);

        NSyncData(NOLO_DATA_TYPE type, const uint8_t *data, const std::size_t &size);

        void tai_shan_vsync(const uint8_t *data, const std::size_t &size);

        void ProcessNSyncData(const uint8_t *data, const std::size_t &size);
    } NSyncData; // namespace nolo


    int getDeviceType(const uint8_t *data,uint8_t dev_type);


    typedef struct NOLO_API TouchPad{
        uint8_t x;
        uint8_t y;
        TouchPad(){
            x = 0;
            y = 0;
        }
    }TouchPad;

    typedef struct NOLO_API OtherData {
        uint8_t Vsync_update_num;//8 同一个数据连续发3次，序号顺序由3变到1
        uint64_t Vsync_time;//9-12 uint32，小端模式， 单位是us
        uint8_t  p_sensor;//13 0-100， 单位是0.1mm, 范围0-10mm
        uint8_t IPD_data;//14 0-100， 单位是0.1mm, 范围0-10mm
        uint8_t t_left; //15 16
        uint8_t t_right;// 17 18
        uint8_t bright_value;// 18 19
        bool _isValid = false;
        OtherData(){

        }

        OtherData(const uint8_t *data, const std::size_t &size){
            ProcessOtherData(data,size);
        }

        void ProcessOtherData(const uint8_t *data, const std::size_t &size);
    } OtherData; // namespace nolo

   void onUsbData(unsigned char *buf, int bSize);

    void doParse(const unsigned char *buf, int bSize);
}
#endif  // NOLO_TYPE_H
