/**
 * @file http_connect.cpp
 * @author chenyinjie
 * @date 2024-09-24
 */

#include "http_connect.h"

using namespace std;

bool HTTPConnect::is_ET;
const char* HTTPConnect::src_dir;
std::atomic<int> HTTPConnect::userCount;


HTTPConnect::HTTPConnect(): socket_fd_(-1), server_addr_{0}, is_close_(true) {}

HTTPConnect::~HTTPConnect() {
    Close();
}

void HTTPConnect::Init(int socket_fd, const sockaddr_in& server_addr) {
    assert(socket_fd > 0);
    ++userCount;
    server_addr_ = server_addr;
    socket_fd_ = socket_fd;
    write_buffer_.ReadAll();
    read_buffer_.ReadAll();
    is_close_ = false;
    is_log_open = Log::getInstance().isOpen();

    if (is_log_open) {
        LOG_INFO("Client[%d](%s:%d) in, userCount:%d", socket_fd_, GetIP(), GetPort(), static_cast<int>(userCount));
    }
}

ssize_t HTTPConnect::Read(int* save_errno) {
    ssize_t len = -1;
    do {
        len = read_buffer_.ReadFromFd(socket_fd_, save_errno);
        if (len <= 0) break;
    } while (is_ET);

    return len;
}

ssize_t HTTPConnect::Write(int* save_errno) {
    ssize_t len = -1;
    do {
        len = writev(socket_fd_, iov_, iov_count_);
        if (len <= 0) {
            *save_errno = errno;
            break;
        }

        if (iov_[0].iov_len + iov_[1].iov_len == 0) {
            break;
        } else if (static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = static_cast<uint8_t*> (iov_[1].iov_base + (len - iov_[0].iov_len));
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len) {
                write_buffer_.ReadAll();
                iov_[0].iov_len = 0;
            }
        } else {
            iov_[0].iov_base = static_cast<uint8_t*>(iov_[0].iov_base + len); 
            iov_[0].iov_len -= len; 
            write_buffer_.ReadLen(len);
        }
    } while (is_ET || ToWriteBytes() > 10240);

    return len;
}

void HTTPConnect::Close() {
    response_.UnmapMemoryFilePtr();
    if (!is_close_) {
        is_close_ = true;
        --userCount;
        close(socket_fd_);
        if (is_log_open) {
            LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", socket_fd_, GetIP(), GetPort(), static_cast<int>(userCount));
        }
    }
}

int HTTPConnect::GetFd() const {
    return socket_fd_;
}

int HTTPConnect::GetPort() const {
    return server_addr_.sin_port;
}

const char* HTTPConnect::GetIP() const {
    return inet_ntoa(server_addr_.sin_addr);
}

sockaddr_in HTTPConnect::GetAddr() const {
    return server_addr_;
}

bool HTTPConnect::process() {
    request_.Init();
    if(read_buffer_.ReadableLen() <= 0) {
        return false;
    }
    else if(request_.Parse(read_buffer_)) {
        if (is_log_open) {
            LOG_DEBUG("%s", request_.GetPath().c_str());
        }
        response_.Init(src_dir, request_.GetPath(), request_.IsKeepAlive(), 200);
    } else {
        response_.Init(src_dir, request_.GetPath(), false, 400);
    }

    response_.GenerateResponse(write_buffer_);
    iov_[0].iov_base = write_buffer_.ReadPtr();
    iov_[0].iov_len = write_buffer_.ReadableLen();
    iov_count_ = 1;
    
    if (response_.GetFileLen() > 0 && response_.GetMapFilePtr()) {
        iov_[1].iov_base = response_.GetMapFilePtr();
        iov_[1].iov_len = response_.GetFileLen();
        iov_count_ = 2;
    }
    
    if (is_log_open) {
        LOG_DEBUG("filesize:%d, %d  to %d", response_.GetFileLen() , iov_count_, ToWriteBytes());
    }

    return true;
}

int HTTPConnect::ToWriteBytes() {
    return iov_[0].iov_len + iov_[1].iov_len;
}

bool HTTPConnect::IsKeepAlive() const {
    return request_.IsKeepAlive();
}
