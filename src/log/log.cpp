/**
 * @file log.cpp
 * @author chenyinjie
 * @date 2024-09-02
 */

#include "log.h"

Log::Log(): is_async_log_(false), 
            is_closed_(false), 
            today_(0), 
            max_queue_size_(0),
            max_lines_(50000),
            file_expire_(30),
            cnt_lines_(0) {}

Log::~Log() {
    Close();
}

Log& Log::GetLogInstance() {
    static Log log_instance;
    return log_instance;
}

void Log::WriteWorker() {
    GetLogInstance().AsyncWriteLog();
}

void Log::AsyncWriteLog() {
    std::string log_str;
    while (msg_deque_->PopFront(log_str)) {
        if (!log_str.empty()) {
            std::lock_guard<std::mutex> locker(log_mtx_);
            if (is_closed_) break;
            log_file_stream_ << log_str << std::endl;
            cnt_lines_ += 1;
            std::tm cur_tm = GetCurTime();
            if (cnt_lines_ != 0 && (cnt_lines_ % max_lines_ == 0 || cur_tm.tm_mday != today_)) {
                BuildLogFile(cur_tm);
            }
        }
    }
    Flush();
}

bool Log::Init(int max_lines, bool is_async, int max_queue_size, int file_expire) {
    log_file_path_ = std::filesystem::absolute("../logfiles").string();
    log_file_name_ = "logfile";
    max_lines_ = max_lines;
    is_async_log_ = is_async;
    max_queue_size_ = max_queue_size;
    file_expire_ = file_expire;

    if (!std::filesystem::exists(log_file_path_)) {
        std::filesystem::create_directories(log_file_path_);
    }

    // 创建日志文件
    try {
        std::unique_lock<std::mutex> locker(log_mtx_);
        BuildLogFile(GetCurTime());
    } catch (const std::exception& e) {
        std::cerr << "Exception during Log initialization: " << e.what() << std::endl;
        return false;
    }

    if (!log_file_stream_.is_open()) {
        std::cerr << "Log system Init failed: open file failed." << std::endl;
        return false;
    }

    // 是否启用异步
    if (is_async_log_ && max_queue_size_ > 0) {
        try {
            msg_deque_ = std::make_unique<BlockDeque<std::string>>(max_queue_size_);
        } catch(const std::exception& e) {
            std::cerr << "Init BlockQueue failed: %s" << e.what() << std::endl;
            return false;
        }
    }

    if (is_async_log_ && msg_deque_) {
        try {
            log_async_thread_ = std::make_unique<std::thread>(&Log::WriteWorker);
        } catch (const std::exception& e) {
            std::cerr << "Async thread create failed." << std::endl;
            return false;
        }
    }
    return true;
}

void Log::BuildLogFile(std::tm cur_tm) {
    if (cnt_lines_ % max_lines_ == 0 || today_ != cur_tm.tm_mday) {
        if (log_file_stream_.is_open()) {
            log_file_stream_.flush();
            log_file_stream_.close();
        }
        if (today_ != cur_tm.tm_mday) {
            today_ = cur_tm.tm_mday;
            cnt_lines_ = 0;
            // CleanLogs();
        }
        std::string new_log_file_name = std::format("{}_{:04}_{:02}_{:02}_{}.log", 
                                        log_file_name_,
                                        cur_tm.tm_year + 1900, 
                                        cur_tm.tm_mon + 1, 
                                        cur_tm.tm_mday,
                                        cnt_lines_ / max_lines_ + 1
                                        );
        std::filesystem::path file_path = log_file_path_;
        file_path /= new_log_file_name;
        log_file_stream_.open(file_path, std::ios::out | std::ios::app);
    }
}

std::tm Log::GetCurTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm cur_tm;
    localtime_r(&t, &cur_tm);
    return cur_tm;
}

std::string Log::GetLogPrefix() {
    std::tm cur_tm = GetCurTime();
    return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}", 
                       cur_tm.tm_year + 1900, 
                       cur_tm.tm_mon + 1, 
                       cur_tm.tm_mday, 
                       cur_tm.tm_hour, 
                       cur_tm.tm_min, 
                       cur_tm.tm_sec);
}

std::string Log::FormatString(const char* format, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (len < 0) return "";

    std::vector<char> buffer(len + 1);
    vsnprintf(buffer.data(), buffer.size(), format, args);
    return std::string(buffer.data(), len);
}

void Log::WriteLog(int level, const char* format, ...) {
    static const char* LogLevels[] = { "[INFO]:", "[DEBUG]:", "[WARN]:", "[ERROR]:" };
    const char* log_level_description = (level >= 0 && level <= 3) ? LogLevels[level] : "[INFO]:";

    std::string msg_time_prefix = GetLogPrefix();

    va_list valst;
    va_start(valst, format);
    std::string formatted_msg = FormatString(format, valst);
    va_end(valst);

    std::string log_msg = std::format("{} {} {}", msg_time_prefix, log_level_description, formatted_msg);
    
    if (is_async_log_) {
        if (!is_closed_) msg_deque_->PushBack(log_msg);
    } else {
        std::lock_guard<std::mutex> locker(log_mtx_);
        if (!is_closed_) {
            log_file_stream_ << log_msg << std::endl;
            cnt_lines_ += 1;
            std::tm cur_tm = GetCurTime();
            if (cnt_lines_ != 0 && (cnt_lines_ % max_lines_ == 0 || cur_tm.tm_mday != today_)) {
                BuildLogFile(cur_tm);
            }
        }
    }
}

void Log::CleanLogs() {
    auto now = std::chrono::system_clock::now();
    for (const auto& file : std::filesystem::directory_iterator(log_file_path_)) {
        if (file.is_regular_file()) {
            auto file_time = std::filesystem::last_write_time(file);
            auto ftime = decltype(file_time)::clock::to_sys(file_time);
            auto age = std::chrono::duration_cast<std::chrono::hours>(now - ftime).count() / 24;
            if (age > file_expire_) {
                std::filesystem::remove(file.path());
            }
        }
    }
}

void Log::Flush() {
    std::lock_guard<std::mutex> lock(log_mtx_);
    log_file_stream_.flush();
}

void Log::Close() {
    {
        std::unique_lock<std::mutex> locker(log_mtx_);
        is_closed_ = true;
    }
    if (is_async_log_ && log_async_thread_ && log_async_thread_->joinable()) {
        msg_deque_->Flush();
        msg_deque_->Close();
        log_async_thread_->join();
    }
    std::lock_guard<std::mutex> locker(log_mtx_);
    if (log_file_stream_.is_open()) {
        log_file_stream_.flush();
        log_file_stream_.close();
    }
}
