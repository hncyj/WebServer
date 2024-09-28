/**
 * @file config.cpp
 * @author chenyinjie
 * @date 2024-09-01
 */

#include "configuration.h"

#include <unistd.h>
#include <cstdlib>

Configuration::Configuration(): PORT(8080), ASYNC_LOG_MODE(0), SQL_CONNECT_NUMS(8), THREAD_NUMS(8) {};

void Configuration::parse_args(int argc, char* argv[]) {
    int opt;
    const char *str = "p:l:c:t:h";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 'p': {
                PORT = std::atoi(optarg);
                break;
            }
            case 'l': {
                ASYNC_LOG_MODE = std::atoi(optarg);
                break;
            }
            case 's': {
                SQL_CONNECT_NUMS = std::atoi(optarg);
                break;
            }
            case 't': {
                THREAD_NUMS = std::atoi(optarg);
                break;
            }
            case 'h': {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  -p <port>                   Set the port number (default: 9090)\n"
                          << "  -l <async_log_mode>         Set the log write mode (0: async, 1: sync)\n"
                          << "  -s <sql_connect_nums>       Set the number of SQL connection (default: 8)\n"
                          << "  -t <thread_mode>            Set the number of threads (default: 8)\n"
                          << "  -h                          Show help\n";
                exit(0);  // 显示帮助信息后退出程序
            }
            default: {
                std::cerr << "Unknown option: " << opt << ". Use -h for help.\n";
                exit(1);  // 遇到未知选项时退出程序
            }
        }
    }
}
