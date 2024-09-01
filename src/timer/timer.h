#ifndef TIMER_H_
#define TIMER_H_

#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <ctime>
#include <memory>
#include <functional>

#include "log.h"

class UtilTimer;

struct ClientData {
    sockaddr_in client_address;         // 客户端地址
    int socket_fd;                      // 套接字文件描述符
    std::shared_ptr<UtilTimer> timer;   // 定时器指针
};

class UtilTimer {
public:
    time_t expire_time;                                 // 任务超时时间
    ClientData* usr_data;                               // 客户端数据
    std::shared_ptr<UtilTimer> pre;                     // 指向前一个定时器的指针
    std::shared_ptr<UtilTimer> next;                    // 指向后一个定时器的指针
    std::function<void(ClientData*)> callback_func;     // 回调函数

    UtilTimer() = default;
    ~UtilTimer() = default;
};


class SortTimerList {
private:
    std::shared_ptr<UtilTimer> head;
    std::shared_ptr<UtilTimer> tail;

    void add_timer(std::shared_ptr<UtilTimer> timer, std::shared_ptr<UtilTimer> lst_head);

public:
    SortTimerList() = default;
    ~SortTimerList() = default;
    
    void add_timer(std::shared_ptr<UtilTimer> timer);
    void adjust_timer(std::shared_ptr<UtilTimer> timer);
    void delete_timer(std::shared_ptr<UtilTimer> timer);
    void tick();
};

class Utils {
public:
    static int* u_pipe_fd;
    static int u_epoll_fd;
    int m_time_solt = 0;
    SortTimerList m_timer_lst;

    Utils() = default;
    ~Utils() = default;

    void init(int time_slot);
    int set_no_block(int fd);
    void add_fd(int epoll_fd, int fd, bool one_shot, int trigger_mode);
    static void sig_handler(int signal);
    void add_signal(int signal, void(handler)(int), bool restart = true);
    void timer_handler();
    void show_error(int connect_fd, const char* info);
};

void callback_fun(ClientData* usr_data);

#endif
