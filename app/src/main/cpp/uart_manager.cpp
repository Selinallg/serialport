//
// Created by kin on 2021/3/23.
//

#include "uart_manager.h"


#include <unistd.h>
#include <fcntl.h>
//#include <KinBase/KinException.hpp>
//#include <KinBase/KinLog.hpp>
#include <vector>
#include <termios.h>
#include <iostream>
#include <fstream>
#include <fmt/format.h>
//#include <nolo_log.h>
#include "nlog.h"
#include "tai/type_taishan.h"

#ifndef TIOCPMGET
#define TIOCPMGET 0x544D  /* PM get */
#endif
#ifndef TIOCPMPUT
#define TIOCPMPUT 0x544E  /* PM put */
#endif

#define TAG "uart_manager"

nolo::uart_manager::uart_manager() {
    //kError("=================build version: {}====================",NOLO_BUILD_TIMESTAMP);
//    open();
    watcherThread = std::thread([this]() {
        watcher_thread();
    });
}

nolo::uart_manager::~uart_manager() {
    isRunning = false;
    if (watcherThread.joinable()) {
        watcherThread.join();
    }
    close();
}

void nolo::uart_manager::open() {
    /*
     * 打开com3串口高级日志
     * adb shell
     * echo 8 >proc/sys/kernel/printk
     * */

    close();

    if (isSonic) {

        //kInfo("start to open uart");

        mcu_pwr_fd = ::open("/sys/devices/platform/vendor/vendor:qcom,fan_c/mcu_pwr", O_RDWR);
//    Must(mcu_pwr_fd > 0, "open mcu_pwr gpio error, %d", errno);
        if (mcu_pwr_fd > 0) {
            const char *close_mcu = "0";
            usleep(10000);
            ::write(mcu_pwr_fd, close_mcu, strlen(close_mcu));
            usleep(10000);
//        NVR_HANDTRACK_DEBUG("mcu_pwr off called\n");
        }
        LOGE("mcu_pwr_fd %d", mcu_pwr_fd);
    }

    if (isSonic) {
        fd = ::open("/dev/ttyHS4", O_RDWR | O_NOCTTY);
        LOGD("open /dev/ttyHS4 error, %d", errno);
    } else {
        //fd = ::open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
        int flags = 0;
        fd = ::open("/dev/ttyACM0", O_RDWR | flags);
        LOGD("open /dev/ttyACM0 error, %d ==》fd=%d", errno,fd);
    }

//    Must(fd >= 0, "open /dev/ttyHS4 error, %d", errno);

    fcntl(fd, F_SETFL, 0);

    if (isSonic) {
        termios options{};
//    Must(tcgetattr(fd, &options) == 0, "tcgetattr failed: %d", errno);

        // SerialPort: /dev/ttyACM0 baudrate=115200 stopBits=1 =dataBits8 parity=0 flowCon=0 flags=0
        initUartConfig(&options);
//    Must(tcsetattr(fd, TCSANOW, &options) == 0,"tcsetattr failed: %d", errno);
        tcflush(fd, TCIOFLUSH);
//    NVR_HANDTRACK_DEBUG("uart_open called\n");
    } else{
        termios options{};
        initUartConfigTai(&options);
    }

    uartReadFlag = true;
    readThread = std::thread([this]() {
        readThreadImpl();
    });

    if (mcu_pwr_fd > 0) {
        const char *open_mcu = "1";
        ::write(mcu_pwr_fd, open_mcu, strlen(open_mcu));
        usleep(600000);
        ::close(mcu_pwr_fd);
//        NVR_HANDTRACK_DEBUG("mcu_pwr on called\n");
        LOGD("mcu_pwr on called\n");
    }

    if (isSonic) {
        SendDefaultCmd();
    }
//    NVR_HANDTRACK_DEBUG("mcu_trans start called\n");
}

void nolo::uart_manager::close() {
    LOGD("uart close start fd=%d", fd);
    if (fd == -1) {
        return;
    }

    if (isSonic) {

        //kInfo("start to close uart");

        SendDefaultStopCmd();
//    NVR_HANDTRACK_DEBUG("mcu_trans stop called\n");

        int mcu_pwr_fd = ::open("/sys/devices/platform/vendor/vendor:qcom,fan_c/mcu_pwr", O_RDWR);
//    Must(mcu_pwr_fd > 0, "open mcu_pwr gpio error, %d", errno);
        if (mcu_pwr_fd > 0) {
            const char *close_mcu = "0";
            usleep(10000);
            ::write(mcu_pwr_fd, close_mcu, strlen(close_mcu));
            usleep(10000);
            ::close(mcu_pwr_fd);
//        NVR_HANDTRACK_DEBUG("mcu_pwr_off called\n");
        }
    }
    int arg = 0;
    ::ioctl(fd, TIOCPMPUT, &arg);

    ::close(fd);
//    NVR_HANDTRACK_DEBUG("uart_close called\n");
    fd = -1;
    uartReadFlag = false;
    if (readThread.joinable()) {
        readThread.join();
    }
    LOGD("uart close over");
}



void nolo::uart_manager::open(int64_t intput_fd) {
    /*
     * 打开com3串口高级日志
     * adb shell
     * echo 8 >proc/sys/kernel/printk
     * */

//    close();

//    if (isSonic) {
//
//        //kInfo("start to open uart");
//
//        mcu_pwr_fd = ::open("/sys/devices/platform/vendor/vendor:qcom,fan_c/mcu_pwr", O_RDWR);
////    Must(mcu_pwr_fd > 0, "open mcu_pwr gpio error, %d", errno);
//        if (mcu_pwr_fd > 0) {
//            const char *close_mcu = "0";
//            usleep(10000);
//            ::write(mcu_pwr_fd, close_mcu, strlen(close_mcu));
//            usleep(10000);
////        NVR_HANDTRACK_DEBUG("mcu_pwr off called\n");
//        }
//        LOGE("mcu_pwr_fd %d", mcu_pwr_fd);
//    }

//    if (isSonic) {
//        fd = ::open("/dev/ttyHS4", O_RDWR | O_NOCTTY);
//        LOGD("open /dev/ttyHS4 error, %d", errno);
//    } else {
//        fd = ::open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
//        LOGD("open /dev/ttyACM0 error, %d ==》fd=%d", errno,fd);
//    }

//    Must(fd >= 0, "open /dev/ttyHS4 error, %d", errno);

    if (isSonic) {
        termios options{};
//    Must(tcgetattr(fd, &options) == 0, "tcgetattr failed: %d", errno);

        // SerialPort: /dev/ttyACM0 baudrate=115200 stopBits=1 =dataBits8 parity=0 flowCon=0 flags=0
        initUartConfig(&options);
//    Must(tcsetattr(fd, TCSANOW, &options) == 0,"tcsetattr failed: %d", errno);
        tcflush(fd, TCIOFLUSH);
//    NVR_HANDTRACK_DEBUG("uart_open called\n");
    }

    fd = intput_fd;
    std::string name = "uart_read";
    //pthread_setname_np(pthread_self(), name.c_str());

    uartReadFlag = true;
    readThread = std::thread([this]() {
        readThreadImpl();
    });

//    readThread = std::thread nolo([this] {
//        readThreadImpl();
//    });

    pthread_setname_np(readThread.native_handle(), name.c_str());

    if (mcu_pwr_fd > 0) {
        const char *open_mcu = "1";
        ::write(mcu_pwr_fd, open_mcu, strlen(open_mcu));
        usleep(600000);
        ::close(mcu_pwr_fd);
//        NVR_HANDTRACK_DEBUG("mcu_pwr on called\n");
        LOGD("mcu_pwr on called\n");
    }

    if (isSonic) {
        SendDefaultCmd();
    }
//    NVR_HANDTRACK_DEBUG("mcu_trans start called\n");
}

void nolo::uart_manager::close(int64_t intput_fd) {
    LOGD("uart close start fd=%d  intput_fd=%d", fd,intput_fd);
    if (fd == -1) {
        return;
    }

    if (isSonic) {

        //kInfo("start to close uart");

        SendDefaultStopCmd();
//    NVR_HANDTRACK_DEBUG("mcu_trans stop called\n");

        int mcu_pwr_fd = ::open("/sys/devices/platform/vendor/vendor:qcom,fan_c/mcu_pwr", O_RDWR);
//    Must(mcu_pwr_fd > 0, "open mcu_pwr gpio error, %d", errno);
        if (mcu_pwr_fd > 0) {
            const char *close_mcu = "0";
            usleep(10000);
            ::write(mcu_pwr_fd, close_mcu, strlen(close_mcu));
            usleep(10000);
            ::close(mcu_pwr_fd);
//        NVR_HANDTRACK_DEBUG("mcu_pwr_off called\n");
        }
    }
    int arg = 0;
    ::ioctl(fd, TIOCPMPUT, &arg);

    ::close(fd);
//    NVR_HANDTRACK_DEBUG("uart_close called\n");
    fd = -1;
    uartReadFlag = false;
    if (readThread.joinable()) {
        readThread.join();
    }
    LOGD("uart close over");
}



void nolo::uart_manager::initUartConfig(termios *cfg) {

    //c_cflag
    cfg->c_cflag |= (CLOCAL | CREAD);
    cfg->c_cflag &= ~CSIZE;
    cfg->c_cflag &= ~PARENB;
    cfg->c_cflag |= CS8;
    cfg->c_cflag &= ~CSTOPB;

    //cfg->c_cflag |= CRTSCTS;//开启流控
    cfg->c_cflag &= ~CRTSCTS;//关闭流控

    //c_lflag
    cfg->c_lflag &= ~(ICANON | PENDIN | ECHO | ECHOE | ECHOK | ECHONL | IEXTEN | ECHOCTL | ECHOPRT |
                      ECHOKE | ISIG | XCASE);

    // c_iflag
    cfg->c_iflag &= ~(ISTRIP | ICRNL | IGNBRK | BRKINT | IXON | IXOFF | IGNCR | INPCK | PARMRK |
                      INLCR);
    cfg->c_iflag |= IGNPAR | IUCLC;

    //c_oflag
    cfg->c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR | ONLRET | OFILL | OFDEL);

    cfsetispeed(cfg, B460800);
    cfsetospeed(cfg, B460800);

    cfg->c_cc[VTIME] = 3;/* 非规范模式读取时的超时时间；*/
    cfg->c_cc[VMIN] = 0; /* 非规范模式读取时的最小字符数*/

    LOGD("initUartConfig over");
}


static speed_t getBaudrate(jint baudrate) {
    switch (baudrate) {
        case 0:
            return B0;
        case 50:
            return B50;
        case 75:
            return B75;
        case 110:
            return B110;
        case 134:
            return B134;
        case 150:
            return B150;
        case 200:
            return B200;
        case 300:
            return B300;
        case 600:
            return B600;
        case 1200:
            return B1200;
        case 1800:
            return B1800;
        case 2400:
            return B2400;
        case 4800:
            return B4800;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        case 2500000:
            return B2500000;
        case 3000000:
            return B3000000;
        case 3500000:
            return B3500000;
        case 4000000:
            return B4000000;
        default:
            return -1;
    }
}

void nolo::uart_manager::initUartConfigTai(termios *cfg) {

    // baudrate=115200 stopBits=1 dataBits=8 parity=0 flowCon=0 flags=0
    int baudrate = 115200;
    speed_t speed;
    speed = getBaudrate(baudrate);


    int stopBits= 1;
    int dataBits = 8;
    int parity = 0;
    int flowCon = 0;
    int flags = 0;

    {
        struct termios cfg;
        LOGD("Configuring serial port");
        if (tcgetattr(fd, &cfg)) {
            LOGE("tcgetattr() failed");
            close(fd);
            /* TODO: throw an exception */
            return ;
        }

        cfmakeraw(&cfg);
        cfsetispeed(&cfg, speed);
        cfsetospeed(&cfg, speed);

        cfg.c_cflag &= ~CSIZE;
        switch (dataBits) {
            case 5:
                cfg.c_cflag |= CS5;    //使用5位数据位
                break;
            case 6:
                cfg.c_cflag |= CS6;    //使用6位数据位
                break;
            case 7:
                cfg.c_cflag |= CS7;    //使用7位数据位
                break;
            case 8:
                cfg.c_cflag |= CS8;    //使用8位数据位
                break;
            default:
                cfg.c_cflag |= CS8;
                break;
        }

        switch (parity) {
            case 0:
                cfg.c_cflag &= ~PARENB;    //无奇偶校验
                break;
            case 1:
                cfg.c_cflag |= (PARODD | PARENB);   //奇校验
                break;
            case 2:
                cfg.c_iflag &= ~(IGNPAR | PARMRK); // 偶校验
                cfg.c_iflag |= INPCK;
                cfg.c_cflag |= PARENB;
                cfg.c_cflag &= ~PARODD;
                break;
            default:
                cfg.c_cflag &= ~PARENB;
                break;
        }

        switch (stopBits) {
            case 1:
                cfg.c_cflag &= ~CSTOPB;    //1位停止位
                break;
            case 2:
                cfg.c_cflag |= CSTOPB;    //2位停止位
                break;
            default:
                break;
        }

        // hardware flow control
        switch (flowCon) {
            case 0:
                cfg.c_cflag &= ~CRTSCTS;    //不使用流控
                break;
            case 1:
                cfg.c_cflag |= CRTSCTS;    //硬件流控
                break;
            case 2:
                cfg.c_cflag |= IXON | IXOFF | IXANY;    //软件流控
                break;
            default:
                cfg.c_cflag &= ~CRTSCTS;
                break;
        }


        if (tcsetattr(fd, TCSANOW, &cfg)) {
            LOGE("tcsetattr() failed");
            close(fd);
            /* TODO: throw an exception */
            return ;
        }

        LOGD("Configuring serial port over");
    }

    LOGD("initUartConfig Configuring serial port over baudrate=%d stopBits=%d dataBits=%d parity=%d flowCon=%d flags=%d "
    ,baudrate,stopBits,dataBits,parity,flowCon,flags);
}

//#include <sys/prctl.h>
int64_t last_timestamp = 0;
int64_t count = 0;


int64_t last_timestamp_read = 0;
int64_t count_read = 0;

void nolo::uart_manager::readThreadImpl() {
    std::vector<uint8_t> buffer;
    //buffer.resize(10240);
    buffer.resize(256);
    int count = 0;
//    device_util::set_self_thread_priority();
//    //device_util::set_cur_thread_affinity(2)
//    device_util::set_thread_priority_self();
//    nice(-20);
//
//    prctl(PR_SET_NAME,"uartRead");

    int64_t first_abnormal_timestamp = 0;
    while (uartReadFlag && fd > 0) {
        //  std::this_thread::sleep_for(std::chrono::milliseconds (4));

        auto rv = read(fd, &buffer[0], buffer.size());
        if (rv > 32) {
            LOGD("read IMU_DATA:%d", rv);
        } else if (rv <= 0) {
            LOGE("=====WWW read IMU_DATA:%d", rv);
        }
        if (rv > 0) {
            count_read++;
            auto current_timestamp = device_util::get_timestamp_ms();
            if (current_timestamp - last_timestamp_read > 1000) {
                LOGD("read IMU_DATA: Hz=%d", count_read);
                count_read = 0;
                last_timestamp_read = current_timestamp;
            }
            if (rv >= 1024) {
//                NVR_LOGD(TAG, "readThreadImpl abnormal");
                if (first_abnormal_timestamp != 0) {
                    auto now = device_util::get_timestamp_ms();
                    if (now >= (first_abnormal_timestamp + 10000)) {
                        reconnectFlag = true;
                        break;
                    }
                } else {
                    first_abnormal_timestamp = device_util::get_timestamp_ms();
                }
            } else {
                first_abnormal_timestamp = 0;
            }
        } else {
            //  //kInfo("test_log_sys_clock: **************** rv:{}", rv);
            LOGD("read IMU_DATA:%d  ==》 do nothing  ", rv);
        }

        //     continue;


        if (rv > 0) {
            auto before_time = std::chrono::steady_clock::now();
            onRead(&buffer[0], rv);
           // process_package(&buffer[0], rv);
            process_package_tai(&buffer[0], rv);
            auto duration = std::chrono::steady_clock::now() - before_time;
            if (duration > std::chrono::milliseconds(4)) {
                //kWarn("uart_process to long time: {}, size: {}", duration.count(), rv);
            }
        }
    }
}

bool nolo::uart_manager::write(uint8_t *buffer, uint32_t size) {
    auto *cmd = (uint16_t *) buffer;

    //ligang 0323 关闭手柄振动临时改动
//    if(cmd[1] == 0x1001){
//        return true;
//    }

    static std::vector<uint16_t> tempBuffer;
    std::lock_guard<std::mutex> lg{write_mutex};

    if (fd == -1) {
        //kWarn("fd is -1 when call write");
        LOGD("fd is -1 when call write");
        return false;
    }
    LOGD("write1");
    // 0700AA5503000101FB
    if (tempBuffer.size() < size + 4) {
        tempBuffer.resize(size + 4);
        tempBuffer[1] = 0x55aa;
    }
    tempBuffer[0] = size + 2;
    memcpy(&tempBuffer[2], cmd, size);
    LOGD("write2");

    auto startIndex = 0;
    size += 4;
//    if(isChirpCommand(cmd[1])){
//        startIndex = 2;
//        size -= 4;
//    }
    lastCheckCmd = cmd[1];
//    auto write_result = ::write(fd, &tempBuffer[startIndex], size);
    auto write_result = ::write(fd, buffer, size);
    LOGD("write3 write_result=%d %s" ,write_result,buffer);
    //LOGD("raw_data_trace IMU_DATA: %s", uart_manager::data_frame_to_string(buffer, tBuffer[2] + 6).c_str());
//    LOGD("write3 write_result: %s", uart_manager::data_frame_to_string(&tempBuffer, size).c_str());
    if (cmd[1] == 0x1001 || cmd[1] == 0x1000) {
        return true;
    }
    LOGD("write4");

    //kInfo("====>cmd write: {}", convert_data_to_string(&tempBuffer[startIndex], size));


    for (int i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (lastCheckCmd != cmd[1]) {
            break;
        }
    }
    if (lastCheckCmd == cmd[1]) {
        //kWarn("not received response type: 0x{:02x}", lastCheckCmd);
    } else if (lastCheckCmd == -1) {
        return false;
        //kWarn("request failed: 0x{:02x}", cmd[1]);
    } else if (lastCheckCmd == -2) {
        //kWarn("request unknown result: 0x{:02x}", cmd[1]);
    } else if (lastCheckCmd == 0) {
        //success
    } else {
        //kWarn("unbelievable lastCheckCmd value: {}", lastCheckCmd);
    }

    bool result = !lastCheckCmd;
//    if(!isChirpCommand(lastCheckCmd)){
//        lastCheckCmd = 0;
//    }
    LOGD("result=%d",result);
    return result;
}

void nolo::uart_manager::SendDefaultCmd() {
    using namespace std::chrono_literals;

    uint16_t cmd[2];
    for (int i = 0; i < 20; ++i) {
        cmd[0] = 2;
        cmd[1] = 0x101;
        if (write((uint8_t *) cmd, sizeof(cmd))) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    //  std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "send init command over" << std::endl;
    LOGD("send init command over");
}

const std::vector<uint16_t> &nolo::uart_manager::getChirpCmdList() {
    static std::vector<uint16_t> ChirpCmdList = {
            0x203,
            0x209,
            0x120,
            0x121,
            0x122,
            0x101,
            0x102
    };
    return ChirpCmdList;
}

bool nolo::uart_manager::isChirpCommand(uint16_t cmd) {
    return false;
    auto &ChirpCmdList = getChirpCmdList();
    return std::find(ChirpCmdList.begin(), ChirpCmdList.end(), cmd) != ChirpCmdList.end();
}

int64_t last_timestamp_vsync = 0;
int64_t count_vsync = 0;

void nolo::uart_manager::process_package_tai(uint8_t *buffer, int32_t size) {
    if (size == 0) {
        return;
    } else if (size < 4) {
        //kWarn("error: size not enough, size: {}",size);
        return;
    }

//    kInfo("process_package IMU_DATA: {}", uart_manager::data_frame_to_string(buffer, size));
//    LOGD("process_package IMU_DATA:%s size=%d", uart_manager::data_frame_to_string(buffer, size).c_str(),size);

    auto tBuffer = (uint16_t *) buffer;

    if (tBuffer[1] == 0x55aa && tBuffer[0] == tBuffer[2] + 4) {


        //Analyze Whether UART Actively Reports OR Commands Reply
        if (tBuffer[2] == 0x001A && tBuffer[3] == 0x26a) {
            //IMU Data Frame
            // 1E00 AA55 1A00 6A02 4E2E3E0400000000F9FF04000900EC07FCFE73014C08DCC1
            if (onIMUFrameData && isValidDataCrc(buffer, tBuffer[0] + 2)) {
                count++;
                auto current_timestamp = device_util::get_timestamp_ms();
                if (current_timestamp - last_timestamp > 1000) {
                    LOGD("dispatch IMU_DATA: Hz=%d", count);
                    count = 0;
                    last_timestamp = current_timestamp;
                }
//                LOGD("raw_data_trace IMU_DATA: %s", uart_manager::data_frame_to_string((uint8_t*)&tBuffer[2], tBuffer[2] + 2).c_str());
//                LOGD("raw_data_trace IMU_DATA: %d", tBuffer[2] + 2);

//                LOGD("raw_data_trace IMU_DATA: %s", uart_manager::data_frame_to_string(buffer, tBuffer[2] + 6).c_str());
                //onIMUFrameData((uint8_t *) &tBuffer[2], tBuffer[2] + 2);
                //onIMUFrameData(buffer, tBuffer[2] + 6);

                // 解析数据
                // nolo::doParse(buffer,tBuffer[2] + 6);
                nolo::onUsbData(buffer, tBuffer[2] + 6);
            }
        } else if (tBuffer[2] == 0x0010 && tBuffer[3] == 0x0f01) {
            // 1400 AA55 1000 010F BE2C3E040000000005030000005B
//             LOGD("raw_data_trace 1 VSYNC_DATA: %s", uart_manager::data_frame_to_string((uint8_t*)&tBuffer[2], tBuffer[2] + 2).c_str());
            //vsync Data Frame
            if (onIPDFrameData && isValidDataCrc(buffer, tBuffer[0] + 2)) {
//                onIPDFrameData((uint8_t *) &tBuffer[2], tBuffer[2] + 2);
                count_vsync++;
                auto current_timestamp = device_util::get_timestamp_ms();
                if (current_timestamp - last_timestamp_vsync > 1000) {
                    LOGD("dispatch vsync_DATA: Hz=%d", count_vsync);
                    count_vsync = 0;
                    last_timestamp_vsync = current_timestamp;
                }

                nolo::onUsbData(buffer, tBuffer[2] + 6);
                //LOGD("raw_data_trace VSYNC_DATA: %s", uart_manager::data_frame_to_string((uint8_t*)&tBuffer[2], tBuffer[2] + 2).c_str());
//                LOGD("raw_data_trace VSYNC_DATA: %s", uart_manager::data_frame_to_string(buffer, tBuffer[2] + 6).c_str());
            }

        } else if (tBuffer[2] == 0x0011 && tBuffer[3] == 0x026C) {
            //Handle General Data Frame
            if (onHandleFrameData && isValidDataCrc(buffer, tBuffer[0] + 2)) {
//                onHandleFrameData((uint8_t *) &tBuffer[2], tBuffer[2] + 2);
            }

        } else {
            if (size < tBuffer[2] + 2) {
                //kError("buffer size {}, cmd size {}",size,tBuffer[2]);
                std::string strErr;
//                strErr = fmt::format("process pack err rev size = {}, cmd length = {}",size,tBuffer[2]);

                putErrorToLogFile(strErr);
                ////kError("cmd replay: {}", data_frame_to_string(buffer, size));
            }

            onCmdFrameData((uint8_t *) &tBuffer[2], tBuffer[2] + 2);
            //cmd replay
            //kInfo("cmd replay: {}", data_frame_to_string(buffer, size));

            if (tBuffer[3] == 0x100f && lastCheckCmd == 0x100f) {
                if (buffer[8] == 0xfe) {
                    if (buffer[15] == 4) {
                        lastCheckCmd = 0;
                    } else if (buffer[15] == 5) {
                        lastCheckCmd = -1;
                    } else {
                        lastCheckCmd = -2;
                        //kWarn("0x100f result invalid: {}, {:02X} {:02X} {:02X} {:02X} {:02X}",buffer[15],
                        //     tBuffer[0], tBuffer[1], tBuffer[2], tBuffer[3], tBuffer[4]);
                    }
                } else if (buffer[8] == 0xff) {
                    lastCheckCmd = -1;
                } else {
                    //kWarn("unknown replay!!!!!!!!: {}", lastCheckCmd);
                    lastCheckCmd = 0;
                }

            } else if (tBuffer[3] == lastCheckCmd) {
                lastCheckCmd = 0;
            } else {
                //kWarn("maybe delay cmd");
            }

        }

        return process_package_tai(buffer + tBuffer[0] + 2, size - tBuffer[0] - 2);
    } else {
        //find next package
        ////kWarn("error head: {:02X} {:02X} {:02X} {:02X}", tBuffer[0], tBuffer[1], tBuffer[2], tBuffer[3]);
        for (int i = 0; i < size - 6; ++i) {
            tBuffer = (uint16_t *) (buffer + i);
            if ((tBuffer[1] == 0x55aa && (tBuffer[0] == (tBuffer[2] + 4)))) {
//                //kWarn("find and skip size: {}, data: {}", i, convert_data_to_string(buffer,i));
                return process_package_tai(buffer + i, size - i);
            }
        }
        return;
    }
}

void nolo::uart_manager::process_package(uint8_t *buffer, int32_t size) {
    if (size == 0) {
        return;
    } else if (size < 4) {
        //kWarn("error: size not enough, size: {}",size);
        return;
    }

//    kInfo("process_package IMU_DATA: {}", uart_manager::data_frame_to_string(buffer, size));
    LOGD("process_package IMU_DATA:%s size=%d",
         uart_manager::data_frame_to_string(buffer, size).c_str(), size);
    auto tBuffer = (uint16_t *) buffer;

    if (tBuffer[1] == 0x55aa && tBuffer[0] == tBuffer[2] + 4) {
        //Analyze Whether UART Actively Reports OR Commands Reply
        if (tBuffer[2] == 0x001B && tBuffer[3] == 0x26a) {
            //IMU Data Frame
            if (onIMUFrameData && isValidDataCrc(buffer, tBuffer[0] + 2)) {
                // //kInfo("raw_data_trace IMU_DATA: {}", uart_manager::data_frame_to_string((uint8_t*)&tBuffer[2], tBuffer[2] + 2));
                onIMUFrameData((uint8_t *) &tBuffer[2], tBuffer[2] + 2);
            }
        } else if (tBuffer[2] == 0x0005 && tBuffer[3] == 0x026B) {
            //IPD Data Frame
            if (onIPDFrameData && isValidDataCrc(buffer, tBuffer[0] + 2)) {
                onIPDFrameData((uint8_t *) &tBuffer[2], tBuffer[2] + 2);
            }

        } else if (tBuffer[2] == 0x0011 && tBuffer[3] == 0x026C) {
            //Handle General Data Frame
            if (onHandleFrameData && isValidDataCrc(buffer, tBuffer[0] + 2)) {
                onHandleFrameData((uint8_t *) &tBuffer[2], tBuffer[2] + 2);
            }

        } else {
            if (size < tBuffer[2] + 2) {
                //kError("buffer size {}, cmd size {}",size,tBuffer[2]);
                std::string strErr;
//                strErr = fmt::format("process pack err rev size = {}, cmd length = {}",size,tBuffer[2]);

                putErrorToLogFile(strErr);
                ////kError("cmd replay: {}", data_frame_to_string(buffer, size));
            }

            onCmdFrameData((uint8_t *) &tBuffer[2], tBuffer[2] + 2);
            //cmd replay
            //kInfo("cmd replay: {}", data_frame_to_string(buffer, size));

            if (tBuffer[3] == 0x100f && lastCheckCmd == 0x100f) {
                if (buffer[8] == 0xfe) {
                    if (buffer[15] == 4) {
                        lastCheckCmd = 0;
                    } else if (buffer[15] == 5) {
                        lastCheckCmd = -1;
                    } else {
                        lastCheckCmd = -2;
                        //kWarn("0x100f result invalid: {}, {:02X} {:02X} {:02X} {:02X} {:02X}",buffer[15],
                        //     tBuffer[0], tBuffer[1], tBuffer[2], tBuffer[3], tBuffer[4]);
                    }
                } else if (buffer[8] == 0xff) {
                    lastCheckCmd = -1;
                } else {
                    //kWarn("unknown replay!!!!!!!!: {}", lastCheckCmd);
                    lastCheckCmd = 0;
                }

            } else if (tBuffer[3] == lastCheckCmd) {
                lastCheckCmd = 0;
            } else {
                //kWarn("maybe delay cmd");
            }

        }

        return process_package(buffer + tBuffer[0] + 2, size - tBuffer[0] - 2);
    } else {
        //find next package
        ////kWarn("error head: {:02X} {:02X} {:02X} {:02X}", tBuffer[0], tBuffer[1], tBuffer[2], tBuffer[3]);
        for (int i = 0; i < size - 6; ++i) {
            tBuffer = (uint16_t *) (buffer + i);
            if ((tBuffer[1] == 0x55aa && (tBuffer[0] == (tBuffer[2] + 4)))) {
//                //kWarn("find and skip size: {}, data: {}", i, convert_data_to_string(buffer,i));
                return process_package(buffer + i, size - i);
            }
        }
        return;
    }
}

std::string nolo::uart_manager::data_frame_to_string(void *v_buffer, uint32_t size) {
    auto tBuffer = (uint16_t *) v_buffer;
    if (size < tBuffer[0] + 2) {
        //kWarn("data_frame_to_string size not enough: need: {}, left:{}", tBuffer[0],size);
        return " size not enough";
    }

    return convert_data_to_string(v_buffer, tBuffer[0] + 2);
}

std::string nolo::uart_manager::convert_data_to_string(void *v_buffer, uint32_t size) {
    std::string dataValue;
    auto buffer = (uint8_t *) v_buffer;

    for (int i = 0; i < size; ++i) {
        dataValue += fmt::format(" {:02X}", buffer[i]);
    }
    return dataValue;
}

void nolo::uart_manager::write_direct(uint8_t *buffer, uint32_t size) {
    static std::vector<uint16_t> tempBuffer;
    std::lock_guard<std::mutex> lg{write_mutex};

    if (fd == -1) {
        //kWarn("fd is -1 when call write");
        return;
    }
    auto *cmd = (uint16_t *) buffer;

    if (tempBuffer.size() < size + 4) {
        tempBuffer.resize(size + 4);
        tempBuffer[1] = 0x55aa;
    }
    tempBuffer[0] = size + 2;
    memcpy(&tempBuffer[2], cmd, size);

    auto startIndex = 0;
    size += 4;
    if (isChirpCommand(cmd[1])) {
        startIndex = 2;
        size -= 4;
    }
    //kInfo("====>cmd write: {}", data_frame_to_string(&tempBuffer[startIndex], size));

    lastCheckCmd = cmd[1];
    ::write(fd, &tempBuffer[startIndex], size);

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if (lastCheckCmd != cmd[1]) {
            break;
        }
    }
    if (lastCheckCmd == cmd[1]) {
        //kWarn("not received response type: 0x{:02x}", lastCheckCmd);
    } else if (lastCheckCmd == -1) {
        //kWarn("request failed: 0x{:02x}", cmd[1]);
    } else if (lastCheckCmd == -2) {
        //kWarn("request unknown result: 0x{:02x}", cmd[1]);
    } else if (lastCheckCmd == 0) {
        //success
    } else {
        //kWarn("unbelievable lastCheckCmd value: {}", lastCheckCmd);
    }

    bool result = !lastCheckCmd;
    if (!isChirpCommand(lastCheckCmd)) {
        lastCheckCmd = 0;
    }

}

void nolo::uart_manager::putErrorToLogFile(std::string &in_strErr) {
    std::ofstream outfile;

    outfile.open("sdcard/processPack.log", std::ios::app | std::ios::out);

    outfile << in_strErr << std::endl;

    outfile.flush();
    outfile.close();
}

bool nolo::uart_manager::isValidDataCrc(uint8_t *buffer, uint16_t size) {
    auto easy_crc = compute_easy_crc(buffer, size - 1);
    if (easy_crc != buffer[size - 1]) {
        auto data_string = uart_manager::convert_data_to_string(buffer, size);
        //kInfo("============= easy_crc:{} raw_crc:{} raw_data:{}",easy_crc, buffer[size-1],data_string);
        return false;
    }

    return true;
}

uint8_t nolo::uart_manager::compute_easy_crc(uint8_t *buf, uint16_t length) {
    uint8_t crc8 = 0;
    while (length--) {
        crc8 = crc8 ^ (*buf++);
    }
    return crc8;
}

void nolo::uart_manager::SendDefaultStopCmd() {
    uint16_t cmd[2];
    for (int i = 0; i < 10; ++i) {
        cmd[0] = 2;
        cmd[1] = 0x102;
        if (write((uint8_t *) cmd, sizeof(cmd))) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    // std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "send init command over" << std::endl;
}

void nolo::uart_manager::watcher_thread() {
    isRunning = true;
    while (isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (reconnectFlag) {
            open();
            reconnectFlag = false;
        }
    }
}


