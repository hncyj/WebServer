/**
 * @file log.cpp
 * @author chenyinjie
 * @date 2024-09-02
 */

#include "log.h"

#include <iostream>
#include <cstring>
#include <ctime>
#include <chrono>
#include <cstdarg>
#include <format>
#include <filesystem>
#include <sys/time.h>

Log::Log()
    :cnt_lines_(0), today_(0), is_log_open_(false), is_async_log(false), buffer_(4096) {}

Log::~Log() {
    close();
}

void Log::asyncWriteLog() {
    std::string log_str = "";
    while (log_block_deque_->popFront(log_str)) {
        std::lock_guard<std::mutex> lock(log_mtx_);
        if (!log_str.empty()) {
            log_file_stream_ << log_str << "\n";
            ++cnt_lines_;
        }
    }
    flush();
}

void Log::buildLogFile(const std::tm& my_tm) {
    // 首先判断是否需要创建新日志
    if (today_ != my_tm.tm_mday || cnt_lines_ % max_lines_ == 0) {
        log_file_stream_.flush();
        log_file_stream_.close();

        if (today_ != my_tm.tm_mday) {
            today_ = my_tm.tm_mday;
            cnt_lines_ = 0;
        }

        std::string new_log_file_name = std::format("{}_{:02}_{:02}_{}-{}", 
                                        my_tm.tm_year + 1900, 
                                        my_tm.tm_mon + 1, 
                                        my_tm.tm_mday, 
                                        log_file_name_, 
                                        cnt_lines_ / max_lines_ + 1
                                        );

        log_file_stream_.open(log_file_path_ + new_log_file_name, std::ios_base::out | std::ios_base::app);
    }
}

Log& Log::getInstance() {
    static Log log_instance;
    return log_instance;
}

void Log::worker() {
    getInstance().asyncWriteLog();
}

bool Log::init(bool is_open, bool is_async, int max_queue_size) {
    if (!is_open) return false;

    log_file_path_ = "./LogFiles/";
    log_file_name_ = "logfile";
    max_lines_ = 50000;
    cnt_lines_ = 0;
    today_ = 0;
    is_log_open_ = is_open;
    is_async_log = is_async;
    max_queue_size_ = max_queue_size;

    if (!std::filesystem::exists(log_file_path_)) {
        std::filesystem::create_directories(log_file_path_);
    }

    time_t t = time(nullptr);
    struct tm* sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    today_ = my_tm.tm_mday;

    // 初始化创建新日志
    std::string new_log_file_name = std::format("{}_{:02}_{:02}_{}-{}", 
                                        my_tm.tm_year + 1900, 
                                        my_tm.tm_mon + 1, 
                                        my_tm.tm_mday, 
                                        log_file_name_, 
                                        cnt_lines_ / max_lines_ + 1
                                        );

    log_file_stream_.open(log_file_path_ + new_log_file_name, std::ios_base::out | std::ios_base::app);
    if (!log_file_stream_.is_open()) {
        std::cerr << "Init Log System failed." << std::endl;
        return false;
    }

    // 是否启用异步
    if (is_async_log && max_queue_size_ > 0) {
        try {
            // 固定队列大小
            log_block_deque_ = std::make_unique<BlockDeque<std::string>>(max_queue_size_);
            log_async_thread_ = std::make_unique<std::thread>(&Log::worker);
        } catch (...) {
            std::cerr << "Failed to init async log block deque" << std::endl;
            return false;
        }
    }
    return true;
}

void Log::writeLog(int level, const char* format, ...) {
    std::string log_level_description = [level]() -> std::string {
        switch (level) {
            case 0: return "[DEBUG]:";
            case 1: return "[INFO]:";
            case 2: return "[WARN]:";
            case 3: return "[ERROR]:";
            default: return "[INFO]:";
        }
    }();


    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
    std::tm my_tm;

    // #ifdef _WIN32
    //     localtime_s(&my_tm, &time_t_now);
    // #else
    //     localtime_r(&time_t_now, &my_tm);
    // #endif

    localtime_r(&time_t_now, &my_tm);

    std::lock_guard<std::mutex> lock(log_mtx_);
    // 判断是否分文件
    buildLogFile(my_tm);

    va_list valst;
    va_start(valst, format);
    buffer_.AppendFormatted(format, valst);
    va_end(valst);

    std::string log_msg;

    try {
        log_msg = std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:06} {} {}", 
                                      my_tm.tm_year + 1900, 
                                      my_tm.tm_mon + 1, 
                                      my_tm.tm_mday,
                                      my_tm.tm_hour, 
                                      my_tm.tm_min, 
                                      my_tm.tm_sec, 
                                      micros.count(), 
                                      log_level_description, 
                                      buffer_.ReadPtr());
    } catch (const std::format_error& e) {
        std::cerr << "Format error in writelog(): " << e.what() << std::endl;
        buffer_.Clear();
        return; 
    }
    
    // 根据同步还是异步判断是存入阻塞队列还是直接写入文件
    if (is_async_log && !log_block_deque_->isFull()) {
        log_block_deque_->pushBack(log_msg);
    } else {
        if (is_async_log && log_block_deque_->isFull()) {
            std::cerr << "Log block deque is full, writing log to file directly!" << std::endl;
        }
        log_file_stream_ << log_msg << std::endl;
        ++cnt_lines_;
    }
    // 清空缓冲区
    buffer_.Clear();
}

void Log::flush() {
    std::lock_guard<std::mutex> lock(log_mtx_);
    log_file_stream_.flush();
}

bool Log::isOpen() const noexcept {
    return is_log_open_;
}

void Log::close() {
    if (is_async_log && log_async_thread_ && log_async_thread_->joinable()) {
        log_block_deque_->flush();
        log_block_deque_->close();
        log_async_thread_->join();
    }
    if (log_file_stream_.is_open()) {
        flush();
        std::lock_guard<std::mutex> locker(log_mtx_);
        log_file_stream_.close();
    }
}
