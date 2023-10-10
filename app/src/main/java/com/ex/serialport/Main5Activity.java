package com.ex.serialport;

import static com.nolovr.core.data.usb.config.Config.CMD_DEVICE_HMD_UPLOAD_START;

import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.nolovr.core.data.usb.serialport.UartImpl;

import java.io.IOException;

import tp.xmaihh.serialport.SeriaFdlHelper;
import tp.xmaihh.serialport.SerialHelper;
import tp.xmaihh.serialport.bean.ComBean;
import tp.xmaihh.serialport.utils.CheckUtils;

public class Main5Activity extends BaseActivity {

    private static final String TAG = "Main5Activity";

    static {
        System.loadLibrary("uartusb");
    }

    private UartImpl proxy;
    private SeriaFdlHelper serialHelper;
    private String devPort;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main3);
        devPort = "dev/ttyACM0";
        proxy = UartImpl.getInstance(getBaseContext());
        proxy.init();
        proxy.updateDeviceType(3);
        serialHelper = new SeriaFdlHelper(devPort, 115200);
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public void doOpen(View view) {
        try {
            serialHelper.open();
            int fd = serialHelper.getFd();
            if (fd > 0) {
                openWithFd(fd);
            }
        } catch (IOException e) {
            Log.d(TAG, "doOpen: fail");
            throw new RuntimeException(e);
        }
       
    }


    public void doStream(View view) {
        doStartData();
    }


    public void doClose(View view) {
        int fd = serialHelper.getFd();
        if (fd > 0) {
            closeWithFd(fd);
        }
        serialHelper.close(fd);
    }


    private void doStartData() {

        for (int i = 0; i < 10; ++i) {
            byte[] cmd2 = makeCmd(null, CMD_DEVICE_HMD_UPLOAD_START);
             write(cmd2,cmd2.length);
            Log.d(TAG, "doStartData: " + CheckUtils.byte2hex(cmd2).toString());
        }
    }

    public native void openWithFd(int fd);

    public native void closeWithFd(int fd);

    public native void write(byte[] datas,int length);


    protected byte[] makeCmd(byte[] data, int cmd) {
        byte[] result = CmdUtils.makeCmd(data, cmd);
        result[result.length - 1] = calcuCrc8(result, result.length - 1);
        return result;
    }

    public static native byte calcuCrc8(byte[] data, int length);

}