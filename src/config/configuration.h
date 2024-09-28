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
    Configuration();
    ~Configuration() = default;

    void parse_args(int argc, char* argv[]);

    int PORT;                       // -p: 端口号
    int ASYNC_LOG_MODE;             // -l: 日志写入模式，0:异步，1:同步
    int SQL_CONNECT_NUMS;           // -c: 数据库连接池数量
    int THREAD_NUMS;                // -t: 线程池内线程数量
};

#endif