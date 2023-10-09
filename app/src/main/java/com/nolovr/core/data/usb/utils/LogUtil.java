package com.nolovr.core.data.usb.utils;

import androidx.annotation.NonNull;

/**
 * description:
 * Author:zhongxj
 * Email:xianjian@nolovr.com
 * Date:2022/11/18
 */
public class LogUtil {
    public static final class Tag {

        final String mValue;

        public Tag(String tag) {
            this.mValue = tag;
        }

        @NonNull
        @Override
        public String toString() {
            return mValue;
        }
    }


    public static void setDebugMode(boolean debugMode) {
        LogUtilAgent.setDebugMode(debugMode);
    }

    public static boolean isDebugMode() {
        return LogUtilAgent.isDebugMode();
    }

    /**
     * Please use new version d(String...).
     *
     * @deprecated
     */
    public static void d(Tag tag, String... msg) {
        LogUtilAgent.d(tag, msg);
    }

    /**
     * Please use new version d(String, Throwable).
     *
     * @deprecated
     */
    public static void d(Tag tag, String msg, Throwable tr) {
        LogUtilAgent.d(tag, msg, tr);
    }

    /**
     * Enabled if running in 'userdebug' variant or Log level is higher than DEBUG.
     */
    public static void d(String... message) {
        LogUtilAgent.d(message);
    }

    /**
     * Enabled if running in 'userdebug' variant or Log level is higher than DEBUG.
     */
    public static void d(String message, Throwable e) {
        LogUtilAgent.d(message, e);
    }

    /**
     * Please use new version i(String...).
     *
     * @deprecated
     */
    public static void i(Tag tag, String... msg) {
        LogUtilAgent.i(tag, msg);
    }

    /**
     * Please use new version i(String, Throwable).
     *
     * @deprecated
     */
    public static void i(Tag tag, String msg, Throwable tr) {
        LogUtilAgent.i(tag, msg, tr);
    }

    /**
     * Always enabled.
     */
    public static void i(String... message) {
        LogUtilAgent.i(message);
    }

    /**
     * Always enabled.
     */
    public static void i(String message, Throwable e) {
        LogUtilAgent.i(message, e);
    }

    /**
     * Please use new version w(String...).
     *
     * @deprecated
     */
    public static void w(Tag tag, String... message) {
        LogUtilAgent.w(tag, message);
    }

    /**
     * Please use new version w(String, Throwable).
     *
     * @deprecated
     */
    public static void w(Tag tag, String message, Throwable e) {
        LogUtilAgent.w(tag, message, e);
    }

    /**
     * Always enabled.
     */
    public static void w(String message, Throwable e) {
        LogUtilAgent.w(message, e);
    }

    /**
     * Always enabled.
     */
    public static void w(String... message) {
        LogUtilAgent.w(message);
    }


    /**
     * Please use new version e(String...).
     *
     * @deprecated
     */
    public static void e(Tag tag, String... msg) {
        LogUtilAgent.e(tag, msg);
    }

    /**
     * Please use new version e(String, Throwable).
     *
     * @deprecated
     */
    public static void e(Tag tag, String msg, Throwable tr) {
        LogUtilAgent.e(tag, msg, tr);
    }

    /**
     * Always enabled.
     */
    public static void e(String... message) {
        LogUtilAgent.e(message);
    }

    /**
     * Always enabled.
     */
    public static void e(String message, Throwable e) {
        LogUtilAgent.e(message, e);
    }

    public static void l(String message) {
        LogUtilAgent.l(message);
    }

}
