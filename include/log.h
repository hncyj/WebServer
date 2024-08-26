#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>

#include "block_queue.h"

class Log {
private:
    Log();
    ~Log();
    void async_write_log(); // 异步写入日志

    std::string m_path;                                      // 路径名
    std::string m_log_name;                                  // 日志文件名
    int m_max_lines;                                         // 日志文件最大行数
    int m_max_buffer_size;                                   // 日志写入缓冲区大小
    long long m_cnt_lines;                                   // 日志行数
    int m_day;                                               // 当前日志日期
    std::ofstream m_file_stream;                             // 日志文件写入流
    std::unique_ptr<char[]> m_log_buffer_ptr;                // 日志缓冲区指针
    std::unique_ptr<BlockQueue<std::string>> m_log_queue;    // 日志阻塞队列
    bool m_is_async;                                         // 是否启用异步日志
    std::mutex m_log_mutex;                                  // 日志锁
    bool m_close_log;                                        // 是否关闭日志系统

public:
    static Log& get_instance() {
        static Log instance;
        return instance;
    }

    static void flush_log_thread() {
        get_instance().async_write_log();
    }

    bool init(const std::string& file_name, bool close_log, int log_buffer_size = 8192, int max_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char* format, ...);
    
    void flush();
};

// 优化后的宏定义，避免重复调用 Log::get_instance()
#define LOG_DEBUG(format, ...) do { \
    auto& log_instance = Log::get_instance(); \
    if(!log_instance.m_close_log) { \
        log_instance.write_log(0, format, ##__VA_ARGS__); \
        log_instance.flush(); \
    } \
} while(0)

#define LOG_INFO(format, ...) do { \
    auto& log_instance = Log::get_instance(); \
    if(!log_instance.m_close_log) { \
        log_instance.write_log(1, format, ##__VA_ARGS__); \
        log_instance.flush(); \
    } \
} while(0)

#define LOG_WARN(format, ...) do { \
    auto& log_instance = Log::get_instance(); \
    if(!log_instance.m_close_log) { \
        log_instance.write_log(2, format, ##__VA_ARGS__); \
        log_instance.flush(); \
    } \
} while(0)

#define LOG_ERROR(format, ...) do { \
    auto& log_instance = Log::get_instance(); \
    if(!log_instance.m_close_log) { \
        log_instance.write_log(3, format, ##__VA_ARGS__); \
        log_instance.flush(); \
    } \
} while(0)

#endif // LOG_H_