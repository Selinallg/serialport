package com.nolovr.core.data.usb.utils;

import android.content.Context;
import android.os.Build;
import android.os.Environment;
import android.os.SystemClock;
import android.util.Log;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.StringDef;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.Executors;

@Keep
public class FileDataUtils {

    private static final String TAG = "FileDataUtils";

    public static final String TYPE_IMU = "IMU";//车机陀螺仪、加速度计原始数据
    public static final String TYPE_VSYNC = "VSYNC";//CUTE陀螺仪、加速度计原始数据
    public static final String TYPE_IMU_FORMAT = "IMU_FORMAT";//
    public static final String TYPE_BLE_FORMAT = "VSYNC_FORMAT";//
    public static final String TYPE_SPEED = "SPEED";//车机四元素,json 格式
    public static final String TYPE_SPEED_RAW = "SPEED_RAW";//车机四元素
    public static final String TYPE_STEELING_WHEEL = "STEELING_WHEEL";//Cute四元素,json 格式
    public static final String TYPE_STEELING_WHEEL_RAW = "STEELING_WHEEL_RAW";//Cute四元素
    public static final String TYPE_GPS = "GPS";//车机四元素 与 Cute四元素 融合后的数据

    @Retention(RetentionPolicy.SOURCE)
    @StringDef({TYPE_IMU, TYPE_VSYNC, TYPE_IMU_FORMAT, TYPE_BLE_FORMAT, TYPE_SPEED, TYPE_SPEED_RAW,
            TYPE_STEELING_WHEEL, TYPE_STEELING_WHEEL_RAW, TYPE_GPS})
    public @interface WriteType {

    }

    private static String LOCAL_FILE_PATH;

    private static final Map<String, ConcurrentLinkedQueue<String>> sDataCacheQueueMap = new HashMap<>();
    private static final Map<String, FileWriter> sFileWriterMap = new HashMap<>();
    private static final Map<String, String> sFileHeaderStringMap = new HashMap<>();

    static {
        sFileHeaderStringMap.put(TYPE_BLE_FORMAT, "timestamp,NRF,xgyro,ygyro,zgyro,xacc,yacc,zacc");
    }

    private static boolean isWriting;
    private static boolean initialized;

    public static boolean isWriteEnable;
    private static long lastTimestamp;//用于所有文件共用一个时间节点命名
    private static final String END_PATH = File.separator+ "00A_IMU" + File.separator;
    public static String init(Context context) {
        Log.d(TAG, "init: ");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            LOCAL_FILE_PATH = context.getApplicationContext().getExternalCacheDir().getAbsolutePath()+File.separator + END_PATH;
        }else {
            LOCAL_FILE_PATH = Environment.getExternalStorageDirectory().getAbsolutePath() + END_PATH;
        }
        File file = new File(LOCAL_FILE_PATH);
        if (!file.exists()) {
            file.mkdirs();
        }

        if (!file.exists()) {
            throw new IllegalStateException("当前无法创建默认文件夹" + LOCAL_FILE_PATH);
        }


        startThread();
        lastTimestamp = System.currentTimeMillis();
        initialized = true;
        return LOCAL_FILE_PATH;
    }

    public static String setIsWriteEnable(boolean enable,Context context) {
        if (isWriteEnable ==enable){
            return "";
        }
        isWriteEnable = enable;
        Log.d(TAG, "setIsWriteEnable: " + enable);
        if (isWriteEnable) {
            return init(context.getApplicationContext());
        } else {
            release();
            return "";
        }
    }

    private static void startThread() {
        Log.d(TAG, "startThread: isWriteEnable=" + isWriteEnable + "\tisWriting=" + isWriting);
        if (!isWriteEnable) {
            return;
        }

        if (!isWriting) {
            Executors.newSingleThreadExecutor().execute(() -> {
                isWriting = true;
                while (isWriting) {
                    int useTime = 0;
                    synchronized (sDataCacheQueueMap) {
                        if (!sDataCacheQueueMap.isEmpty()) {
                            for (String type : sDataCacheQueueMap.keySet()) {
                                useTime += realWriteToFile(type);
                            }
                        }
                    }

                    if (useTime <= 10) {
                        SystemClock.sleep(100);
                    }
                }
            });
        }
    }

    public static void reset(@WriteType String type) {
        Log.d(TAG, "reset: " + type);
        synchronized (sFileWriterMap) {
            sFileWriterMap.remove(type);
        }

        synchronized (sDataCacheQueueMap) {
            sDataCacheQueueMap.remove(type);
        }

        synchronized (sFileHeaderStringMap) {
            sFileHeaderStringMap.remove(type);
        }
    }

    public static void setFileHeaderString(@WriteType String type, String header) {
        sFileHeaderStringMap.put(type, header);
    }

    public static void writeToFile(@WriteType String type, String data) {
        if (!initialized || !isWriteEnable) {
            return;
        }

        ConcurrentLinkedQueue<String> result = getCacheByWriteType(type);
        result.offer(data);
    }

    private static @NonNull
    ConcurrentLinkedQueue<String> getCacheByWriteType(@WriteType String type) {
        synchronized (sDataCacheQueueMap) {
            ConcurrentLinkedQueue<String> queue = sDataCacheQueueMap.get(type);
            if (queue == null) {
                queue = new ConcurrentLinkedQueue<>();
                sDataCacheQueueMap.put(type, queue);
            }
            return queue;
        }
    }

    private static FileWriter getByWriteType(@WriteType String type) throws IOException {

        FileWriter fileWriter = sFileWriterMap.get(type);
        if (fileWriter == null) {
            synchronized (sFileWriterMap) {
                fileWriter = sFileWriterMap.get(type);
                if (fileWriter==null) {
                    fileWriter = new FileWriter(LOCAL_FILE_PATH + type + "." + lastTimestamp + ".txt");
                    sFileWriterMap.put(type, fileWriter);
                }
            }
        }
        return fileWriter;

    }

    private static int realWriteToFile(@WriteType String type) {
        int currentCount = 0;
        try {
            ConcurrentLinkedQueue<String> queue = getCacheByWriteType(type);
            String data = queue.poll();
            if (data == null) {
                return 0;
            }

            FileWriter writer = getByWriteType(type);

            do {
                writer.append(data).append("\n");
                currentCount++;
            } while ((data = queue.poll()) != null);

            if (currentCount > 0) {
                writer.flush();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return currentCount;
    }

    /**
     * 停止的先后顺序要留意
     */
    public static void release() {
        Log.d(TAG, "release: ");
        isWriting = false;

        if (!sFileWriterMap.isEmpty()) {
            synchronized (sFileWriterMap) {
                Iterator<Map.Entry<String, FileWriter>> writerIterator = sFileWriterMap.entrySet().iterator();
                while (writerIterator.hasNext()) {
                    Map.Entry<String, FileWriter> entry = writerIterator.next();
                    FileWriter writer = entry.getValue();
                    close(writer);// 先关闭再移除
                    writerIterator.remove();
                }
            }
        }

        if (!sDataCacheQueueMap.isEmpty()) {
            synchronized (sDataCacheQueueMap) {
                Iterator<Map.Entry<String, ConcurrentLinkedQueue<String>>> queueIterator = sDataCacheQueueMap.entrySet().iterator();
                while (queueIterator.hasNext()) {
                    Map.Entry<String, ConcurrentLinkedQueue<String>> entry = queueIterator.next();
                    ConcurrentLinkedQueue<String> queue = entry.getValue();
                    if (queue != null) {
                        queue.clear();
                    }

                    queueIterator.remove();
                }
            }
        }

        initialized = false;
    }

    private static void close(FileWriter writer) {
        if (writer == null) {
            return;
        }

        try {
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
