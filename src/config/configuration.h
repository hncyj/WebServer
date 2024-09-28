/**
 * @file config.h
 * @author chenyinjie
 * @date 2024-09-01
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <iostream>

class Configuration {
public:
    Configuration(int port = 8080, int db_connect_nums = 8, int thread_nums = 8, int async = 1);
    ~Configuration() = default;

    void parse_args(int argc, char* argv[]);

    int PORT;                       // -p: 端口号
    int DB_CONNECT_NUMS;            // -c: 数据库连接池数量
    int THREAD_NUMS;                // -t: 线程池内线程数量
    int ASYNC_MODE;                // -l: 日志写入模式，0:同步，1:异步
};

#endif