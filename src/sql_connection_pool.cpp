#include "sql_connection_pool.h"

ConnectionPool::ConnectionPool(): m_current_connects_nums(0), m_free_connects_nums(0) {}

ConnectionPool& ConnectionPool::get_instance() {
    static ConnectionPool connect_pool_instance;
    return connect_pool_instance;
}

bool ConnectionPool::init(std::string host_name, std::string usr_name, std::string usr_password, std::string db_name, int port, int max_connect_nums, bool open_log) {
    m_host_addr = host_name;
    m_db_port = port;
    m_usr_name = usr_name;
    m_usr_password = usr_password;
    m_db_name = db_name;
    m_open_log = open_log;
    m_max_connect_nums = max_connect_nums;

    for (int i = 0; i < m_max_connect_nums; ++i) {
        MYSQL* connect = mysql_init(nullptr);
        if (connect == nullptr) {
            LOG_ERROR("MySQL: init connection error.");
            return false;
        }
        connect = mysql_real_connect(connect, m_host_addr.c_str(), m_usr_name.c_str(), m_usr_password.c_str(), m_db_name.c_str(), port, nullptr, 0);
        if (connect == nullptr) {
            LOG_ERROR("MySQL: connect to database error.");
            return false;
        }
        m_connection_pool.emplace_back(connect);
        ++m_free_connects_nums;
    }
}

MYSQL* ConnectionPool::get_free_connection() {
    MYSQL* connect = nullptr;
    std::unique_lock<std::mutex> lock(m_connect_pool_mutex);
    while (m_free_connects_nums == 0) {
        m_connect_pool_condition_var.wait(lock);
    }
    connect = m_connection_pool.front();
    m_connection_pool.pop_front();
    --m_free_connects_nums;
    ++m_current_connects_nums;

    return connect;
}

bool ConnectionPool::release_connection(MYSQL* connect) {
    if (connect == nullptr) return false;
    std::lock_guard<std::mutex> lock(m_connect_pool_mutex);
    m_connection_pool.emplace_back(connect);
    ++m_free_connects_nums;
    --m_current_connects_nums;
    m_connect_pool_condition_var.notify_all();

    return true;
}

int ConnectionPool::get_free_connection_nums() {
    std::lock_guard<std::mutex> lock(m_connect_pool_mutex);
    return m_free_connects_nums;
}

void ConnectionPool::destroy_pool() {
    std::lock_guard<std::mutex> lock(m_connect_pool_mutex);
    for (auto& connect : m_connection_pool) { 
        mysql_close(connect);
    }
    m_current_connects_nums = 0;
    m_free_connects_nums = 0;
    m_connection_pool.clear();
}

ConnectionPool::~ConnectionPool() {
    destroy_pool();
}

ConnectionRAII::ConnectionRAII(MYSQL** connection, ConnectionPool& connect_pool): m_connection_RAII(nullptr), m_connection_pool_RAII(connect_pool) {
    m_connection_RAII = m_connection_pool_RAII.get_free_connection();
    *connection = m_connection_RAII;
}

ConnectionRAII::~ConnectionRAII() {
    if (m_connection_RAII != nullptr) {
        m_connection_pool_RAII.release_connection(m_connection_RAII);
    }
}
