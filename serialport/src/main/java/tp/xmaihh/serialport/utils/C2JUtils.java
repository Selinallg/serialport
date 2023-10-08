/*
 * *
 *   Created by LG
 *  项目:MountTai
 *  邮箱：liu.lg@163.com
 *  创建时间：2023年09月28日 11:39:39
 *  修改人：
 *  修改时间：
 *  修改备注：
 *  版权所有违法必究
 *  Copyright(c) 北京凌宇智控科技有限公司 https://www.nolovr.com
 *
 *
 */

package tp.xmaihh.serialport.utils;

import android.util.Log;

import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class C2JUtils {
    /**
     * 16进制转字符串格式
     *
     * @param src
     * @return
     */
    public static String hexString2String(String src) {
        String temp = "";
        for (int i = 0; i < src.length() / 2; i++) {
            temp = temp + (char) Integer.valueOf(src.substring(i * 2, i * 2 + 2), 16).byteValue();
        }
        return temp;
    }

    /**
     * 字节数组转16进制字符
     *
     * @param bytes
     * @return
     */
    public static String bytesToHexString(byte[] bytes) {
        String result = "";
        for (int i = 0; i < bytes.length; i++) {
            String hexString = Integer.toHexString(bytes[i] & 0xFF);
            if (hexString.length() == 1) {
                hexString = '0' + hexString;
            }
            result += hexString.toUpperCase();
        }
        return result;
    }


    /**
     * 十六进制数转字节数组
     *
     * @param s
     * @return
     */
    public static byte[] hexStringToByteArray(String s) {
        if (s.length() % 2 != 0) {
            StringBuilder stringBuilder = new StringBuilder(s);
            stringBuilder.insert(s.length() - 1, "0");
            s = stringBuilder.toString();
        }
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4) + Character.digit(s.charAt(i + 1), 16));
        }
        return data;
    }

    /**
     * 16进制转10进制
     *
     * @param str
     * @return
     */
    public static int hexString2Decimal(String str) {

        try {
            int in = Integer.parseInt(str, 16);
            return in;
        } catch (NumberFormatException e) {
            e.printStackTrace();
            return 0;
        }
    }


    /**
     * string 转 16进制
     *
     * @param src
     * @return
     */
    public static byte[] getString2HexBytes(String src) {
        byte[] ret = new byte[src.length() / 2];
        byte[] tmp = src.getBytes();
        for (int i = 0; i < src.length() / 2; i++) {
            ret[i] = uniteBytes(tmp[i * 2], tmp[i * 2 + 1]);
        }
        return ret;
    }

    /**
     * 将两个ASCII字符合成一个字节；
     * 如："EF"--> 0xEF
     *
     * @param src0 byte
     * @param src1 byte
     * @return byte
     */
    public static byte uniteBytes(byte src0, byte src1) {
        try {
            byte b0 = Byte.decode("0x" + new String(new byte[]{src0})).byteValue();
            b0 = (byte) (b0 << 4);
            byte _b1 = Byte.decode("0x" + new String(new byte[]{src1})).byteValue();
            byte ret = (byte) (b0 ^ _b1);
            return ret;
        } catch (Exception e) {
            Log.i("Tag", e.getMessage());
        }
        return 0;
    }

    /**
     * 将字符串转换成16进制
     *
     * @param message
     * @return
     */
    public static byte[] getHexBytes(String message) {
        int len = message.length() / 2;
        char[] chars = message.toCharArray();
        String[] hexStr = new String[len];
        byte[] bytes = new byte[len];
        try {
            for (int i = 0, j = 0; j < len; i += 2, j++) {
                hexStr[j] = "" + chars[i] + chars[i + 1];
                bytes[j] = (byte) Integer.parseInt(hexStr[j], 16);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return bytes;
    }

    /**
     * 检查是否是标准格式的hex数据
     *
     * @param str
     * @return
     */
    public static boolean isRightHexStr(String str) {
        String reg = "^[0-9a-fA-F]+$";
        return str.matches(reg);
    }


    /**
     * 执行截取字符串的转换工作
     *
     * @param str 截取完成的字符串
     * @return 返回转换结果
     */
    public static String change(String str) {
        int i = Integer.parseInt(str, 16);
        // 将10进制转成小端格式
        byte[] bytes = intToMinByteArray(i);
        // 将数组转成16进制 result 要截取前四位
        String result = bytesToHexString(bytes);
        return result.substring(0, 4);
    }

    /**
     * 四字节转小端格式
     *
     * @param i
     * @return
     */
    public static byte[] intToMinByteArray(int i) {
        byte[] result = new byte[4];
        // 由高位到低位
        result[3] = (byte) ((i >> 24) & 0xFF);
        result[2] = (byte) ((i >> 16) & 0xFF);
        result[1] = (byte) ((i >> 8) & 0xFF);
        result[0] = (byte) (i & 0xFF);
        return result;
    }

    /**
     * 两个字节的小端转换
     *
     * @param i
     * @return
     */
    public static byte[] intToMin2Bytes(int i) {
        byte[] result = new byte[2];
        result[1] = (byte) ((i >> 8) & 0xFF);
        result[0] = (byte) (i & 0xFF);
        return result;
    }

    /**
     * str --- hex
     *
     * @param str
     * @return
     */
    public static String str2HexStr(String str) {
        char[] chars = "0123456789ABCDEF".toCharArray();
        StringBuilder sb = new StringBuilder("");
        byte[] bs = str.getBytes();
        int bit;

        for (int i = 0; i < bs.length; i++) {
            bit = (bs[i] & 0x0f0) >> 4;
            sb.append(chars[bit]);
            bit = bs[i] & 0x0f;
            sb.append(chars[bit]);
//	        sb.append(' ');
        }
        return sb.toString().trim();
    }

    /**
     * 将字节长度用16进制 2字节表示
     *
     * @param i
     * @return
     */
    public static String intTohexString(int i) {
        //转变高低校验值位置,低字节在前高字节在后
        String format = String.format("%04x", i);
        String hight = format.substring(0, 2);
        String low = format.substring(2);
        String crc = low + hight;
        return crc;
    }

    /**
     * 校验输入字符串中是不是纯数字
     *
     * @param str
     * @return
     */
    public static boolean isNumeric(String str) {
        Pattern pattern = Pattern.compile("[0-9]*");
        Matcher isNum = pattern.matcher(str);
        if (!isNum.matches()) {
            return false;
        }
        return true;
    }

    /**
     * 大小端转换 返回16进制字符串
     *
     * @param hex
     * @return
     */
    public static String intToMinByteArray(String hex) {
        int i = Integer.parseInt(hex, 16);
        byte[] result = new byte[4];
        // 由高位到低位
        result[3] = (byte) ((i >> 24) & 0xFF);
        result[2] = (byte) ((i >> 16) & 0xFF);
        result[1] = (byte) ((i >> 8) & 0xFF);
        result[0] = (byte) (i & 0xFF);
        String hexString = bytesToHexString(result);
        // Log.i("Tag", "intToMinByteArray: "+hexString);
        return hexString;
    }

    /**
     * 将int float类数值转换成小端格式
     *
     * @param hex    大端的16进制数
     * @param length 2 表示int 数值 4表示float 数值
     * @return
     */
    public static String hex2Float(String hex, int length) {
        byte[] result = new byte[length];
        String little = null;
        int i = 0;
        if (length == 4) {
            String s = hex.substring(0, 1);
            String st = hex.substring(6, 7);
            if (Integer.parseInt(s, 16) > 7 || Integer.parseInt(st, 16) > 7) {
                if (Integer.parseInt(s, 16) > 7) {
                    String s2 = hex.replaceFirst(s, "7");
                    // 将hex 转成 int
                    i = Integer.parseInt(s2, 16);
                } else if (Integer.parseInt(st, 16) > 7) {
                    String t1 = hex.substring(0, 6);
                    String t2 = hex.substring(6, 7);
                    String t3 = t2.replace(t2, "7");
                    String t4 = hex.substring(7, 8);
                    String t5 = t1 + t3 + t4;
                    // 将hex 转成 int
                    i = Integer.parseInt(t5, 16);
                } else if (Integer.parseInt(s, 16) > 7 && Integer.parseInt(st, 16) > 7) {
                    String s2 = hex.replace(s, "7");
                    String t1 = s2.substring(0, 6);
                    String t2 = s2.substring(6, 7);
                    String t3 = t2.replace(t2, "7");
                    String t4 = hex.substring(7, 8);
                    String t5 = t1 + t3 + t4;
                    // 将hex 转成 int
                    i = Integer.parseInt(t5, 16);
                }
            } else {
                i = Integer.parseInt(hex, 16);
            }

            // 由高位到低位
            result[3] = (byte) ((i >> 24) & 0xFF);
            result[2] = (byte) ((i >> 16) & 0xFF);
            result[1] = (byte) ((i >> 8) & 0xFF);
            result[0] = (byte) (i & 0xFF);
            little = C2JUtils.bytesToHexString(result);
            // 转成float类型
            try {
                Float value = Float.intBitsToFloat(Integer.valueOf(little.replaceAll(" ", ""), 16));
                return String.valueOf(value);
            } catch (Exception e) {
            }
        } else if (length == 2) {
            i = Integer.parseInt(hex, 16);
            result[1] = (byte) ((i >> 8) & 0xFF);
            result[0] = (byte) (i & 0xFF);
            // 转成 int 类型
            little = C2JUtils.bytesToHexString(result);
            int a = Integer.parseInt(little, 16);
            return String.valueOf(a);
        }

        return "0.00";
    }


    //-------------------------------------------

    /**
     * 将int转为高字节在前，低字节在后的byte数组（大端）
     *
     * @param n int
     * @return byte[]
     */
    public static byte[] intToByteBig(int n) {
        byte[] b = new byte[4];
        b[3] = (byte) (n & 0xff);
        b[2] = (byte) (n >> 8 & 0xff);
        b[1] = (byte) (n >> 16 & 0xff);
        b[0] = (byte) (n >> 24 & 0xff);
        return b;
    }

    /**
     * 将int转为低字节在前，高字节在后的byte数组（小端）
     *
     * @param n int
     * @return byte[]
     */
    public static byte[] intToByteLittle(int n) {
        byte[] b = new byte[4];
        b[0] = (byte) (n & 0xff);
        b[1] = (byte) (n >> 8 & 0xff);
        b[2] = (byte) (n >> 16 & 0xff);
        b[3] = (byte) (n >> 24 & 0xff);
        return b;
    }

    /**
     * byte数组到int的转换(小端)
     *
     * @param bytes
     * @return
     */
    public static int bytes2IntLittle(byte[] bytes) {
        int int1 = bytes[0] & 0xff;
        int int2 = (bytes[1] & 0xff) << 8;
        int int3 = (bytes[2] & 0xff) << 16;
        int int4 = (bytes[3] & 0xff) << 24;

        return int1 | int2 | int3 | int4;
    }

    /**
     * byte数组到int的转换(大端)
     *
     * @param bytes
     * @return
     */
    public static int bytes2IntBig(byte[] bytes) {
        int int1 = bytes[3] & 0xff;
        int int2 = (bytes[2] & 0xff) << 8;
        int int3 = (bytes[1] & 0xff) << 16;
        int int4 = (bytes[0] & 0xff) << 24;

        return int1 | int2 | int3 | int4;
    }

    /**
     * 将short转为高字节在前，低字节在后的byte数组（大端）
     *
     * @param n short
     * @return byte[]
     */
    public static byte[] shortToByteBig(short n) {
        byte[] b = new byte[2];
        b[1] = (byte) (n & 0xff);
        b[0] = (byte) (n >> 8 & 0xff);
        return b;
    }

    /**
     * 将short转为低字节在前，高字节在后的byte数组(小端)
     *
     * @param n short
     * @return byte[]
     */
    public static byte[] shortToByteLittle(short n) {
        byte[] b = new byte[2];
        b[0] = (byte) (n & 0xff);
        b[1] = (byte) (n >> 8 & 0xff);
        return b;
    }

    /**
     * 读取小端byte数组为short
     *
     * @param b
     * @return
     */
    public static short byteToShortLittle(byte[] b) {
        return (short) (((b[1] << 8) | b[0] & 0xff));
    }

    /**
     * 读取大端byte数组为short
     *
     * @param b
     * @return
     */
    public static short byteToShortBig(byte[] b) {
        return (short) (((b[0] << 8) | b[1] & 0xff));
    }

    /**
     * long类型转byte[] (大端)
     *
     * @param n
     * @return
     */
    public static byte[] longToBytesBig(long n) {
        byte[] b = new byte[8];
        b[7] = (byte) (n & 0xff);
        b[6] = (byte) (n >> 8 & 0xff);
        b[5] = (byte) (n >> 16 & 0xff);
        b[4] = (byte) (n >> 24 & 0xff);
        b[3] = (byte) (n >> 32 & 0xff);
        b[2] = (byte) (n >> 40 & 0xff);
        b[1] = (byte) (n >> 48 & 0xff);
        b[0] = (byte) (n >> 56 & 0xff);
        return b;
    }

    /**
     * long类型转byte[] (小端)
     *
     * @param n
     * @return
     */
    public static byte[] longToBytesLittle(long n) {
        byte[] b = new byte[8];
        b[0] = (byte) (n & 0xff);
        b[1] = (byte) (n >> 8 & 0xff);
        b[2] = (byte) (n >> 16 & 0xff);
        b[3] = (byte) (n >> 24 & 0xff);
        b[4] = (byte) (n >> 32 & 0xff);
        b[5] = (byte) (n >> 40 & 0xff);
        b[6] = (byte) (n >> 48 & 0xff);
        b[7] = (byte) (n >> 56 & 0xff);
        return b;
    }

    /**
     * byte[]转long类型(小端)
     *
     * @param array
     * @return
     */
    public static long bytesToLongLittle(byte[] array) {
        return ((((long) array[0] & 0xff) << 0)
                | (((long) array[1] & 0xff) << 8)
                | (((long) array[2] & 0xff) << 16)
                | (((long) array[3] & 0xff) << 24)
                | (((long) array[4] & 0xff) << 32)
                | (((long) array[5] & 0xff) << 40)
                | (((long) array[6] & 0xff) << 48)
                | (((long) array[7] & 0xff) << 56));
    }

    /**
     * byte[]转long类型(大端)
     *
     * @param array
     * @return
     */
    public static long bytesToLongBig(byte[] array) {
        return ((((long) array[0] & 0xff) << 56)
                | (((long) array[1] & 0xff) << 48)
                | (((long) array[2] & 0xff) << 40)
                | (((long) array[3] & 0xff) << 32)
                | (((long) array[4] & 0xff) << 24)
                | (((long) array[5] & 0xff) << 16)
                | (((long) array[6] & 0xff) << 8)
                | (((long) array[7] & 0xff) << 0));
    }

    //--------------------------------------------------------




}