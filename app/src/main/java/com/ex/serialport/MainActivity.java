package com.ex.serialport;

import android.os.Bundle;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.RecyclerView;;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.Toast;

import java.io.IOException;
import java.io.UnsupportedEncodingException;

import android_serialport_api.SerialPortFinder;
import tp.xmaihh.serialport.SerialFileHelper;
import tp.xmaihh.serialport.SerialHelper;
import tp.xmaihh.serialport.bean.ComBean;
import tp.xmaihh.serialport.utils.ByteUtil;
import tp.xmaihh.serialport.utils.CheckUtils;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("uartusb");
    }

    private static final String TAG = "_MainActivity_";
    private RecyclerView recy;
    private Spinner spSerial;
    private EditText edInput;
    private Button btSend;
    private RadioGroup radioGroup;
    private RadioButton radioButton1;
    private RadioButton radioButton2;
    private SerialPortFinder serialPortFinder;
    private SerialFileHelper serialFileHelper;
    private SerialHelper serialHelper;
    private Spinner spBote;
    private Button btOpen;
    private Button btStream;
    //    private LogListAdapter logListAdapter;
    private Spinner spDatab;
    private Spinner spParity;
    private Spinner spStopb;
    private Spinner spFlowcon;

    private boolean isSonic = false;
    private boolean fileRead = false;
    private boolean dispathInThread = false;
    private String devPort;

    protected Handler dispatchHandler;
    protected HandlerThread dispatchThread;

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (fileRead) {
            serialFileHelper.close();
        } else {
            serialHelper.close();
        }

        if (dispatchThread != null) {
            dispatchHandler.removeCallbacks(null);
            dispatchThread.quitSafely();
            dispatchThread = null;
            dispatchHandler = null;
        }
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        if (isSonic) {
            devPort = "dev/ttyHS4";
        } else {
            devPort = "dev/ttyACM0";
        }
//        recy = (RecyclerView) findViewById(R.id.recyclerView);
        spSerial = (Spinner) findViewById(R.id.sp_serial);
        edInput = (EditText) findViewById(R.id.ed_input);
        btSend = (Button) findViewById(R.id.btn_send);
        spBote = (Spinner) findViewById(R.id.sp_baudrate);
        btOpen = (Button) findViewById(R.id.btn_open);
        btStream = (Button) findViewById(R.id.btn_stream);

        radioGroup = (RadioGroup) findViewById(R.id.radioGroup);
        radioButton1 = (RadioButton) findViewById(R.id.radioButton1);
        radioButton2 = (RadioButton) findViewById(R.id.radioButton2);

        spDatab = (Spinner) findViewById(R.id.sp_databits);
        spParity = (Spinner) findViewById(R.id.sp_parity);
        spStopb = (Spinner) findViewById(R.id.sp_stopbits);
        spFlowcon = (Spinner) findViewById(R.id.sp_flowcon);

        if (dispatchHandler == null) {
            dispatchThread = new HandlerThread("dispatch");
            dispatchThread.start();
            dispatchHandler = new Handler(dispatchThread.getLooper());
        }

//        logListAdapter = new LogListAdapter(null);
//        recy.setLayoutManager(new LinearLayoutManager(this));
//        recy.setAdapter(logListAdapter);
//        recy.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));

//        serialPortFinder = new SerialPortFinder();


        if (fileRead) {
            serialFileHelper = new SerialFileHelper(devPort) {
                long last = 0;
                int count = 0;

                @Override
                protected void onDataReceived(ComBean comBean) {

                    if (dispathInThread) {

                        dispatchHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                _dipatch(comBean);
                            }
                        });

                    } else {
                        _dipatch(comBean);

                    }


                }

                private void _dipatch(ComBean comBean) {



                    count++;
                    try {
                        long c = System.currentTimeMillis();
                        long chazhi = c - last;
//                        Log.d(TAG, "====}}onDataReceived-Hex: " + comBean.sComPort + "|" + comBean.sRecTime + " " + CheckUtils.byte2hex(comBean.bRec) + " length=" + comBean.bRec.length);
//                        Log.d(TAG, "====--onDataReceived-Hex: " + comBean.sComPort + "|" + comBean.sRecTime + " " + ByteUtil.ByteArrToHex(comBean.bRec) + " length=" + comBean.bRec.length);
                        if (chazhi > 1000) {
                            Log.d(TAG, "onDataReceived-Hex: " + comBean.sComPort + "|" + comBean.sRecTime + " " + ByteUtil.ByteArrToHex(comBean.bRec));
                            Log.d(TAG, "onDataReceived: Hz=" + count);
                            last = c;
                            count = 0;
                        }

//                    Log.d(TAG, "onDataReceived-string: " + comBean.sComPort + "|" + comBean.sRecTime + " " + new String(comBean.bRec, "UTF-8"));
                    } catch (Exception e) {
                        throw new RuntimeException(e);
                    }
                }
            };
        } else {
            serialHelper = new SerialHelper(devPort, 115200) {

                long last = 0;
                int count = 0;

                @Override
                protected void onDataReceived(final ComBean comBean) {


                    if (dispathInThread) {
                        dispatchHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                _dipatch(comBean);
                            }
                        });

                    } else {
                        _dipatch(comBean);

                    }
//                runOnUiThread(new Runnable() {
//                    @Override
//                    public void run() {
//                        if (radioGroup.getCheckedRadioButtonId() == R.id.radioButton1) {
//                            try {
//                                Toast.makeText(getBaseContext(), new String(comBean.bRec, "UTF-8"), Toast.LENGTH_SHORT).show();
//                                logListAdapter.addData(comBean.sRecTime + ":   " + new String(comBean.bRec, "UTF-8"));
//                                if (logListAdapter.getData() != null && logListAdapter.getData().size() > 0) {
//                                    recy.smoothScrollToPosition(logListAdapter.getData().size());
//                                }
//                            } catch (UnsupportedEncodingException e) {
//                                e.printStackTrace();
//                            }
//                        } else {
//                            Toast.makeText(getBaseContext(), ByteUtil.ByteArrToHex(comBean.bRec), Toast.LENGTH_SHORT).show();
//                            logListAdapter.addData(comBean.sRecTime + ":   " + ByteUtil.ByteArrToHex(comBean.bRec));
//                            if (logListAdapter.getData() != null && logListAdapter.getData().size() > 0) {
//                                recy.smoothScrollToPosition(logListAdapter.getData().size());
//                            }
//                        }
//                    }
//                });
                }

                private synchronized void _dipatch(ComBean comBean) {
                    updateDatas(comBean.bRec,comBean.bRec.length);
                    count++;
                    try {
                        long c = System.currentTimeMillis();
                        long chazhi = c - last;
//                        Log.d(TAG, "====}}onDataReceived-Hex: " + comBean.sComPort + "|" + comBean.sRecTime + " " + CheckUtils.byte2hex(comBean.bRec) + " length=" + comBean.bRec.length);
//                        Log.d(TAG, "====--onDataReceived-Hex: " + comBean.sComPort + "|" + comBean.sRecTime + " " + ByteUtil.ByteArrToHex(comBean.bRec) + " length=" + comBean.bRec.length);
                        if (chazhi > 1000) {
//                            Log.d(TAG, "onDataReceived-Hex: " + comBean.sComPort + "|" + comBean.sRecTime + " " + ByteUtil.ByteArrToHex(comBean.bRec));
//                            Log.d(TAG, "onDataReceived: Hz=" + count);
                            last = c;
                            count = 0;
                        }

//                    Log.d(TAG, "onDataReceived-string: " + comBean.sComPort + "|" + comBean.sRecTime + " " + new String(comBean.bRec, "UTF-8"));
                    } catch (Exception e) {
                        throw new RuntimeException(e);
                    }
                }
            };
        }


//        final String[] ports = serialPortFinder.getAllDevicesPath();
        final String[] botes = new String[]{"0", "50", "75", "110", "134", "150", "200", "300", "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400", "57600", "115200", "230400", "460800", "500000", "576000", "921600", "1000000", "1152000", "1500000", "2000000", "2500000", "3000000", "3500000", "4000000"};
        final String[] databits = new String[]{"8", "7", "6", "5"};
        final String[] paritys = new String[]{"NONE", "ODD", "EVEN"};
        final String[] stopbits = new String[]{"1", "2"};
        final String[] flowcons = new String[]{"NONE", "RTS/CTS", "XON/XOFF"};


//        SpAdapter spAdapter = new SpAdapter(this);
//        spAdapter.setDatas(ports);
//        spSerial.setAdapter(spAdapter);

        spSerial.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                serialHelper.close();
//                serialHelper.setPort(ports[position]);
                btOpen.setEnabled(true);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

//        SpAdapter spAdapter2 = new SpAdapter(this);
//        spAdapter2.setDatas(botes);
//        spBote.setAdapter(spAdapter2);
//
//        spBote.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
//            @Override
//            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
//                serialHelper.close();
//                serialHelper.setBaudRate(botes[position]);
//                btOpen.setEnabled(true);
//            }
//
//            @Override
//            public void onNothingSelected(AdapterView<?> parent) {
//
//            }
//        });

//        SpAdapter spAdapter3 = new SpAdapter(this);
//        spAdapter3.setDatas(databits);
//        spDatab.setAdapter(spAdapter3);
//
//        spDatab.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
//            @Override
//            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
//                serialHelper.close();
//                serialHelper.setDataBits(Integer.parseInt(databits[position]));
//                btOpen.setEnabled(true);
//            }
//
//            @Override
//            public void onNothingSelected(AdapterView<?> parent) {
//
//            }
//        });

//        SpAdapter spAdapter4 = new SpAdapter(this);
//        spAdapter4.setDatas(paritys);
//        spParity.setAdapter(spAdapter4);
//
//        spParity.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
//            @Override
//            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
//                serialHelper.close();
//                serialHelper.setParity(position);
//                btOpen.setEnabled(true);
//            }
//
//            @Override
//            public void onNothingSelected(AdapterView<?> parent) {
//
//            }
//        });

//        SpAdapter spAdapter5 = new SpAdapter(this);
//        spAdapter5.setDatas(stopbits);
//        spStopb.setAdapter(spAdapter5);
//
//        spStopb.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
//            @Override
//            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
//                serialHelper.close();
//                serialHelper.setStopBits(Integer.parseInt(stopbits[position]));
//                btOpen.setEnabled(true);
//            }
//
//            @Override
//            public void onNothingSelected(AdapterView<?> parent) {
//
//            }
//        });

//        SpAdapter spAdapter6 = new SpAdapter(this);
//        spAdapter6.setDatas(flowcons);
//        spFlowcon.setAdapter(spAdapter6);
//
//        spFlowcon.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
//            @Override
//            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
//                serialHelper.close();
//                serialHelper.setFlowCon(position);
//                btOpen.setEnabled(true);
//            }
//
//            @Override
//            public void onNothingSelected(AdapterView<?> parent) {
//
//            }
//        });

        btStream.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Handler().postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        doStartData();
                    }
                },2000);
            }
        });


        btOpen.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    if (fileRead) {
                        serialFileHelper.open();
                        //doStartData();
                    } else {
                        serialHelper.open();
                    }
//                    btOpen.setEnabled(false);
                } catch (IOException e) {
                    Toast.makeText(MainActivity.this, "msg: " + e.getMessage(), Toast.LENGTH_SHORT).show();
                    e.printStackTrace();
                } catch (SecurityException ex) {
                    Toast.makeText(MainActivity.this, "msg: " + ex.getMessage(), Toast.LENGTH_SHORT).show();
                }
            }
        });

        btSend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (radioGroup.getCheckedRadioButtonId() == R.id.radioButton1) {
                    if (edInput.getText().toString().length() > 0) {
                        if (serialHelper.isOpen()) {
                            serialHelper.sendTxt(edInput.getText().toString());
                        } else {
                            Toast.makeText(getBaseContext(), "串口没打开", Toast.LENGTH_SHORT).show();
                        }
                    } else {
                        Toast.makeText(getBaseContext(), "先填数据吧", Toast.LENGTH_SHORT).show();
                    }
                } else {
                    if (edInput.getText().toString().length() > 0) {
                        if (serialHelper.isOpen()) {
                            serialHelper.sendHex(edInput.getText().toString());
                        } else {
                            Toast.makeText(getBaseContext(), "串口没打开", Toast.LENGTH_SHORT).show();
                        }
                    } else {
                        Toast.makeText(getBaseContext(), "先填数据吧", Toast.LENGTH_SHORT).show();
                    }
                }
            }
        });
    }

    public static byte[] cmd_start_imu = {0x06, 0x00, (byte) 0xAA, 0x55, 0x02, 0x00, 0x01, 0x01};// 泰山
    public static final int CMD_DEVICE_HMD_UPLOAD_START = 0x0101;

    private void doStartData() {



        byte cmd[] = new byte[2];
        for (int i = 0; i < 10; ++i) {
//            cmd[0] = 2;
//            cmd[1] = (byte) 0x101;

            byte[] cmd2 = makeCmd(null, CMD_DEVICE_HMD_UPLOAD_START);
            serialHelper.send(cmd2);
            Log.d(TAG, "doStartData: " + CheckUtils.byte2hex(cmd2).toString());
            Log.d(TAG, "doStartData: " + CheckUtils.byte2hex(cmd_start_imu).toString());
            ;
            //serialHelper.send(cmd_start_imu);

//            if (fileRead) {
//                serialFileHelper.send(cmd);
//            } else {
//                serialHelper.send(cmd);
//            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_clean:
//                logListAdapter.clean(); //清空
                break;
            default:
                break;
        }
        return super.onOptionsItemSelected(item);
    }


    protected byte[] makeCmd(byte[] data, int cmd) {
        byte[] result = CmdUtils.makeCmd(data, cmd);
        result[result.length - 1] = calcuCrc8(result, result.length - 1);
        return result;
    }

    public static native byte calcuCrc8(byte[] data, int length);

    public static native void updateDatas(byte[] datas, int length);
}
