/**
 * @file epoll.h
 * @author chenyinjie
 * @date 2024-09-12
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "../log/log.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// 将epoll实例封装为一个类
class Epoll {
public:
    explicit Epoll(int max_event_nums = 1024);
    ~Epoll();

    bool AddFd(int fd, uint32_t interest_ev);
    bool ModifyFd(int fd, uint32_t interest_ev);
    bool DeleteFd(int fd);
    int EpollWait(int timeoutMs = -1);
    int GetEventFd(size_t idx) const;
    uint32_t GetEventsInterest(size_t idx) const;

private:
    int epoll_fd_;
    std::vector<struct epoll_event> events_;
    bool is_log_open;
};

#endif