/*
 * *
 *   Created by LG
 *  项目:MountTai
 *  邮箱：liu.lg@163.com
 *  创建时间：2023年09月01日 17:54:28
 *  修改人：
 *  修改时间：
 *  修改备注：
 *  版权所有违法必究
 *  Copyright(c) 北京凌宇智控科技有限公司 https://www.nolovr.com
 *
 *
 */

package com.nolovr.core.data.usb;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Process;
import android.util.Log;
import com.nolovr.core.data.usb.utils.CRC8;
import com.nolovr.core.data.usb.utils.HexDump;
import com.nolovr.core.data.usb.utils.LogUtil;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import java.util.Locale;
import tp.xmaihh.serialport.utils.CheckUtils;

public class UsbProxyAgent {


    private static final String TAG = "UsbProxyAgent";


    public Context mContext;

    private static final int GAP_SYNC = 17;

    private int currentPSensor = -1;
    private final int P_SENSOR_THRESHOLD = 5;
    protected static final boolean useOneThread = true;// 数据读取和分发在一个线程

    protected boolean openPrint = false;
    protected boolean imuDispatchBindBigCpus = false;// imudispatch 绑定大核
    protected boolean vsyncDispatchBindBigCpus = false;// vsyncdispatch 绑定大核

    private boolean isWritedClib = true;// 谨慎使用，只有在需要写标定文件的时候才置为 false

    private boolean wakeMode;

    protected Handler vsyncDispatchHandler;
    protected HandlerThread vsyncDispatchThread;

    protected Handler imuDispatchHandler;
    protected HandlerThread imuDispatchThread;

    Handler handler = new Handler();

    UsbBase.Clientcallback clientcallback;

    public void setClientcallback(UsbBase.Clientcallback clientcallback) {
        this.clientcallback = clientcallback;
    }


    public UsbProxyAgent(Context context) {
        LogUtil.d(TAG, "USBProxyAgent: ");
        mContext = context;

    }

    public void init() {
        LogUtil.d(TAG, "init: ");
        LogUtil.i(TAG, "init: wakeMode=" + wakeMode);
        if (!useOneThread) {
            if (vsyncDispatchHandler == null) {
                vsyncDispatchThread = new HandlerThread("vsync-dispatch", Process.THREAD_PRIORITY_URGENT_AUDIO);
                vsyncDispatchThread.start();
                vsyncDispatchHandler = new Handler(vsyncDispatchThread.getLooper());
            }

            if (imuDispatchHandler == null) {
                imuDispatchThread = new HandlerThread("imu-dispatch", Process.THREAD_PRIORITY_URGENT_AUDIO);
                imuDispatchThread.start();
                imuDispatchHandler = new Handler(imuDispatchThread.getLooper());
            }
        }

    }

    public void release() {
        isWritedClib = true;
        if (vsyncDispatchHandler != null) {
            vsyncDispatchHandler.removeCallbacks(null);
            vsyncDispatchThread.quitSafely();
            vsyncDispatchHandler = null;
        }
        if (imuDispatchHandler != null) {
            imuDispatchHandler.removeCallbacks(null);
            imuDispatchThread.quitSafely();
            imuDispatchHandler = null;
        }
        LogUtil.d(TAG, "release: ");
    }

    public void dispatchImuDataQuaternion(long timestampAndroid, long timestamp, double acc_X, double acc_Y, double acc_Z, double gyro_X, double gyro_Y, double gyro_Z,
                                          double quat_W, double quat_X, double quat_Y, double quat_Z) {
        if (useOneThread) {
            _imuDataQuaternion(timestampAndroid, timestamp, acc_X, acc_Y, acc_Z, gyro_X, gyro_Y, gyro_Z, quat_W, quat_X, quat_Y, quat_Z);
        } else {

            if (imuDispatchHandler == null) return;
            imuDispatchHandler.removeCallbacks(null);
            imuDispatchHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (!imuDispatchBindBigCpus) {
                       // Affinity.affinityToBigCpu();
                        imuDispatchBindBigCpus = true;
                    }
                    _imuDataQuaternion(timestampAndroid, timestamp, acc_X, acc_Y, acc_Z, gyro_X, gyro_Y, gyro_Z, quat_W, quat_X, quat_Y, quat_Z);
                }
            });

        }
    }

    //----------------------------内部实现----------------------------

    private long updateTimeMillis;

    private int countDouble;
    private long lastTimeDouble;

    private int count;
    private long lastTime;


    private int countSync;
    private long lastTimeSync;

    private long lastTimestamp;

    long last_android;
    long last_firmware;
    private long lastResetTime;

    /**
     * 内部调用
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
    private void _imuDataQuaternion(long timestampAndroid, long timestamp, double acc_X, double acc_Y, double acc_Z, double gyro_X, double gyro_Y, double gyro_Z,
                                    double quat_W, double quat_X, double quat_Y, double quat_Z) {

        updateTimeMillis = System.currentTimeMillis();

        if (LogUtil.isDebugMode()) {

            countDouble++;
            long currentTimeMillis = System.currentTimeMillis();
            long difftime = currentTimeMillis - lastTimeDouble;
            if (difftime > 1000) {
                LogUtil.d(TAG, "《=== map Hz imuDataQuaternion =》" + countDouble + "|timestamp= " + timestamp + "|" + "imuDataDouble: acc_X=" + acc_X + " acc_Y=" + acc_Y + " acc_Z=" + acc_Z + " gyro_X=" + gyro_X + " gyro_Y=" + gyro_Y + " gyro_Z=" + gyro_Z +
                        "quat_W=" + quat_W + " | " + "quat_X=" + quat_X + " | " + "quat_Y=" + quat_Y + " | " + "quat_Z=" + quat_Z
                );
                lastTimeDouble = currentTimeMillis;
                countDouble = 0;
            }
        }


        // 小概率 0 dof 情况，尝试重新打开数据流
        if (false) {
            handler.removeCallbacks(null);
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    long bomb = System.currentTimeMillis();
                    if (bomb - updateTimeMillis > 1500) {
                        if (clientcallback != null) {
                            clientcallback.onReStartTransfer();
                        }
                    }
                }
            }, 2000);
        }
    }


    /**
     * 内部调用
     *
     * @param timestamp_android
     * @param timestamp_firmware
     * @param ipd
     * @param pSensor
     * @param brightness
     * @param left
     * @param right
     */
    private void _nsyncData(long timestamp_android, long timestamp_firmware,
                            int ipd, int pSensor, int brightness, int left, int right) {


        if (LogUtil.isDebugMode()) {


            if (openPrint) {
                long nanoTime = System.nanoTime();
                float chazhi = (nanoTime - timestamp_android) / 1000000;
                if (chazhi > 3) {
                    Log.e(TAG, "_nsyncDataExt: chazhi=" + chazhi + " | " + nanoTime + " | " + timestamp_android);
                } else {
                    Log.d(TAG, "_nsyncDataExt: chazhi=" + chazhi + " | " + nanoTime + " | " + timestamp_android);
                }
            }

            long sync = (timestamp_android - lastTimestamp) / (1000000);
            if (sync > GAP_SYNC) {
                LogUtil.e(TAG, "nsyncData: timestamp ============================ " + sync);
            } else {
//            LogUtil.d(TAG, "nsyncData: timestamp = " + sync);
            }
            lastTimestamp = timestamp_android;

            //lastTimestamp = timestamp_firmware;  ??????????????
            countSync++;
            long currentTimeMillis = System.currentTimeMillis();
            long difftime = currentTimeMillis - lastTimeSync;
            if (difftime > 1000) {
                LogUtil.d(TAG, String.format(Locale.getDefault(),
                        "Hz onNewData nsyncData nsyncDataExt =%d,map size=%d,timestamp_a:%d,timestamp_f:%d,ipd:%d,pSensor:%d,brightness:%d,left:%d,right:%d",
                        countSync, 0, timestamp_android, timestamp_firmware, ipd, pSensor, brightness, left, right));
                lastTimeSync = currentTimeMillis;
                countSync = 0;

                //isWritedClib = false;
                //_handleCalib(true);

            }


        }

        // todo 逻辑处理 如果息屏眼镜，是否需要关闭数据回调
        if (currentPSensor != pSensor) {
            LogUtil.i(TAG, "currentPSensor changed! pSensor" + pSensor + " currentPSensor: " + currentPSensor);
            currentPSensor = pSensor;
            // pSensor返回值只有0/5，0为戴上情况，5为未戴

            if (!wakeMode) {
                if (currentPSensor >= P_SENSOR_THRESHOLD) {
                    Log.d(TAG, "nsyncData: 远离p-sensor");
                } else {
                    Log.d(TAG, "nsyncData: 靠近p-sensor");
                    //1秒钟只允许重置一次 靠近p-sensor 执行recenter
                    if (System.currentTimeMillis() - lastResetTime >= 1000) {
                        if (clientcallback != null) {
                            clientcallback.onReCenter();
                        }
                        lastResetTime = System.currentTimeMillis();
                    }
                }
            }
        }


    }


    private void _handleCalib(boolean open) {
        if (open) {
            if (!isWritedClib) {

                // a == acc g -- groy
                // K scale
                // B offset
//                Ka=0.00478827332859428	0.00479769817785433	0.00477073223308767
//                Ba=-3.64586491231660    -12.6361886550360    -12.2023466954797
//                Kg=0.00107878825793819	 0.00106755452097306	0.00107831553293257
//                Bg=-7.54313478592477     -3.32170074188190    -8.78956329442199

                //7号机器
//                float Ka_0 = 0.00478827332859428f, Ka_1 = 0.00479769817785433f, Ka_2 = 0.00477073223308767f;
//                float Ba_0 = -3.64586491231660f, Ba_1 = -12.6361886550360f, Ba_2 = -12.2023466954797f;
//                float Kg_0 = 0.00107878825793819f, Kg_1 = 0.00106755452097306f, Kg_2 = 0.00107831553293257f;
//                float Bg_0 = -7.54313478592477f, Bg_1 = -3.32170074188190f, Bg_2 = -8.78956329442199f;

                //9号机器
//                float Ka_0 = 0.00479322027937182f, Ka_1 = 0.00479408683328329f, Ka_2 = 0.00478504278979190f;
//                float Ba_0 = 0.393951812929572f, Ba_1 = -0.00537200398613891f, Ba_2 = -10.0741497272004f;
//                float Kg_0 = 0.00107563111335274f, Kg_1 = 0.00107655457126801f, Kg_2 = 0.00107831553293257f;
//                float Bg_0 = 7.54006990139470f, Bg_1 = 7.48027134555245f, Bg_2 = -8.32346544060749f;

                //1-1 号机器
//                float Ka_0 = 0.00478201816488835f, Ka_1 = 0.00479491601342186f, Ka_2 = 0.00477993178764784f;
//                float Ba_0 = 1.98991577426588f, Ba_1 = -3.86135977156486f, Ba_2 = -5.55749520176562f;
//                float Kg_0 = 0.00107670004813470f, Kg_1 = 0.00107543024205329f, Kg_2 = 0.00107695571400323f;
//                float Bg_0 = -35.0071575338681f, Bg_1 = 25.6957805374301f, Bg_2 = 6.85887724104062f;

                // 2-1 号机器
//                float Ka_0 = 0.00477536792805946f, Ka_1 = 0.00479465268508078f, Ka_2 = 0.00477465038062411f;
//                float Ba_0 = -0.210793854457904f, Ba_1 = 5.04685145268962f, Ba_2 = 2.80113918187436f;
//                float Kg_0 = 0.00108178506965142f, Kg_1 = 0.00108188350228720f, Kg_2 = 0.00107076156196880f;
//                float Bg_0 = -2.58851175831848f, Bg_1 = 6.79088253769905f, Bg_2 = 1.83991428502071f;

                // 2-2 号机器
//                float Ka_0 = 0.00478110228279164f, Ka_1 = 0.00478408177534275f, Ka_2 = 0.00477680403359550f;
//                float Ba_0 = 5.49535886878596f, Ba_1 = -2.35716846361677f, Ba_2 = 1.57316477895112f;
//                float Kg_0 = 0.00107753424694625f, Kg_1 = 0.00107917683028869f, Kg_2 = 0.00107678128197476f;
//                float Bg_0 = 12.6642701851632f, Bg_1 = 10.7759116399702f, Bg_2 = -13.8275182679343f;

                // 2-3 号机器
                float Ka_0 = 0.00479078818490731f, Ka_1 = 0.00478757245358461f, Ka_2 = 0.00477578065730860f;
                float Ba_0 = 4.26967238441205f, Ba_1 = -2.89996136905550f, Ba_2 = 9.96415237759038f;
                float Kg_0 = 0.00108113074948884f, Kg_1 = 0.00107714119887665f, Kg_2 = 0.00108016357643783f;
                float Bg_0 = -9.48548133967388f, Bg_1 = 14.2971375655937f, Bg_2 = 2.01651164479930f;

                //2-6 号机器
//                float Ka_0 = 0.00478071743154333f, Ka_1 = 0.00478499838547150f, Ka_2 = 0.00477444554911536f;
//                float Ba_0 = 5.63707907013611f, Ba_1 = -0.00864487024435334f, Ba_2 = 9.49761595588411f;
//                float Kg_0 = 0.00108515263471238f, Kg_1 = 0.00107167245370128f, Kg_2 = 0.00107011970061719f;
//                float Bg_0 = -0.890388114393180f, Bg_1 = 3.91461427528072f, Bg_2 = -9.63804643440533f;

                // 2-8 号机器
//                float Ka_0 = 0.00479068451043011f, Ka_1 = 0.00478757245358461f, Ka_2 = 0.00477578065730860f;
//                float Ba_0 = 0.0257133588428360f, Ba_1 = -1.93288798945677f, Ba_2 = 8.03748167896225f;
//                float Kg_0 = 0.00107867985782912f, Kg_1 = 0.00108127062355353f, Kg_2 = 0.00108087941293263f;
//                float Bg_0 = -4.90614378091429f, Bg_1 = 4.81463203394445f, Bg_2 = -7.68482747259239f;

//                 2-7 号机器  1230zx
//                float Ka_0 = 0.00478827332859428f, Ka_1 = 0.00478657283875008f, Ka_2 = 0.00477073223308767f;
//                float Ba_0 = -0.761623545575333f, Ba_1 = -11.1325385264792f, Ba_2 = -9.57566983571115f;
//                float Kg_0 = 0.00107878825793819f, Kg_1 = 0.00107919760622212f, Kg_2 = 0.00108002547174932f;
//                float Bg_0 = -7.03189235177013f, Bg_1 = -1.50481481655470f, Bg_2 = -6.57780197389434f;

                //2-10 号机器 1230zx
//                float Ka_0 = 0.00477447031727080f, Ka_1 = 0.00478875112902940f, Ka_2 = 0.00477285953111812f;
//                float Ba_0 = -2.98039857378933f, Ba_1 = -3.43896186826942f, Ba_2 = 4.13088994509960f;
//                float Kg_0 = 0.00107352298552118f, Kg_1 = 0.00107240754867629f, Kg_2 = 0.00107490429238012f;
//                float Bg_0 = -9.61769670778827f, Bg_1 = 10.8853126632808f, Bg_2 = 4.09847285548983f;

                byte[] cmd_write_param_calib_head = {0x37, 0x00, (byte) 0xAA, 0x55, 0x33, 0x00, 0x29, 0x01, 0x01};//
                int cmdLength = cmd_write_param_calib_head.length;

                byte[] acc_scale_0 = HexDump.float2byte(Ka_0);
                int acc_scale_0_length = acc_scale_0.length;
                byte[] acc_scale_1 = HexDump.float2byte(Ka_1);
                int acc_scale_1_length = acc_scale_1.length;
                byte[] acc_scale_2 = HexDump.float2byte(Ka_2);
                int acc_scale_2_length = acc_scale_2.length;

                byte[] acc_offset_0 = HexDump.float2byte(Ba_0);
                int acc_offset_0_length = acc_offset_0.length;
                byte[] acc_offset_1 = HexDump.float2byte(Ba_1);
                int acc_offset_1_length = acc_offset_1.length;
                byte[] acc_offset_2 = HexDump.float2byte(Ba_2);
                int acc_offset_2_length = acc_offset_2.length;

                byte[] gyro_scale_0 = HexDump.float2byte(Kg_0);
                int gyro_scale_0_length = gyro_scale_0.length;
                byte[] gyro_scale_1 = HexDump.float2byte(Kg_1);
                int gyro_scale_1_length = gyro_scale_1.length;
                byte[] gyro_scale_2 = HexDump.float2byte(Kg_2);
                int gyro_scale_2_length = gyro_scale_2.length;

                byte[] gyro_offset_0 = HexDump.float2byte(Bg_0);
                int gyro_offset_0_length = gyro_offset_0.length;
                byte[] gyro_offset_1 = HexDump.float2byte(Bg_1);
                int gyro_offset_1_length = gyro_offset_1.length;
                byte[] gyro_offset_2 = HexDump.float2byte(Bg_2);
                int gyro_offset_2_length = gyro_offset_2.length;

                int totalSize = cmdLength + acc_offset_0_length + acc_scale_1_length + acc_scale_2_length + acc_offset_0_length + acc_offset_1_length + acc_offset_2_length + gyro_scale_0_length + gyro_scale_1.length + gyro_scale_2_length + gyro_offset_0_length + gyro_offset_1_length + gyro_offset_2_length;

                byte[] totalArray = new byte[totalSize];
                LogUtil.d(TAG, "totalArray: size====> " + totalArray.length + " ,gyro_offset_1_length: " + gyro_offset_1_length);
                //head
                System.arraycopy(cmd_write_param_calib_head, 0, totalArray, 0, cmdLength);

                //acc_scale
                System.arraycopy(acc_scale_0, 0, totalArray, cmdLength, acc_scale_0_length);

                int tmpLength = cmdLength + acc_scale_0_length;
                System.arraycopy(acc_scale_1, 0, totalArray, tmpLength, acc_scale_1_length);

                int tmpLength1 = tmpLength + acc_scale_1_length;
                System.arraycopy(acc_scale_2, 0, totalArray, tmpLength1, acc_scale_2_length);

                //acc_offset
                int tmpAccOffLength = tmpLength1 + acc_scale_2_length;
                System.arraycopy(acc_offset_0, 0, totalArray, tmpAccOffLength, acc_offset_0_length);

                int tmpAccOffLength1 = tmpAccOffLength + acc_offset_0_length;
                System.arraycopy(acc_offset_1, 0, totalArray, tmpAccOffLength1, acc_offset_1_length);

                int tmpAccOffLen2 = tmpAccOffLength1 + acc_offset_1_length;
                System.arraycopy(acc_offset_2, 0, totalArray, tmpAccOffLen2, acc_offset_2_length);

                //gyro_scale
                int tmpGyroLen = tmpAccOffLen2 + acc_offset_2_length;
                System.arraycopy(gyro_scale_0, 0, totalArray, tmpGyroLen, gyro_scale_0_length);

                int tmpGyroLen1 = tmpGyroLen + gyro_scale_0_length;
                System.arraycopy(gyro_scale_1, 0, totalArray, tmpGyroLen1, gyro_scale_1_length);

                int tmpGyroLen2 = tmpGyroLen1 + gyro_scale_1_length;
                System.arraycopy(gyro_scale_2, 0, totalArray, tmpGyroLen2, gyro_scale_2_length);

                //gyro_offset
                int tmpOffsetLen = tmpGyroLen2 + gyro_scale_2_length;
                System.arraycopy(gyro_offset_0, 0, totalArray, tmpOffsetLen, gyro_offset_0_length);

                int tmpOffsetLen1 = tmpOffsetLen + gyro_offset_0_length;
                System.arraycopy(gyro_offset_1, 0, totalArray, tmpOffsetLen1, gyro_offset_1_length);

                int tmpOffsetLen2 = tmpOffsetLen1 + gyro_offset_1_length;
                System.arraycopy(gyro_offset_2, 0, totalArray, tmpOffsetLen2, gyro_offset_2_length);

                //crc8
                byte res = CRC8.calcCrc8(totalArray);
                LogUtil.d(TAG, "Crc result=====>" + HexDump.toHexString(res));

                byte[] finalArr = new byte[totalArray.length + 1];
                System.arraycopy(totalArray, 0, finalArr, 0, totalArray.length);

                byte[] tmpArr = new byte[]{res};
                System.arraycopy(tmpArr, 0, finalArr, finalArr.length - 1, tmpArr.length);


                //System.arraycopy(cc, 0, ee, aa.length + bb.length, cc.length);
                LogUtil.d(TAG, "_handleCalib nsyncData:send  ===>" + CheckUtils.byte2hex(finalArr));

                if (clientcallback != null) {
                    clientcallback.onWrite(finalArr);
                }
                /******写标定失败的时候，可以临时更新 标定参数 *****/
//                if (!isWritedClib) {
//                    updateDatas(finalArr);
//                    LogUtil.e(TAG, "_handleCalib updateDatas:write  ===>" + CheckUtils.byte2hex(finalArr));
//                }
                isWritedClib = true;


//                send(cmd_read_param_calib);
                Log.e(TAG, "_handleCalib: cmd_read_param_calib.");
            }
        }
    }

    public void dispatchResetQuaternion(double X, double Y, double Z, double quat_W, double quat_X, double quat_Y, double quat_Z) {
        Intent intent = new Intent();
        intent.setAction("com.nolovr.glass.reset.data");

        Bundle bundle = new Bundle();
        bundle.putDouble("X", X);
        bundle.putDouble("Y", Y);
        bundle.putDouble("Z", Z);
        bundle.putDouble("quat_W", quat_W);
        bundle.putDouble("quat_X", quat_X);
        bundle.putDouble("quat_Y", quat_Y);
        bundle.putDouble("quat_Z", quat_Z);

        intent.putExtra("Data", bundle);
        mContext.sendBroadcast(intent);
    }

    public void dispatchImuDataDouble(long timestamp, double acc_X, double acc_Y, double acc_Z, double gyro_X, double gyro_Y, double gyro_Z) {

        countDouble++;
        long currentTimeMillis = System.currentTimeMillis();
        long difftime = currentTimeMillis - lastTimeDouble;
        if (difftime > 1000) {
            LogUtil.d(TAG, 0 + "《=== map Hz imuDataDouble =》" + countDouble + "|timestamp= " + timestamp + "|" + "imuDataDouble: acc_X=" + acc_X + " acc_Y=" + acc_Y + " acc_Z=" + acc_Z + " gyro_X=" + gyro_X + " gyro_Y=" + gyro_Y + " gyro_Z=" + gyro_Z);
            lastTimeDouble = currentTimeMillis;
            countDouble = 0;
        }

    }


    public void dispatchImuData(long timestampAndroid, long timestamp, short acc_X, short acc_Y, short acc_Z,
                                short gyro_X, short gyro_Y, short gyro_Z) {

        count++;
        long currentTimeMillis = System.currentTimeMillis();
        long difftime = currentTimeMillis - lastTime;
        if (difftime > 1000) {
            LogUtil.e(TAG, 0 + "  <= map size ;Hz onNewData imuData =》"
                    + count + "|" + "imuData: acc_X=" + acc_X + " acc_Y=" + acc_Y + " acc_Z="
                    + acc_Z + " gyro_X=" + gyro_X + " gyro_Y=" + gyro_Y + " gyro_Z=" + gyro_Z);
            lastTime = currentTimeMillis;
            count = 0;
        }

    }

    public void dispatchNsyncData(long timestamp_android, long timestamp_firmware) {

        // LogUtil.d(TAG, "nsyncData: " + timestamp);
        _handleCalib(false);
        long sync = (timestamp_android - lastTimestamp) / (1000000);
        if (sync > GAP_SYNC) {
            LogUtil.d(TAG, "nsyncData: timestamp ============================ " + sync);
        } else {
//            LogUtil.d(TAG, "nsyncData: timestamp = " + sync);
        }
        lastTimestamp = timestamp_android;

        countSync++;
        long currentTimeMillis = System.currentTimeMillis();
        long difftime = currentTimeMillis - lastTimeSync;
        if (difftime > 1000) {
            LogUtil.d(TAG, 0 + "  <= map size ;Hz onNewData nsyncData =》" + countSync + "");
            lastTimeSync = currentTimeMillis;
            countSync = 0;
        }

    }

    public void dispatchNsyncData(long timestamp_android, long timestamp_firmware,
                                  int ipd, int pSensor, int brightness, int left, int right) {

        if (useOneThread) {
            _nsyncData(timestamp_android, timestamp_firmware, ipd, pSensor, brightness, left, right);
        } else {
            if (vsyncDispatchHandler == null) return;
            vsyncDispatchHandler.removeCallbacks(null);
            vsyncDispatchHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (!vsyncDispatchBindBigCpus) {
                        //Affinity.affinityToBigCpu();
                        vsyncDispatchBindBigCpus = true;
                    }
                    _nsyncData(timestamp_android, timestamp_firmware, ipd, pSensor, brightness, left, right);
                }
            });
        }
    }

    public void dispatchCalibrationData(int saveState, float[] acc_scale, float[] acc_offset, float[] gyro_scale, float[] gyro_offset) {
        if (saveState != 0) {
            //增加写入标定参数
            // 转json
            try {
                JSONObject jsonObject = new JSONObject();
                JSONArray jsonArray = null;
                float[] cacheArray = null;

                cacheArray = acc_scale;
                jsonArray = new JSONArray();
                for (int i = 0; i < 3; i++) {
                    jsonArray.put(i, cacheArray[i]);
                }
                jsonObject.put("Ka", jsonArray);

                cacheArray = acc_offset;
                jsonArray = new JSONArray();
                for (int i = 0; i < 3; i++) {
                    jsonArray.put(i, cacheArray[i]);
                }
                jsonObject.put("Ba", jsonArray);

                cacheArray = gyro_scale;
                jsonArray = new JSONArray();
                for (int i = 0; i < 3; i++) {
                    jsonArray.put(i, cacheArray[i]);
                }
                jsonObject.put("Kg", jsonArray);

                cacheArray = gyro_offset;
                jsonArray = new JSONArray();
                for (int i = 0; i < 3; i++) {
                    jsonArray.put(i, cacheArray[i]);
                }
                jsonObject.put("Bg", jsonArray);
                takeInCalibration(jsonObject.toString());
            } catch (JSONException e) {
                LogUtil.e(TAG, e);
            }
            return;
        }

        if (((acc_offset[0] == 0f) &&
                (acc_offset[1] == 0f) &&
                (acc_offset[2] == 0f) &&
                (acc_scale[0] == 0f) &&
                (acc_scale[1] == 0f) &&
                (acc_scale[2] == 0f)) ||

                ((gyro_offset[0] == 0f) &&
                        (gyro_offset[1] == 0f) &&
                        (gyro_offset[2] == 0f) &&
                        (gyro_scale[0] == 0f) &&
                        (gyro_scale[1] == 0f) &&
                        (gyro_scale[2] == 0f))

        ) {
            LogUtil.e(TAG, "calibrationData: 写入默认标定参数 start ");
            isWritedClib = false;
            _handleCalib(true);
            LogUtil.e(TAG, "calibrationData: 写入默认标定参数 over ");
        }


        //setDeviceHmdUploadState(true);

    }

    // 把json转回去？ Why ？Why ？Why ？
    private void takeInCalibration(String IMUIntrinsics) {
        // 写入数据到 usb

        LogUtil.d(TAG, "takeInCalibration: 写入标定参数 " + IMUIntrinsics);
        //ces 2
        float Ka_0 = 0.00480719912680317f, Ka_1 = 0.00479163126797840f, Ka_2 = 0.00479638612655039f;
        float Ba_0 = 3.75438745982581f, Ba_1 = 39.7640818637666f, Ba_2 = -21.8134373436029f;
        float Kg_0 = 0.00107809145508912f, Kg_1 = 0.00109914948365165f, Kg_2 = 0.00108343849903781f;
        float Bg_0 = 5.78183906866130f, Bg_1 = 3.54605776548800f, Bg_2 = 10.2271613890521f;
        try {
            String data = IMUIntrinsics;

            JSONObject jsonObject = new JSONObject(data);
            JSONArray jsonArrayKa = jsonObject.optJSONArray("Ka");
            JSONArray jsonArrayBa = jsonObject.optJSONArray("Ba");

            JSONArray jsonArrayKg = jsonObject.optJSONArray("Kg");
            JSONArray jsonArrayBg = jsonObject.optJSONArray("Bg");
            Ka_0 = (float) jsonArrayKa.optDouble(0);
            Ka_1 = (float) jsonArrayKa.optDouble(1);
            Ka_2 = (float) jsonArrayKa.optDouble(2);

            Kg_0 = (float) jsonArrayKg.optDouble(0);
            Kg_1 = (float) jsonArrayKg.optDouble(1);
            Kg_2 = (float) jsonArrayKg.optDouble(2);

            Bg_0 = (float) jsonArrayBg.optDouble(0);
            Bg_1 = (float) jsonArrayBg.optDouble(1);
            Bg_2 = (float) jsonArrayBg.optDouble(2);

            Ba_0 = (float) jsonArrayBa.optDouble(0);
            Ba_1 = (float) jsonArrayBa.optDouble(1);
            Ba_2 = (float) jsonArrayBa.optDouble(2);

        } catch (JSONException e) {
            LogUtil.e(TAG, "setCalibration err");
        }
        byte[] acc_scale_0 = HexDump.float2byte(Ka_0);
        byte[] acc_scale_1 = HexDump.float2byte(Ka_1);
        byte[] acc_scale_2 = HexDump.float2byte(Ka_2);
        byte[] acc_offset_0 = HexDump.float2byte(Ba_0);
        byte[] acc_offset_1 = HexDump.float2byte(Ba_1);
        byte[] acc_offset_2 = HexDump.float2byte(Ba_2);
        byte[] gyro_scale_0 = HexDump.float2byte(Kg_0);
        byte[] gyro_scale_1 = HexDump.float2byte(Kg_1);
        byte[] gyro_scale_2 = HexDump.float2byte(Kg_2);
        byte[] gyro_offset_0 = HexDump.float2byte(Bg_0);
        byte[] gyro_offset_1 = HexDump.float2byte(Bg_1);
        byte[] gyro_offset_2 = HexDump.float2byte(Bg_2);
        byte[] totalArray = new byte[48];
        int float2ByteLength = 4;
        int index = 0;
        //acc_scale
        System.arraycopy(acc_scale_0, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(acc_scale_1, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(acc_scale_2, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(acc_offset_0, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(acc_offset_1, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(acc_offset_2, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(gyro_scale_0, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(gyro_scale_1, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(gyro_scale_2, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(gyro_offset_0, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(gyro_offset_1, 0, totalArray, index, float2ByteLength);
        index += float2ByteLength;
        System.arraycopy(gyro_offset_2, 0, totalArray, index, float2ByteLength);

        if (clientcallback != null) {
            clientcallback.onWritecalibConfig(totalArray);
        }
    }

    // usb连接建立
    public void sendUsbConnectedBroadcast() {
        LogUtil.d(TAG, "sendUsbConnectedBroadcast");
        Intent intent = new Intent();
        //intent.setAction(Constants.ACTION_USB_CONNECTED);
        mContext.sendBroadcast(intent);
    }
}
