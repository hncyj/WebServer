/**
 * @file main.cpp
 * @author chenyinjie
 * @date 2024-09-24
 * @copyright Apache 2.0
 */


#include "config/configuration.h"
#include "server/server.h"

#include <unistd.h>

int main(int argc, char* argv[]) {
    Configuration config;
    config.ParseArgs(argc, argv);

    std::cout << "============= Server starting =============" << std::endl;
    std::cout << "PORT: " << config.PORT << std::endl;
    std::cout << "Log mode: " << (config.ASYNC_MODE == 1 ? "Asynchronous" : "Synchronous") << std::endl;
    std::cout << "SQL connection pool size: " << config.DB_CONNECT_NUMS << std::endl;
    std::cout << "Thread pool size: " << config.THREAD_NUMS << std::endl;

    enum class TRIGGERMODE {
    BOTH_LT = 0,      // 连接事件和监听事件均使用LT模式
    CONNECT_ET = 1,   // 连接事件使用ET模式，监听事件使用LT模式
    LISTEN_ET = 2,    // 连接事件使用LT模式，监听事件使用ET模式
    BOTH_ET = 3       // 连接事件和监听事件均使用ET模式
    };

    const int port = config.PORT;
    const int triggermode = static_cast<int>(TRIGGERMODE::BOTH_ET);
    const bool islinger = true;
    const int dbport = 3306;
    const char* username = "chenyinjie";
    const char* password = "MySQL123456.";
    const char* database = "WebServer";
    const int dbconnectnums = config.DB_CONNECT_NUMS;
    const int threadnums = config.THREAD_NUMS;
    const bool isasync = (config.ASYNC_MODE == 1);
    const int blockqueuesize = 128;
    const int timeout = 0;

    WebServer server(port, triggermode, islinger, dbport, username, password, database, dbconnectnums, threadnums, isasync, blockqueuesize, timeout);
    server.Start();
    
    return 0;
}
