/**
 * @file epoll.cpp
 * @author chenyinjie
 * @date 2024-09-12
 */

#include "epoll.h"

Epoll::Epoll(int max_event_nums): epoll_fd_(epoll_create1(0)), events_(max_event_nums) {
    if (epoll_fd_ < 0) {
        LOG_ERROR("Epoll: Failed to create epoll instance, Error: %d.", errno);
        throw std::runtime_error("Epoll: Failed to create epoll instance.");    
    }

    if (events_.size() == 0) {
        LOG_ERROR("Epoll: Invalid event vector size.");
        throw std::runtime_error("Epoll: Invalid event vector size.");
    }
}

Epoll::~Epoll() {
    close(epoll_fd_);
}

bool Epoll::AddFd(int fd, uint32_t event) {
    if (fd < 0) {
        // 文件描述符无效，直接返回 false
        LOG_ERROR("Epoll: Invalid fd: %d.", fd);
        return false;
    }

    struct epoll_event ev = {0};
    ev.events = event;
    ev.data.fd = fd;

    // 使用 epoll_ctl 添加文件描述符
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        if (errno == EEXIST) {
            // 如果文件描述符已经存在，尝试使用 EPOLL_CTL_MOD 修改事件
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
                LOG_ERROR("Epoll: Failed to modify existing fd: %d, Error: %d", fd, errno);
                return false;
            }
        } else {
            // 添加文件描述符失败，处理其他错误
            LOG_ERROR("Epoll: Failed to add fd: %d, Error: %d.", fd, errno);
            return false;
        }
    }
    return true;
}


bool Epoll::ModifyFd(int fd, uint32_t events) {
    if (fd < 0) {
        LOG_ERROR("Epoll: Invalid fd for modification: %d.", fd);
        return false;
    }

    struct epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        LOG_ERROR("Epoll: Failed to modify fd: %d, Error: %d.", fd, errno);
        return false;
    }
    return true;
}

bool Epoll::DeleteFd(int fd) {
    if (fd < 0) {
        LOG_ERROR("Epoll: Invalid delete fd: %d", fd);
        return false;
    }

    struct epoll_event ev = {0};  // 初始化事件
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev) == -1) {
        LOG_ERROR("Epoll: Failed to delete fd: %d, Error: %d.", fd, errno);
        return false;
    }
    return true;
}

int Epoll::EpollWait(int timeoutMs) {
    // int nfds;
    // do {
    //     nfds = epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    // } while (nfds == -1 && errno == EINTR);

    // if (nfds == -1) {
    //     LOG_ERROR("Epoll: epoll_wait failed, Error: %d.", errno);
    // }
    int nfds = epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    if (nfds == -1) {
        LOG_WARN("Epoll: epoll_wait with nothing, Warning: %d.", errno);
    }
    return nfds;
}

int Epoll::GetEventFd(size_t idx) const {
    if (idx < 0 || idx >= events_.size()) {
        LOG_ERROR("Epoll: Attempt to get invalid fd: %zu.", idx);
    }
    return events_[idx].data.fd;
}

uint32_t Epoll::GetEvents(size_t idx) const {
    if (idx < 0 || idx >= events_.size()) {
        LOG_ERROR("Epoll: Attempt to get in valid fd: %zu 's event.")
    }
    return events_[idx].events;
}