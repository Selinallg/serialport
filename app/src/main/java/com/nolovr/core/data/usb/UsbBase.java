/*
 * *
 *   Created by LG
 *  项目:MountTai
 *  邮箱：liu.lg@163.com
 *  创建时间：2023年09月01日 20:02:24
 *  修改人：
 *  修改时间：
 *  修改备注：
 *  版权所有违法必究
 *  Copyright(c) 北京凌宇智控科技有限公司 https://www.nolovr.com
 *
 *
 */

package com.nolovr.core.data.usb;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Environment;
import android.util.Log;

import com.ex.serialport.CmdUtils;
import com.nolovr.core.data.usb.config.Config;
import com.nolovr.core.data.usb.utils.LogUtil;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.Random;
import java.util.concurrent.ExecutorService;

public abstract class UsbBase {

    private static final String TAG = "_UsbBase_";

//    protected static final String INTENT_ACTION_GRANT_USB = Constants.ACTION_DEVICE_PERMISSION;

    protected static final int CMD_DEVICE_HMD_UPLOAD_START = Config.CMD_DEVICE_HMD_UPLOAD_START;
    protected static final int CMD_DEVICE_HMD_UPLOAD_END = Config.CMD_DEVICE_HMD_UPLOAD_END;
    protected static final int CMD_DEVICE_ENTER_CALIBRATION_MODE = Config.CMD_DEVICE_ENTER_CALIBRATION_MODE;
    protected static final int CMD_DEVICE_SET_CALIBRATION_PARAMS = Config.CMD_DEVICE_SET_CALIBRATION_PARAMS;
    protected static final int CMD_DEVICE_GET_CALIBRATION_PARAMS = Config.CMD_DEVICE_GET_CALIBRATION_PARAMS;
    protected static final int CMD_DEVICE_SET_BRIGHTNESS = Config.CMD_DEVICE_SET_BRIGHTNESS;
    protected static final int CMD_DEVICE_SET_LOW_POWER = Config.CMD_DEVICE_SET_LOW_POWER;
    protected static final int CMD_DEVICE_ENTER_OTA_MODE = Config.CMD_DEVICE_ENTER_OTA_MODE;
    protected static final int CMD_DEVICE_SET_SCREEN_MODE = Config.CMD_DEVICE_SET_SCREEN_MODE;
    protected static final int CMD_DEVICE_GET_VERSION = Config.CMD_DEVICE_GET_VERSION;
    protected static final int CMD_DEVICE_SET_SN = Config.CMD_DEVICE_SET_SN;
    protected static final int CMD_DEVICE_UPLOAD_7911 = Config.CMD_DEVICE_UPLOAD_7911;


    protected static final int CMD_DEVICE_SET_SCREEN_OFF_INTERVAL = Config.CMD_DEVICE_SET_SCREEN_OFF_INTERVAL;

    protected byte[] cmd_start_imu = Config.cmd_start_imu;// 泰山
    protected byte[] cmd_read_param_calib = Config.cmd_read_param_calib;//


    protected final String ACTION_VERSION = "readDeviceVersion";
    protected final String ACTION_SET_MODE = "setMode";
    protected final String ACTION_SET_BRIGHTNESS = "setScreenLBrightness";
    protected final String ACTION_OTA = "startOta";
    protected final String ACTION_SET_SCREEN_OFF_INTERVAL = "setScreenOffInterval";


    protected static String imu = "%d,%d,%d,%d,%d,%d,%d,%d";
    protected static String imuFormat = "%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f";


    protected enum UsbPermission {Unknown, Requested, Granted, Denied}

    protected UsbPermission usbPermission = UsbPermission.Unknown;


    public Context mContext;
    protected static boolean callbackIMU = true;
    protected static boolean callbackVsync = true;

    //写入文件统计
    protected ExecutorService dataWriteThreadPool;
    protected String filePath = Environment.getExternalStorageDirectory().getPath() + File.separator + "imu_data.txt";


    //-----------------说明--------------------------
    public interface OnInvQ0UpdateListener {
        void onQ0UpdateListener(double w, double x, double y, double z);
    }

    protected static OnInvQ0UpdateListener onInvQ0UpdateListener;

    public static void setOnInvQ0UpdateListener(OnInvQ0UpdateListener onInvQ0UpdateListener1) {
        onInvQ0UpdateListener = onInvQ0UpdateListener1;
    }


    public interface Clientcallback {

        void onReStartTransfer();

        void onReCenter();

        void onWrite(byte[] datas);

        void onWritecalibConfig(byte[] datas);
    }

    //-----------------说明--------------------------
    protected OnUsb onUsb;

    public void setListener(OnUsb onUsb) {
        this.onUsb = onUsb;
    }

    public interface OnUsb {
        void onConnectState(int state);

        void onData(byte[] data, int length);
    }


    public void init() {
        File imuFile = new File(filePath);
        if (imuFile.exists()) imuFile.delete();
        registerCommReceiver();
    }

    public void release() {
        try {
            releaseCommReceiver();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }


    protected void writeData(String conent) {
        LogUtil.i("write_data", conent);
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                Thread.currentThread().setName("file-write");
                BufferedWriter out = null;
                try {
                    out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(filePath, true)));
                    out.write(conent);
                    LogUtil.i("write_data2", "success");
                } catch (Exception e) {
                    LogUtil.i("write_data3", e.getMessage());
                    e.printStackTrace();
                } finally {
                    try {
                        out.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        };
        // 将任务交给线程池管理
        if (dataWriteThreadPool != null) {
            dataWriteThreadPool.execute(runnable);
        }
    }


    protected byte[] makeCmd(byte[] data, int cmd) {
        byte[] result = CmdUtils.makeCmd(data, cmd);
        result[result.length - 1] = calcuCrc8(result, result.length - 1);
        return result;
    }

    protected abstract byte calcuCrc8(byte[] result, int i);


    //---------------------

    public void registerCommReceiver() {
        IntentFilter usbFilter = new IntentFilter();
        usbFilter.addAction("com.test.taishan");
//        usbFilter.addAction(INTENT_ACTION_GRANT_USB);
        mContext.registerReceiver(commReceiver, usbFilter);
    }


    public void releaseCommReceiver() {
        if (commReceiver != null) {
            mContext.unregisterReceiver(commReceiver);
            commReceiver = null;
        }
    }

    BroadcastReceiver commReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive: " + action);
            switch (action) {
                //adb shell am broadcast -a com.test.taishan --ei code 6
                //adb shell am broadcast -a com.test.taishan --ei code 22 --ei interval 0
                case "com.test.taishan": {
                    if (intent.hasExtra("code")) {
                        int code = intent.getIntExtra("code", 0);
                        Log.e(TAG, "onReceive: " + code);
                        switch (code) {
                            case 0:
                                int light = new Random().nextInt(254);
                                if (light <= 10) {
                                    light = 10;
                                }
                                Log.e(TAG, "onReceive: 设置亮度->" + light);
                                setScreenBrightness(light);
                                break;
                            case 1:
                                Log.e(TAG, "onReceive: 设置亮度关闭->");
                                setScreenBrightness(0);
                                break;
                            case 2:
                                Log.e(TAG, "onReceive: 设置mode 0");
                                setScreenMode(0);
                                break;
                            case 3:
                                Log.e(TAG, "onReceive: 设置mode 1");
                                setScreenMode(1);
                                break;
                            case 4:
                                Log.e(TAG, "onReceive: 设置mode 2");
                                setScreenMode(2);
                                break;
                            case 5:
                                Log.e(TAG, "onReceive: 设置mode 3");
                                setScreenMode(3);
                                break;
                            case 6:
                                Log.e(TAG, "onReceive: 读取版本信息");
                                readDeviceVersion();
                                break;
                            case 7:
                                //广播手动调用，升级到2.4版本
//                                TestGlassOTAUtils.getInstance().setDeviceIntoOta(7);
                                break;
                            case 8:
                                //广播手动调用,升级到2.5版本
//                                TestGlassOTAUtils.getInstance().setDeviceIntoOta(8);
                                break;
                            case 20:
                                //广播 关闭数据流
                                Log.e(TAG, "onReceive: 广播 关闭数据流");
                                setDeviceHmdUploadState(false);
                                break;
                            case 21:
                                //广播 打开数据流
                                Log.e(TAG, "onReceive: 广播 打开数据流");
                                setDeviceHmdUploadState(true);
                                break;
                            case 22:
                                //广播 设置休眠时间
                                int interval = intent.getIntExtra("interval", 0);
                                Log.e(TAG, "onReceive: 广播 设置休眠时间  interval =" + interval);
                                if (interval != 0) {
                                    interval = 1;
                                }
                                setScreenOffInterval(interval);
                            case 23:
                                // 设置指定亮度
                                int britht = intent.getIntExtra("britht", 0);
                                Log.e(TAG, "onReceive: 广播 接收亮度  britht =" + britht);
                                if (britht > 255) {
                                    britht = 100;
                                } else if (britht < 0) {
                                    britht = 10;
                                }
                                setScreenBrightness(britht);
                                Log.e(TAG, "onReceive: 广播 设置指定亮度  britht =" + britht);
                                break;
                            case 24: {
                                //boolean canWrite = (boolean) SPUtils.get(mContext, Constants.SP_KEY_WRITE_FEATURE, true);
                                int param = intent.getIntExtra("param", 0);
//                                if (param >= 0) {
//                                    SPUtils.put(mContext, Constants.SP_KEY_WRITE_FEATURE, true);
//                                    Log.e(TAG, "往固件写数据打开");
//                                } else {
//                                    SPUtils.put(mContext, Constants.SP_KEY_WRITE_FEATURE, false);
//                                    Log.e(TAG, "往固件写数据关闭");
//                                }
                                break;
                            }
                            case 25: {
                                int param = intent.getIntExtra("param", 0);
                                if (param >= 0) {
                                    LogUtil.setDebugMode(true);
                                    Log.e(TAG, "实时日志打开");
                                } else {
                                    LogUtil.setDebugMode(false);
                                    Log.e(TAG, "实时日志关闭");
                                }
                                break;
                            }
                            case 26: {
                                int param = intent.getIntExtra("param", 1);
                                if (param>0){
                                    callbackIMU = true;
                                }else {
                                    callbackIMU = false;
                                }
                                break;
                            }
                            case 27: {
                                int param = intent.getIntExtra("param", 1);
                                if (param>0){
                                    callbackVsync = true;
                                }else {
                                    callbackVsync = false;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    };

    protected abstract void setScreenOffInterval(int interval);

    protected abstract void setDeviceHmdUploadState(boolean b);

    protected abstract void readDeviceVersion();


    protected abstract void setScreenMode(int i);

    protected abstract void setScreenBrightness(int light);


    //---------------------


}
