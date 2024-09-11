/**
 * @file log.cpp
 * @author chenyinjie
 * @date 2024-09-02
 */

#include <iostream>
#include <cstring>
#include <ctime>
#include <chrono>
#include <cstdarg>
#include <sys/time.h>
#include <format>

#include "log.h"

Log::Log():cnt_lines_(0), today_(0), is_open_(false), is_async_(false), buffer_(4096) {}

Log::~Log() {
    if (is_async_ && log_async_thread_ && log_async_thread_->joinable()) {
        log_block_deque_->flush();
        log_block_deque_->close();
        log_async_thread_->join();
    }
    if (log_file_stream_.is_open()) {
        Flush();
        std::lock_guard<std::mutex> lock(log_mtx_);
        log_file_stream_.close();
    }
}

void Log::AsyncWriteLog() {
    std::string log_str = "";
    while (log_block_deque_->pop(log_str)) {
        std::lock_guard<std::mutex> lock(log_mtx_);
        if (!log_str.empty()) {
            log_file_stream_ << log_str << "\n";
            ++cnt_lines_;
        }
    }
    Flush();
}

void Log::BuildLogFile(const std::tm& my_tm) {
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

        log_file_stream_.open(log_path_ + new_log_file_name, std::ios_base::out | std::ios_base::app);
    }
}

Log* Log::GetInstance() {
    static Log log_instance;
    return &log_instance;
}

void Log::Worker() {
    GetInstance()->AsyncWriteLog();
}

bool Log::Init(bool is_open, bool is_async, int max_queue_size) {
    if (!is_open_) return false;

    log_path_ = "./LogFiles/";
    log_file_name_ = "logfile";
    max_lines_ = 50000;
    cnt_lines_ = 0;
    today_ = 0;
    is_open_ = is_open;
    is_async_ = is_async;
    max_queue_size_ = max_queue_size;

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

    log_file_stream_.open(log_path_ + new_log_file_name, std::ios_base::out | std::ios_base::app);
    if (!log_file_stream_.is_open()) {
        std::cerr << "Init Log System failed." << std::endl;
        return false;
    }

    // 是否启用异步
    if (is_async_ && max_queue_size_ > 0) {
        // 固定队列大小
        log_block_deque_ = std::make_unique<BlockDeque<std::string>>(max_queue_size_);
        log_async_thread_ = std::make_unique<std::thread>(&Log::Worker, this);
        // log_writer_thread.detach();
    } else {
        std::cerr << "Async Log Write init failed." << std::endl;
        return false;
    }

    return true;
}

void Log::WriteLog(int level, const char* format, ...) {
    std::string log_level_description = [level]() -> std::string {
        switch (level) {
            case 0: return "[DEBUG]:";
            case 1: return "[INFO]:";
            case 2: return "[WARN]:";
            case 3: return "[ERROR]:";
            default: return "[INFO]:";
        }
    }();

    std::lock_guard<std::mutex> lock(log_mtx_);

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
    std::tm my_tm;
    localtime_r(&time_t_now, &my_tm);

    // 判断是否分文件
    BuildLogFile(my_tm);

    va_list valst;
    va_start(valst, format);
    buffer_.AppendFormatted(format, valst);
    va_end(valst);

    std::string log_msg = std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:06} {} {}", 
                                      my_tm.tm_year + 1900, 
                                      my_tm.tm_mon + 1, 
                                      my_tm.tm_mday,
                                      my_tm.tm_hour, 
                                      my_tm.tm_min, 
                                      my_tm.tm_sec, 
                                      micros.count(), 
                                      log_level_description, 
                                      buffer_.ReadPtr());
    
    // 根据同步还是异步判断是存入阻塞队列还是直接写入文件
    if (is_async_ && !log_block_deque_->isFull()) {
        log_block_deque_->push_back(log_msg);
    } else {
        log_file_stream_ << log_msg << std::endl;
        ++cnt_lines_;
    }

    // 清空缓冲区
    buffer_.Clear();
}

void Log::Flush() {
    std::lock_guard<std::mutex> lock(log_mtx_);
    log_file_stream_.flush();
}

bool Log::IsOpen() const noexcept {
    std::lock_guard<std::mutex> lock(log_mtx_);
    return is_open_;
}
