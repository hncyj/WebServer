/**
 * @file http_connect.h
 * @author chenyinjie
 * @date 2024-09-24
 * @copyright Apache 2.0
 */

#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../pool/db_connect_pool_RAII.h"
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

    Buffer& GetWriteBuffer();
    Buffer& GetReadBuffer();

    bool Process();

    size_t ToWriteBytes() const;
    bool IsKeepAlive() const;

    static bool is_ET;                                              // 事件触发通知模式
    static std::filesystem::path src_dir;                           // 资源路径
    static std::atomic<int> user_cnt;                               // 当前连接用户数

private:
    int socket_fd_;                                                 // 连接套接字文件描述符
    struct sockaddr_in addr_;                                       // 地址结构体
    bool is_close_;                                                 // 连接关闭标记
    int iov_count_;                                                 // `iov`结构体数组中有效缓冲区数量
    struct iovec iov_[2];                                           // I/O缓冲数组

    Buffer read_buffer_;                                            // 读取客户端传输数据缓冲区
    Buffer write_buffer_;                                           // 服务器数据发送缓冲区

    HTTPRequest request_;                                           // 报文解析器
    HTTPResponse response_;                                         // 报文生成器
};

#endif
