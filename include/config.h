#ifndef CONFIG_H_
#define CONFIG_H_

#include <iostream>

class Config {
public:
    Config();
    ~Config() = default;

    void parse_args(int argc, char* argv[]);

    int PORT;                   // 端口号
    int LOG_WRITE_MODE;         // 日志写入模式，0:异步，1:同步
    int TRIGGER_MODE;           // 事件触发模式，0:ET，1:LT
    int LISTEN_TRIGGER_MODE;    // 监听事件触发模式，0:ET，1:LT
    int CONNECT_TRIGGER_MODE;   // 连接事件触发模式，0:ET，1:LT
    int OPT_LINGER;             // 优雅关闭连接，0:关闭，1:开启
    int SQL_CONNECT_POOL_NUMS;  // 数据库连接池数量
    int THREAD_MODE;            // 线程池内线程数量
    int LOG_CLOSE;              // 是否启用日志，0:关闭，1:启用
    int ACTOR_MODE;             // 并发模型选择，0:Proactor，1:Reactor
};

#endif