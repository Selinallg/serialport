package tp.xmaihh.serialport;

import android.os.Process;
import android.os.SystemClock;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.InvalidParameterException;

import android_serialport_api.SerialPort;
import tp.xmaihh.serialport.bean.ComBean;
import tp.xmaihh.serialport.stick.AbsStickPackageHelper;
import tp.xmaihh.serialport.stick.BaseStickPackageHelper;
import tp.xmaihh.serialport.stick.SpecifiedStickPackageHelper;
import tp.xmaihh.serialport.stick.StaticLenStickPackageHelper;
import tp.xmaihh.serialport.utils.ByteUtil;

public abstract class SerialFileHelper {
    private static final String TAG = "SerialFileHelper";
    private OutputStream mOutputStream;
    private InputStream mInputStream;
    private ReadThread mReadThread;
    private SendThread mSendThread;
    private String sPort = "dev/ttyACM0";

    private boolean _isOpen = false;
    private byte[] _bLoopData = {48};
    private int iDelay = 500;


    public SerialFileHelper(String sPort) {
        this.sPort = sPort;

    }

    public void open()
            throws SecurityException, IOException, InvalidParameterException {
        Log.d(TAG, "open: 1");
        try {
            File file = new File(sPort);
            if (file.exists()) {
                mInputStream = new FileInputStream(file);
                mOutputStream = new FileOutputStream(file);
                this.mReadThread = new ReadThread();
                Log.d(TAG, "open: 5");
                this.mReadThread.start();
                Log.d(TAG, "open: 6");
            }
        } catch (Exception e) {
            Log.e(TAG, "doOpen: ", e);
            Log.e(TAG, "doOpen: " + e.getMessage());
        }
        this._isOpen = true;
        Log.d(TAG, "open: 10");
    }

    public void close() {
        if (this.mReadThread != null) {
            this.mReadThread.interrupt();
        }

        this._isOpen = false;
    }

    public void send(byte[] bOutArray) {
        try {
            this.mOutputStream.write(bOutArray);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void sendHex(String sHex) {
        byte[] bOutArray = ByteUtil.HexToByteArr(sHex);
        send(bOutArray);
    }

    public void sendTxt(String sTxt) {
        byte[] bOutArray = sTxt.getBytes();
        send(bOutArray);
    }

    private class ReadThread
            extends Thread {
        private ReadThread() {
        }

        public void run() {
            super.run();
            Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
            while (!isInterrupted()) {
                try {
                    if (SerialFileHelper.this.mInputStream == null) {
                        return;
                    }

                    if (true) {


                        byte[] buffer = getStickPackageHelper().execute(SerialFileHelper.this.mInputStream);
                        if (buffer != null && buffer.length > 0) {
                            ComBean ComRecData = new ComBean(SerialFileHelper.this.sPort, buffer, buffer.length);
                            SerialFileHelper.this.onDataReceived(ComRecData);
                        }
                    } else {


                        int available = SerialFileHelper.this.mInputStream.available();

                        if (available > 0) {
                            byte[] buffer = new byte[available];
                            int size = SerialFileHelper.this.mInputStream.read(buffer);
                            if (size > 0) {
                                ComBean ComRecData = new ComBean(SerialFileHelper.this.sPort, buffer, size);
                                SerialFileHelper.this.onDataReceived(ComRecData);
                            }
                        } else {
                            SystemClock.sleep(50);
                            Log.e(TAG, "run: ======================sleep========");
                        }
                    }

                } catch (Exception e) {
                    Log.e("error", e.getMessage());
                    return;
                }
            }
        }
    }

    private class SendThread
            extends Thread {
        public boolean suspendFlag = true;

        private SendThread() {
        }

        public void run() {
            super.run();
            while (!isInterrupted()) {
                synchronized (this) {
                    while (this.suspendFlag) {
                        try {
                            wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
                SerialFileHelper.this.send(SerialFileHelper.this.getbLoopData());
                try {
                    Thread.sleep(SerialFileHelper.this.iDelay);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        public void setSuspendFlag() {
            this.suspendFlag = true;
        }

        public synchronized void setResume() {
            this.suspendFlag = false;
            notify();
        }
    }


    public String getPort() {
        return this.sPort;
    }

    public boolean setPort(String sPort) {
        if (this._isOpen) {
            return false;
        }
        this.sPort = sPort;
        return true;
    }

    public boolean isOpen() {
        return this._isOpen;
    }

    public byte[] getbLoopData() {
        return this._bLoopData;
    }

    public void setbLoopData(byte[] bLoopData) {
        this._bLoopData = bLoopData;
    }

    public void setTxtLoopData(String sTxt) {
        this._bLoopData = sTxt.getBytes();
    }

    public void setHexLoopData(String sHex) {
        this._bLoopData = ByteUtil.HexToByteArr(sHex);
    }

    public int getiDelay() {
        return this.iDelay;
    }

    public void setiDelay(int iDelay) {
        this.iDelay = iDelay;
    }

    public void startSend() {
        if (this.mSendThread != null) {
            this.mSendThread.setResume();
        }
    }

    public void stopSend() {
        if (this.mSendThread != null) {
            this.mSendThread.setSuspendFlag();
        }
    }

    protected abstract void onDataReceived(ComBean paramComBean);

    // 002A00AA5526006C0
    public static byte[] cmd_head = {0x2A, 0x00, (byte) 0xAA, 0x55, 0x26, 0x00, 0x6C, 0x02};//
    public static byte[] cmd_tail = {0x42, 0x00};//

//        private AbsStickPackageHelper mStickPackageHelper = new BaseStickPackageHelper();  // 默认不处理粘包，直接读取返回
//    private AbsStickPackageHelper mStickPackageHelper = new StaticLenStickPackageHelper(64);  //
    private AbsStickPackageHelper mStickPackageHelper = new SpecifiedStickPackageHelper(cmd_head, cmd_tail);  //

    public AbsStickPackageHelper getStickPackageHelper() {
        return mStickPackageHelper;
    }

    public void setStickPackageHelper(AbsStickPackageHelper mStickPackageHelper) {
        this.mStickPackageHelper = mStickPackageHelper;
    }

}
