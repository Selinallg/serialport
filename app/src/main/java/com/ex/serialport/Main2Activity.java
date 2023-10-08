package com.ex.serialport;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

import tp.xmaihh.serialport.SerialHelper;
import tp.xmaihh.serialport.bean.ComBean;
import tp.xmaihh.serialport.utils.ByteUtil;

public class Main2Activity extends AppCompatActivity {

    private static final String TAG = "_Main2Activity_";
    private FileInputStream mFileInputStream;
    private FileOutputStream mFileOutputStream;

    ReadThread mReadThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        try {
            if (this.mReadThread != null) {
                this.mReadThread.interrupt();
                mReadThread = null;
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public void doOpen(View view) {

        try {
            File file = new File("dev/ttyACM0");
            if (file.exists()) {
                mFileInputStream = new FileInputStream(file);
                mFileOutputStream = new FileOutputStream(file);


                this.mReadThread = new ReadThread();
                Log.d(TAG, "open: 5");
                this.mReadThread.start();
                Log.d(TAG, "open: 6");
            }
        } catch (Exception e) {
            Log.e(TAG, "doOpen: ", e);
            Log.e(TAG, "doOpen: " + e.getMessage());
        }

    }

    private class ReadThread
            extends Thread {
        private ReadThread() {
        }

        public void run() {
            super.run();
            while (!isInterrupted()) {
                try {
                    if (mFileInputStream == null) {
                        return;
                    }

                    if (true) {


//                    byte[] buffer = getStickPackageHelper().execute(SerialHelper.this.mInputStream);
//                    if (buffer != null && buffer.length > 0) {
//                        ComBean ComRecData = new ComBean(SerialHelper.this.sPort, buffer, buffer.length);
//                        SerialHelper.this.onDataReceived(ComRecData);
//                    }
                    } else {


                        int available = mFileInputStream.available();
                        Log.d(TAG, "run: available=" + available);

                        if (available > 0) {
                            byte[] buffer = new byte[available];
                            int size = mFileInputStream.read(buffer);
                            if (size > 0) {
//                            ComBean ComRecData = new ComBean(SerialHelper.this.sPort, buffer, size);
//                            SerialHelper.this.onDataReceived(ComRecData);
                                Log.d(TAG, "run: " + ByteUtil.ByteArrToHex(buffer));
                            }
                        } else {
                            SystemClock.sleep(2);
                            Log.d(TAG, "run: SystemClock.sleep(2)");
                        }
                    }

                } catch (Throwable e) {
                    Log.e("error", e.getMessage());
                    return;
                }
            }
        }
    }
}