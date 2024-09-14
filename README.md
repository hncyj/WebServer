# WebServer

Linux环境下C++实现的轻量级WebServer。

- [WebServer](#webserver)
  - [环境配置](#环境配置)
  - [一、配置模块](#一配置模块)
  - [二、缓冲区模块](#二缓冲区模块)
  - [三、日志模块](#三日志模块)
    - [3.1 阻塞队列](#31-阻塞队列)
    - [3.2 日志类](#32-日志类)
  - [http模块](#http模块)
  - [线程池模块](#线程池模块)
  - [定时器模块](#定时器模块)
  - [服务器模块](#服务器模块)

## 环境配置

- Ubuntu 24.04.1 LTS
- g++ (Ubuntu 13.2.0-23ubuntu4) 13.2.0
- cmake version 3.28.3
- mysql Ver 8.0.39-0ubuntu0.24.04.2 for Linux on x86_64

## 一、配置模块
## 二、缓冲区模块
## 三、日志模块

### 3.1 阻塞队列
使用单消费者-多生产者模式的[阻塞队列](src/log/block_queue.h)，将其作为日志异步写入的缓冲区。

支持优雅关闭：调用`close()`方法时，会禁止再向队列中加入日志消息，并让异步线程完成队列中剩余日志消息的写入。

### 3.2 日志类




## http模块
## 线程池模块
## 定时器模块
## 服务器模块