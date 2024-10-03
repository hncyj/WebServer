/**
 * @file db_connect_pool.cpp
 * @author chenyinjie
 * @date 2024-09-11
 */

#include "db_connect_pool.h"

SQLConnectPool* SQLConnectPool::GetSQLConnectPoolInstance() {
    static SQLConnectPool sql_connect_pool_instance;
    return &sql_connect_pool_instance;
}

void SQLConnectPool::Init(const char* host, const char* user, const char* password, const char* db_name, int port, int connect_nums) {
    if (connect_nums <= 0) {
        LOG_ERROR("Connect Pool: Init Database connect number error.");
        throw std::runtime_error("Failed to initialize connection pool.");
    }

    int nums = 0;
    for (int i = 0; i < connect_nums; ++i) {
        MYSQL* sql = nullptr;
        sql = mysql_init(nullptr);
        if (!sql) {
            LOG_ERROR("Connect Pool: MySQL connect i: %d init error.", i);
            continue;
        }
        sql = mysql_real_connect(sql, host, user, password, db_name, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("Connect Pool: MySQL i: %d create connect error: %s", i, mysql_error(sql));
            continue;
        }
        connect_pool_.emplace(sql);
        ++nums;
    }
    
    if (nums <= 0) {
        LOG_ERROR("Connect Pool: Init Database Connect Pool Failed.");
        return;
    }

    max_connect_nums_ = nums;
    if (sem_init(&sems_, 0, max_connect_nums_) != 0) {
        LOG_ERROR("Connect Pool: Failed to initialize semaphore.");
        throw std::runtime_error("Connect Pool: Failed to initialize semaphore.");
    }
}

MYSQL* SQLConnectPool::GetConnection() {
    MYSQL* sql = nullptr;
    std::lock_guard<std::mutex> locker(connect_pool_mtx_);
    if (connect_pool_.empty()) {
        LOG_WARN("Connect Pool: No free connection.");
    } else {
        sem_wait(&sems_);
        sql = connect_pool_.front();
        connect_pool_.pop();
    }
    return sql;
}

void SQLConnectPool::FreeConnection(MYSQL* sql) {
    if (sql == nullptr) {
        LOG_ERROR("Connect Pool: Attempt to free nullptr.");
        return;
    }
    std::lock_guard<std::mutex> locker(connect_pool_mtx_);
    connect_pool_.push(sql);
    sem_post(&sems_);
}

int SQLConnectPool::GetFreeConnectNums() {
    std::lock_guard<std::mutex> locker(connect_pool_mtx_);
    return connect_pool_.size();
}

void SQLConnectPool::CloseConnectPool() {
    std::lock_guard<std::mutex> locker(connect_pool_mtx_);
    while (!connect_pool_.empty()) {
        auto free_sql = connect_pool_.front();
        connect_pool_.pop();
        mysql_close(free_sql);
    }
    sem_destroy(&sems_);
    mysql_library_end();
}

SQLConnectPool::~SQLConnectPool() {
    CloseConnectPool();
}
