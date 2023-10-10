package com.ex.serialport;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.nolovr.core.data.usb.serialport.UartImpl;

public class Main4Activity extends BaseActivity {

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

        proxy.open();
    }


    public void doClose(View view) {
        proxy.close();
    }


}