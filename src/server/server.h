/**
 * @file server.h
 * @author chenyinjie
 * @date 2024-09-24
 * @copyright Apache 2.0
 */

#ifndef SERVER_H
#define SERVER_H

#include "../log/log.h"
#include "../timer/timer.h"
#include "../epoll/epoll.h"
#include "../pool/thread_pool.h"
#include "../http/http_connect.h"
#include "../pool/db_connect_pool.h"
#include "../pool/db_connect_pool_RAII.h"

class WebServer {
public:
    WebServer(
        int port, int trigger_mode, bool is_linger,
        int sql_port, const char* sql_user, const char* sql_pwd, const char* db_name,
        int connect_pool_nums, int thread_pool_nums,
        bool is_async, int block_queue_size, int timesout
    );
              
    ~WebServer();
    void Start();

private:
    bool InitSocket(); 
    void InitEventMode(int trigger_mode);
    void AddClient(int fd, struct sockaddr_in& addr);
  
    void DealListen();
    void DealWrite(HTTPConnect* client);
    void DealRead(HTTPConnect* client);

    void SendError(int fd, const char* info);
    void ExtentTime(HTTPConnect* client);
    void CloseConnect(HTTPConnect* client);

    void OnRead(HTTPConnect* client);
    void OnWrite(HTTPConnect* client);
    void OnProcess(HTTPConnect* client);

    static int SetFdNonblock(int fd);
    static const int MAX_FD = 65536;
    
    int port_;                                      // 服务器端口号                           
    int timeoutMS_;                                 // 连接超时时间                              
    int listen_fd_;                                 // 监听文件描述符
    bool is_linger_;                                // 套接字优雅关闭标识符
    bool is_close_;                                 // 服务器关闭标志符
    uint32_t listen_event_;                         // 设置套接字监听事件类型
    uint32_t connect_event_;                        // 存储连接套接字发生的事件类型
    std::filesystem::path src_dir_;                 // 资源目录
   
    std::unique_ptr<TimerHeap> timer_;              // 小顶堆计时器
    std::unique_ptr<ThreadPool> thread_pool_;       // 线程池
    std::unique_ptr<Epoll> epolls_;                 // epoll实例
    std::unordered_map<int, HTTPConnect> users_;    // 用户连接管理
};

#endif

