#include "server.h"
/**
 * @file server.cpp
 * @author chenyinjie
 * @date 2024-09-24
 */

WebServer::WebServer(int port, int triggerMode, 
                        bool is_linger, int sql_port, 
                        const char* sql_user, const char* sql_pwd, 
                        const char* db_name, int connect_pool_nums, 
                        int thread_nums, bool is_log_open, bool is_async,
                        int block_queue_size) {
    src_dir_ = getcwd(nullptr, 256);
    assert(src_dir_);
    strncat(src_dir_, "/resources/", 16);
    HTTPConnect::userCount = 0;
    HTTPConnect::src_dir = src_dir_;
    is_linger_ = is_linger;
    is_log_open_ = is_log_open;

    SQLConnectPool::GetInstance()->Init("localhost", sql_user, sql_pwd, db_name, port, connect_pool_nums);
    InitEventMode(triggerMode);

    if (!InitSocket()) {
        is_close_ = true;
    }

    if (is_log_open_) {
        Log::getInstance().init(is_log_open, is_async, block_queue_size);
        if (is_close_) {
            LOG_ERROR("=============== Server init error ===============");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, is_linger ? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConnect Mode: %s", (listen_event_ & EPOLLET ? "ET": "LT"), (connect_event_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("src Director: %s", HTTPConnect::src_dir);
            LOG_INFO("SQL Connect Pool nums: %d, ThreadPool nums: %d", connect_pool_nums, thread_nums);
        }
    }
}

WebServer::~WebServer() {
    close(listen_fd_);
    is_close_ = true;
    free(src_dir_);
    SQLConnectPool::GetInstance()->CloseConnectPool();
}

void WebServer::Start() {
    int timeMS = -1;  /* epoll wait timeout == -1 无事件将阻塞 */
    if (!is_close_) { 
        if (is_log_open_) {
            LOG_INFO("========== Server start =========="); 
        }
    }

    while (!is_close_) {
        if (timeoutMS_ > 0) {
            timeMS = timer_->GetNextTimerExpireTime();
        }

        int event_cnt = epolls_->EpollWait(timeMS);
        for (int i = 0; i < event_cnt; i++) {
            int fd = epolls_->GetEventFd(i);
            uint32_t events = epolls_->GetEventFd(i);
            if (fd == listen_fd_) {
                DealListen();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConnect(&users_[fd]);
            } else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead(&users_[fd]);
            } else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite(&users_[fd]);
            } else {
                if (is_log_open_) LOG_ERROR("Unexpected event");
            }
        }
    }
}

bool WebServer::InitSocket() {
    struct sockaddr_in server_addr;
    if (port_ > 65535 || port_ < 1024) {
        if (is_log_open_) {
            LOG_ERROR("Port:%d error.", port_);
        }
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_);

    struct linger optlinger = {0};
    if (is_linger_) {
        optlinger.l_onoff = 1;
        optlinger.l_linger = 1;
    }

    
    if ((listen_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        if (is_log_open_) {
            LOG_ERROR("Create socket error!", port_);
        }
        return false;
    }

    if (setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &optlinger, sizeof(optlinger)) < 0) {
        close(listen_fd_);
        if (is_log_open_) {
            LOG_ERROR("Init linger error!", port_);
        }
        return false;
    }

    int optval = 1;

    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const void*) &optval, sizeof(int)) == -1) {
        if (is_log_open_) {
            LOG_ERROR("set socket setsockopt error!");
        }
        close(listen_fd_);
        return false;
    }

    if (bind(listen_fd_, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        if (is_log_open_) {
            LOG_ERROR("Bind Port: %d error!", port_);
        }
        close(listen_fd_);
        return false;
    }

    if (listen(listen_fd_, 6)) {
        if (is_log_open_) {
            LOG_ERROR("Listen port:%d error!", port_);
        }
        close(listen_fd_);
        return false;
    }

    if (epolls_->AddFd(listen_fd_, listen_event_ | EPOLLIN) == 0) {
        if (is_log_open_) {
            LOG_ERROR("Add listen error!");
        }
        close(listen_fd_);
        return false;
    }

    SetFdNonblock(listen_fd_);
    if (is_log_open_) {
        LOG_INFO("Server port:%d", port_);
    }
    return true;
}

void WebServer::InitEventMode(int triggerMode) {
    listen_event_ = EPOLLRDHUP;
    connect_event_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (triggerMode) {
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
    assert(fd > 0);
    users_[fd].Init(fd, addr);
    if (timeoutMS_ > 0) {
        timer_->AddTimer(fd, timeoutMS_, std::bind(&WebServer::CloseConnect, this, &users_[fd]));
    }
    epolls_->AddFd(fd, EPOLLIN | connect_event_);
    SetFdNonblock(fd);
    if (is_log_open_) {
        LOG_INFO("Client[%d] in!", users_[fd].GetFd());
    }
}

void WebServer::DealListen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listen_fd_, (struct sockaddr*) &addr, &len);
        if (fd <= 0) {
            return;
        }
        else if (HTTPConnect::userCount >= MAX_FD) {
            SendError(fd, "Server busy!");
            if (is_log_open_) {
                LOG_WARN("Clients is full!");
            }
            return;
        }
        AddClient(fd, addr);
    } while (listen_event_ & EPOLLET);
}

void WebServer::DealWrite(HTTPConnect* client) {
    assert(client);
    ExtentTime(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::DealRead(HTTPConnect* client) {
    assert(client);
    ExtentTime(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnRead, this, client));
}

void WebServer::SendError(int fd, const char* info) {
    assert(fd > 0);
    if (send(fd, info, strlen(info), 0)) {
        if (is_log_open_) {
            LOG_WARN("send error to client[%d] error!", fd);
        }
        close(fd);
    }
}

void WebServer::ExtentTime(HTTPConnect* client) {
    assert(client);
    if (timeoutMS_ > 0) {
        timer_->UpdateTimer(client->GetFd(), timeoutMS_);
    }
}

void WebServer::CloseConnect(HTTPConnect* client) {
    assert(client);
    if (is_log_open_) {
        LOG_INFO("Client[%d] quit!", client->GetFd());
    }
    epolls_->DeleteFd(client->GetFd());
    client->Close();
}

void WebServer::OnRead(HTTPConnect* client) {
    assert(client);
    int val = -1;
    int readErrno = 0;
    val = client->Read(&readErrno);
    if (val <= 0 && readErrno != EAGAIN) {
        CloseConnect(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite(HTTPConnect* client) {
    assert(client);
    int val = -1;
    int writeErrno = 0;
    val = client->Write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    }
    else if(val < 0) {
        if(writeErrno == EAGAIN) {
            epolls_->ModifyFd(client->GetFd(), connect_event_ | EPOLLOUT);
            return;
        }
    }
    CloseConnect(client);
}

void WebServer::OnProcess(HTTPConnect* client) {}

int WebServer::SetFdNonblock(int socket_fd) {
    assert(socket_fd > 0);
    return fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFD, 0) | O_NONBLOCK);;
}
