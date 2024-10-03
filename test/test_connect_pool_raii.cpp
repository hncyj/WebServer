/**
 * @file test_connect_pool_raii.cpp
 * @author chenyinjie
 * @date 2024-10-03
 */


#include "../src/pool/db_connect_pool_RAII.h"

#include <gtest/gtest.h>


// 测试 SQLConnectPoolRAII 的功能
TEST(SQLConnectPoolRAII, AcquireAndReleaseConnection) {
    // 获取数据库连接池的单例实例
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();
    
    // 初始化连接池，假设连接池的大小为3，参数可以根据实际设置进行调整
    pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 3);

    // 检查初始空闲连接数是否为 3
    EXPECT_EQ(pool->GetFreeConnectNums(), 3);

    // 创建一个 MYSQL 指针，用于存储通过 SQLConnectPoolRAII 获得的连接
    MYSQL* connection = nullptr;

    // 使用 SQLConnectPoolRAII 获取连接，并确保连接成功分配
    {
        SQLConnectPoolRAII raii(&connection, pool);
        ASSERT_NE(connection, nullptr);  // 检查连接是否有效

        // 确保在 RAII 对象作用域内，空闲连接数减少了 1
        EXPECT_EQ(pool->GetFreeConnectNums(), 2);
    }

    // 由于 RAII 对象已经超出作用域，连接应已自动归还到连接池
    EXPECT_EQ(pool->GetFreeConnectNums(), 3);
}

int main(int argc, char **argv) {
    Log::GetLogInstance().Init(4, true, 16, 30);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}