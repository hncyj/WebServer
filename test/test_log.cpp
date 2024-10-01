/**
 * @file test_log.cpp
 * @author chenyinjie
 * @date 2024-09-29
 */

#include "../src/log/log.h"

#include <gtest/gtest.h>

/**
 * @brief 
 * 方便起见，下面每一个单元均单独测试
 * 这样可以避免每次初始化后都需要考虑前一个测试生成的日志文件造成的影响
 * 测试前确保先前创建的日志不会与当前日志同名
 */

// 同步日志初始化测试
// TEST(LogTest, InitLogSystem) {
//     Log& logger = Log::GetLogInstance();
//     EXPECT_TRUE(logger.Init(100, false, 128, 30));
//     logger.Close();
// }

// // 异步日志初始化测试
// TEST(LogTest, InitLogSystem) {
//     Log& logger = Log::GetLogInstance();
//     EXPECT_TRUE(logger.Init(100, false, 128, 30));
//     logger.Close();
// }

// 测试同步日志文件创建
// TEST(LogTest, LogFileCreation) {
//     Log& logger = Log::GetLogInstance();
//     logger.Init(100, false, 128, 30);
//     std::string log_file_path = "/home/chenyinjie/github/WebServer/build/logfiles/logfile_2024_09_30_1.log";
//     std::fstream log_file(log_file_path);
//     EXPECT_TRUE(log_file.is_open()) << "日志文件没有成功创建";
//     // 关闭日志系统
//     logger.Close();
// }

// 测试同步日志写入
// TEST(LogTest, SynchronousLogWriting) {
//     Log& logger = Log::GetLogInstance();
//     logger.Init(100, false, 128, 30);
//     LOG_INFO("This is an info log");
//     std::string log_file_path = "/home/chenyinjie/github/WebServer/build/logfiles/logfile_2024_09_30_1.log";
//     std::ifstream log_file(log_file_path);
//     ASSERT_TRUE(log_file.is_open());
//     std::string line;
//     bool found_log = false;
//     while (std::getline(log_file, line)) {
//         if (line.find("This is an info log") != std::string::npos) {
//             found_log = true;
//             break;
//         }
//     }
//     EXPECT_TRUE(found_log) << "日志内容没有正确写入文件";
//     logger.Close();
// }

// // 测试异步日志写入
// TEST(LogTest, AsynchronousLogWriting) {
//     Log& logger = Log::GetLogInstance();
//     logger.Init(100, true, 1024, 30);
//     LOG_INFO("This is an async info log");
//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     std::string log_file_path = "/home/chenyinjie/github/WebServer/build/test/logfiles/logfile_2024_09_30_1.log";
//     std::ifstream log_file(log_file_path);
//     ASSERT_TRUE(log_file.is_open());

//     std::string line;
//     bool found_log = false;
//     while (std::getline(log_file, line)) {
//         if (line.find("This is an async info log") != std::string::npos) {
//             found_log = true;
//             break;
//         }
//     }
//     EXPECT_TRUE(found_log) << "异步日志内容没有正确写入文件";

//     // 关闭日志系统
//     logger.Close();
// }

// // 测试日志系统关闭后不能再写入日志
// TEST(LogTest, LogAfterClose) {
//     Log& logger = Log::GetLogInstance();

//     // 初始化日志系统（同步模式）
//     logger.Init(100, false, 1024, 30);

//     // 关闭日志系统
//     logger.Close();

//     // 试图写入日志
//     LOG_INFO("This log should not be written after close");

//     // 读取日志文件内容，确保没有写入日志
//     std::ifstream log_file("/home/chenyinjie/github/WebServer/build/test/logfiles/logfile_2024_09_30_1.log");
//     ASSERT_TRUE(log_file.is_open());

//     std::string line;
//     bool found_log = false;
//     while (std::getline(log_file, line)) {
//         if (line.find("This log should not be written after close") != std::string::npos) {
//             found_log = true;
//             break;
//         }
//     }

//     EXPECT_FALSE(found_log) << "日志系统关闭后仍然写入了日志";
// }

/**
 * @brief 
 * 测试同步日志轮换
 * 测试时可以暂时将 max_lines_ 设置为一个小值
 * 本次测试设置：max_lines_ = 2;
 */

// TEST(LogTest, LogRotate) {
//     Log& logger = Log::GetLogInstance();
//     logger.Init(2, false, 10, 30);

//     for (size_t i = 0; i < 5; ++i) {
//         LOG_INFO("lines: %d", i);
//     }

//     int cnt = 1;
//     std::string prefix = "/home/chenyinjie/github/WebServer/build/test/logfiles/logfile_2024_09_30_";
//     std::string tail = ".log";
//     while (true) {
//         if (std::filesystem::exists(prefix + std::to_string(cnt) + tail)) {
//             cnt += 1;
//         } else {
//             break;
//         }
//     }
//     ASSERT_TRUE(cnt == 4);
//     logger.Close();
// }

// TODO : CleanLogs函数待测试

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}