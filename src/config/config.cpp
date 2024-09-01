/**
 * @file config.cpp
 * @author chenyinjie
 * @date 2024-09-01
 */

#include <unistd.h>
#include <cstdlib>

#include "config.h"

Config::Config() 
    : PORT(9090),                     // 默认端口号为9090
      LOG_WRITE_MODE(0),              // 默认日志写入模式为0（0代表异步）
      TRIGGER_MODE(0),                // 默认事件触发模式为0（0代表ET）
      LISTEN_TRIGGER_MODE(0),         // 默认监听事件触发模式为0
      CONNECT_TRIGGER_MODE(0),        // 默认连接事件触发模式为0
      OPT_LINGER(0),                  // 默认优雅关闭连接为0（关闭）
      SQL_CONNECT_POOL_NUMS(4),       // 默认数据库连接池数量为4
      THREAD_MODE(4),                 // 默认线程池内线程数量为4
      LOG_CLOSE(0),                   // 默认启用日志（0代表关闭）
      ACTOR_MODE(0) {};               // 默认并发模型为0（假设0代表Proactor）

void Config::parse_args(int argc, char* argv[]) {
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:h";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 'p': {
                PORT = std::atoi(optarg);
                break;
            }
            case 'l': {
                LOG_WRITE_MODE = std::atoi(optarg);
                break;
            }
            case 'm': {
                TRIGGER_MODE = std::atoi(optarg);
                break;
            }
            case 'o': {
                OPT_LINGER = std::atoi(optarg);
                break;
            }
            case 's': {
                SQL_CONNECT_POOL_NUMS = std::atoi(optarg);
                break;
            }
            case 't': {
                THREAD_MODE = std::atoi(optarg);
                break;
            }
            case 'c': {
                LOG_CLOSE = std::atoi(optarg);
                break;
            }
            case 'a': {
                ACTOR_MODE = std::atoi(optarg);
                break;
            }
            case 'h': {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  -p <port>                   Set the port number (default: 9090)\n"
                          << "  -l <log_write_mode>         Set the log write mode (0: async, 1: sync)\n"
                          << "  -m <trigger_mode>           Set the trigger mode (0: ET, 1: LT)\n"
                          << "  -o <opt_linger>             Set the option for graceful connection close (0: off, 1: on)\n"
                          << "  -s <sql_connect_pool_nums>  Set the number of SQL connection pool (default: 8)\n"
                          << "  -t <thread_mode>            Set the number of threads in the pool (default: 8)\n"
                          << "  -c <log_close>              Close logging (0: enable, 1: disable)\n"
                          << "  -a <actor_mode>             Set the actor mode (0: Proactor, 1: Reactor)\n"
                          << "  -h                          Show this help message\n";
                exit(0);  // 显示帮助信息后退出程序
            }
            default: {
                std::cerr << "Unknown option: " << opt << ". Use -h for help.\n";
                exit(1);  // 遇到未知选项时退出程序
            }
        }
    }
}
