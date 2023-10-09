/*
 * *
 *   Created by LG
 *  项目:MountTai
 *  邮箱：liu.lg@163.com
 *  创建时间：2023年10月08日 19:33:12
 *  修改人：
 *  修改时间：
 *  修改备注：
 *  版权所有违法必究
 *  Copyright(c) 北京凌宇智控科技有限公司 https://www.nolovr.com
 *
 *
 */

package com.nolovr.core.data.usb.serialport;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

public class UartImpl extends UartBase {

    private static final String TAG = "UartImpl";

    static {
        System.loadLibrary("uartusb");
    }

    private static volatile UartImpl instance;
    private String devPort;
    private boolean dispathInThread = false;
    protected Handler dispatchHandler;
    protected HandlerThread dispatchThread;

    //提供一个静态的公有方法，直接返回 SingletonInstance.INSTANCE
    public static UartImpl getInstance(Context context) {
        if (instance == null) {
            synchronized (UartImpl.class) {
                if (instance == null) {
                    instance = new UartImpl(context);
                }
            }
        }
        return instance;
    }

    private UartImpl(Context context) {
        super(context);
        devPort = "/dev/ttyACM0";
        if (dispatchHandler == null) {
            dispatchThread = new HandlerThread("dispatch");
            dispatchThread.start();
            dispatchHandler = new Handler(dispatchThread.getLooper());
        }
    }

    public void init() {
        super.init();
        Log.d(TAG, "init: ");

    }

    public void release() {

        if (dispatchThread != null) {
            dispatchHandler.removeCallbacks(null);
            dispatchThread.quitSafely();
            dispatchThread = null;
            dispatchHandler = null;
        }


    }


    //-----------------对外接口-------------------------
    @Override
    public void open() {

    }

    @Override
    public void close() {

    }

    //------------------------------------------
//
//    public static native void initUart();
//
//    public static native void updateRawDatas(byte[] datas, int length);

}
