/**
 * @file test_connect_pool.cpp
 * @author chenyinjie
 * @date 2024-10-02
 */


#include "../src/pool/db_connect_pool.h"

#include <gtest/gtest.h>

// 测试数据库连接池的初始化
TEST(SQLConnectPoolTest, InitTest) {
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();
    
    // 假设数据库配置是本地的，并且数据库名是test_db
    pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 5);
    
    // 连接池成功初始化后，空闲连接数应该为5
    EXPECT_EQ(pool->GetFreeConnectNums(), 5);

    pool->CloseConnectPool();
}

// 测试获取和释放连接
TEST(SQLConnectPoolTest, GetAndReleaseConnectionTest) {
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();
    
    pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 5);
    // 获取一个连接
    MYSQL* conn = pool->GetConnection();
    
    // 检查连接是否成功获取
    ASSERT_NE(conn, nullptr);
    
    // 检查获取后空闲连接数应该减少
    EXPECT_EQ(pool->GetFreeConnectNums(), 4);
    
    // 释放连接
    pool->FreeConnection(conn);
    
    // 释放后空闲连接数应该恢复
    EXPECT_EQ(pool->GetFreeConnectNums(), 5);

    pool->CloseConnectPool();
}

// 测试获取超出连接池大小的连接
TEST(SQLConnectPoolTest, ExhaustConnectionTest) {
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();

    pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 5);
    
    // 获取所有连接
    for (int i = 0; i < 5; ++i) {
        MYSQL* sql = pool->GetConnection();
        ASSERT_NE(sql, nullptr);
    }
    
    // 此时空闲连接数应为0
    EXPECT_EQ(pool->GetFreeConnectNums(), 0);
    
    // 尝试获取第6个连接，应该返回nullptr
    MYSQL* sql = pool->GetConnection();
    EXPECT_EQ(sql, nullptr);

    pool->CloseConnectPool();
}

// 测试关闭连接池
TEST(SQLConnectPoolTest, ClosePoolTest) {
    SQLConnectPool* pool = SQLConnectPool::GetSQLConnectPoolInstance();
    
    // 初始化连接池并确认空闲连接数
    EXPECT_NO_THROW(pool->Init("localhost", "chenyinjie", "MySQL123456.", "WebServer", 3306, 5));
    EXPECT_EQ(pool->GetFreeConnectNums(), 5);

    // 关闭连接池
    pool->CloseConnectPool();
    
    // 检查关闭后，空闲连接数应为0
    EXPECT_EQ(pool->GetFreeConnectNums(), 0);
}

int main(int argc, char **argv) {
    Log::GetLogInstance().Init(4, true, 16, 30);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

