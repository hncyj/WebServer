/**
 * @file server.h
 * @author chenyinjie
 * @date 2024-09-24
 */

#ifndef SERVER_H
#define SERVER_H

#include "../log/log.h"
#include "../timer/timer.h"
#include "../epoll/epoll.h"
#include "../pool/thread_pool.h"
#include "../http/http_connect.h"
#include "../pool/sql_connect_pool.h"
#include "../pool/sql_connect_pool_RAII.h"

class WebServer {
public:
    WebServer(int port, int triggerMode, bool is_linger, int sql_port,
              const char* sql_user, const char* sql_pwd, const char* db_name,
              int connect_pool_nums, int thread_nums, bool is_open_log, 
              bool is_async, int block_queue_size);
              
    ~WebServer();
    void Start();

private:
    bool InitSocket(); 
    void InitEventMode(int triggerMode);
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
    

    int port_;
    bool open_linger_;
    int timeoutMS_;
    bool is_close_;
    int listen_fd_;
    char* src_dir_;
    bool is_log_open_;
    bool is_linger_;
    
    uint32_t listen_event_;
    uint32_t connect_event_;
   
    std::unique_ptr<TimerHeap> timer_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<Epoll> epolls_;
    std::unordered_map<int, HTTPConnect> users_;
};




#endif

