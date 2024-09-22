/**
 * @file sql_connect_pool.h
 * @author chenyinjie
 * @date 2024-09-11
 */

#ifndef SQL_CONNECT_POOL_H_
#define SQL_CONNECT_POOL_H_

#include "../log/log.h"

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <mysql/mysql.h>

class SQLConnectPool {
public:
    SQLConnectPool(const SQLConnectPool&) = delete;
    SQLConnectPool& operator=(const SQLConnectPool&) = delete;
    static SQLConnectPool* GetInstance();

    MYSQL* GetConnect();            // 获取空闲连接
    void FreeConnect(MYSQL*);       // 释放连接
    int GetFreeConnectNums();       // 获取当前空闲连接数
    void Init(const char* host, const char* user, const char* password, const char* db_name, int port, int connect_nums);
    void CloseConnectPool();        // 关闭连接池

private:
    SQLConnectPool() = default;
    ~SQLConnectPool();

    int max_connect_nums_;
    std::queue<MYSQL*> connect_pool_;
    std::mutex connect_pool_mtx_;
    sem_t sems_;
};



#endif