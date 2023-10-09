/*
 * *
 *   Created by LG
 *  项目:MountTai
 *  邮箱：liu.lg@163.com
 *  创建时间：2023年09月02日 16:57:18
 *  修改人：
 *  修改时间：
 *  修改备注：
 *  版权所有违法必究
 *  Copyright(c) 北京凌宇智控科技有限公司 https://www.nolovr.com
 *
 *
 */

package com.nolovr.core.data.usb.config;

public class Config {

    public static final String PRODUCT_ID = "0x0302";
    public static final String VENDOR_ID = "0x0483";

    public static final int PRODUCT_ID_KUAFU = 21007;
    public static final int VENDOR_ID_KUAFU = 6421;

    public static final int PRODUCT_ID_TAI_NEW = 16725;
    public static final int VENDOR_ID_TAI_NEW = 19530;

    public static final int CMD_DEVICE_HMD_UPLOAD_START = 0x0101;
    public static final int CMD_DEVICE_HMD_UPLOAD_END = 0x0102;
    public static final int CMD_DEVICE_ENTER_CALIBRATION_MODE = 0x0202;
    public static final int CMD_DEVICE_SET_CALIBRATION_PARAMS = 0x0129;
    public static final int CMD_DEVICE_GET_CALIBRATION_PARAMS = 0x012A;
    public static final int CMD_DEVICE_SET_BRIGHTNESS = 0x012B;
    public static final int CMD_DEVICE_SET_LOW_POWER = 0x012C;
    public static final int CMD_DEVICE_ENTER_OTA_MODE = 0x012D;
    public static final int CMD_DEVICE_SET_SCREEN_MODE = 0x012E;
    public static final int CMD_DEVICE_GET_VERSION = 0x012F;
    public static final int CMD_DEVICE_SET_SN = 0x0132;
    public static final int CMD_DEVICE_UPLOAD_7911 = 0x0133;


    public static final int CMD_DEVICE_SET_SCREEN_OFF_INTERVAL = 0x0135;

    public static byte[] cmd_start_imu = {0x06, 0x00, (byte) 0xAA, 0x55, 0x02, 0x00, 0x01, 0x01};// 泰山
    public static byte[] cmd_read_param_calib = {0x07, 0x00, (byte) 0xAA, 0x55, 0x03, 0x00, 0x29, 0x01, 0x01};//


}
