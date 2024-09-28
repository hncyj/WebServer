/**
 * @file config.cpp
 * @author chenyinjie
 * @date 2024-09-01
 */

#include "configuration.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

Configuration::Configuration(int port = 8080, int db_connect_nums = 8, int thread_nums = 8, int async = 1)
: PORT(port), DB_CONNECT_NUMS(db_connect_nums), THREAD_NUMS(thread_nums), ASYNC_MODE(async) {}

void Configuration::parse_args(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            char option = argv[i][1];
            if (option == 'h') {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  -p[:]<port>                Set the port number (default: 8080)\n"
                          << "  -l[:]<async_log_mode>      Set the log write mode (0: sync, 1: async)\n"
                          << "  -c[:]<sql_connect_nums>    Set the number of SQL connections (default: 8)\n"
                          << "  -t[:]<thread_nums>         Set the number of threads (default: 8)\n"
                          << "  -h                         Show help\n";
                exit(0);
            }
            
            const char* value = nullptr;
            if (argv[i][2] == ':') {
                value = argv[i] + 3;
            } else if (argv[i][2] != '\0') {
                value = argv[i] + 2;
            } else if (i + 1 < argc) {
                value = argv[++i];
            } else {
                std::cerr << "Option -" << option << " requires a value.\n";
                exit(1);
            }

            switch (option) {
                case 'p':
                    PORT = std::atoi(value);
                    break;
                case 'l':
                    ASYNC_MODE = std::atoi(value);
                    break;
                case 'c':
                    DB_CONNECT_NUMS = std::atoi(value);
                    break;
                case 't':
                    THREAD_NUMS = std::atoi(value);
                    break;
                default:
                    std::cerr << "Unknown option: -" << option << ". Use -h for help.\n";
                    exit(1);
            }
        } else {
            std::cerr << "Invalid argument: " << argv[i] << ". Use -h for help.\n";
            exit(1);
        }
    }
}
