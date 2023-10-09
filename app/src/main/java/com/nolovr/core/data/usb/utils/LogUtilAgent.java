/*
 * *
 *   Created by LG
 *  项目:MountTai
 *  邮箱：liu.lg@163.com
 *  创建时间：2023年09月01日 16:40:35
 *  修改人：
 *  修改时间：
 *  修改备注：
 *  版权所有违法必究
 *  Copyright(c) 北京凌宇智控科技有限公司 https://www.nolovr.com
 *
 *
 */

package com.nolovr.core.data.usb.utils;

import static android.util.Log.isLoggable;

import android.text.TextUtils;
import android.util.Log;

import com.ex.serialport.BuildConfig;

/**
 * agent LogUtils ,防止 LogUtils 被逆向打穿
 */
public class LogUtilAgent {

    public static final String GLOBAL_TAG = "MountainTai";

    public static final boolean DEBUG;
    public static boolean DEBUG_MODE = true;

    /**
     * VERBOSE is true if running in 'userdebug' variant and
     * log level is higher than VERBOSE.
     */

    static {
        DEBUG = BuildConfig.DEBUG;
    }

    public static void setDebugMode(boolean debugMode) {
        DEBUG_MODE = debugMode;
    }

    public static boolean isDebugMode() {
        return DEBUG_MODE;
    }

    /**
     * Please use new version d(String...).
     *
     * @deprecated
     */
    public static void d(LogUtil.Tag tag, String... msg) {
        if (DEBUG || DEBUG_MODE || isLoggable(GLOBAL_TAG, Log.DEBUG)) {
            Log.d(GLOBAL_TAG, makeLogStringWithLongInfo(tag, msg));
        }
    }

    /**
     * Please use new version d(String, Throwable).
     *
     * @deprecated
     */
    public static void d(LogUtil.Tag tag, String msg, Throwable tr) {
        if (DEBUG || DEBUG_MODE || isLoggable(GLOBAL_TAG, Log.DEBUG)) {
            Log.d(GLOBAL_TAG, makeLogStringWithLongInfo(tag, msg), tr);
        }
    }

    /**
     * Enabled if running in 'userdebug' variant or Log level is higher than DEBUG.
     */
    public static void d(String... message) {
        if (DEBUG || DEBUG_MODE || isLoggable(GLOBAL_TAG, Log.DEBUG)) {
            Log.d(GLOBAL_TAG, makeLogStringWithLongInfo(message));
        }
    }

    /**
     * Enabled if running in 'userdebug' variant or Log level is higher than DEBUG.
     */
    public static void d(String message, Throwable e) {
        if (DEBUG || DEBUG_MODE || isLoggable(GLOBAL_TAG, Log.DEBUG)) {
            Log.d(GLOBAL_TAG, makeLogStringWithLongInfo(message), e);
        }
    }

    /**
     * Please use new version i(String...).
     *
     * @deprecated
     */
    public static void i(LogUtil.Tag tag, String... msg) {
        Log.i(GLOBAL_TAG, makeLogStringWithLongInfo(tag, msg));
    }

    /**
     * Please use new version i(String, Throwable).
     *
     * @deprecated
     */
    public static void i(LogUtil.Tag tag, String msg, Throwable tr) {
        Log.i(GLOBAL_TAG, makeLogStringWithLongInfo(tag, msg), tr);
    }

    /**
     * Always enabled.
     */
    public static void i(String... message) {
        Log.i(GLOBAL_TAG, makeLogStringWithLongInfo(message));
    }

    /**
     * Always enabled.
     */
    public static void i(String message, Throwable e) {
        Log.i(GLOBAL_TAG, makeLogStringWithLongInfo(message), e);
    }

    /**
     * Please use new version w(String...).
     *
     * @deprecated
     */
    public static void w(LogUtil.Tag tag, String... message) {
        Log.w(GLOBAL_TAG, makeLogStringWithLongInfo(tag, message));
    }

    /**
     * Please use new version w(String, Throwable).
     *
     * @deprecated
     */
    public static void w(LogUtil.Tag tag, String message, Throwable e) {
        Log.e(GLOBAL_TAG, makeLogStringWithLongInfo(tag, message), e);
    }

    /**
     * Always enabled.
     */
    public static void w(String message, Throwable e) {
        Log.e(GLOBAL_TAG, makeLogStringWithLongInfo(message), e);
    }

    /**
     * Always enabled.
     */
    public static void w(String... message) {
        Log.w(GLOBAL_TAG, makeLogStringWithLongInfo(message));
    }


    /**
     * Please use new version e(String...).
     *
     * @deprecated
     */
    public static void e(LogUtil.Tag tag, String... msg) {
        Log.e(GLOBAL_TAG, makeLogStringWithLongInfo(tag, msg));
    }

    /**
     * Please use new version e(String, Throwable).
     *
     * @deprecated
     */
    public static void e(LogUtil.Tag tag, String msg, Throwable tr) {
        Log.e(GLOBAL_TAG, makeLogStringWithLongInfo(tag, msg), tr);
    }

    /**
     * Always enabled.
     */
    public static void e(String... message) {
        Log.e(GLOBAL_TAG, makeLogStringWithLongInfo(message));
    }

    /**
     * Always enabled.
     */
    public static void e(String message, Throwable e) {
        Log.e(GLOBAL_TAG, makeLogStringWithLongInfo(message), e);
    }


    private static String makeLogStringWithLongInfo(LogUtil.Tag tag, String... message) {
        StackTraceElement stackTrace = Thread.currentThread().getStackTrace()[5];
        StringBuilder builder = new StringBuilder();
        if (tag != null) {
            builder.append(tag.toString());
        }
        appendTag(builder, stackTrace);
        appendTraceInfo(builder, stackTrace);
        for (String i : message) {
            builder.append(i);
        }
        return builder.toString();
    }

    private static String makeLogStringWithLongInfo(String... message) {
        StackTraceElement stackTrace = Thread.currentThread().getStackTrace()[5];
        StringBuilder builder = new StringBuilder();
        appendTag(builder, stackTrace);
        appendTraceInfo(builder, stackTrace);
        for (String i : message) {
            builder.append(i).append(" ");
        }
        return builder.toString();
    }

    private static void appendTag(StringBuilder builder, StackTraceElement stackTrace) {
        builder.append('[');
        builder.append(suppressFileExtension(stackTrace.getFileName()));
        builder.append("] ");
    }

    private static void appendTraceInfo(StringBuilder builder, StackTraceElement stackTrace) {
        builder.append(stackTrace.getMethodName());
        builder.append(" L");
        builder.append(stackTrace.getLineNumber());
        builder.append(" ");
    }

    private static String suppressFileExtension(String filename) {
        if (TextUtils.isEmpty(filename)){
            return "--null--";
        }
        int extensionPosition = filename.lastIndexOf('.');
        if (extensionPosition > 0 && extensionPosition < filename.length()) {
            return filename.substring(0, extensionPosition);
        } else {
            return filename;
        }
    }

    public static void l(String message) {
        Log.d(GLOBAL_TAG, makeLogStringWithLongInfoL(message));
    }

    private static String makeLogStringWithLongInfoL(String... message) {
        StackTraceElement stackTraceCall6 = null;
        StackTraceElement stackTraceCall5 = null;
        StackTraceElement[] stackTraceArray = Thread.currentThread().getStackTrace();

        if (stackTraceArray.length > 6) {
            stackTraceCall6 = Thread.currentThread().getStackTrace()[6];
        }
        stackTraceCall5 = Thread.currentThread().getStackTrace()[5];
        StackTraceElement stackTrace = Thread.currentThread().getStackTrace()[5];
        StringBuilder builder = new StringBuilder();
        appendTag(builder, stackTrace);
        appendTraceInfo(builder, stackTrace);
        for (String i : message) {
            builder.append(i);
        }
        appendCall(builder, stackTraceCall5);
        if (stackTraceCall6 != null) {
            appendCall2(builder, stackTraceCall6);
        }
        return builder.toString();
    }

    private static void appendCall(StringBuilder builder, StackTraceElement stackTrace) {
        builder.append("  <- ");
        String file = stackTrace.getFileName();
        String fileSub = file.substring(0, file.length() - 6);
        builder.append(fileSub);
        builder.append(":");
        builder.append(stackTrace.getMethodName());
        builder.append(":");
        builder.append(stackTrace.getLineNumber());
    }

    private static void appendCall2(StringBuilder builder, StackTraceElement stackTrace) {
        builder.append("  <- ");
        String file = stackTrace.getFileName();
        if (file != null) {
            String fileSub = file.substring(0, file.length() - 6);
            builder.append(fileSub);
            builder.append(":");
        }
        builder.append(stackTrace.getMethodName());
        builder.append(":");
        builder.append(stackTrace.getLineNumber());
    }
}
