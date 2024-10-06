/**
 * @file http_connect.cpp
 * @author chenyinjie
 * @date 2024-09-24
 * @copyright Apache 2.0
 */

#include "http_connect.h"

using namespace std;

bool HTTPConnect::is_ET;
const char* HTTPConnect::src_dir;
std::atomic<int> HTTPConnect::user_cnt;


HTTPConnect::HTTPConnect(): socket_fd_(-1), addr_{0}, is_close_(true) {}

HTTPConnect::~HTTPConnect() {
    Close();
}

void HTTPConnect::Init(int socket_fd, const sockaddr_in& server_addr) {
    if (socket_fd < 0) {
        LOG_ERROR("HTTP CONNECT: invalid socket fd.");
        return;
    }
    user_cnt.fetch_add(1);
    socket_fd_ = socket_fd;
    addr_ = server_addr;
    write_buffer_.Clear();
    read_buffer_.Clear();
    is_close_ = false;

    LOG_INFO("Client[%d](%s:%d) in, user_cnt:%d", socket_fd_, GetIP(), GetPort(), static_cast<int>(user_cnt));
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
            iov_[1].iov_base = static_cast<uint8_t*> (iov_[1].iov_base) + len - iov_[0].iov_len;
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len) {
                write_buffer_.ReadAll();
                iov_[0].iov_len = 0;
            }
        } else {
            iov_[0].iov_base = static_cast<uint8_t*>(iov_[0].iov_base) + len; 
            iov_[0].iov_len -= len; 
            write_buffer_.ReadLen(len);
        }
    } while (is_ET || ToWriteBytes() > 10240);

    return len;
}

void HTTPConnect::Close() {
    response_.UnmapFilePtr();
    if (!is_close_) {
        is_close_ = true;
        user_cnt.fetch_sub(1);
        close(socket_fd_);
        socket_fd_ = -1;
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", socket_fd_, GetIP(), GetPort(), static_cast<int>(user_cnt));
    }
}

int HTTPConnect::GetFd() const {
    return socket_fd_;
}

int HTTPConnect::GetPort() const {
    return ntohs(addr_.sin_port);
}

const char* HTTPConnect::GetIP() const {
    static char IP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr_.sin_addr, IP, sizeof(IP));
    return IP;
}

sockaddr_in HTTPConnect::GetAddr() const {
    return addr_;
}

Buffer& HTTPConnect::GetWriteBuffer() {
    return write_buffer_;
}

Buffer& HTTPConnect::GetReadBuffer() {
    return read_buffer_;
}

bool HTTPConnect::Process() {
    request_.Init();
    if (read_buffer_.ReadableLen() <= 0) {
        return false;
    } else if (request_.Parse(read_buffer_)) {
        LOG_INFO("HTTP Connect: Parse request: %s", request_.GetPath().c_str());
        response_.Init(src_dir, request_.GetPath(), request_.IsKeepAlive(), 200);
    } else {
        response_.Init(src_dir, request_.GetPath(), false, 400);
    }

    response_.GenerateResponse(write_buffer_);
    iov_[0].iov_base = write_buffer_.ReadPtr();
    iov_[0].iov_len = write_buffer_.ReadableLen();
    iov_count_ = 1;
    
    if (response_.GetFileLen() > 0 && response_.GetFilePtr()) {
        iov_[1].iov_base = response_.GetFilePtr();
        iov_[1].iov_len = response_.GetFileLen();
        iov_count_ = 2;
    }
    
    LOG_DEBUG("HTTP Connect: filesize: %d, %d to %d", response_.GetFileLen() , iov_count_, ToWriteBytes());
    return true;
}

size_t HTTPConnect::ToWriteBytes() const {
    return iov_[0].iov_len + iov_[1].iov_len;
}

bool HTTPConnect::IsKeepAlive() const {
    return request_.IsKeepAlive();
}
