package com.nolovr.core.data.usb.utils;

import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;

public class NamedThreadFactory implements ThreadFactory {

    private final AtomicInteger counter = new AtomicInteger();
    private static final String THREAD_NAME_PATTERN = "%s-%d";
    private String namePrefix;

    public NamedThreadFactory(String namePrefix) {
        this.namePrefix = namePrefix;
    }

    @Override
    public Thread newThread(Runnable runnable) {
        final String threadName = String.format(THREAD_NAME_PATTERN, namePrefix, counter.incrementAndGet());
        return new Thread(runnable, threadName);
    }
}
