#include <gtest/gtest.h>

#include <config.h>

// Test default constructor values
TEST(ConfigTest, DefaultValues) {
    Config config;
    EXPECT_EQ(config.PORT, 9090);  // 默认端口应为 9090
    EXPECT_EQ(config.LOG_WRITE_MODE, 0);  // 假设默认日志写入模式为 0
    EXPECT_EQ(config.TRIGGER_MODE, 0);  // 假设默认事件触发模式为 0
    EXPECT_EQ(config.LISTEN_TRIGGER_MODE, 0);  // 假设默认监听事件触发模式为 0
    EXPECT_EQ(config.CONNECT_TRIGGER_MODE, 0);  // 假设默认连接事件触发模式为 0
    EXPECT_EQ(config.OPT_LINGER, 0);  // 假设默认优雅关闭连接为 0
    EXPECT_EQ(config.SQL_CONNECT_POOL_NUMS, 4);  // 假设默认数据库连接池数量为 4
    EXPECT_EQ(config.THREAD_MODE, 4);  // 假设默认线程池内线程数量为 4
    EXPECT_EQ(config.LOG_CLOSE, 0);  // 假设默认日志启用（0表示启用）
    EXPECT_EQ(config.ACTOR_MODE, 0);  // 假设默认并发模型为 0
}

// Test argument parsing
TEST(ConfigTest, ParseArgs) {
    Config config;
    const char* argv[] = {"program_name", "-p", "8080", "-l", "1", "-m", "1", "-o", "1", "-s", "16", "-t", "4", "-c", "1", "-a", "1"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    config.parse_args(argc, const_cast<char**>(argv));

    EXPECT_EQ(config.PORT, 8080);
    EXPECT_EQ(config.LOG_WRITE_MODE, 1);
    EXPECT_EQ(config.TRIGGER_MODE, 1);
    EXPECT_EQ(config.LISTEN_TRIGGER_MODE, 0);  // 假设未指定此参数，保持默认值
    EXPECT_EQ(config.CONNECT_TRIGGER_MODE, 0);  // 假设未指定此参数，保持默认值
    EXPECT_EQ(config.OPT_LINGER, 1);
    EXPECT_EQ(config.SQL_CONNECT_POOL_NUMS, 16);
    EXPECT_EQ(config.THREAD_MODE, 4);
    EXPECT_EQ(config.LOG_CLOSE, 1);
    EXPECT_EQ(config.ACTOR_MODE, 1);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

