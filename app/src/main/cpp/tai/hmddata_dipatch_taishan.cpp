#include <jni.h>
#include <string>
#include "type_taishan.h"
#include <android/log.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "imudata.h"
#include "ESKF_3dofcute.h"

#define TAG "UsbProxy-jni" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"UsbProxy-jni-Raw" ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型
static int deviceType = 2;
static bool use_imu_raw= false;
static bool isFirst = true;
static bool printLog = false;
static jobject jThis = nullptr;
static JavaVM *jvm = nullptr;
static int checkImuCount = 0;
ESKF_3dofcute_status *g_status = NULL;
static bool isNeedReset = false;

// 默认标定参数
float Ka_0 = 0.00479078818490731f, Ka_1 = 0.00478757245358461f, Ka_2 = 0.00477578065730860f;
float Ba_0 = 4.26967238441205f, Ba_1 = -2.89996136905550f, Ba_2 = 9.96415237759038f;
float Kg_0 = 0.00108113074948884f, Kg_1 = 0.00107714119887665f, Kg_2 = 0.00108016357643783f;
float Bg_0 = -9.48548133967388f, Bg_1 = 14.2971375655937f, Bg_2 = 2.01651164479930f;

uint64_t GetTimeNano(clockid_t clk_id) {
    struct timespec t;
    clock_gettime(clk_id, &t);
    uint64_t result = t.tv_sec * 1000000000LL + t.tv_nsec;
    return result;
}

static jmethodID _method_imu = nullptr;
static jmethodID _method_imu_double = nullptr;
static jmethodID _method_nsync = nullptr;
static jmethodID _method_nsync_all = nullptr;
static jmethodID _method_calibration = nullptr;
static jmethodID _method_quaternion = nullptr;
static jmethodID _method_read_device_info_ack = nullptr;
static nolo::IMUIntrinsics *intrinsics = nullptr;
static nolo::ImuCallBack _imuCallBack = nullptr;


#define STATIC_YUZHI 0.02

void static_lock(ESKF_3dofcute_fp_type* gyro_out, ESKF_3dofcute_fp_type* gyro_in) {
    if (fabs(gyro_in[0]) < STATIC_YUZHI && fabs(gyro_in[1]) < STATIC_YUZHI &&
        fabs(gyro_in[2]) < STATIC_YUZHI) {
        gyro_out[0] = 0;
        gyro_out[1] = 0;
        gyro_out[2] = 0;
    } else {
        gyro_out[0] = gyro_in[0];
        gyro_out[1] = gyro_in[1];
        gyro_out[2] = gyro_in[2];
    }
}

void setImuDataCB(nolo::ImuCallBack cb) {
    if (!_imuCallBack && cb) {
        _imuCallBack = cb;
    }
}


ESKF_3dofcute_fp_type invq0[4] = {1, 0, 0, 0};

static ESKF_3dofcute_fp_type* Q_result = nullptr;
static ESKF_3dofcute_fp_type* ACC_result = nullptr;
static ESKF_3dofcute_fp_type* GYRO_result = nullptr;
static ESKF_3dofcute_fp_type* GYRO_OUT = nullptr;

bool settlement_QuaternionNew(JNIEnv *env, jobject jobj, nolo::IMUData *imuData, jlong timestamp,
                              int32_t acx, int32_t acy, int32_t acz, int32_t gx, int32_t gy,
                              int32_t gz){

    int32_t acc_x = acx;
    int32_t acc_y = acy;
    int32_t acc_z = acz;

    int32_t gyro_x = gx;
    int32_t gyro_y = gy;
    int32_t gyro_z = gz;

    acc_x = acy;
    acc_y = acz;
    acc_z = acx;

    gyro_x = gy;
    gyro_y = gz;
    gyro_z = gx;

    if (deviceType == 3) {
        acc_x = -acy;
        acc_y = -acz;
        acc_z = acx;
        gyro_x = -gy;
        gyro_y = -gz;
        gyro_z = gx;
        //x=-y;
        //y=-z;
        //z=x;
    }


    if (!Q_result){
        Q_result = new ESKF_3dofcute_fp_type[4];
        ACC_result = new ESKF_3dofcute_fp_type[3];
        GYRO_result = new ESKF_3dofcute_fp_type[3];
        GYRO_OUT = new ESKF_3dofcute_fp_type[3];
    }

    static double timePre;
    double timeNow = timestamp * 10e-10;

    double interval =(timeNow - timePre);
    if (interval - 0 <=0.000001f){
        LOGE("--------------------------------%lf",timestamp);
        return false;
    }
    timePre = timeNow;
    if (isNeedReset && !isFirst && g_status != NULL) {
        ESKF_3dofcute_status_delete(&g_status);
        isFirst = true;
        g_status = NULL;
    }

    calculateAccAndGyro(ACC_result,GYRO_result,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z);


    if (isFirst) {
        isFirst = false;
        ESKF_3dofcute_status_new(&g_status);
        isNeedReset = false;
        ESKF_3dofcute_status_init(Q_result, g_status, ACC_result[0], ACC_result[1], ACC_result[2]);
//        LOGE("calcal -----------------------------------------");
//        LOGE("calcal acc %f,%f,%f",ACC_result[0], ACC_result[1], ACC_result[2]);
//        LOGE("calcal res %f,%f,%f,%f",Q_result[0], Q_result[1], Q_result[2], Q_result[3]);

        ESKF_3dofcute_fp_type* Qscreen = new ESKF_3dofcute_fp_type[4];
        ESKF_3dofcute_fp_type* QscreenUnity = new ESKF_3dofcute_fp_type[4];
        ESKF_3dofcute_fp_type* Pscreen = new ESKF_3dofcute_fp_type[3];
        ESKF_3dofcute_fp_type* PscreenUnity = new ESKF_3dofcute_fp_type[3];

        get_Screen_Attitude(Qscreen, Pscreen, Q_result);
//        LOGE("calcal qsc %f,%f,%f,%f",Qscreen[0], Qscreen[1], Qscreen[2], Qscreen[3]);
//        LOGE("calcal psc %f,%f,%f",Pscreen[0], Pscreen[1], Pscreen[2]);

        changePscreen2Unity(PscreenUnity, Pscreen);
        changeQscreen2Unity(QscreenUnity, Qscreen);

//        LOGE("calcal qsce %f,%f,%f,%f",QscreenUnity[0], QscreenUnity[1], QscreenUnity[2], QscreenUnity[3]);
//        LOGE("calcal psce %f,%f,%f",PscreenUnity[0], PscreenUnity[1], PscreenUnity[2]);
//        LOGE("calcal -----------------------------------------");

        jclass clz = env->GetObjectClass(jThis);
        if (clz) {
            jmethodID ack = env->GetMethodID(clz, "onResetQuaternion",
                                                "(DDDDDDD)V");
            if (ack) {
                env->CallVoidMethod(jThis, ack, PscreenUnity[0], PscreenUnity[1], PscreenUnity[2], QscreenUnity[0],
                                    QscreenUnity[1], QscreenUnity[2], QscreenUnity[3]);
            }
        }

        delete[] Qscreen;
        delete[] QscreenUnity;
        delete[] Pscreen;
        delete[] PscreenUnity;
    } else {
        if (interval < 0) {
//             interval = 0;
        }
        if (interval > 0.1) {
            // interval = 0.1;
        }

        static_lock(GYRO_OUT, GYRO_result);
        g_status->t = interval;
        ESKF_3dofcute_step(Q_result, g_status ,ACC_result[0], ACC_result[1], ACC_result[2],
                           GYRO_result[0], GYRO_result[1],GYRO_result[2]);
    }

//    static int logImuCount = 0;
//    logImuCount++;
//    if (logImuCount>=100){
//        logImuCount = 0;
//        LOGD("imuDataQuaternion %d|%d|%d %d|%d|%d",acx,acy,acz,gx,gy,gz);
//        LOGD("imuDataQuaternion %d|%d|%d %.4f|%.4f|%.4f",acc_x,acc_y,acc_z,ACC_result[0],ACC_result[1],ACC_result[2]);
//        LOGD("imuDataQuaternion %d|%d|%d %.4f|%.4f|%.4f",gyro_x,gyro_y,gyro_z,GYRO_result[0],GYRO_result[1],GYRO_result[2]);
//        LOGD("imuDataQuaternion %.4f|%.4f|%.4f|%.4f",Q_result[0],Q_result[1],Q_result[2],Q_result[3]);
//    }
    if (imuData){
//        imuData->rotation[0] = Q_result[0]; //w
//        imuData->rotation[1] = Q_result[1]; //x
//        imuData->rotation[2] = Q_result[3]; //y
//        imuData->rotation[3] = -Q_result[2]; //z
//
//        imuData->_cGyro[0] = GYRO_result[2];
//        imuData->_cGyro[1] = -GYRO_result[0];
//        imuData->_cGyro[2] = -GYRO_result[1];
//
//        imuData->_cAccel[0] = ACC_result[2];
//        imuData->_cAccel[1] = -ACC_result[0];
//        imuData->_cAccel[2] = -ACC_result[1];


        imuData->rotation[0] = Q_result[0]; //w
        imuData->rotation[1] = Q_result[1]; //x
        imuData->rotation[2] = Q_result[2]; //y
        imuData->rotation[3] = Q_result[3]; //z

        imuData->_cGyro[0] = GYRO_result[0];
        imuData->_cGyro[1] = GYRO_result[1];
        imuData->_cGyro[2] = GYRO_result[2];

        imuData->_cAccel[0] = ACC_result[0];
        imuData->_cAccel[1] = ACC_result[1];
        imuData->_cAccel[2] = ACC_result[2];
        return true;
    }
    return false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_nolovr_core_data_usb_UsbProxy_updateDatas(JNIEnv *env, jobject thiz, jbyteArray datas) {

    if (jThis== nullptr){
        jThis = env->NewWeakGlobalRef(thiz);
    }
    if (jvm== nullptr){
        env->GetJavaVM(&jvm);
    }

    jbyte *bBuffer = env->GetByteArrayElements(datas, NULL);
    jsize bSize = env->GetArrayLength(datas); //获取长度
    //LOGD("===TerminalFragment_updateDatas bSize = %d",bSize );

    if (bBuffer) {
        unsigned char *buf = (unsigned char *) bBuffer;
        nolo::doParse(buf, bSize);
    }

    env->ReleaseByteArrayElements(datas, bBuffer, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_nolovr_core_data_usb_UsbProxy_updateDeviceType(JNIEnv *env, jobject thiz, jint type) {
    // 1 kuafu  2 taishan
    deviceType = type;
    jThis = env->NewWeakGlobalRef(thiz);
    env->GetJavaVM(&jvm);
    LOGE("updateDeviceType  deviceType=%d",deviceType);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_nolovr_core_data_usb_UsbProxy_test(JNIEnv *env, jobject thiz, jdouble acc_x, jdouble acc_y,
                                            jdouble acc_z) {
    // TODO: implement test
}
extern "C"
JNIEXPORT void JNICALL
Java_com_nolovr_core_data_usb_UsbProxy_updateLogprint(JNIEnv *env, jobject thiz, jint type) {
    LOGE("updateDeviceType  updateLogprint=%d",type);
    if (type > 0) {
        printLog = true;
    } else {
        printLog = false;
    }
}

void nolo::onUsbData(unsigned char *buf, int bSize) {


    // 不做线程切换

//    if (jvm == NULL) {
//        return;
//    }
//
//    static JNIEnv *jniEnv = nullptr;
//    if (!jniEnv) {
//        if (jvm->AttachCurrentThread(&jniEnv, NULL) != JNI_OK) {
//            LOGE("AttachCurrentThread failed onUsbData. 1-2");
//            return;
//        }
//    }

    doParse(buf, bSize);
}


void callbackIMUIntrinsics(nolo::IMUIntrinsics *intrinsicsData, JNIEnv *jniEnv, bool isNeedSave){
    if (!_method_calibration) {
        //LOGD("USBProxy-jni calibrationData  1-2");
        jclass clz = jniEnv->GetObjectClass(jThis);
        if (clz) {
            //LOGD("USBProxy-jni calibrationData  1-3");
            _method_calibration = jniEnv->GetMethodID(clz, "calibrationData", "(I[F[F[F[F)V");
            //LOGD("USBProxy-jni calibrationData  1-4");
        }
    }

    //LOGD("USBProxy-jni calibrationData  3");
    jfloatArray resultKa = jniEnv->NewFloatArray(3);
    jfloat *abc = new float[3];
    abc[0] = intrinsicsData->Ka_data[0];
    abc[1] = intrinsicsData->Ka_data[1];
    abc[2] = intrinsicsData->Ka_data[2];
    jniEnv->SetFloatArrayRegion(resultKa, 0, 3, abc);
    jniEnv->ReleaseFloatArrayElements(resultKa, abc, 0);


    jfloatArray resultBa = jniEnv->NewFloatArray(3);
    abc = new float[3];
    abc[0] = intrinsicsData->Ba_data[0];
    abc[1] = intrinsicsData->Ba_data[1];
    abc[2] = intrinsicsData->Ba_data[2];
    jniEnv->SetFloatArrayRegion(resultBa, 0, 3, abc);
    jniEnv->ReleaseFloatArrayElements(resultBa, abc, 0);

    jfloatArray resultKg = jniEnv->NewFloatArray(3);
    abc = new float[3];
    abc[0] = intrinsicsData->Kg_data[0];
    abc[1] = intrinsicsData->Kg_data[1];
    abc[2] = intrinsicsData->Kg_data[2];
    jniEnv->SetFloatArrayRegion(resultKg, 0, 3, abc);
    jniEnv->ReleaseFloatArrayElements(resultKg, abc, 0);

    jfloatArray resultBg = jniEnv->NewFloatArray(3);
    abc = new float[3];
    abc[0] = intrinsicsData->Bg_data[0];
    abc[1] = intrinsicsData->Bg_data[1];
    abc[2] = intrinsicsData->Bg_data[2];
    jniEnv->SetFloatArrayRegion(resultBg, 0, 3, abc);
    jniEnv->ReleaseFloatArrayElements(resultBg, abc, 0);

    //LOGD("USBProxy-jni calibrationData  4");

    if (_method_calibration && intrinsicsData) {
        jniEnv->CallVoidMethod(jThis, _method_calibration, isNeedSave?1:0, resultKa, resultBa, resultKg,
                               resultBg);
    } else {
        LOGE("calibrationData   no no no");
    }

}

void nolo::doParse(const unsigned char *buf, int bSize) {
    // LOGD("doParse====sync start");


    if (jvm == NULL) {
        return;
    }

    static JNIEnv *jniEnv = nullptr;
    if (!jniEnv) {
        if (jvm->AttachCurrentThread(&jniEnv, NULL) != JNI_OK) {
            LOGE("AttachCurrentThread failed onUsbData. 1-2");
            return;
        }
    }


    // 打印 解析时间用
//    uint64_t revTime;
//    if (bSize==32){
//        revTime = GetTimeNano(CLOCK_MONOTONIC);
//    }
    if ((jThis != nullptr) && (buf != NULL) && (bSize > 0)) {

        int type = getDeviceType(buf, deviceType);
        if (type == 2) {
            // imu 原始数据
            IMUData *imuData = new IMUData(buf, bSize,deviceType == 3?NOLO_DEVICE_TYPE::TAI_SHAN_NEW: NOLO_DEVICE_TYPE::TAI_SHAN);
            if (deviceType == 3){
                if (!imuData->_isValid){
                    LOGE(" #######do nothing about crc check %d  #########",type);
                    delete imuData;
                    return;
                }
                imuData->_gyroTimestamp = imuData->_gyroTimestamp * 1000;
            }
            checkImuCount++;
//            if (intrinsics == nullptr) {
//                LOGE("checkImuCount=%d", checkImuCount);
//            } else {
//                LOGD("checkImuCount=%d", checkImuCount);
//            }

            if (intrinsics && checkImuCount > 10) {
                checkImuCount = 10;
                static int imuCount = 0;
                imuCount++;
                if (imuCount > 72000) {
                    //3分钟需要写入一次标定参数
                    imuCount = 0;
                    if (intrinsics) {
                        getIntrinsics(intrinsics->Ka_data, intrinsics->Ba_data, intrinsics->Kg_data,
                                      intrinsics->Bg_data);
                        callbackIMUIntrinsics(intrinsics, jniEnv, true);
                    }
                }

                bool isUse = settlement_QuaternionNew(jniEnv, jThis, imuData,
                                                      (jlong) imuData->_gyroTimestamp,
                                                      imuData->_accel[0], imuData->_accel[1],
                                                      imuData->_accel[2],
                                                      imuData->_gyro[0], imuData->_gyro[1],
                                                      imuData->_gyro[2]);

                if (!isUse) {
                    delete imuData;
                    return;
                }

                //LOGD("_method_quaternion 0");
                if (!_method_quaternion && imuData) {
                    jclass clz = jniEnv->GetObjectClass(jThis);
                    if (clz) {
                        _method_quaternion = jniEnv->GetMethodID(clz, "imuDataQuaternion",
                                                                 "(JJDDDDDDDDDD)V");
                    }
                }

                // 回调 四元数
                //LOGD("_method_quaternion 1");
                if (_method_quaternion && imuData->_isValid) {
                    // LOGD("_method_quaternion 2");
                    jniEnv->CallVoidMethod(jThis, _method_quaternion,
                                           (jlong) GetTimeNano(CLOCK_MONOTONIC),
                                           (jlong) imuData->_gyroTimestamp,
                                           imuData->_cAccel[0], imuData->_cAccel[1],
                                           imuData->_cAccel[2],
                                           imuData->_cGyro[0], imuData->_cGyro[1],
                                           imuData->_cGyro[2],
                                           imuData->rotation[0], imuData->rotation[1],
                                           imuData->rotation[2], imuData->rotation[3]);

                    if (printLog) {
                        LOGE("USBProxy-jni_updateDatas  time:%lld, acc0:%d, acc1:%d, acc2:%d, gyro0:%d, gyro1:%d, gyro2:%d",
                             imuData->_gyroTimestamp,
                             imuData->_accel[0], imuData->_accel[1], imuData->_accel[2],
                             imuData->_gyro[0], imuData->_gyro[1], imuData->_gyro[2]);

                        LOGE("USBProxy-jni_updateDatas-calib[%f,%f,%f,%f,%f,%f]",
                             imuData->_cAccel[0], imuData->_cAccel[1], imuData->_cAccel[2],
                             imuData->_cGyro[0], imuData->_cGyro[1], imuData->_cGyro[2]);

                        LOGE("USBProxy-jni_updateDatas-out[%f,%f,%f,%f]", imuData->rotation[0],
                             imuData->rotation[1], imuData->rotation[2], imuData->rotation[3]);
                    }
                }

                if (!use_imu_raw) {
                    delete imuData;
                    return;
                }
            }
            ////// callback  原始数据不再回调，回调标定计算后的 数据回调 imuDataDouble
            if (!_method_imu) {
                jclass clz = jniEnv->GetObjectClass(jThis);
                if (clz) {
                    _method_imu = jniEnv->GetMethodID(clz, "imuData", "(JJSSSSSS)V");
                }
            }
            ////////////
            if (_method_imu && imuData && imuData->_isValid && (intrinsics == nullptr || use_imu_raw)) {

                // 打印解析+结算 用时
//                uint64_t reportTime = GetTimeNano(CLOCK_MONOTONIC);
//                uint64_t chazhi = reportTime - revTime;
//                if (chazhi > 1000000) {
//                    LOGE("__USBProxy-jni NImuData %d     %f", chazhi, ((float) chazhi / (1000000)));
//                } else {
//                   // LOGD("USBProxy-jni NSyncData %d     %f", chazhi, ((float) chazhi / (1000000)));
//                }

                jniEnv->CallVoidMethod(jThis, _method_imu,(jlong) GetTimeNano(CLOCK_MONOTONIC), (jlong) imuData->_gyroTimestamp,
                                    imuData->_accel[0], imuData->_accel[1], imuData->_accel[2],
                                    imuData->_gyro[0], imuData->_gyro[1], imuData->_gyro[2]);

                if (_imuCallBack) {
                    ImuData imudata = {
                            ._gyroTimestamp = imuData->_gyroTimestamp,
                            ._size = imuData->_size,
                            ._cmd = imuData->_cmd,
                            // 0 -- x 1 -- y 2 -- z
                            ._gyro[0] =  imuData->_gyro[0],
                            ._gyro[1] =  imuData->_gyro[1],
                            ._gyro[2] =  imuData->_gyro[2],
                            ._accel[0] = imuData->_accel[0],
                            ._accel[1] = imuData->_accel[1],
                            ._accel[2] = imuData->_accel[2],
                    };
                    (*_imuCallBack)(imudata);
                }
            }
            delete imuData;
        } else if (type == 3) {
            // vsync
            ////// callback
            if (!_method_nsync) {
                jclass clz = jniEnv->GetObjectClass(jThis);
                if (clz) {
                    _method_nsync = jniEnv->GetMethodID(clz, "nsyncData", "(JJ)V");
                }
            }
            ////////////
            NSyncData *nsync = new NSyncData(NOLO_DATA_TYPE::VSYNC, buf, bSize);
            if (nsync->_syncTimestamp32 == 0) {
                delete nsync;
                return;
            }
            if (_method_nsync && nsync) {
                jniEnv->CallVoidMethod(jThis, _method_nsync, (jlong) GetTimeNano(CLOCK_MONOTONIC),(jlong) nsync->_syncTimestamp);
            }
            delete nsync;
        } else if (type == 4) {
            checkImuCount = 0;
            if (intrinsics){
                delete intrinsics;
                intrinsics = nullptr;
            }
            intrinsics = new IMUIntrinsics(buf, bSize, 8, deviceType == 3? NOLO_DEVICE_TYPE::TAI_SHAN_NEW:NOLO_DEVICE_TYPE::TAI_SHAN);
            if (printLog) {
                LOGE("TerminalFragment_updateDatas calibrationData:gyro_offset %.4f|%.4f|%.4f",
                     intrinsics->Bg_data[0], intrinsics->Bg_data[1], intrinsics->Bg_data[2]);
                LOGE("TerminalFragment_updateDatas calibrationData:gyro_scale %.4f|%.4f|%.4f",
                     intrinsics->Kg_data[0], intrinsics->Kg_data[1], intrinsics->Kg_data[2]);
                LOGE("TerminalFragment_updateDatas calibrationData:acc_offset %.4f|%.4f|%.4f",
                     intrinsics->Ba_data[0], intrinsics->Ba_data[1], intrinsics->Ba_data[2]);
                LOGE("TerminalFragment_updateDatas calibrationData:acc_scale %.4f|%.4f|%.4f",
                     intrinsics->Ka_data[0], intrinsics->Ka_data[1], intrinsics->Ka_data[2]);
            }
            //更新算法默认参数


//            float Ka_0 = 0.00479078818490731f, Ka_1 = 0.00478757245358461f, Ka_2 = 0.00477578065730860f;
//            float Ba_0 = 4.26967238441205f, Ba_1 = -2.89996136905550f, Ba_2 = 9.96415237759038f;
//            float Kg_0 = 0.00108113074948884f, Kg_1 = 0.00107714119887665f, Kg_2 = 0.00108016357643783f;
//            float Bg_0 = -9.48548133967388f, Bg_1 = 14.2971375655937f, Bg_2 = 2.01651164479930f;

            if (deviceType == 3){
                initIntrinsics(intrinsics->Ba_data[0], intrinsics->Ba_data[1], intrinsics->Ba_data[2],
                               intrinsics->Ka_data[0], intrinsics->Ka_data[1], intrinsics->Ka_data[2],
                               intrinsics->Bg_data[0], intrinsics->Bg_data[1], intrinsics->Bg_data[2],
                               intrinsics->Kg_data[0], intrinsics->Kg_data[1], intrinsics->Kg_data[2]);
            } else {
                initIntrinsics(intrinsics->Ba_data[1], intrinsics->Ba_data[2], intrinsics->Ba_data[0],
                               intrinsics->Ka_data[1], intrinsics->Ka_data[2], intrinsics->Ka_data[0],
                               intrinsics->Bg_data[1], intrinsics->Bg_data[2], intrinsics->Bg_data[0],
                               intrinsics->Kg_data[1], intrinsics->Kg_data[2], intrinsics->Kg_data[0]);
            }
            callbackIMUIntrinsics(intrinsics,jniEnv, false);
            //LOGD("USBProxy-jni calibrationData  over");

        } else if (type == 5) {// vsync 和 其他数据
            if (!_method_nsync_all) {
                jclass clz = jniEnv->GetObjectClass(jThis);
                if (clz) {
                    _method_nsync_all = jniEnv->GetMethodID(clz, "nsyncData", "(JJIIIII)V");
                }
            }
            //LOGD("USBProxy-jni calibrationData otherData type5    1");
            OtherData *otherData = new OtherData(buf, bSize);
            //LOGD("USBProxy-jni calibrationData otherData type5    2");
            if (otherData->Vsync_time == 0||!otherData->_isValid) {
                // LOGD("USBProxy-jni calibrationData otherData type5    4");
                delete otherData;
                return;
            }
            if (deviceType == 3){
                otherData->Vsync_time = otherData->Vsync_time * 1000;
            }

            if (printLog) {
                static int vsyCou = 0;
                vsyCou++;
                if (vsyCou >= 40) {
                    static uint32_t lastTT = 0;
                    LOGE("TerminalFragment_updateDatas otherData num->%d|time->%d|interval->%d|p-sensor->%d|ipd->%d",
                         otherData->Vsync_update_num, otherData->Vsync_time,
                         otherData->Vsync_time - lastTT, otherData->p_sensor, otherData->IPD_data);

                    lastTT = otherData->Vsync_time;
                    LOGE("TerminalFragment_updateDatas otherData left->%d|right->%d|brightness->%d",
                         otherData->t_left, otherData->t_right, otherData->bright_value);
                    vsyCou = 0;
                }
            }
            // LOGD("USBProxy-jni calibrationData otherData type5    3");
            if (_method_nsync_all && otherData) {
                //LOGD("USBProxy-jni calibrationData otherData type5    5");
                uint64_t reportTime = GetTimeNano(CLOCK_MONOTONIC);
//                uint64_t chazhi = reportTime - revTime;
//                if (chazhi>1000000){
//                    LOGE("__USBProxy-jni NSyncData %d     %f", chazhi, ((float )chazhi / (1000000)));
//                } else{
//                    LOGD("USBProxy-jni NSyncData %d     %f", chazhi, ((float )chazhi / (1000000)));
//                }

                jniEnv->CallVoidMethod(jThis, _method_nsync_all, (jlong) reportTime,(jlong) otherData->Vsync_time,
                                    otherData->IPD_data, otherData->p_sensor,
                                    otherData->bright_value, otherData->t_left,
                                    otherData->t_right);
                //LOGD("USBProxy-jni calibrationData otherData type5    6");
            }
            //LOGD("USBProxy-jni calibrationData otherData type5    7 over");
            delete otherData;
        }else if (type == 9) {
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onSetScreenModeAck",
                                                    "(I)V");
                if (jThis && ack) {
                    jniEnv->CallVoidMethod(jThis, ack, buf[8] & 0xFF);
                }
            }
        } else if (type == 10) {
            if (!_method_read_device_info_ack) {
                jclass clz = jniEnv->GetObjectClass(jThis);
                if (clz) {
                    _method_read_device_info_ack = jniEnv->GetMethodID(clz, "onVersionInfo",
                                                                       "(IIIIII[B[B)V");
                }
            }
            int SNByteLength =17;
            jbyteArray SNBytes = jniEnv->NewByteArray(SNByteLength);
            jniEnv->SetByteArrayRegion(SNBytes, 0, SNByteLength, (jbyte*)(buf + 22));
            int UUIDLength = 8;
            jbyteArray  MACBytes = jniEnv->NewByteArray(UUIDLength);
            jniEnv->SetByteArrayRegion(MACBytes, 0, UUIDLength, (jbyte*)(buf + 14));
            if (_method_read_device_info_ack) {
                jniEnv->CallVoidMethod(jThis, _method_read_device_info_ack, buf[8] & 0xff,
                                       buf[9] & 0xff, buf[10] & 0xff, buf[11] & 0xff,
                                       buf[12] & 0xff, buf[13] & 0xff,MACBytes,SNBytes);
            }
            jniEnv->DeleteLocalRef(MACBytes);
            jniEnv->DeleteLocalRef(SNBytes);
        }else if (type == 7){
            //低功耗
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onSetLowPowerAck",
                                                    "(I)V");
                if (ack) {
                    jniEnv->CallVoidMethod(jThis, ack, buf[8] & 0xFF);
                }
            }
        } else if (type == 8) {
            //进入固件升级
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onSetDeviceIntoOtaMode",
                                                    "(I)V");
                if (ack) {
                    jniEnv->CallVoidMethod(jThis, ack, buf[8] & 0xFF);
                }
            }
        } else if (type == 11) {
            //TODO 开启IMU
            //回执代表成功
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onSetDeviceHmdUploadStateOnAck",
                                                    "()V");
                if (ack) {
                    jniEnv->CallVoidMethod(jThis, ack);
                }
            }
        } else if (type == 12) {
            //TODO 关闭IMU
            //回执代表成功
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onSetDeviceHmdUploadStateOffAck",
                                                    "()V");
                if (ack) {
                    jniEnv->CallVoidMethod(jThis, ack);
                }
            }
        } else if (type == 13) {
            //TODO 进入标定模式//回执代表成功
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onSetIntoCalibrationMode",
                                                    "()V");
                if (ack) {
                    jniEnv->CallVoidMethod(jThis, ack);
                }
            }
        } else if (type == 14) {
            //写入SN
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onWriteSnAck",
                                                    "(I)V");
                if (ack) {
                    jniEnv->CallVoidMethod(jThis, ack, buf[8] & 0xFF);
                }
            }
        } else if (type == 15) {
            //设置标定参数结果
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onSetCalibrationParamsAck",
                                                    "(I)V");
                if (ack) {
                    jniEnv->CallVoidMethod(jThis, ack, buf[8] & 0xFF);
                }
            }
        }else if (type == 17){
            jclass clz = jniEnv->GetObjectClass(jThis);
            if (clz) {
                jmethodID ack = jniEnv->GetMethodID(clz, "onSetScreenOffInterval",
                                                    "(I)V");
                if (ack) {
                    jniEnv->CallVoidMethod(jThis, ack, buf[8] & 0xFF);
                }
            }
        } else{

            LOGE(" #######do nothing about type=%d  ######### ->%s",type,buf);
        }
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_nolovr_core_data_usb_UsbProxy_initLogIMU(JNIEnv *env, jobject thiz, jboolean is_log_imu) {
    // TODO: implement initLogIMU()
    use_imu_raw = is_log_imu;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_nolovr_core_data_usb_TestImu_updateImuByLocal(JNIEnv *env, jobject thiz,jdoubleArray resultDouble,
                                                       jint acc_x, jint acc_y,jint acc_z,
                                                       jint gyro_x, jint gyro_y, jint gyro_z,
                                                jlong timestamp) {
    // TODO: implement updateImu()
    if (!Q_result){
        Q_result = new ESKF_3dofcute_fp_type[4];
        ACC_result = new ESKF_3dofcute_fp_type[3];
        GYRO_result = new ESKF_3dofcute_fp_type[3];
        GYRO_OUT = new ESKF_3dofcute_fp_type[3];
    }

    static double timePre;
    double timeNow = timestamp * 10e-10;

    double interval =(timeNow - timePre);
    if (interval - 0 <=0.000001f){
        return;
    }
    timePre = timeNow;
    if (isNeedReset && !isFirst && g_status != NULL) {
        ESKF_3dofcute_status_delete(&g_status);
        isFirst = true;
        g_status = NULL;
    }
    calculateAccAndGyro(ACC_result,GYRO_result,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z);
    if (isFirst) {
        isFirst = false;
        ESKF_3dofcute_status_new(&g_status);
        isNeedReset = false;
        ESKF_3dofcute_status_init(Q_result, g_status, ACC_result[0], ACC_result[1], ACC_result[2]);
    } else {
        if (interval < 0) {
//             interval = 0;
        }
        if (interval > 0.1) {
            // interval = 0.1;
        }

        static_lock(GYRO_OUT, GYRO_result);
        g_status->t = interval;
        ESKF_3dofcute_step(Q_result, g_status ,ACC_result[0], ACC_result[1], ACC_result[2],
                           GYRO_result[0], GYRO_result[1],GYRO_result[2]);
    }
    LOGE("imuimu %f %f %f %f",Q_result[0],Q_result[1],Q_result[2],Q_result[3]);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_nolovr_core_data_usb_UsbProxy_native_1execRecenter(JNIEnv *env, jobject thiz) {
    // TODO: implement native_execRecenter()
    LOGD("USBProxy-jni execRecenter  over");
    isNeedReset = true;
}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_nolovr_core_data_usb_serialport_UartImpl_updateRawDatas(JNIEnv *env, jclass clazz,
//                                                                 jbyteArray datas, jint length) {
//    // 实现包封装
//    LOGD("updateDatas 1");
//    uint8_t *cs = new uint8_t[length];//申明字符长度，与源数组长度一致
//    LOGD("updateDatas 2");
//    env->GetByteArrayRegion(datas, 0, length, (jbyte *) cs);//赋值到cs
//    LOGD("updateDatas 3");
//    env->DeleteLocalRef(datas);//删除引用
//    LOGD("updateDatas 4");
//    updateDatas(cs, length);
//    LOGD("updateDatas 5");
//    delete[] cs;
//    LOGD("updateDatas 6");
//
//
//}
//
//
//void on_imu_frame_data(uint8_t* buffer, uint32_t size){
//    // 拿到数据后进行解析操作
//
//    LOGD("on_imu_frame_data size=%d",size);
//
//    if (jvm == NULL) {
//        return;
//    }
//
//    static JNIEnv *jniEnv = nullptr;
//    if (!jniEnv) {
//        if (jvm->AttachCurrentThread(&jniEnv, NULL) != JNI_OK) {
//            LOGE("AttachCurrentThread failed onUsbData. 1-2");
//            return;
//        }
//    }
//
//    // 解析数据
//    nolo::doParse(buffer, size, jniEnv);
//}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_nolovr_core_data_usb_serialport_UartImpl_initUart(JNIEnv *env, jclass clazz) {
//    // 注册回调函数
//   // onIMUFrameData();
//
//    //kInfo("init uart");
//    onBridgeIMUFrameData = on_imu_frame_data;
//
//    initEnv();
//
//
//
//}