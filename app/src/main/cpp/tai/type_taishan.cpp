/*
 * @Copyright: nolo
 * @Author: qujunyang
 * @Date: 2022-05-04 19:50:50
 * @LastEditTime: 2022-05-04 19:59:41
 * @LastEditors: qujunyang
 * @FilePath: \NOLOSensor\src\nolo\type.cpp
 * @Description:
 */
#include "utils.h"
#include "type_taishan.h"
#include <android/log.h>
#define TAG "UsbProxy-jni" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型



namespace nolo {

    IMUData::IMUData() {}

    IMUData::IMUData(const uint8_t *data, const std::size_t &size,
                     const NOLO_DEVICE_TYPE &type) {
        switch (type) {
            case NOLO_DEVICE_TYPE::OV580: {
                OV580IMU(data);
                break;
            }
            case NOLO_DEVICE_TYPE::OV680: {
                OV680IMU(data, size);
                break;
            }
            case NOLO_DEVICE_TYPE::TAI_SHAN_NEW:
            case NOLO_DEVICE_TYPE::TAI_SHAN: {
                 tai_shan_imu(data, size,type);
                 break;
            }
            default:
                // TODO: default imu device
                break;
        }
    }

    IMUData::IMUData(const uint8_t *data, const std::size_t &size,
                     const NOLO_DEVICE_TYPE &type,
                     const IMUIntrinsics &intrinsics) {
        _isCalib = true;
        switch (type) {
            case NOLO_DEVICE_TYPE::OV580: {
                OV580IMU(data);
                _cAccel[0] = intrinsics._Ka[0][0] * (_accel[0] - intrinsics._Ba[0]);
                _cAccel[1] = intrinsics._Ka[1][1] * (_accel[1] - intrinsics._Ba[1]);
                _cAccel[2] = intrinsics._Ka[2][2] * (_accel[2] - intrinsics._Ba[2]);

                _cGyro[0] = intrinsics._Kg[0][0] * (_gyro[0] - intrinsics._Bg[0]);
                _cGyro[1] = intrinsics._Kg[1][1] * (_gyro[1] - intrinsics._Bg[1]);
                _cGyro[2] = intrinsics._Kg[2][2] * (_gyro[2] - intrinsics._Bg[2]);
                break;
            }
            case NOLO_DEVICE_TYPE::OV680: {
                OV680IMU(data, size);
                _cAccel[0] = intrinsics._Ka[0][0] * (_accel[0] - intrinsics._Ba[0]);
                _cAccel[1] = intrinsics._Ka[1][1] * (_accel[1] - intrinsics._Ba[1]);
                _cAccel[2] = intrinsics._Ka[2][2] * (_accel[2] - intrinsics._Ba[2]);

                _cGyro[0] = intrinsics._Kg[0][0] * (_gyro[0] - intrinsics._Bg[0]);
                _cGyro[1] = intrinsics._Kg[1][1] * (_gyro[1] - intrinsics._Bg[1]);
                _cGyro[2] = intrinsics._Kg[2][2] * (_gyro[2] - intrinsics._Bg[2]);
                break;
            }
            default:
                // TODO: default imu device
                break;
        }
    }

    void IMUData::OV580IMU(const uint8_t *data) {
        int32_t offset = 44;
        _type = 0;
        _isValid = true;
        OnUnpackNum<uint64_t>(data + offset, &_gyroTimestamp);
        //    static uint64_t last = _gyroTimestamp;
        //    if (_gyroTimestamp - last > 1500000) {
        //      std::cout << "lost imu: " << _gyroTimestamp << " last: " << last
        //                << std::endl;
        //    }
        //    last = _gyroTimestamp;
        offset += 16;
        OnUnpackNum<int16_t>(data + offset, &_gyro[0]);
        offset += 4;
        OnUnpackNum<int16_t>(data + offset, &_gyro[1]);
        offset += 4;
        OnUnpackNum<int16_t>(data + offset, &_gyro[2]);
        offset += 4;

        OnUnpackNum<uint64_t>(data + offset, &_accelTimestamp);
        offset += 16;
        OnUnpackNum<int16_t>(data + offset, &_accel[0]);
        offset += 4;
        OnUnpackNum<int16_t>(data + offset, &_accel[1]);
        offset += 4;
        OnUnpackNum<int16_t>(data + offset, &_accel[2]);

        //    if (_accel[0] > 32767) {
        //      _accel[0] = 65535 - _accel[0];
        //    }
        //    if (_accel[0] < -32767) {
        //      _accel[0] = 65535 + _accel[0];
        //    }
        //    if (_accel[1] > 32767) {
        //      _accel[1] = 65535 - _accel[1];
        //    }
        //    if (_accel[1] < -32767) {
        //      _accel[1] = 65535 + _accel[1];
        //    }
        //    if (_accel[2] > 32767) {
        //      _accel[2] = 65535 - _accel[2];
        //    }
        //    if (_accel[2] < -32767) {
        //      _accel[2] = 65535 + _accel[2];
        //    }
    }

    bool IMUData::isOV680(const uint8_t *data, const std::size_t &size) {
        if (size < 2) {
            return false;
        }

        int32_t offset = 0;
        _type = data[offset];
        OnUnpackNum<int16_t>(data + offset, &_size);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_cmd);
        if (_cmd == 0x036A)
            return true;

        return false;
    }

    void IMUData::OV680IMU(const uint8_t *data, const std::size_t &size) {
        int32_t offset = 4;
        _type = data[offset];
        // std::cout << "type: " << _type << std::endl;
        //    if (data[offset] != 0) {
        //      return;
        //    }
        offset += 1;
        OnUnpackNum<uint64_t>(data + offset, &_gyroTimestamp);
        // std::cout << "imu timestamp: " << _gyroTimestamp << std::endl;
        //    if (_type == 0) {
        //      static uint64_t last = _gyroTimestamp;
        //      if (_gyroTimestamp - last > 1500000) {
        //        std::cout << "lost imu: " << _gyroTimestamp << " last: " << last
        //                  << std::dec << std::endl;
        //      }
        //      last = _gyroTimestamp;
        //    }
        offset += 8;
        OnUnpackNum<int16_t>(data + offset, &_gyro[0]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_gyro[1]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_gyro[2]);
        OnUnpackNum<int16_t>(data + offset, &_accel[0]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_accel[1]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_accel[2]);

        //    std::cout << "gyro norm: "
        //              << _gyro[0] * _gyro[0] + _gyro[1] * _gyro[1] + _gyro[2] *
        //              _gyro[2]
        //              << std::endl;
    }

    void IMUData::tai_shan_imu(const uint8_t *data, const std::size_t &size,const NOLO_DEVICE_TYPE &type) {

        int32_t offset = 8;
//        LOGD("USBProxy-jni tai_shan_imu length = %x", *(int16_t*)(data));
//        LOGD("USBProxy-jni tai_shan_imu head = %x", *(int16_t*)(data + 2));
//        LOGD("USBProxy-jni tai_shan_imu length2 = %x", *(int16_t*)(data + 4));
//        LOGD("USBProxy-jni tai_shan_imu cmd = %x", *(int16_t*)(data + 6));

        // std::cout << "type: " << _type << std::endl;
        //    if (data[offset] != 0) {
        //      return;
        //    }
        _isValid = true;
        if (type != NOLO_DEVICE_TYPE::TAI_SHAN_NEW){
            _type = data[offset];
            offset += 1;
        } else{
            _isValid = nolo::checkCrc(data,size);
        }
        OnUnpackNum<uint64_t>(data + offset, &_gyroTimestamp);

//        LOGD("USBProxy-jni tai_shan_imu timestamp = %x",_gyroTimestamp );
        // std::cout << "imu timestamp: " << _gyroTimestamp << std::endl;
        //    if (_type == 0) {
        //      static uint64_t last = _gyroTimestamp;
        //      if (_gyroTimestamp - last > 1500000) {
        //        std::cout << "lost imu: " << _gyroTimestamp << " last: " << last
        //                  << std::dec << std::endl;
        //      }
        //      last = _gyroTimestamp;
        //    }
        offset += 8;
        OnUnpackNum<int16_t>(data + offset, &_gyro[0]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_gyro[1]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_gyro[2]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_accel[0]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_accel[1]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_accel[2]);
        offset += 2;
        nrf = data[offset]&0xff;

        //    std::cout << "gyro norm: "
        //              << _gyro[0] * _gyro[0] + _gyro[1] * _gyro[1] + _gyro[2] *
        //              _gyro[2]
        //              << std::endl;
    }

    NSyncData::NSyncData() {}

    NSyncData::NSyncData(const uint8_t *data, const std::size_t &size) {
        ProcessNSyncData(data, size);
    }

    NSyncData::NSyncData(NOLO_DATA_TYPE type, const uint8_t *data, const std::size_t &size) {
        switch (type) {
            case NOLO_DATA_TYPE::VSYNC:
                tai_shan_vsync(data, size);
                break;
            default:
                break;
        }
    }

    void NSyncData ::tai_shan_vsync(const uint8_t *data, const std::size_t &size) {
        int32_t offset = 8;
        int8_t vsync_update_num;
        OnUnpackNum<int8_t>(data + offset, &vsync_update_num);
        offset += 1;
//        LOGD("USBProxy-jni tai_shan_vsync vsync_update_num = %d", vsync_update_num);
//
//
//        LOGD("USBProxy-jni tai_shan_vsync length = %x", *(int16_t *) (data));
//        LOGD("USBProxy-jni tai_shan_vsync head = %x", *(int16_t *) (data + 2));
//        LOGD("USBProxy-jni tai_shan_vsync length2 = %x", *(int16_t *) (data + 4));
//        LOGD("USBProxy-jni tai_shan_vsync cmd = %x", *(int16_t *) (data + 6));

        if (vsync_update_num == 3) {
            // uint32_t vsync_time = 0;
            // OnUnpackNum<uint32_t>(data + offset, &vsync_time);
            OnUnpackNum<uint32_t>(data + offset, &_syncTimestamp32);
            // _syncTimestamp = static_cast<uint64_t>(vsync_time);
//            LOGD("USBProxy-jni tai_shan_vsync timestamp = %u", _syncTimestamp32);
        }
    }

    void NSyncData::ProcessNSyncData(const uint8_t *data, const std::size_t &size) {
        int32_t offset = 0;
        OnUnpackNum<int16_t>(data + offset, &_size);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_cmd);
        offset += 2;
        OnUnpackNum<uint64_t>(data + offset, &_syncTimestamp);
    }

    int getDeviceType(const uint8_t *data,uint8_t dev_type) {
        int32_t offset = 6;// taishan
        //int32_t offset = 2;// kuafu
        if (dev_type == 1) {
            offset = 2;
        }
        int16_t _cmd = 0;

//        int16_t length, head, length2;
//        OnUnpackNum<int16_t>(data, &length);
//        OnUnpackNum<int16_t>(data + 2, &head);
//        OnUnpackNum<int16_t>(data + 4, &length2);
        OnUnpackNum<int16_t>(data + offset, &_cmd);
//        std::cout << "length: " << length << std::endl;
//        std::cout << "head: " << head << std::endl;
//        std::cout << "length2: " << length2 << std::endl;
//        std::cout << "cmd: " << _cmd << std::endl;
//        LOGD("USBProxy-jni getDeviceType _cmd = %x", _cmd );// 查看指令
//        LOGD("TerminalFragment_updateDatas length = %x", length );
//        LOGD("TerminalFragment_updateDatas head = %x",head );
//        LOGD("TerminalFragment_updateDatas length2 = %x", length2 );
        if (_cmd == 0x036A) {
            return 0;
        }
        else if (_cmd == 0x036B) {
            return 1;
        } else if (_cmd == 0x026A) {
            // this imu
            return 2;
        } else if (_cmd == 0x0F01) {
            // vsync
            if (dev_type ==3){
                return 5;
            }
            return 3;
        } else if (_cmd == 0x012A) {
            return 4;
        } else if (_cmd == 0x012B) {
            return 6;
        } else if (_cmd == 0x012C) {
            return 7;
        }else if (_cmd == 0x012D) {
            return 8;
        }
        else if (_cmd == 0x012E) {
            return 9;
        }
        else if (_cmd == 0x012F) {
            return 10;
        }else if(_cmd == 0x0101){
            //开启IMU
            return 11;
        } else if (_cmd == 0x0102){
            //关闭IMU
            return 12;
        } else if (_cmd == 0x0202){
            return 13;
        }else if (_cmd == 0x0132){
            return 14;
        } else if (_cmd == 0x0129) {
            LOGE("devices code  %x device_type %d",_cmd,dev_type);
            if (dev_type == 3){
                return 15;
            }
            return 4;
        }else if (_cmd == 0x0134){
            return 16;
        }else if (_cmd == 0x0135){
            return 17;
        }
        else {
            return 99;
        }
    }

    float charBuf2Float(const uint8_t *data,int start){
        float res= 0.0f;
        memcpy(&res,data+start, sizeof(float ));
        return res;
    }

    IMUIntrinsics::IMUIntrinsics(const uint8_t *data, const std::size_t &size, const uint8_t start,
                                 const NOLO_DEVICE_TYPE &type) {
        if (type == NOLO_DEVICE_TYPE::TAI_SHAN_NEW ){
            int float2ByteLength = 4;
            Ka_data[0] = charBuf2Float(data, start);
            Ka_data[1] = charBuf2Float(data, start + float2ByteLength);
            Ka_data[2] = charBuf2Float(data, start + float2ByteLength * 2);
            Ba_data[0] = charBuf2Float(data, start + float2ByteLength * 3);
            Ba_data[1] = charBuf2Float(data, start + float2ByteLength * 4);
            Ba_data[2] = charBuf2Float(data, start + float2ByteLength * 5);

            Kg_data[0] = charBuf2Float(data, start + float2ByteLength * 6);
            Kg_data[1] = charBuf2Float(data, start + float2ByteLength * 7);
            Kg_data[2] = charBuf2Float(data, start + float2ByteLength * 8);

            Bg_data[0] = charBuf2Float(data, start + float2ByteLength * 9);
            Bg_data[1] = charBuf2Float(data, start + float2ByteLength * 10);
            Bg_data[2] = charBuf2Float(data, start + float2ByteLength * 11);
        } else {
            Ka_data[0] = charBuf2Float(data,9);
            Ka_data[1] = charBuf2Float(data,9+4);
            Ka_data[2] = charBuf2Float(data,9+8);
            Ba_data[0] = charBuf2Float(data,9+12);
            Ba_data[1] = charBuf2Float(data,9+16);
            Ba_data[2] = charBuf2Float(data,9+20);

            Kg_data[0] = charBuf2Float(data,9+24);
            Kg_data[1] = charBuf2Float(data,9+28);
            Kg_data[2] = charBuf2Float(data,9+32);

            Bg_data[0] = charBuf2Float(data,9+36);
            Bg_data[1] = charBuf2Float(data,9+40);
            Bg_data[2] = charBuf2Float(data,9+44);
        }


    }

    void OtherData::ProcessOtherData(const uint8_t *data, const std::size_t &size) {
        _isValid = nolo::checkCrc(data,size);
        int offset = 8;
        OnUnpackNum<uint64_t>(data + offset, &Vsync_time);
        offset += 8;
        p_sensor = data[offset];
        offset += 1;
        IPD_data = data[offset];
        offset += 1;
        t_left = data[offset];
        offset += 1;
        t_right = data[offset];
        offset += 1;
        OnUnpackNum<uint8_t>(data + offset, &bright_value);
    }

    uint8_t compute_easy_crc(const uint8_t *buf , uint16_t length){
        uint8_t crc8 = 0;
        while(length --){
            crc8 = crc8 ^ (*buf++);
        }
        return crc8;
    }

    bool checkCrc(const uint8_t *buf,int len){
        return buf[len-1] == compute_easy_crc(buf,len-1);
    }

}