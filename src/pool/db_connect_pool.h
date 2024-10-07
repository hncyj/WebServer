/**
 * @file db_connect_pool.h
 * @author chenyinjie
 * @date 2024-09-11
 * @copyright Apache 2.0
 */

#ifndef DB_CONNECT_POOL_H
#define DB_CONNECT_POOL_H

#include "../log/log.h"

#include <mysql/mysql.h>

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <thread>

/**
 * @brief 
 * 单例模式实现的MySQL数据库连接池
 */

class SQLConnectPool {
public:
    static SQLConnectPool* GetSQLConnectPoolInstance();

    SQLConnectPool(const SQLConnectPool&) = delete;
    SQLConnectPool& operator=(const SQLConnectPool&) = delete;
    
    bool Init(const char* host, const char* user, const char* password, const char* db_name, int db_port, int connect_nums = 16);
    MYSQL* GetConnection();                         // 获取空闲连接
    void FreeConnection(MYSQL*);                    // 释放连接
    int GetFreeConnectNums();                       // 获取当前空闲连接数
    void CloseConnectPool();                        // 关闭连接池

private:
    SQLConnectPool() = default;
    ~SQLConnectPool();

    int max_connect_nums_;                          // 最大连接数
    std::queue<MYSQL*> connect_pool_;               // 连接池
    std::mutex connect_pool_mtx_;                   // 互斥锁
    sem_t sems_;                                    // 信号量
};

#endif