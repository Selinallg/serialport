/*
 * *
 *   Created by LG
 *  项目:MountTai
 *  邮箱：liu.lg@163.com
 *  创建时间：2023年09月02日 16:40:23
 *  修改人：
 *  修改时间：
 *  修改备注：
 *  版权所有违法必究
 *  Copyright(c) 北京凌宇智控科技有限公司 https://www.nolovr.com
 *
 *
 */

package com.ex.serialport;

public class CmdUtils {

    public static byte[] makeCmd(byte[] data, int cmd) {
        int dataLength = 0;
        if (data != null && data.length > 0) {
            dataLength = data.length;
        }
        byte[] result = new byte[dataLength + 9];
        //头部两位放入长度
        int index = 0;
        result[index] = (byte) (result.length - 2);
        index += 1;
        result[index] = (byte) (((result.length - 2) >> 8) & 0xFF);
        index += 1;
        result[index] = (byte) 0xAA;
        index += 1;
        result[index] = (byte) 0x55;
        index += 1;
        result[index] = (byte) (result.length - 6);
        index += 1;
        result[index] = (byte) (((result.length - 6) >> 8) & 0xFF);

        index += 1;
        result[index] = (byte) (cmd);
        index += 1;
        result[index] = (byte) ((cmd >> 8) & 0xFF);
        index += 1;
        if (dataLength > 0) {
            System.arraycopy(data, 0, result, index, dataLength);
        }
        //result[result.length - 1] = calcuCrc8(result, result.length - 1);
        return result;
    }
}
