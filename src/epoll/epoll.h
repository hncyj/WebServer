/**
 * @file epoll.h
 * @author chenyinjie
 * @date 2024-09-12
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "../log/log.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

class Epoll {
public:
    explicit Epoll(int max_event_nums = 1024);
    ~Epoll();

    bool AddFd(int fd, uint32_t events);
    bool ModifyFd(int fd, uint32_t events);
    bool DeleteFd(int fd, uint32_t events);
    int Wait(int timeoutMs = -1);
    int GetEventFd(size_t idx) const;
    uint32_t GetEvents(size_t idx) const;

private:
    int epoll_fd_;
    std::vector<struct epoll_event> events_;
    bool is_log_open;
};

#endif