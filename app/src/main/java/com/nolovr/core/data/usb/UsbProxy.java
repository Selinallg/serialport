package com.nolovr.core.data.usb;

import android.content.Context;
import android.util.Log;

import com.nolovr.core.data.usb.utils.FileDataUtils;
import com.nolovr.core.data.usb.utils.LogUtil;
import com.nolovr.core.data.usb.utils.LogUtilAgent;
import com.nolovr.core.data.usb.utils.NamedThreadFactory;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.concurrent.Executors;

import tp.xmaihh.serialport.utils.CheckUtils;


public abstract class UsbProxy extends UsbBase {

    // Used to load the 'nativedemo' library on application startup.
    static {
        System.loadLibrary("uartusb");
    }

    UsbProxyAgent agent;

    protected boolean isLogImu = false;
    private static boolean isImuOpened = false;// imu 数据流 是否开启标记

    private static final String TAG = "UsbProxy";
    protected static boolean needCallback = true;
    protected boolean writeData = false;

    protected boolean connected = false;
    protected static boolean calibed = false;// 是否返回标定参数
    protected boolean openLog = LogUtilAgent.DEBUG;


    Clientcallback hostlistener = new Clientcallback() {
        @Override
        public void onReStartTransfer() {
            Log.d(TAG, "onReStartTransfer: ");
            setDeviceHmdUploadState(true);
        }

        @Override
        public void onReCenter() {
            execRecenter();
        }

        @Override
        public void onWrite(byte[] datas) {
//            send(datas);
        }

        @Override
        public void onWritecalibConfig(byte[] datas) {
            updateCalibConfig(datas);
        }

    };


    protected UsbProxy(Context context) {
        LogUtil.d(TAG, "USBProxy: ");
        mContext = context;

    }


    public void init() {
        super.init();
        if (agent == null) {
            agent = new UsbProxyAgent(mContext.getApplicationContext());
        }
        agent.setClientcallback(hostlistener);
        boolean saveRawData = false;
        LogUtil.i(TAG, "init: saveRawData=" + saveRawData);

        agent.init();


        isLogImu = saveRawData;
        if (isLogImu) {
            dataWriteThreadPool = Executors.newFixedThreadPool(1, new NamedThreadFactory(TAG));
            initLogIMU(false);
            String s = FileDataUtils.setIsWriteEnable(true, mContext.getApplicationContext());
            LogUtil.d(TAG, "init: ==>" + s);
        }
    }

    public void release() {
        super.release();
        calibed = false;
        isImuOpened = false;

        if (agent != null) {
            agent.setClientcallback(null);
            agent.release();
            agent = null;
        }
        LogUtil.d(TAG, "release: ");
    }


    public abstract void updateCalibConfig(byte[] data);// 更新标定参数

    public boolean isConnected() {
        return connected;
    }


    public void setRTS(boolean rts) throws IOException {
        LogUtil.d(TAG, "setRTS: ");
    }

    public void setDTR(boolean dtr) throws IOException {
        LogUtil.d(TAG, "setDTR: ");
    }

    // ===============回调 开始===================


    public void onResetQuaternion(double X, double Y, double Z, double quat_W, double quat_X, double quat_Y, double quat_Z) {
        LogUtil.i(TAG, String.format("onResetQuaternion calcal data %f|%f|%f", X, Y, Z));
        LogUtil.i(TAG, String.format("onResetQuaternion calcal data %f|%f|%f|%f", quat_W, quat_X, quat_Y, quat_Z));
        agent.dispatchResetQuaternion(X, Y, Z, quat_W, quat_X, quat_Y, quat_Z);
    }


    /**
     * imu 数据native 回调
     *
     * @param timestampAndroid
     * @param timestamp
     * @param acc_X
     * @param acc_Y
     * @param acc_Z
     * @param gyro_X
     * @param gyro_Y
     * @param gyro_Z
     * @param quat_W
     * @param quat_X
     * @param quat_Y
     * @param quat_Z
     */
    public void imuDataQuaternion(long timestampAndroid, long timestamp, double acc_X, double acc_Y, double acc_Z, double gyro_X, double gyro_Y, double gyro_Z,
                                  double quat_W, double quat_X, double quat_Y, double quat_Z) {
        //LogUtil.d(TAG, "imuDataQuaternion: " + "quat_W=" + quat_W + " | " + "quat_X=" + quat_X + " | " + "quat_Y=" + quat_Y + " | " + "quat_Z=" + quat_Z);
        if (isLogImu) {
            FileDataUtils.writeToFile(FileDataUtils.TYPE_IMU_FORMAT, String.format(imuFormat, timestampAndroid, timestamp, -acc_Y, -acc_Z, acc_X, -gyro_Y, -gyro_Z, gyro_X, quat_W, quat_X, quat_Y, quat_Z));
        }

        if (!needCallback) {
            return;
        }

        if (!callbackIMU) {
            return;
        }

        agent.dispatchImuDataQuaternion(timestampAndroid, timestamp, acc_X, acc_Y, acc_Z, gyro_X, gyro_Y, gyro_Z, quat_W, quat_X, quat_Y, quat_Z);

    }

    public void imuDataDouble(long timestamp, double acc_X, double acc_Y, double acc_Z, double gyro_X, double gyro_Y, double gyro_Z) {
        // LogUtil.d(TAG, "imuData: acc_X=" + acc_X + " | " + "gyro_X=" + gyro_X);
        agent.dispatchImuDataDouble(timestamp, acc_X, acc_Y, acc_Z, gyro_X, gyro_Y, gyro_Z);
    }


    public void imuData(long timestampAndroid, long timestamp, short acc_X, short acc_Y, short acc_Z,
                        short gyro_X, short gyro_Y, short gyro_Z) {
        LogUtil.e(TAG, 0 + "imuData" + "\r\n" +
                "timestamp: Android firmware| " + timestampAndroid + " " + timestamp + "|" + "\r\n" +
                "imuData: acc_X acc_Y acc_Z gyro_X gyro_Y gyro_Z" +
                " " + acc_X + " " + acc_Y + " "+ acc_Z + " " + gyro_X + " " + gyro_Y + " " + gyro_Z);
        if (isLogImu) {
            FileDataUtils.writeToFile(FileDataUtils.TYPE_IMU, String.format(imu, timestampAndroid,
                    timestamp, -acc_Y, -acc_Z, acc_X, -gyro_Y, -gyro_Z, gyro_X));
        } else {
            //send(cmd_read_param_calib);
            //LogUtil.d(TAG, "imuData: 规避方式，重新读标定参数");
        }
        //send(cmd_start_imu);
        agent.dispatchImuData(timestampAndroid, timestamp, acc_X, acc_Y, acc_Z, gyro_X, gyro_Y, gyro_Z);
    }


    public void nsyncData(long timestamp_android, long timestamp_firmware) {
        if (isLogImu) {
            FileDataUtils.writeToFile(FileDataUtils.TYPE_VSYNC, String.format(Locale.getDefault(),
                    "%d,%d", timestamp_android, timestamp_firmware));
        }
        //LogUtil.d(TAG, "nsyncData: timestamp ===============timestamp_android" + timestamp_android + " timestamp_firmware=" + timestamp_firmware);
        agent.dispatchNsyncData(timestamp_android, timestamp_firmware);
    }


    /**
     * vsync native 数据回调
     *
     * @param timestamp_android  Android时间戳
     * @param timestamp_firmware 固件时间戳
     * @param ipd
     * @param pSensor
     * @param brightness
     * @param left
     * @param right
     */
    public void nsyncData(long timestamp_android, long timestamp_firmware,
                          int ipd, int pSensor, int brightness, int left, int right) {

        if (isLogImu) {
            FileDataUtils.writeToFile(FileDataUtils.TYPE_VSYNC,
                    String.format(Locale.getDefault(), "%d,%d", timestamp_android, timestamp_firmware));
        }

        if (!callbackVsync) {
            return;
        }

        agent.dispatchNsyncData(timestamp_android, timestamp_firmware, ipd, pSensor, brightness, left, right);


    }


    public void onVersionInfo(int hardwareMajor, int hardwareMinor, int debugMajor, int debugMinor,
                              int softwareMajor, int softwareMinor, byte[] mac, byte[] sn) {

        String glassSN = new String(sn, StandardCharsets.UTF_8);
        String glassMac = CheckUtils.byte2hex(mac).toString();
        LogUtil.d(TAG, String.format(Locale.getDefault(), "deviceInfo onVersionInfo hardware:%d.%d," +
                        "debug:%d.%d,software:%d.%d mac:%s sn:%s",
                hardwareMajor, hardwareMinor, debugMajor, debugMinor, softwareMajor, softwareMinor,
                glassMac, glassSN));
        // hardware:1.1,debug:0.6,software:1.1 mac:2216211710B1 sn:TAI_VR_GLASS


    }


    public void onWriteSnAck(int result) {
        LogUtil.i(TAG, "onWriteSnAck: " + result);
    }

    public void onSetScreenModeAck(int result) {
        LogUtil.i(TAG, "onSetScreenModeAck: " + result);

    }

    public void onSetDeviceIntoOtaMode(int result) {
        LogUtil.i(TAG, "onSetDeviceIntoOtaMode: " + result);

    }

    public void onSetLowPowerAck(int result) {
        LogUtil.i(TAG, "onSetLowPowerAck: " + result);
    }

    public void onSetCalibrationParamsAck(int result) {
        LogUtil.i(TAG, "onSetCalibrationParamsAck: " + result);
    }

    public void onSetDeviceHmdUploadStateOnAck() {
        LogUtil.i(TAG, "onSetDeviceHmdUploadStateOnAck: ");
    }

    public void onSetDeviceHmdUploadStateOffAck() {
        LogUtil.i(TAG, "onSetDeviceHmdUploadStateOffAck: ");
    }

    // 结果回调
    // 1 成功 ，非1 失败
    public void onSetBrightnessAck(int result) {
        LogUtil.i(TAG, "onSetBrightnessAck: " + result);

    }

    public void onSetIntoCalibrationMode(int result) {
        LogUtil.i(TAG, "onSetIntoCalibrationMode: " + result);
    }

    public void onSetScreenOffInterval(int result) {
        LogUtil.i(TAG, "onSetScreenOffInterval: " + result);
    }

    // jni 回调 子类 具体实现，把标定数据，写入 usb 设备
    public void calibrationData(int saveState, float[] acc_scale, float[] acc_offset, float[] gyro_scale, float[] gyro_offset) {
        LogUtil.d(TAG, String.format("deviceInfo calibrationData:gyro_offset %.4f|%.4f|%.4f", gyro_offset[0], gyro_offset[1], gyro_offset[2]));
        LogUtil.d(TAG, String.format("deviceInfo calibrationData:gyro_scale %.4f|%.4f|%.4f", gyro_scale[0], gyro_scale[1], gyro_scale[2]));
        LogUtil.d(TAG, String.format("deviceInfo calibrationData:acc_offset %.4f|%.4f|%.4f", acc_offset[0], acc_offset[1], acc_offset[2]));
        LogUtil.d(TAG, String.format("deviceInfo calibrationData:acc_scale %.4f|%.4f|%.4f", acc_scale[0], acc_scale[1], acc_scale[2]));
        calibed = true;
        agent.dispatchCalibrationData(saveState, acc_scale, acc_offset, gyro_scale, gyro_offset);

        // 开启数据流的最佳时机，其他情况下，都会有断开的问题
        //  自动标定完成，每次都执行该逻辑？
        if (!isImuOpened) {
            //  send(cmd_start_imu);
            isImuOpened = true;
        }

    }


    // ===============回调 结束===================

    //  ==============native 开始==============


    public native void updateLogprint(int type);

    public native void updateDeviceType(int type);

    public native void updateDatas(byte[] datas);

    //测试验证代码
    public native void test(double accX, double accY, double accZ);

    private native void native_execRecenter();

    public native void initLogIMU(boolean isLogImu);


    // ==============native 结束==============

    // 执行 recenter 标定
    public void execRecenter() {
        if (isLogImu && mContext != null) {
            FileDataUtils.setIsWriteEnable(false, mContext);
            FileDataUtils.setIsWriteEnable(true, mContext);
        }
        native_execRecenter();
    }

    public void doConnected() {
        agent.sendUsbConnectedBroadcast();
    }

    // 是否有调用？
    public void updateRotNQuaInvQ0(double w, double x, double y, double z) {
        if (onInvQ0UpdateListener != null) {
            onInvQ0UpdateListener.onQ0UpdateListener(w, x, y, z);
        }
    }

}
