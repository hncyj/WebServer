/**
 * @file log.h
 * @author chenyinjie
 * @date 2024-09-02
 */

#ifndef LOG_H
#define LOG_H


#include "block_queue.h"

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <ctime>
#include <chrono>
#include <cstdarg>
#include <memory>
#include <mutex>
#include <format>
#include <fstream>
#include <filesystem>

/**
 * @brief 
 * 单例模式设计的同步/异步日志系统
 */

class Log {
public:
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static Log& GetLogInstance();                               // 获取日志单例
    static void WriteWorker();                                  // 异步线程调用函数

    bool Init(int max_lines = 50000, bool is_async = false, int max_queue_size = 1024, int file_expire = 30);
    void WriteLog(int level, const char* format, ...);          // 写入日志
    void Flush();                                               // 刷新日志
    void Close();                                               // 关闭日志

private:
    Log();
    ~Log();
    void AsyncWriteLog();                                        // 异步写入日志
    void BuildLogFile(std::tm);                                  // 创建日志文件
    std::tm GetCurTime();                                        // 获取当前时间
    std::string GetLogPrefix();                                  // 获取日志消息时间前缀
    std::string FormatString(const char* format, va_list args);  // 格式化字符串                                   
    void CleanLogs();                                            // 过期日志清理函数


    bool is_async_log_;                                          // 是否异步写入日志
    bool is_closed_;                                             // 日志关闭标志

    std::mutex log_mtx_;                                         // 日志文件互斥锁

    std::string log_file_path_;                                  // 日志文件存储路径            
    std::string log_file_name_;                                  // 日志文件名
    
    int today_;                                                  // 记录日志系统时间，用于日志轮换
    int max_queue_size_;                                         // 阻塞队列大小
    int max_lines_;                                              // 日志文件最大行数
    int file_expire_;                                            // 日志过期时间设置
    int cnt_lines_;                                              // 当前日志行数
    
    std::fstream log_file_stream_;                               // 日志文件流
    std::unique_ptr<BlockDeque<std::string>> msg_deque_;         // 日志消息阻塞队列
    std::unique_ptr<std::thread> log_async_thread_;              // 日志异步工作线程
};

// 日志级别大于 2 时刷新文件流
#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = &Log::GetLogInstance();\
        log->WriteLog(level, format, ##__VA_ARGS__); \
        if (level >= 2) log->Flush();\
    } while(0);

#define LOG_INFO(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_DEBUG(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif  // LOG_H_