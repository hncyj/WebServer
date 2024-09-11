/**
 * @file sql_connect_pool_RAII.h
 * @author chenyinjie
 * @date 2024-09-11
 */

#ifndef SQL_CONNECT_POOL_RAII_H_
#define SQL_CONNECT_POOL_RAII_H_

#include "sql_connect_pool.h"

class SQLConnectPoolRAII {
public:
    SQLConnectPoolRAII(MYSQL** sql, SQLConnectPool* connect_pool) {
        assert(connect_pool);
        *sql = connect_pool->GetConnect();
        sql_ = *sql;
        connect_pool_ = connect_pool;
    }

    ~SQLConnectPoolRAII() {
        if (sql_) connect_pool_->FreeConnect(sql_);
    }

private:
    MYSQL* sql_;
    SQLConnectPool* connect_pool_;
};


#endif