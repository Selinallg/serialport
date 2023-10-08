package tp.xmaihh.serialport;

import android.os.SystemClock;
import android.util.Log;

import java.io.File;
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
import tp.xmaihh.serialport.utils.CheckUtils;

public abstract class SerialHelper {
    private static final String TAG = "SerialHelper";
    private SerialPort mSerialPort;
    private OutputStream mOutputStream;
    private InputStream mInputStream;
    private ReadThread mReadThread;
    private SendThread mSendThread;
    private String sPort = "/dev/ttyHS1";
    private int iBaudRate = 9600;
    private int stopBits = 1;
    private int dataBits = 8;
    private int parity = 0;
    private int flowCon = 0;
    private int flags = 0;
    private boolean _isOpen = false;
    private byte[] _bLoopData = {48};
    private int iDelay = 500;


    public SerialHelper(String sPort, int iBaudRate) {
        this.sPort = sPort;
        this.iBaudRate = iBaudRate;
    }

    public void open()
            throws SecurityException, IOException, InvalidParameterException {
        Log.d(TAG, "open: 1");
        this.mSerialPort = new SerialPort(new File(this.sPort), this.iBaudRate, this.stopBits, this.dataBits, this.parity, this.flowCon, this.flags);
        Log.d(TAG, "open: 2");
        this.mOutputStream = this.mSerialPort.getOutputStream();
        Log.d(TAG, "open: 3");
        this.mInputStream = this.mSerialPort.getInputStream();
        Log.d(TAG, "open: 4");
        this.mReadThread = new ReadThread();
        Log.d(TAG, "open: 5");
        this.mReadThread.start();
        Log.d(TAG, "open: 6");
        this.mSendThread = new SendThread();
        Log.d(TAG, "open: 7");
        this.mSendThread.setSuspendFlag();
        Log.d(TAG, "open: 8");
        this.mSendThread.start();
        Log.d(TAG, "open: 9");
        this._isOpen = true;
        Log.d(TAG, "open: 10");
    }

    public void close() {
        if (this.mReadThread != null) {
            this.mReadThread.interrupt();
        }
        if (this.mSerialPort != null) {
            this.mSerialPort.close();
            this.mSerialPort = null;
        }
        this._isOpen = false;
    }

    public void send(byte[] bOutArray) {
        try {
            this.mOutputStream.write(bOutArray);
            Log.d(TAG, "send: sucess");
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

        public void run() {
            super.run();
            while (!isInterrupted()) {
                try {
                    if (SerialHelper.this.mInputStream == null) {
                        return;
                    }

                    byte[] buffer = getStickPackageHelper().execute(SerialHelper.this.mInputStream);
                    if (buffer != null && buffer.length > 0) {
                        ComBean ComRecData = new ComBean(SerialHelper.this.sPort, buffer, buffer.length);
                        SerialHelper.this.onDataReceived(ComRecData);
                    }

//                    int available = SerialHelper.this.mInputStream.available();
//
//                    if (available > 0) {
//                        byte[] buffer = new byte[available];
//                        int size = SerialHelper.this.mInputStream.read(buffer);
//                        if (size > 0) {
//                            ComBean ComRecData = new ComBean(SerialHelper.this.sPort, buffer, size);
//                            SerialHelper.this.onDataReceived(ComRecData);
//                        }
//                    } else {
//                        SystemClock.sleep(50);
//                    }

                } catch (Throwable e) {
                    Log.e("error", e.getMessage());
                    return;
                }
            }
        }
    }


    public class ReadThread2 extends Thread {
        @Override
        public void run() {
            super.run();
            Log.d(TAG, "run: start");
            int length = 512;
            byte[] buffer = new byte[length];
            byte[] cmd = new byte[length];
            int bytes = 0;
            int index = 0;
            while (!isInterrupted()) {
                int size;
                try {
                    if (mInputStream == null) {
                        return;
                    }
                    /*
                     * 这里的read要注意，它会一直等待数据,直到天荒地老。如果要判断是否接受完成，只有设置结束标识，或作其他特殊的处理
                     */
                    size = mInputStream.read(buffer);

                    // 2A00AA5526006C02A272099B00000000F9FF040009009905A70560FE0000000005C2000700000000000005B100000000000000000000000000000000000000004200

//                    Log.d(TAG, "run: ---------------");
                    if (size > 0) {
                        boolean isStop = false;
                        for (int i = 0; i < size; i++) {
                            //如果是以$符号开始
                            if ((buffer[i] == (byte) 0x2A) && (buffer[i] == (byte) 0x00)) {
                                index = 0;
                                Log.i(TAG, "start flag $");
                            }
                            if ((i > 0) && (buffer[i] == (byte) 0x00)
                                    && (buffer[i - 1] == (byte) 0x42)) {
                                isStop = true;
                                Log.i(TAG, "end flag \n");
                            }
                            //如果长度大于512则进行重新创建数组对象，丢弃原先的命令。
                            if (index >= length) {
                                cmd = new byte[length];
                                index = 0;
                            } else {
                                cmd[index] = buffer[i];
                                Log.d(TAG, "run: 2222");
                                if (isStop) {
                                    Log.d(TAG, "run: " + CheckUtils.byte2hex(cmd));
                                    // TODO: 2023/9/28 回调数据
                                    // 发送命令，并清除已发送的命令.
//                                    if (cmd[0] == 36) {// $
//                                        // parseReceiveData(cmd, index);
//                                    } else {
//                                        Log.i(TAG, "the cmd is wrong");
//                                    }
                                    isStop = false;
                                    index = 0;
                                } else {
                                    index++;
                                }
                            }
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
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
                SerialHelper.this.send(SerialHelper.this.getbLoopData());
                try {
                    Thread.sleep(SerialHelper.this.iDelay);
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

    public int getBaudRate() {
        return this.iBaudRate;
    }

    public boolean setBaudRate(int iBaud) {
        if (this._isOpen) {
            return false;
        }
        this.iBaudRate = iBaud;
        return true;
    }

    public boolean setBaudRate(String sBaud) {
        int iBaud = Integer.parseInt(sBaud);
        return setBaudRate(iBaud);
    }

    public int getStopBits() {
        return this.stopBits;
    }

    public boolean setStopBits(int stopBits) {
        if (this._isOpen) {
            return false;
        }
        this.stopBits = stopBits;
        return true;
    }

    public int getDataBits() {
        return this.dataBits;
    }

    public boolean setDataBits(int dataBits) {
        if (this._isOpen) {
            return false;
        }
        this.dataBits = dataBits;
        return true;
    }

    public int getParity() {
        return this.parity;
    }

    public boolean setParity(int parity) {
        if (this._isOpen) {
            return false;
        }
        this.parity = parity;
        return true;
    }

    public int getFlowCon() {
        return this.flowCon;
    }

    public boolean setFlowCon(int flowCon) {
        if (this._isOpen) {
            return false;
        }
        this.flowCon = flowCon;
        return true;
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

    //    private AbsStickPackageHelper mStickPackageHelper = new BaseStickPackageHelper();  // 默认不处理粘包，直接读取返回
//    private AbsStickPackageHelper mStickPackageHelper = new StaticLenStickPackageHelper(64);  // 默认不处理粘包，直接读取返回

    public static byte[] cmd_head = {0x00, 0x2A};//
    //    public static byte[] cmd_head = {0x2A, 0x00};//
    public static byte[] cmd_tail = {0x00, 0x42};//
    //    public static byte[] cmd_tail = {0x42, 0x00};//
    private AbsStickPackageHelper mStickPackageHelper = new SpecifiedStickPackageHelper(cmd_head, cmd_tail);  //

    public AbsStickPackageHelper getStickPackageHelper() {
        return mStickPackageHelper;
    }

    public void setStickPackageHelper(AbsStickPackageHelper mStickPackageHelper) {
        this.mStickPackageHelper = mStickPackageHelper;
    }

}
