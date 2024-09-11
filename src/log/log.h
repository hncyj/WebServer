/**
 * @file log.h
 * @author chenyinjie
 * @date 2024-09-02
 */

#ifndef LOG_H_
#define LOG_H_

#include <mutex>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <fstream>
#include <optional>
#include <functional>

#include "block_queue.h"
#include "../buffer/buffer.h"

class Log {
public:
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static Log& GetInstance();                                  // 获取日志单例
    static void Worker();                                       // 日志写入线程函数

    bool Init(bool is_open, bool is_async = false, int max_queue_size = 1024);
    void WriteLog(int level, const char* format, ...);          // 写入日志
    void Flush();                                               // 刷新日志
    bool IsOpen() const noexcept;                               // 检查日志系统是否打开

private:
    std::string log_path_;                                       // 日志文件存储路径            
    std::string log_file_name_;                                  // 日志文件名
    int max_lines_;                                              // 日志文件最大行数
    int cnt_lines_;                                              // 日志文件当前行数
    int today_;                                                  // 日志文件当前日期
    bool is_open_;                                               // 是否启用日志
    bool is_async_;                                              // 是否异步写入日志
    int max_queue_size_;                                         // 阻塞队列大小

    std::fstream log_file_stream_;                               // 日志文件流
    Buffer buffer_;                                              // 自定义缓冲区 
    std::unique_ptr<BlockDeque<std::string>> log_block_deque_;   // 日志消息阻塞队列
    mutable std::mutex log_mutex_;                               // 日志文件互斥锁

    Log();
    ~Log();
    void AsyncWriteLog();                                        // 异步写入日志
    void BuildLogFile(const std::tm&);                           // 创建日志文件
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log log = Log::GetInstance();\
        if (log->IsOpen()) {\
            log->WriteLog(level, format, ##__VA_ARGS__); \
            log->Flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif  // LOG_H_