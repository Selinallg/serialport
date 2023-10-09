package com.ex.serialport;

import static com.nolovr.core.data.usb.config.Config.CMD_DEVICE_GET_CALIBRATION_PARAMS;

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

import com.nolovr.core.data.usb.UsbProxy;
import com.nolovr.core.data.usb.serialport.UartImpl;
import com.nolovr.core.data.usb.utils.CRC8;
import com.nolovr.core.data.usb.utils.HexDump;
import com.nolovr.core.data.usb.utils.LogUtil;

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
    private Button btStream,btCalib;
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

    UartImpl proxy;

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
        btCalib = (Button) findViewById(R.id.btn_Calib);

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


        proxy = UartImpl.getInstance(getBaseContext());
        proxy.init();
        proxy.updateDeviceType(3);


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
                    updateDatas(comBean.bRec, comBean.bRec.length);
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


        btCalib.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                _handleCalib();
            }
        });

        btStream.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Handler().postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        doStartData();
                    }
                }, 2000);
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
           // readCalibrationParams();

            byte[] cmd2 = makeCmd(null, CMD_DEVICE_HMD_UPLOAD_START);
            serialHelper.send(cmd2);
            Log.d(TAG, "doStartData: " + CheckUtils.byte2hex(cmd2).toString());
            Log.d(TAG, "doStartData: " + CheckUtils.byte2hex(cmd_start_imu).toString());


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


    //=============================


    private void _handleCalib() {
        if (true) {

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
            serialHelper.send(finalArr);
            LogUtil.e(TAG, "_handleCalib nsyncData:send  ===>" + CheckUtils.byte2hex(finalArr));


            readCalibrationParams();

            /******写标定失败的时候，可以临时更新 标定参数 *****/
//                if (!isWritedClib) {
//                    updateDatas(finalArr);
//                    LogUtil.e(TAG, "_handleCalib updateDatas:write  ===>" + CheckUtils.byte2hex(finalArr));
//                }


//                send(cmd_read_param_calib);
            Log.e(TAG, "_handleCalib: cmd_read_param_calib.");
        }
    }


    public void readCalibrationParams() {
        LogUtil.e(TAG, "readCalibrationParams: ");
        byte[] cmd = makeCmd(null, CMD_DEVICE_GET_CALIBRATION_PARAMS);
        serialHelper.send(cmd);
    }

    //=============================


}
