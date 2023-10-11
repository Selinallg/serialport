package com.ex.serialport;

import static com.nolovr.core.data.usb.config.Config.CMD_DEVICE_HMD_UPLOAD_START;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.nolovr.core.data.usb.serialport.UartImpl;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

import tp.xmaihh.serialport.utils.CheckUtils;

public class Main3Activity extends BaseActivity {

    private static final String TAG = "Main3Activity";

    static {
       System.loadLibrary("uartusb");
    }

    private UartImpl proxy;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main3);
        proxy = UartImpl.getInstance(getBaseContext());
        proxy.init();
        proxy.updateDeviceType(3);
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public void doOpen(View view) {

        open();
    }


    public void doStream(View view) {
        doStartData();
    }


    public void doClose(View view) {
        close();
    }



    public void goJava(View view) {

        Intent intent = new Intent(this,MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }



    private void doStartData() {

        for (int i = 0; i < 10; ++i) {
            byte[] cmd2 = makeCmd(null, CMD_DEVICE_HMD_UPLOAD_START);
            write(cmd2,cmd2.length);
            Log.d(TAG, "doStartData: " + CheckUtils.byte2hex(cmd2).toString());
        }
    }

    public native void open();
    public native void close();

    public native void write(byte[] datas,int length);



    protected byte[] makeCmd(byte[] data, int cmd) {
        byte[] result = CmdUtils.makeCmd(data, cmd);
        result[result.length - 1] = calcuCrc8(result, result.length - 1);
        return result;
    }

    public static native byte calcuCrc8(byte[] data, int length);

}