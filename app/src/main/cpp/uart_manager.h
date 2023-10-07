//
// Created by kin on 2021/3/23.
//

#ifndef NOLO_UART_MANAGER_H
#define NOLO_UART_MANAGER_H

#include <thread>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "device_util.h"

struct termios;
namespace nolo
{

/**
 * @brief
 * @date 2021/3/23
 * @author kin
  */
    class uart_manager
    {
    public:
        uart_manager();

        ~uart_manager();

        std::function<void(uint8_t* buffer, uint32_t size)> onRead = [](auto,auto){};
        std::function<void(uint8_t* buffer, uint32_t size)> onIMUFrameData = [](auto, auto){};
        std::function<void(uint8_t* buffer, uint32_t size)> onIPDFrameData = [](auto, auto){};
        std::function<void(uint8_t* buffer, uint32_t size)> onHandleFrameData = [](auto, auto){};
        std::function<void(uint8_t* buffer, uint32_t size)> onCmdFrameData = [](auto,auto){};

        bool write(uint8_t* buffer, uint32_t size);

        void write_direct(uint8_t* buffer, uint32_t size);

        static const std::vector<uint16_t>& getChirpCmdList();

        static bool isChirpCommand(uint16_t cmd);
        void open();
        void close();

        static std::string data_frame_to_string(void* buffer, uint32_t size);
        static std::string convert_data_to_string(void* buffer, uint32_t size);

    private:

        void initUartConfig(termios* cfg);

        void readThreadImpl();
        void SendDefaultCmd();
        void SendDefaultStopCmd();

        void process_package(uint8_t* buffer, int32_t size);

        void putErrorToLogFile(std::string &in_strErr);

        bool isValidDataCrc(uint8_t *buffer , uint16_t size);

        uint8_t compute_easy_crc(uint8_t *buf , uint16_t length);

        void watcher_thread();
    private:
        int fd = -1;
        std::thread readThread;
        std::thread watcherThread;
        std::unordered_map<uint8_t,uint64_t> cmdUpdateTime;
        uint32_t lastCheckCmd = 0;
        std::mutex write_mutex;
        bool isRunning = false;
        volatile bool reconnectFlag = false;
        volatile bool uartReadFlag = false;
    };

}
#endif //NOLO_UART_MANAGER_H
