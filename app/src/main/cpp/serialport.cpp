#include <jni.h>
#include "uart_manager.h"

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


nolo::uart_manager* get_uart(){
    static nolo::uart_manager uart;
    return &uart;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ex_serialport_Main3Activity_open(JNIEnv *env, jobject thiz) {
    get_uart()->open();

}