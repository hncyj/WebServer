/**
 * @file epoll.h
 * @author chenyinjie
 * @date 2024-09-12
 * @copyright Apache 2.0
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "../log/log.h"

#include <vector>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// 将epoll实例封装为一个类
class Epoll {
public:
    explicit Epoll(int max_event_nums = 1024);
    ~Epoll();

    bool AddFd(int fd, uint32_t event);                 // 添加事件描述符
    bool ModifyFd(int fd, uint32_t event);              // 修改事件描述符
    bool DeleteFd(int fd);                              // 删除事件描述符
    int EpollWait(int timeoutMs = -1);                  // 等待事件事件
    int GetEventFd(size_t idx) const;                   // 获取事件描述符
    uint32_t GetEvents(size_t idx) const;               // 获取监听事件

private:
    int epoll_fd_;                                      // epoll实例描述符
    std::vector<struct epoll_event> events_;            // 监听事件数组
};

#endif