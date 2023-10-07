package tp.xmaihh.serialport.stick;

import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import tp.xmaihh.serialport.utils.CheckUtils;

/**
 * 特定字符的粘包处理,首尾各一个Byte[],不可以同时为空，如果其中一个为空，那么以不为空的作为分割标记
 * 例：协议制定为  ^+数据+$，首就是^，尾是$
 */
public class SpecifiedStickPackageHelper implements AbsStickPackageHelper {

    private static final String TAG = "SpecifiedStickPackageHe";
    private byte[] head;
    private byte[] tail;
    private List<Byte> bytes;
    private int headLen, tailLen;

    public SpecifiedStickPackageHelper(byte[] head, byte[] tail) {
        this.head = head;
        this.tail = tail;
        if (head == null || tail == null) {
            throw new IllegalStateException(" head or tail ==null");
        }
        if (head.length == 0 && tail.length == 0) {
            throw new IllegalStateException(" head and tail length==0");
        }
        headLen = head.length;
        tailLen = tail.length;
        bytes = new ArrayList<>();
    }

    private boolean endWith(Byte[] src, byte[] target) {
        if (src.length < target.length) {
            return false;
        }
        for (int i = 0; i < target.length; i++) {//逆序比较
            if (target[target.length - i - 1] != src[src.length - i - 1]) {
                return false;
            }
        }
        return true;
    }

    private byte[] getRangeBytes(List<Byte> list, int start, int end) {
//        Log.d(TAG, "getRangeBytes: ");
        Byte[] temps = Arrays.copyOfRange(list.toArray(new Byte[0]), start, end);
        byte[] result = new byte[temps.length];
        for (int i = 0; i < result.length; i++) {
            result[i] = temps[i];
        }
        return result;
    }

    @Override
    public byte[] execute(InputStream is) {
        bytes.clear();
        int len = -1;
        byte temp;
        int startIndex = -1;
        byte[] result = null;
        boolean isFindStart = false, isFindEnd = false;
        try {
            while ((len = is.read()) != -1) {
                temp = (byte) len;
//                Log.d(TAG, "execute: temp=" + temp + " | len=" + len);
                bytes.add(temp);
                Byte[] byteArray = bytes.toArray(new Byte[]{});
                if (headLen == 0 || tailLen == 0) {
//                    Log.d(TAG, "execute: 只有头或尾标记");
                    //只有头或尾标记
                    if (endWith(byteArray, head) || endWith(byteArray, tail)) {
                        if (startIndex == -1) {
                            startIndex = bytes.size() - headLen;
                        } else {//找到了
                            result = getRangeBytes(bytes, startIndex, bytes.size());
                            break;
                        }
                    }
                } else {
//                    Log.d(TAG, "execute: 1111");
                    if (!isFindStart) {
//                        Log.d(TAG, "execute: 222");
                        if (endWith(byteArray, head)) {
//                            Log.d(TAG, "execute: 3333");
                            startIndex = bytes.size() - headLen;
                            isFindStart = true;
                        }
                    } else if (!isFindEnd) {
//                        Log.d(TAG, "execute: 444");
                        if (endWith(byteArray, tail)) {
//                            Log.d(TAG, "execute: 555");
                            if (startIndex + headLen <= bytes.size() - tailLen) {
                                isFindEnd = true;
                                result = getRangeBytes(bytes, startIndex, bytes.size());
                                break;
                            }
                        }
                    }

                }
            }
            if (len == -1) {
                return null;
            }
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return result;
    }
}
