/**
 * @file sql_connect_pool.cpp
 * @author chenyinjie
 * @date 2024-09-11
 */

#include "sql_connect_pool.h"

SQLConnectPool* SQLConnectPool::GetInstance() {
    static SQLConnectPool sql_connect_pool_instance;
    return &sql_connect_pool_instance;
}

MYSQL* SQLConnectPool::GetConnect() {
    MYSQL* sql = nullptr;
    sem_wait(&sems_);
    {
        std::lock_guard<std::mutex> locker(connect_pool_mtx_);
        if (!connect_pool_.empty()) {
            sql = connect_pool_.front();
            connect_pool_.pop();
        }
    }
    return sql;
}

void SQLConnectPool::FreeConnect(MYSQL* sql) {
    assert(sql);
    std::lock_guard<std::mutex> locker(connect_pool_mtx_);
    connect_pool_.emplace(sql);
    sem_post(&sems_);
}

int SQLConnectPool::GetFreeConnectNums() {
    std::lock_guard<std::mutex> locker(connect_pool_mtx_);
    return connect_pool_.size();
}

void SQLConnectPool::Init(const char* host, const char* user, const char* password, const char* db_name, int port, int connect_nums = 10) {
    assert(connect_nums > 0);

    int nums = 0;
    for (int i = 0; i < connect_nums; ++i) {
        MYSQL* sql = nullptr;
        sql = mysql_init(nullptr);
        bool is_log_open = Log::GetInstance()->IsOpen();
        if (!sql) {
            if (is_log_open) {
                LOG_ERROR("MYSQL init error.");
            } else {
                std::cerr << "MYSQL init error.";
            }
            continue;
        }
        sql = mysql_real_connect(sql, host, user, password, db_name, port, nullptr, 0);
        if (!sql) {
            if (is_log_open) {
                LOG_ERROR("MYSQL connect error.");
            } else {
                std::cerr << "MYSQL connect error." << std::endl; 
            }
            mysql_close(sql);
            continue;
        }
        connect_pool_.emplace(sql);
        ++nums;
    }
    max_connect_nums_ = nums;
    sem_init(&sems_, 0, max_connect_nums_);
}

void SQLConnectPool::CloseConnectPool() {
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        std::lock_guard<std::mutex> locker(connect_pool_mtx_);
        while (!connect_pool_.empty()) {
            auto free_sql = connect_pool_.front();
            connect_pool_.pop();
            mysql_close(free_sql);
        }
        sem_destroy(&sems_);
        mysql_library_end();
    });
}

SQLConnectPool::~SQLConnectPool() {
    CloseConnectPool();
}
