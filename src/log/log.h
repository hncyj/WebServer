/**
 * @file log.h
 * @author chenyinjie
 * @date 2024-09-02
 */

#ifndef LOG_H_
#define LOG_H_


#include "block_queue.h"
#include "../buffer/buffer.h"

#include <mutex>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <fstream>
#include <optional>
#include <functional>

class Log {
public:
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static Log& getInstance();                                  // 获取日志单例
    static void worker();                                       // 异步线程调用函数

    bool init(bool is_open, bool is_async = false, int max_queue_size = 1024);
    void writeLog(int level, const char* format, ...);          // 写入日志
    void flush();                                               // 刷新日志
    bool isOpen() const noexcept;                               // 检查日志系统是否打开
    void close();                                               // 优雅关闭日志

private:
    Log();
    ~Log();
    void asyncWriteLog();                                        // 异步写入日志
    void buildLogFile(const std::tm&);                           // 创建日志文件

    std::string log_file_path_;                                  // 日志文件存储路径            
    std::string log_file_name_;                                  // 日志文件名
    int max_lines_;                                              // 日志文件最大行数
    int cnt_lines_;                                              // 当前日志文件行数
    int today_;                                                  // 当前日志文件日期
    bool is_log_open_;                                           // 是否启用日志
    bool is_async_log;                                           // 是否异步写入日志
    int max_queue_size_;                                         // 阻塞队列大小

    std::fstream log_file_stream_;                               // 日志文件流
    Buffer buffer_;                                              // 动态缓冲区 
    std::unique_ptr<BlockDeque<std::string>> log_block_deque_;   // 日志消息阻塞队列
    std::unique_ptr<std::thread> log_async_thread_;              // 日志异步工作线程
    mutable std::mutex log_mtx_;                                 // 日志文件互斥锁
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = &Log::getInstance();\
        if (log->isOpen()) {\
            log->writeLog(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif  // LOG_H_