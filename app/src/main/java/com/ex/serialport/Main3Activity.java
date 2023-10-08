package com.ex.serialport;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

public class Main3Activity extends AppCompatActivity {

    static {
       System.loadLibrary("uartusb");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main3);
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public void doOpen(View view) {

        open();
    }


    public void doClose(View view) {
        close();
    }

    public native void open();
    public native void close();
}