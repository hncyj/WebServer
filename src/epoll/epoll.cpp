/**
 * @file epoll.cpp
 * @author chenyinjie
 * @date 2024-09-12
 */

#include "epoll.h"

Epoll::Epoll(int max_event_nums): epoll_fd_(epoll_create1(0)), events_(max_event_nums) {
    is_log_open = Log::getInstance().isOpen();
    if (epoll_fd_ < 0 && events_.size() <= 0) {
        if (is_log_open) {
            LOG_ERROR("Failed to create epoll instance, Error: %d", errno);
        } else {
            std::cerr << "Failed to create epoll instance, Error: " << errno << std::endl;
        }
        throw std::runtime_error("Failed to create epoll instance.");
    }
}

Epoll::~Epoll() {
    close(epoll_fd_);
}

bool Epoll::AddFd(int fd, uint32_t interest_ev) {
    if (fd < 0) {
        // 文件描述符无效，直接返回 false
        if (is_log_open) {
            LOG_ERROR("Invalid file descriptor: %d", fd);
        } else {
            std::cerr << "Invalid file descriptor: " << fd << std::endl;
        }
        return false;
    }

    struct epoll_event ev = {0};
    ev.events = interest_ev;
    ev.data.fd = fd;

    // 使用 epoll_ctl 添加文件描述符
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        if (errno == EEXIST) {
            // 如果文件描述符已经存在，尝试使用 EPOLL_CTL_MOD 修改事件
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
                if (is_log_open) {
                    LOG_ERROR("Failed to modify existing file descriptor in epoll, fd: %d, Error: %d", fd, errno);
                } else {
                    std::cerr << "Failed to modify existing file descriptor in epoll, fd: " 
                              << fd << ", Error: " << errno << std::endl;
                }
                return false;
            }
        } else {
            // 添加文件描述符失败，处理其他错误
            if (is_log_open) {
                LOG_ERROR("Failed to add file descriptor to epoll, fd: %d, Error: %d", fd, errno);
            } else {
                std::cerr << "Failed to add file descriptor to epoll, fd: " 
                          << fd << ", Error: " << errno << std::endl;
            }
            return false;
        }
    }

    return true;
}


bool Epoll::ModifyFd(int fd, uint32_t events) {
    if (fd < 0) {
        if (is_log_open) {
            LOG_ERROR("Invalid file descriptor for modification: %d", fd);
        } else {
            std::cerr << "Invalid file descriptor for modification: " << fd << std::endl;
        }
        return false;
    }

    struct epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        if (is_log_open) {
            LOG_ERROR("Failed to modify file descriptor in epoll, fd: %d, Error: %d", fd, errno);
        } else {
            std::cerr << "Failed to modify file descriptor in epoll, fd: " 
                      << fd << ", Error: " << errno << std::endl;
        }
        return false;
    }

    return true;
}

bool Epoll::DeleteFd(int fd, uint32_t events) {
    if (fd < 0) {
        if (is_log_open) {
            LOG_ERROR("Invalid file descriptor for deletion: %d", fd);
        } else {
            std::cerr << "Invalid file descriptor for deletion: " << fd << std::endl;
        }
        return false;
    }

    struct epoll_event ev = {0};  // 初始化事件
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev) == -1) {
        if (is_log_open) {
            LOG_ERROR("Failed to delete file descriptor from epoll, fd: %d, Error: %d", fd, errno);
        } else {
            std::cerr << "Failed to delete file descriptor from epoll, fd: " 
                      << fd << ", Error: " << errno << std::endl;
        }
        return false;
    }

    return true;
}

int Epoll::EpollWait(int timeoutMs) {
    int nfds = epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    if (nfds == -1) {
        if (is_log_open) {
            LOG_WARN("epoll_wait failed, Warning: %d", errno);
        } else {
            std::cerr << "epoll_wait failed, Warning: " << errno << std::endl;
        }
    }
    return nfds;
}

int Epoll::GetEventFd(size_t idx) const {
    assert(idx >= 0 && idx < events_.size());
    return events_[idx].data.fd;
}

uint32_t Epoll::GetEventsInterest(size_t idx) const {
    assert(idx >= 0 && idx < events_.size());
    return events_[idx].events;
}