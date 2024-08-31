#include <cstring>
#include <ctime>
#include <chrono>
#include <cstdarg>
#include <sys/time.h>
#include <format>

#include "log.h"

Log::Log(): 
      m_path("./LogFiles/"),
      m_log_name("logfile"),
      m_cnt_lines(0),
      m_is_sync(false),
      m_day(0),
      m_max_lines(5000),
      m_max_buffer_size(2048),
      m_open_log(false) {};

Log::~Log() {
    if (m_log_file_stream.is_open()) {
        m_log_file_stream.close();
    }
}

void Log::async_write_log() {
    std::string log_string;
    while (m_log_queue->pop(log_string)) {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        if (!log_string.empty()) {
            m_log_file_stream << log_string << '\n';
        }
    }
}

void Log::build_log_file(const std::tm& my_tm) {
    if (m_day != my_tm.tm_mday || m_cnt_lines % m_max_lines == 0) {
        m_log_file_stream.flush();
        m_log_file_stream.close();

        if (m_day != my_tm.tm_mday) {
            m_day = my_tm.tm_mday;
            m_cnt_lines = 0;  // 当日期变化时重置行数
        }

       std::string new_log_file_name = std::format("{}_{:02}_{:02}_{}-{}", 
                                        my_tm.tm_year + 1900, 
                                        my_tm.tm_mon + 1, 
                                        my_tm.tm_mday, 
                                        m_log_name, 
                                        m_cnt_lines / m_max_lines + 1
                                        );

        m_log_file_stream.open(m_path + new_log_file_name, std::ios_base::out | std::ios_base::app);
    }
}

Log& Log::get_instance() {
    static Log instance;
    return instance;
}

void Log::write_log_thread_func() {
    get_instance().async_write_log();
}

// 初始化日志模块
bool Log::init(bool open_log, int max_lines, int log_buffer_size, int max_queue_size) {
    // 根据阻塞队列的大小设置判断是否启用异步日志写入
    if (max_queue_size >= 1) {
        m_is_sync = true;
        m_log_queue = std::make_unique<BlockQueue<std::string>>(max_queue_size);
        std::thread log_writer_thread(&Log::write_log_thread_func);
        log_writer_thread.detach();
    }
    // 设置写入缓冲区
    m_open_log = open_log;
    m_max_buffer_size = log_buffer_size;
    m_max_lines = max_lines;
    m_log_buffer_ptr = std::make_unique<char[]>(m_max_buffer_size);
    memset(m_log_buffer_ptr.get(), '\0', m_max_buffer_size);

    // 获取当前的日期和时间
    time_t t = time(nullptr);
    struct tm* sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    m_day = my_tm.tm_mday;

    // 生成日志文件名
    std::string new_log_file_name = std::format("{}_{:02}_{:02}_{}-{}", 
                                        my_tm.tm_year + 1900, 
                                        my_tm.tm_mon + 1, 
                                        my_tm.tm_mday, 
                                        m_log_name, 
                                        m_cnt_lines / m_max_lines + 1
                                        );

    // 打开日志文件
    m_log_file_stream.open(m_path + new_log_file_name, std::ios_base::out | std::ios_base::app);
    if (!m_log_file_stream.is_open()) {
        return false;
    }
    return true;
}

// 日志写入系统
void Log::write_log(int level, const char* format, ...) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
    std::tm my_tm;
    localtime_r(&time_t_now, &my_tm);

    std::string log_level_description = [level]() -> std::string {
        switch (level) {
            case 0: return "[DEBUG]:";
            case 1: return "[INFO]:";
            case 2: return "[WARN]:";
            case 3: return "[ERROR]:";
            default: return "[INFO]:";
        }
    }();

    std::lock_guard<std::mutex> lock(m_log_mutex);

    // 判断是否需要创建新文件
    build_log_file(my_tm);

    va_list valst;
    va_start(valst, format);
    std::vector<char> buffer(m_max_buffer_size);
    int n = std::vsnprintf(buffer.data(), buffer.size(), format, valst);
    va_end(valst);

    if (n >= 0 && static_cast<size_t>(n) < buffer.size()) {
        buffer[n] = '\0';  // 保证字符串结尾
    } else {
        // 处理缓冲区不足的情况
        buffer.resize(n + 1);
        va_start(valst, format);
        std::vsnprintf(buffer.data(), buffer.size(), format, valst);
        va_end(valst);
    }

    std::string log_str = std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:06} {} {}", 
                                      my_tm.tm_year + 1900, 
                                      my_tm.tm_mon + 1, 
                                      my_tm.tm_mday,
                                      my_tm.tm_hour, 
                                      my_tm.tm_min, 
                                      my_tm.tm_sec, 
                                      micros.count(), 
                                      log_level_description, 
                                      buffer.data());
    
    // 根据同步还是异步判断是存入阻塞队列还是直接写入文件
    if (m_is_sync && !m_log_queue->isFull()) {
        m_log_queue->push(log_str);
    } else {
        m_log_file_stream << log_str << '\n';
    }

    ++m_cnt_lines;
}

void Log::flush() {
    std::lock_guard<std::mutex> lock(m_log_mutex);
    m_log_file_stream.flush();
}
