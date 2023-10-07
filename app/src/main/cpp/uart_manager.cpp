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
//#include <deviceLib\nolo_log.h>
#ifndef TIOCPMGET
#define TIOCPMGET 0x544D  /* PM get */
#endif
#ifndef TIOCPMPUT
#define TIOCPMPUT 0x544E  /* PM put */
#endif

#define TAG "uart_manager"

nolo::uart_manager::uart_manager()
{
    //kError("=================build version: {}====================",NOLO_BUILD_TIMESTAMP);
//    open();
    watcherThread = std::thread([this](){
        watcher_thread();
    });
}

nolo::uart_manager::~uart_manager()
{
    isRunning = false;
    if (watcherThread.joinable()){
        watcherThread.join();
    }
    close();
}

void nolo::uart_manager::open()
{
    /*
     * 打开com3串口高级日志
     * adb shell
     * echo 8 >proc/sys/kernel/printk
     * */

    close();
    //kInfo("start to open uart");

    int mcu_pwr_fd = ::open("/sys/devices/platform/vendor/vendor:qcom,fan_c/mcu_pwr", O_RDWR  );
//    Must(mcu_pwr_fd > 0, "open mcu_pwr gpio error, %d", errno);
    if(mcu_pwr_fd>0){
        const char * close_mcu = "0";
        usleep(10000);
        ::write(mcu_pwr_fd,close_mcu,strlen(close_mcu));
        usleep(10000);
//        NVR_HANDTRACK_DEBUG("mcu_pwr off called\n");
    }

    fd = ::open("/dev/ttyHS4", O_RDWR | O_NOCTTY );
//    Must(fd >= 0, "open /dev/ttyHS4 error, %d", errno);

    termios options{};
//    Must(tcgetattr(fd, &options) == 0, "tcgetattr failed: %d", errno);

    initUartConfig(&options);
//    Must(tcsetattr(fd, TCSANOW, &options) == 0,"tcsetattr failed: %d", errno);
    tcflush(fd, TCIOFLUSH);
//    NVR_HANDTRACK_DEBUG("uart_open called\n");

    uartReadFlag = true;
    readThread = std::thread([this](){
        readThreadImpl();
    });

    if(mcu_pwr_fd>0){
        const char * open_mcu = "1";
        ::write(mcu_pwr_fd,open_mcu,strlen(open_mcu));
        usleep(600000);
        ::close(mcu_pwr_fd);
//        NVR_HANDTRACK_DEBUG("mcu_pwr on called\n");
    }

    SendDefaultCmd();
//    NVR_HANDTRACK_DEBUG("mcu_trans start called\n");
}

void nolo::uart_manager::close()
{
    if(fd == -1){
        return;
    }

    //kInfo("start to close uart");

    SendDefaultStopCmd();
//    NVR_HANDTRACK_DEBUG("mcu_trans stop called\n");

    int mcu_pwr_fd = ::open("/sys/devices/platform/vendor/vendor:qcom,fan_c/mcu_pwr", O_RDWR  );
//    Must(mcu_pwr_fd > 0, "open mcu_pwr gpio error, %d", errno);
    if(mcu_pwr_fd>0){
        const char * close_mcu = "0";
        usleep(10000);
        ::write(mcu_pwr_fd,close_mcu,strlen(close_mcu));
        usleep(10000);
        ::close(mcu_pwr_fd);
//        NVR_HANDTRACK_DEBUG("mcu_pwr_off called\n");
    }
    int arg = 0;
    ::ioctl(fd, TIOCPMPUT, &arg);

    ::close(fd);
//    NVR_HANDTRACK_DEBUG("uart_close called\n");
    fd = -1;
    uartReadFlag = false;
    if(readThread.joinable()){
        readThread.join();
    }
    //kInfo("uart close over");
}

void nolo::uart_manager::initUartConfig(termios *cfg)
{

    //c_cflag
    cfg->c_cflag |= (CLOCAL | CREAD);
    cfg->c_cflag &= ~CSIZE;
    cfg->c_cflag &= ~PARENB;
    cfg->c_cflag |= CS8;
    cfg->c_cflag &= ~CSTOPB;

    //cfg->c_cflag |= CRTSCTS;//开启流控
    cfg->c_cflag &= ~CRTSCTS;//关闭流控

    //c_lflag
    cfg->c_lflag &= ~(ICANON |PENDIN| ECHO | ECHOE |ECHOK|ECHONL |IEXTEN |ECHOCTL|ECHOPRT|ECHOKE | ISIG |XCASE);

    // c_iflag
    cfg->c_iflag &= ~(ISTRIP|ICRNL |IGNBRK|BRKINT|IXON|IXOFF| IGNCR|INPCK|PARMRK|INLCR );
    cfg->c_iflag |= IGNPAR|IUCLC;

    //c_oflag
    cfg->c_oflag &= ~(OLCUC|ONLCR|OCRNL|ONOCR|ONLRET|OFILL|OFDEL);

    cfsetispeed(cfg, B460800);
    cfsetospeed(cfg, B460800);

    cfg->c_cc[VTIME] = 3;/* 非规范模式读取时的超时时间；*/
    cfg->c_cc[VMIN] = 0; /* 非规范模式读取时的最小字符数*/
}

//#include <sys/prctl.h>
void nolo::uart_manager::readThreadImpl()
{
    std::vector<uint8_t> buffer;
    buffer.resize(10240);
    int count = 0;
//    device_util::set_self_thread_priority();
//    //device_util::set_cur_thread_affinity(2);
//    device_util::set_thread_priority_self();
//    nice(-20);
//
//    prctl(PR_SET_NAME,"uartRead");

    int64_t first_abnormal_timestamp = 0;
    while(uartReadFlag && fd > 0){
      //  std::this_thread::sleep_for(std::chrono::milliseconds (4));

        auto rv = read(fd, &buffer[0], buffer.size());
        if(rv > 0){
            if (rv >= 1024)
            {
//                NVR_LOGD(TAG, "readThreadImpl abnormal");
                if (first_abnormal_timestamp != 0)
                {
                    auto now = device_util::get_timestamp_ms();
                    if (now >= (first_abnormal_timestamp + 10000)){
                        reconnectFlag = true;
                        break;
                    }
                } else
                {
                    first_abnormal_timestamp = device_util::get_timestamp_ms();
                }
            } else
            {
                first_abnormal_timestamp = 0;
            }
        }
        else{
          //  //kInfo("test_log_sys_clock: **************** rv:{}", rv);
        }

   //     continue;


        if(rv > 0){
            auto before_time = std::chrono::steady_clock::now();
            onRead(&buffer[0], rv);
            process_package(&buffer[0],rv);
            auto duration = std::chrono::steady_clock::now() - before_time;
            if(duration > std::chrono::milliseconds(4)){
                //kWarn("uart_process to long time: {}, size: {}", duration.count(), rv);
            }
        }
    }
}

bool nolo::uart_manager::write(uint8_t *buffer, uint32_t size)
{
    auto *cmd = (uint16_t*)buffer;

    //ligang 0323 关闭手柄振动临时改动
//    if(cmd[1] == 0x1001){
//        return true;
//    }

    static std::vector<uint16_t> tempBuffer;
    std::lock_guard<std::mutex> lg{write_mutex};

    if(fd == -1){
        //kWarn("fd is -1 when call write");
        return false;
    }

    if(tempBuffer.size() < size+4){
        tempBuffer.resize(size+4);
        tempBuffer[1] = 0x55aa;
    }
    tempBuffer[0] = size + 2;
    memcpy(&tempBuffer[2],cmd,size);

    auto startIndex = 0;
    size += 4;
//    if(isChirpCommand(cmd[1])){
//        startIndex = 2;
//        size -= 4;
//    }

    lastCheckCmd = cmd[1];
    auto write_result = ::write(fd, &tempBuffer[startIndex], size);

    if(cmd[1] == 0x1001 || cmd[1] == 0x1000){
        return true;
    }

    //kInfo("====>cmd write: {}", convert_data_to_string(&tempBuffer[startIndex], size));


    for(int i = 0; i < 20; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if(lastCheckCmd != cmd[1]){
            break;
        }
    }
    if(lastCheckCmd == cmd[1]){
        //kWarn("not received response type: 0x{:02x}", lastCheckCmd);
    }else if(lastCheckCmd == -1){
        return false;
        //kWarn("request failed: 0x{:02x}", cmd[1]);
    }else if(lastCheckCmd == -2){
        //kWarn("request unknown result: 0x{:02x}", cmd[1]);
    }else if(lastCheckCmd == 0){
        //success
    }else{
        //kWarn("unbelievable lastCheckCmd value: {}", lastCheckCmd);
    }

    bool result = !lastCheckCmd;
//    if(!isChirpCommand(lastCheckCmd)){
//        lastCheckCmd = 0;
//    }

    return result;
}

void nolo::uart_manager::SendDefaultCmd()
{
    using namespace std::chrono_literals;

    uint16_t cmd[2];
    for(int i = 0; i < 20; ++i)
    {
        cmd[0] = 2;
        cmd[1] = 0x101;
        if(write((uint8_t*)cmd, sizeof(cmd))){
            break;
        }
       std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


  //  std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout<<"send init command over"<<std::endl;
}

const std::vector<uint16_t> &nolo::uart_manager::getChirpCmdList()
{
    static std::vector<uint16_t> ChirpCmdList ={
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

bool nolo::uart_manager::isChirpCommand(uint16_t cmd)
{
    return  false;
    auto& ChirpCmdList = getChirpCmdList();
    return std::find(ChirpCmdList.begin(), ChirpCmdList.end(), cmd) != ChirpCmdList.end();
}

void nolo::uart_manager::process_package(uint8_t *buffer, int32_t size)
{
    if(size == 0){
        return;
    }else if(size < 4 ){
        //kWarn("error: size not enough, size: {}",size);
        return;
    }

   // //kInfo("process_package IMU_DATA: {}", uart_manager::data_frame_to_string(buffer, size));

    auto tBuffer = (uint16_t*)buffer;

    if(tBuffer[1] == 0x55aa && tBuffer[0] == tBuffer[2] + 4)
    {
        //Analyze Whether UART Actively Reports OR Commands Reply
        if(tBuffer[2] == 0x001B && tBuffer[3] == 0x26a){
            //IMU Data Frame
            if(onIMUFrameData && isValidDataCrc(buffer,tBuffer[0] + 2)){
               // //kInfo("raw_data_trace IMU_DATA: {}", uart_manager::data_frame_to_string((uint8_t*)&tBuffer[2], tBuffer[2] + 2));
                onIMUFrameData((uint8_t*)&tBuffer[2], tBuffer[2] + 2);
            }
        }
        else if(tBuffer[2] == 0x0005 && tBuffer[3] == 0x026B){
            //IPD Data Frame
            if(onIPDFrameData && isValidDataCrc(buffer,tBuffer[0] + 2)){
                onIPDFrameData((uint8_t*)&tBuffer[2], tBuffer[2] + 2);
            }

        }
        else if(tBuffer[2] == 0x0011 && tBuffer[3] == 0x026C){
            //Handle General Data Frame
            if(onHandleFrameData && isValidDataCrc(buffer,tBuffer[0] + 2)){
                onHandleFrameData((uint8_t*)&tBuffer[2], tBuffer[2] + 2);
            }

        }
        else
        {
            if(size < tBuffer[2] + 2){
                //kError("buffer size {}, cmd size {}",size,tBuffer[2]);
                std::string  strErr;
//                strErr = fmt::format("process pack err rev size = {}, cmd length = {}",size,tBuffer[2]);

                putErrorToLogFile(strErr);
                ////kError("cmd replay: {}", data_frame_to_string(buffer, size));
            }

            onCmdFrameData((uint8_t*)&tBuffer[2], tBuffer[2]+2);
            //cmd replay
            //kInfo("cmd replay: {}", data_frame_to_string(buffer, size));

            if(tBuffer[3] == 0x100f && lastCheckCmd == 0x100f){
                if(buffer[8] == 0xfe){
                    if(buffer[15] == 4){
                        lastCheckCmd = 0;
                    }else if(buffer[15] == 5){
                        lastCheckCmd = -1;
                    }else{
                        lastCheckCmd = -2;
                        //kWarn("0x100f result invalid: {}, {:02X} {:02X} {:02X} {:02X} {:02X}",buffer[15],
                         //     tBuffer[0], tBuffer[1], tBuffer[2], tBuffer[3], tBuffer[4]);
                    }
                }else if(buffer[8] == 0xff){
                    lastCheckCmd = -1;
                }else{
                    //kWarn("unknown replay!!!!!!!!: {}", lastCheckCmd);
                    lastCheckCmd = 0;
                }

            }
            else if(tBuffer[3] == lastCheckCmd){
                lastCheckCmd = 0;
            }
            else{
                //kWarn("maybe delay cmd");
            }

        }

        return process_package(buffer + tBuffer[0]+2, size - tBuffer[0]-2);
    }
    else {
        //find next package
        ////kWarn("error head: {:02X} {:02X} {:02X} {:02X}", tBuffer[0], tBuffer[1], tBuffer[2], tBuffer[3]);
        for (int i = 0; i < size - 6; ++i) {
            tBuffer = (uint16_t *) (buffer + i);
            if ((tBuffer[1] == 0x55aa && (tBuffer[0] == (tBuffer[2] + 4))) ) {
//                //kWarn("find and skip size: {}, data: {}", i, convert_data_to_string(buffer,i));
                return process_package(buffer + i, size - i);
            }
        }
        return;
    }
}

std::string nolo::uart_manager::data_frame_to_string(void *v_buffer, uint32_t size)
{
    auto tBuffer = (uint16_t*)v_buffer;
    if(size < tBuffer[0]+2){
        //kWarn("data_frame_to_string size not enough: need: {}, left:{}", tBuffer[0],size);
        return " size not enough";
    }

    return convert_data_to_string(v_buffer, tBuffer[0]+2);
}

std::string nolo::uart_manager::convert_data_to_string(void *v_buffer, uint32_t size)
{
    std::string dataValue;
    auto buffer = (uint8_t*)v_buffer;

    for(int i = 0; i < size; ++i)
    {
        dataValue += fmt::format(" {:02X}", buffer[i]);
    }
    return dataValue;
}

void nolo::uart_manager::write_direct(uint8_t *buffer, uint32_t size)
{
    static std::vector<uint16_t> tempBuffer;
    std::lock_guard<std::mutex> lg{write_mutex};

    if(fd == -1){
        //kWarn("fd is -1 when call write");
        return;
    }
    auto *cmd = (uint16_t*)buffer;
    if(tempBuffer.size() < size+4){
        tempBuffer.resize(size+4);
        tempBuffer[1] = 0x55aa;
    }
    tempBuffer[0] = size + 2;
    memcpy(&tempBuffer[2],cmd,size);

    auto startIndex = 0;
    size += 4;
    if(isChirpCommand(cmd[1])){
        startIndex = 2;
        size -= 4;
    }
    //kInfo("====>cmd write: {}", data_frame_to_string(&tempBuffer[startIndex], size));


    lastCheckCmd = cmd[1];
    ::write(fd, &tempBuffer[startIndex], size);

    for(int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if(lastCheckCmd != cmd[1]){
            break;
        }
    }
    if(lastCheckCmd == cmd[1]){
        //kWarn("not received response type: 0x{:02x}", lastCheckCmd);
    }else if(lastCheckCmd == -1){
        //kWarn("request failed: 0x{:02x}", cmd[1]);
    }else if(lastCheckCmd == -2){
        //kWarn("request unknown result: 0x{:02x}", cmd[1]);
    }else if(lastCheckCmd == 0){
        //success
    }else{
        //kWarn("unbelievable lastCheckCmd value: {}", lastCheckCmd);
    }

    bool result = !lastCheckCmd;
    if(!isChirpCommand(lastCheckCmd)){
        lastCheckCmd = 0;
    }

}
void nolo::uart_manager::putErrorToLogFile(std::string &in_strErr)
{
    std::ofstream outfile;

    outfile.open("sdcard/processPack.log", std::ios::app | std::ios::out);

    outfile << in_strErr << std::endl;

    outfile.flush();
    outfile.close();
}

bool nolo::uart_manager::isValidDataCrc(uint8_t *buffer, uint16_t size) {
    auto easy_crc = compute_easy_crc(buffer, size - 1);
    if(easy_crc != buffer[size-1]){
        auto data_string = uart_manager::convert_data_to_string(buffer, size);
        //kInfo("============= easy_crc:{} raw_crc:{} raw_data:{}",easy_crc, buffer[size-1],data_string);
        return false;
    }

    return true;
}

uint8_t nolo::uart_manager::compute_easy_crc(uint8_t *buf , uint16_t length){
    uint8_t crc8 = 0;
    while(length --){
        crc8 = crc8 ^ (*buf++);
    }
    return crc8;
}

void nolo::uart_manager::SendDefaultStopCmd()
{
    uint16_t cmd[2];
    for(int i = 0; i < 10; ++i)
    {
        cmd[0] = 2;
        cmd[1] = 0x102;
        if(write((uint8_t*)cmd, sizeof(cmd))){
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


   // std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout<<"send init command over"<<std::endl;
}

void nolo::uart_manager::watcher_thread() {
    isRunning = true;
    while (isRunning){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (reconnectFlag)
        {
            open();
            reconnectFlag = false;
        }
    }
}


