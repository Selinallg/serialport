#include <jni.h>
#include "uart_manager.h"
#include "nlog.h"

// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("serialport");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("serialport")
//      }
//    }


nolo::uart_manager *get_uart() {
    static nolo::uart_manager uart;
    return &uart;
}


uint8_t compute_easy_crc(const uint8_t *buf, uint16_t length) {
    uint8_t crc8 = 0;
    while (length--) {
        crc8 = crc8 ^ (*buf++);
    }
    return crc8;
}

bool checkCrc(const uint8_t *buf, int len) {
    return buf[len - 1] == compute_easy_crc(buf, len - 1);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ex_serialport_Main3Activity_open(JNIEnv *env, jobject thiz) {
    get_uart()->isSonic = false;
    get_uart()->open();

}
extern "C"
JNIEXPORT void JNICALL
Java_com_ex_serialport_Main3Activity_close(JNIEnv *env, jobject thiz) {
    get_uart()->close();
}
extern "C"
JNIEXPORT jbyte JNICALL
Java_com_ex_serialport_MainActivity_calcuCrc8(JNIEnv *env, jclass clazz, jbyteArray data,
                                              jint length) {
    unsigned char *cs = new unsigned char[length];//申明字符长度，与源数组长度一致
    env->GetByteArrayRegion(data, 0, length, (jbyte *) cs);//赋值到cs
    env->DeleteLocalRef(data);//删除引用
    uint8_t crc8 = ::compute_easy_crc(cs, length);
    delete[] cs;
    return crc8;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ex_serialport_MainActivity_updateDatas(JNIEnv *env, jclass clazz, jbyteArray datas,
                                                jint length) {
//    LOGD("updateDatas 1");
    uint8_t *cs = new uint8_t [length];//申明字符长度，与源数组长度一致
//    LOGD("updateDatas 2");
    env->GetByteArrayRegion(datas, 0, length, (jbyte *) cs);//赋值到cs
//    LOGD("updateDatas 3");
    env->DeleteLocalRef(datas);//删除引用
//    LOGD("updateDatas 4");
    get_uart()->process_package_tai(cs,length);
//    LOGD("updateDatas 5");
    delete[] cs;
//    LOGD("updateDatas 6");
}

