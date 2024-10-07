/**
 * @file server.cpp
 * @author chenyinjie
 * @date 2024-09-24
 * @copyright Apache 2.0
 */

#include "server.h"

WebServer::WebServer(
    int port, int trigger_mode, bool is_linger, 
    int sql_port, const char* sql_user, const char* sql_pwd, const char* db_name, 
    int connect_pool_nums, int thread_pool_nums, 
    bool is_async, int block_queue_size, int timeout
    )
{   
    port_ = port;    
    timeoutMS_ = timeout;

    // 初始化日志系统
    if (!Log::GetLogInstance().Init(500, is_async, block_queue_size, 3)) {
        LOG_ERROR("Server: Init Log system failed.");
        is_close_ = true;
    } else {
        LOG_INFO("Server: Init Log system success.");
    }

    // 初始化线程池
    try {
        thread_pool_ = std::make_unique<ThreadPool>(thread_pool_nums, 16);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to init thread pool: %s.", e.what());
        is_close_ = true;  // 设置服务器关闭标志
    }

    // 初始化数据库连接池
    if (!SQLConnectPool::GetSQLConnectPoolInstance()->Init("localhost", sql_user, sql_pwd, db_name, sql_port, connect_pool_nums)) {
        LOG_ERROR("Sever: Init SQL Connect Pool failed.");
        is_close_ = true;
    } else {
        LOG_INFO("Sever: Init SQL Connect Pool sucess.");
    }

    // 初始化计时器小顶堆
    try {
        timer_ = std::make_unique<TimerHeap>();
    } catch (const std::exception& e) {
        LOG_ERROR("Server: Failed to init timer heap: %s.", e.what());
        is_close_ = true;
    }

    // 初始化Epoll
    try {
        epolls_ = std::make_unique<Epoll>();
    } catch (const std::exception& e) {
        LOG_ERROR("Server: Failed to init epolls: %s.", e.what());
        is_close_ = true;
    }
    
    // 初始化事件模式
    InitEventMode(trigger_mode);

    // 初始化路径
    // PROJECT_ROOT 是定义在CMakeLists中的全局变量
    src_dir_ = std::filesystem::path(PROJECT_ROOT) / "resources";
    if (!std::filesystem::exists(src_dir_)) {
        LOG_ERROR("Server: resources directory does not exist.");
        is_close_ = true;
    } else {
        LOG_INFO("Server: current work directory: %s.", src_dir_.string().c_str());
    }

    HTTPConnect::user_cnt = 0;
    HTTPConnect::src_dir = src_dir_;
    is_linger_ = is_linger;

    if (!InitSocket()) {
        is_close_ = true;
    }
    
    if (is_close_) {
        LOG_ERROR("Server: Server init error.");
        return;
    } else {
        LOG_INFO("========== Server Init ==========");
        LOG_INFO("Port:%d, Socket close linger: %s.", port_, is_linger ? "true":"false");
        LOG_INFO("Listen Mode: %s, Connect Mode: %s.", (listen_event_ & EPOLLET ? "ET": "LT"), (connect_event_ & EPOLLET ? "ET": "LT"));
        LOG_INFO("Source Directory: %s.", HTTPConnect::src_dir.c_str());
        LOG_INFO("SQL Connect Pool nums: %d, ThreadPool nums: %d.", connect_pool_nums, thread_pool_nums);
    }
}

WebServer::~WebServer() {
    close(listen_fd_);
    is_close_ = true;
    SQLConnectPool::GetSQLConnectPoolInstance()->CloseConnectPool();
}

void WebServer::Start() {
    int timeMS = -1;

    LOG_INFO("========== Server start =========="); 
    while (!is_close_) {
        if (timeoutMS_ > 0) {
            timeMS = timer_->GetNextExpireTime();
        }
        // 处理事件
        // 若 timeMS == -1, 则表示阻塞IO
        // 若 timeMS >= 0 则表示阻塞一定时间(为0表示非阻塞)
        int event_cnt = epolls_->EpollWait(timeMS);
        for (int i = 0; i < event_cnt; i++) {
            int fd = epolls_->GetEventFd(i);
            uint32_t events = epolls_->GetEvents(i);
            if (fd == listen_fd_) {
                // 处理新连接
                DealListen();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // 是否出现半关闭、挂起、错误
                if (!users_.count(fd)) {
                    LOG_ERROR("Server: No corresponding user found for fd: %d during disconnection or error event.", fd);
                    continue;
                }
                CloseConnect(&users_[fd]);
            } else if (events & EPOLLIN) {
                if (!users_.count(fd)) {
                    LOG_ERROR("Server: No corresponding user found for fd: %d during read event.", fd);
                    continue;
                }
                DealRead(&users_[fd]);
            } else if (events & EPOLLOUT) {
                if (!users_.count(fd)) {
                    LOG_ERROR("Server: No corresponding user found for fd: %d during write event.", fd);
                    continue;
                }
                DealWrite(&users_[fd]);
            } else {
                LOG_ERROR("Server: Unexpected event.");
            }
        }
    }
}

bool WebServer::InitSocket() {
    struct sockaddr_in server_addr;
    if (port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Server: invalid server port:%d.", port_);
        is_close_ = true;
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_);

    // 设置套接字优雅关闭
    struct linger optlinger = {0};
    if (is_linger_) {
        optlinger.l_onoff = 1;
        optlinger.l_linger = 1;
    }

    if ((listen_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_ERROR("Server: Create socket error, port: %d.", port_);
        return false;
    }

    if (setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &optlinger, sizeof(optlinger)) < 0) {
        close(listen_fd_);
        LOG_ERROR("Server: Init linger error, port: %d.", port_);
        return false;
    }

    int optval = 1;

    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const void*) &optval, sizeof(int)) == -1) {
        LOG_ERROR("Server: set socket error.");
        close(listen_fd_);
        return false;
    }

    if (bind(listen_fd_, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        LOG_ERROR("Server: Listen socket bind port: %d error.", port_);
        close(listen_fd_);
        return false;
    }

    if (listen(listen_fd_, 6) < 0) {
        LOG_ERROR("Server: Listen port:%d error.", port_);
        close(listen_fd_);
        return false;
    }

    if (epolls_->AddFd(listen_fd_, listen_event_ | EPOLLIN) == 0) {
        LOG_ERROR("Add listen fd error!");
        close(listen_fd_);
        return false;
    }

    SetFdNonblock(listen_fd_);
    LOG_INFO("Server: Server port:%d init socket success.", port_);
    return true;
}

/**
 * @brief 
 * 初始化事件模式
 * @param triggerMode
 * 0: 连接事件和监听事件均使用LT模式;
 * 1: 连接事件使用ET模式，监听事件使用LT模式;
 * 2: 连接事件使用LT模式，监听事件使用ET模式;
 * 3: 连接事件和监听事件均使用ET模式;
 */
void WebServer::InitEventMode(int trigger_mode) {
    listen_event_ = EPOLLRDHUP;
    connect_event_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigger_mode) {
        case 0:
            break;

        case 1:
            connect_event_ |= EPOLLET;
            break;

        case 2:
            listen_event_ |= EPOLLET;
            break;

        case 3:
            listen_event_ |= EPOLLET;
            connect_event_ |= EPOLLET;
            break;

        default:
            listen_event_ |= EPOLLET;
            connect_event_ |= EPOLLET;
            break;
    }
    HTTPConnect::is_ET = (connect_event_ & EPOLLET);
}

void WebServer::AddClient(int fd, sockaddr_in& addr) {
    if (fd <= 0) {
        LOG_ERROR("Server: Invalid client fd: %d.", fd);
        return;
    }
    // 保存客户端连接套接字的文件描述符以及客户端地址结构体
    users_[fd].Init(fd, addr);
    if (timeoutMS_ > 0) {
        // 绑定关闭套接字连接的函数作为回调函数
        timer_->AddTimer(fd, timeoutMS_, std::bind(&WebServer::CloseConnect, this, &users_[fd]));
    }
    epolls_->AddFd(fd, EPOLLIN | connect_event_);
    SetFdNonblock(fd);
    LOG_INFO("Server: Client[%d] connect.", users_[fd].GetFd());
}

void WebServer::DealListen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listen_fd_, (struct sockaddr*) &addr, &len);
        if (fd <= 0) {
            LOG_ERROR("Server: Failed to accept new client connection.");
            return;
        } else if (HTTPConnect::user_cnt >= MAX_FD) {
            SendError(fd, "Server is busy.");
            LOG_WARN("Server: Server connect is full.");
            return;
        }
        AddClient(fd, addr);
    } while (listen_event_ & EPOLLET);
}

void WebServer::DealRead(HTTPConnect* client) {
    if (client == nullptr) {
        LOG_ERROR("Server: Client is null in DealRead.");
        return;
    }
    ExtentTime(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnRead, this, client));
}

void WebServer::DealWrite(HTTPConnect* client) {
    if (client == nullptr) {
        LOG_ERROR("Server: Client is null in DealWrite.");
        return;
    }
    ExtentTime(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::SendError(int fd, const char* info) {
    if (fd <= 0) {
        LOG_ERROR("Server: Invalid fd: %d, in SendError.", fd);
        return;
    }
    if (send(fd, info, strlen(info), 0) == -1) {
        LOG_WARN("Server: Failed to send error message to client [%d]: %s.", fd, strerror(errno));
    } else {
        LOG_INFO("Server: Sent error message to client [%d].", fd);
    }
    close(fd);
    LOG_INFO("Server: SE: Closed connection with client [%d].", fd);
}

void WebServer::ExtentTime(HTTPConnect* client) {
    if (client == nullptr) {
        LOG_ERROR("Server: Client is null in ExtentTime.");
        return;
    }
    if (timeoutMS_ > 0) {
        timer_->UpdateTimer(client->GetFd(), timeoutMS_);
        LOG_INFO("Server: Updated timer for client [%d] with timeout [%d] ms", client->GetFd(), timeoutMS_);
    }
}

void WebServer::CloseConnect(HTTPConnect* client) {
    if (client == nullptr) {
        LOG_ERROR("Server: Client is null in Close Connect.");
        return;
    }
    LOG_INFO("Client[%d] quit.", client->GetFd());
    if (!epolls_->DeleteFd(client->GetFd())) {
        LOG_WARN("Server: Failed to delete client fd[%d] from epoll.", client->GetFd());
    }
    client->Close();
    LOG_INFO("Server: Client[%d] connection closed successfully.", client->GetFd());
}

void WebServer::OnRead(HTTPConnect* client) {
    if (client == nullptr) {
        LOG_ERROR("Server: Client is null in OnRead");
        return;
    }
    int val = -1;
    int readErrno = 0;
    val = client->Read(&readErrno);
    if (val <= 0 && readErrno != EAGAIN) {
        LOG_WARN("Server: Read from client [%d] failed with errno: %d", client->GetFd(), readErrno);
        CloseConnect(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite(HTTPConnect* client) {
    if (client == nullptr) {
        LOG_ERROR("Server: Client is null in OnWrite");
        return;
    }
    int val = -1;
    int writeErrno = 0;
    val = client->Write(&writeErrno);
    if (client->ToWriteBytes() == 0) {
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    } else if (val < 0) {
        if (writeErrno == EAGAIN) {
            LOG_INFO("Server: Write buffer full for client [%d], retrying later", client->GetFd());
            epolls_->ModifyFd(client->GetFd(), connect_event_ | EPOLLOUT);
            return;
        }
        LOG_WARN("Server: Write to client [%d] failed with errno: %d", client->GetFd(), writeErrno);
    } else {
        LOG_INFO("Server: Wrote %d bytes to client [%d]", val, client->GetFd());
    }
    CloseConnect(client);
}

void WebServer::OnProcess(HTTPConnect* client) {
    if (client->Process()) {
        epolls_->ModifyFd(client->GetFd(), connect_event_ | EPOLLOUT);
    } else {
        epolls_->ModifyFd(client->GetFd(), connect_event_ | EPOLLIN);
    }
}

int WebServer::SetFdNonblock(int fd) {
    if (fd <= 0) {
        LOG_ERROR("Server: Invalid fd: %d.", fd);
    }
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);;
}
