/*
 * *
 *   Created by LG
 *  项目:MountTai
 *  邮箱：liu.lg@163.com
 *  创建时间：2023年10月09日 11:02:24
 *  修改人：
 *  修改时间：
 *  修改备注：
 *  版权所有违法必究
 *  Copyright(c) 北京凌宇智控科技有限公司 https://www.nolovr.com
 *
 *
 */

package com.nolovr.core.data.usb.serialport;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import com.nolovr.core.data.usb.UsbProxy;

public abstract class UartBase extends UsbProxy {

    private static final String TAG = "UartBase";

    private static final String ACTION_UART_OPEN = "com.nolovr.intent.ACTION_UART_OPEN";
    private static final String ACTION_UART_CLOSE = "com.nolovr.intent.ACTION_UART_CLOSE";

    protected UartBase(Context context) {
        super(context);
    }


    public void init() {
        super.init();
        registerInfoReceiver();
    }

    public void release() {
        releaseInfoReceiver();
    }


    @Override
    protected byte calcuCrc8(byte[] result, int i) {
        return 0;
    }

    @Override
    protected void setScreenOffInterval(int interval) {

    }

    @Override
    protected void setDeviceHmdUploadState(boolean b) {

    }

    @Override
    protected void readDeviceVersion() {

    }

    @Override
    protected void setScreenMode(int i) {

    }

    @Override
    protected void setScreenBrightness(int light) {

    }


    @Override
    public void updateCalibConfig(byte[] data) {

    }


    //-------------------------------------------------

    public abstract void open();

    public abstract void close();

    //-------------------------------------------------
    private void registerInfoReceiver() {
        IntentFilter uartFilter = new IntentFilter();
        uartFilter.addAction(ACTION_UART_OPEN);
        uartFilter.addAction(ACTION_UART_CLOSE);
        mContext.registerReceiver(infoReceiver, uartFilter);
    }


    private void releaseInfoReceiver() {
        if (infoReceiver != null) {
            mContext.unregisterReceiver(infoReceiver);
            infoReceiver = null;
        }
    }

    BroadcastReceiver infoReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            String action = intent.getAction();
            Log.d(TAG, "onReceive: " + action);
            switch (action) {
                case ACTION_UART_OPEN: {
                    open();
                    break;
                }

                case ACTION_UART_CLOSE: {
                    close();
                    break;
                }
            }

        }
    };
}
