/**
 * @file db_connect_pool_RAII.h
 * @author chenyinjie
 * @date 2024-09-11
 */

#ifndef DB_CONNECT_POOL_RAII_H
#define DB_CONNECT_POOL_RAII_H

#include "db_connect_pool.h"

/**
 * @brief 
 * SQL Connect Pool RAII
 */

class SQLConnectPoolRAII {
public:
    SQLConnectPoolRAII(MYSQL** sql, SQLConnectPool* sql_connect_pool) {
        if (!sql_connect_pool) {
            LOG_ERROR("MySQL Connect RAII: sql_connect_pool is nullptr.");
        }
        *sql = sql_connect_pool->GetConnection();
        sql_ = *sql;
        connect_pool_ = sql_connect_pool;
    }

    ~SQLConnectPoolRAII() {
        if (sql_) {
            connect_pool_->FreeConnection(sql_);
        }
    }

private:
    MYSQL* sql_;
    SQLConnectPool* connect_pool_;
};


#endif