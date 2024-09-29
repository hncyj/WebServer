/**
 * @file config.cpp
 * @author chenyinjie
 * @date 2024-09-01
 */

#include "configuration.h"

Configuration::Configuration(int port, int db_connect_nums, int thread_nums, int async)
    : PORT(port), DB_CONNECT_NUMS(db_connect_nums), THREAD_NUMS(thread_nums), ASYNC_MODE(async) {}

void Configuration::ParseArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            char option = argv[i][1];
            if (option == 'h') {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  -p[:]<port>                Set the port number (default: 8080)\n"
                          << "  -l[:]<async_log_mode>      Set the log write mode (0: sync, 1: async)\n"
                          << "  -c[:]<db_connect_nums>     Set the number of database connections (default: 8)\n"
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
                std::cerr << "[ERROR]: Option -" << option << " requires a value.\n";
                exit(1);
            }

            switch (option) {
                case 'p':
                    if (value == nullptr || !std::isdigit(value[0])) {
                        std::cerr << "[ERROR]: Option -p requires a valid port number.\n";
                        exit(1);
                    }
                    PORT = std::atoi(value);
                    break;
                case 'l':
                    if (value == nullptr || (std::atoi(value) != 0 && std::atoi(value) != 1)) {
                        std::cerr << "[ERROR]: Option -l requires a valid async log mode (0 or 1).\n";
                        exit(1);
                    }
                    ASYNC_MODE = std::atoi(value);
                    break;
                case 'c':
                    if (value == nullptr || !std::isdigit(value[0])) {
                        std::cerr << "[ERROR]: Option -c requires a valid number of Database connections.\n";
                        exit(1);
                    }
                    DB_CONNECT_NUMS = std::atoi(value);
                    break;
                case 't':
                    if (value == nullptr || !std::isdigit(value[0])) {
                        std::cerr << "[ERROR]: Option -t requires a valid number of threads.\n";
                        exit(1);
                    }
                    THREAD_NUMS = std::atoi(value);
                    break;
                default:
                    std::cerr << "[ERROR]: Unknown option: -" << option << ". Use -h for help.\n";
                    exit(1);
            }
        } else {
            std::cerr << "[ERROR]: Invalid argument: " << argv[i] << ". Use -h for help.\n";
            exit(1);
        }
    }
}