/**
 * @file epoll.cpp
 * @author chenyinjie
 * @brief 
 * @date 2024-09-12
 */

#include "epoll.h"

Epoll::Epoll(int max_event_nums): epoll_fd_(epoll_create1(0)), events_(max_event_nums) {
    is_log_open = Log::GetInstance()->IsOpen();
    if (epoll_fd_ < 0 && events_.size() > 0) {
        if (is_log_open) {
            LOG_ERROR("Failed to create epoll instance. Error: %d", errno);
        } else {
            std::cerr << "Failed to create epoll instance. Error: " << errno << std::endl;
        }
        throw std::runtime_error("Failed to create epoll instance.");
    }
}

Epoll::~Epoll() {
    close(epoll_fd_);
}

bool Epoll::AddFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event event = {0};
    event.data.fd = fd;
    event.events = events;
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
}

bool Epoll::ModifyFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event event = {0};
    event.data.fd = fd;
    event.events = events;
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
}

bool Epoll::DeleteFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event event = {0};
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event);
}

int Epoll::Wait(int timeoutMs) {
    return epoll_wait(epoll_fd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

int Epoll::GetEventFd(size_t idx) const {
    assert(idx >= 0 && idx < events_.size());
    return events_[idx].data.fd;
}

uint32_t Epoll::GetEvents(size_t idx) const {
    assert(idx >= 0 && idx < events_.size());
    return events_[idx].events;
}
