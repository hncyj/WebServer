#include "log.h"

#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include <cstdio>

class TestLog : public ::testing::Test {
protected:
    std::string test_log_file_prefix;
    
    void SetUp() override {
        time_t t = time(nullptr);
        struct tm* sys_tm = std::localtime(&t);
        struct tm my_tm = *sys_tm;

        test_log_file_prefix = std::format("./LogFiles/{}_{:02}_{:02}_logfile", 
                                           my_tm.tm_year + 1900, 
                                           my_tm.tm_mon + 1, 
                                           my_tm.tm_mday
                                           );

        Log& log = Log::get_instance();
        log.init(false, 4, 1024, 10);
    }

    void TearDown() override {
        std::remove((test_log_file_prefix + "-1").c_str());
        std::remove((test_log_file_prefix + "-2").c_str());
    }
};

TEST_F(TestLog, InitializationTest) {
    Log& log = Log::get_instance();
    bool init_result = log.init(false, 4, 1024, 10);
    EXPECT_TRUE(init_result);
}

TEST_F(TestLog, SynchronousLoggingTest) {
    Log& log = Log::get_instance();

    log.write_log(0, "this is DEBUG log.");
    log.write_log(1, "this is INFO log.");
    log.write_log(2, "this is WARN.");

    std::ifstream log_file(test_log_file_prefix + "-1");
    ASSERT_TRUE(log_file.is_open());

    std::string line;
    int line_cnt = 0;
    while (std::getline(log_file, line)) {
        ++line_cnt;
    }
    log_file.close();
    EXPECT_EQ(line_cnt, 3);
}

TEST_F(TestLog, AsynchronousLoggingTest) {
    // 初始化日志系统为异步模式
    Log& log = Log::get_instance();
    log.init(false, 4, 512, 4);

    // 写入日志
    log.write_log(0, "This is a DEBUG log.");
    log.write_log(1, "This is an INFO log.");
    log.write_log(2, "This is a WARN log.");
    log.write_log(3, "This is an ERROR log.");

    // 等待异步日志线程写入完成
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 验证日志文件是否创建成功并写入内容
    std::ifstream log_file(test_log_file_prefix + "-1");
    ASSERT_TRUE(log_file.is_open());

    std::string line;
    int line_count = 0;
    while (std::getline(log_file, line)) {
        ++line_count;
    }
    log_file.close();
    EXPECT_EQ(line_count, 4);
}

TEST_F(TestLog, SynchronousLoggingWithBuildTest) {
    // 初始化日志系统为同步模式，设置最大行数为2
    Log& log = Log::get_instance();
    log.init(false, 2, 1024, 5);

    // 写入多行日志，超过最大行数
    log.write_log(0, "This is DEBUG log 1.");
    log.write_log(1, "This is INFO log 2.");
    log.write_log(2, "This is WARN log 3.");
    log.write_log(3, "This is ERROR log 4.");

    // 验证第一个日志文件是否创建成功并写入前两行内容
    std::ifstream log_file_1(test_log_file_prefix + "-1");
    ASSERT_TRUE(log_file_1.is_open());

    std::string line;
    int line_count = 0;
    while (std::getline(log_file_1, line)) {
        ++line_count;
    }
    log_file_1.close();
    EXPECT_EQ(line_count, 2);

    // 验证第二个日志文件是否创建成功并写入后两行内容
    std::ifstream log_file_2(test_log_file_prefix + "-2");
    ASSERT_TRUE(log_file_2.is_open());

    line_count = 0;
    while (std::getline(log_file_2, line)) {
        ++line_count;
    }
    log_file_2.close();
    EXPECT_EQ(line_count, 2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(color) = "yes";
    return RUN_ALL_TESTS();
}

