#ifndef SQL_CONNECTION_POOL_H_
#define SQL_CONNECTION_POOL_H_

#include <mysql/mysql.h>
#include <list>
#include <string>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <unordered_set>

#include "log.h"

class ConnectionPool {
private:
    int m_max_connect_nums;                                 // 连接池最大连接数
    int m_current_connects_nums;                            // 当前连接数
    int m_free_connects_nums;                                // 空闲连接数
    std::mutex m_connect_pool_mutex;                        // 连接池锁
    std::condition_variable m_connect_pool_condition_var;   // 连接池条件变量
    std::list<MYSQL*> m_connection_pool;                    // 连接池

private:
    ConnectionPool();
    ~ConnectionPool();

public:
    std::string m_host_addr;                                 // 主机地址
    int m_db_port;                                           // 数据库端口号
    std::string m_usr_name;                                  // 登陆数据库用户名
    std::string m_usr_password;                              // 登陆数据库密码
    std::string m_db_name;                                   // 使用数据库名
    bool m_open_log;                                         // 是否启用日志，0:否，1:是

public:
    // 单例模式
    static ConnectionPool& get_instance();

    // 初始化
    bool init(std::string host_name, std::string usr_name, std::string m_usr_password, std::string db_name, int port, int max_connect_nums, bool open_log);
    // 获取一个空闲链接
    MYSQL* get_free_connection();                
    // 释放一个链接并返还给连接池
    bool release_connection(MYSQL*);
    // 获取空闲连接数
    int get_free_connection_nums();
    // 销毁连接池
    void destroy_pool();
};

class ConnectionRAII {
private:
    MYSQL* m_connection_RAII;
    ConnectionPool& m_connection_pool_RAII;

public:
    explicit ConnectionRAII(MYSQL** connection, ConnectionPool& connect_pool);
    ~ConnectionRAII();
};

#endif
