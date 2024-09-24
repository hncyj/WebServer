/**
 * @file http_connect.h
 * @author chenyinjie
 * @date 2024-09-24
 */

#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../pool/sql_connect_pool_RAII.h"
#include "http_request.h"
#include "http_response.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <error.h>
#include <netinet/in.h>
#include <atomic>

class HTTPConnect {
public:
    HTTPConnect();
    ~HTTPConnect();

    void Init(int socket_fd, const struct sockaddr_in& server_addr);
    ssize_t Read(int* save_errno);
    ssize_t Write(int* save_errno);

    void Close();

    int GetFd() const;
    int GetPort() const;

    const char* GetIP() const;
    struct sockaddr_in GetAddr() const;

    bool process();

    int ToWriteBytes();
    bool IsKeepAlive() const;

    static bool is_ET;
    static const char* src_dir;
    static std::atomic<int> userCount;

private:
    int socket_fd_;
    struct sockaddr_in server_addr_;
    bool is_close_;
    int iov_count_;
    struct iovec iov_[2];

    Buffer read_buffer_;
    Buffer write_buffer_;

    HTTPRequest request_;
    HTTPResponse response_;

    bool is_log_open;
};

#endif
