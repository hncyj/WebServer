# WebServer

Linux环境下C++实现的轻量级WebServer。

- [WebServer](#webserver)
  - [环境配置](#环境配置)
  - [配置模块](#配置模块)
  - [缓冲区模块](#缓冲区模块)
  - [日志模块](#日志模块)
  - [HTTP模块](#http模块)
  - [线程池模块](#线程池模块)
  - [定时器模块](#定时器模块)
  - [服务器模块](#服务器模块)

## 环境配置

- Ubuntu 24.04.1 LTS
- g++ (Ubuntu 13.2.0-23ubuntu4) 13.2.0
- cmake version 3.28.3
- mysql Ver 8.0.39-0ubuntu0.24.04.2 for Linux on x86_64

## 配置模块

[配置模块](/src/config/)提供了一个用于配置服务器参数的类 `Configuration`。
该类通过解析命令行参数来设置服务器的关键配置项，包括端口号、日志写入模式、数据库连接池数量以及线程池内线程数量，解析后的参数作为服务器启动初始化的参数。

**支持的命令行参数如下：**

- -p[:]<port>: 设置服务器端口号，默认值为 8080。
- -c[:]<db_connect_nums>: 设置数据库连接池的连接数量，默认值为 8。
- -t[:]<thread_nums>: 设置线程池的线程数量，默认值为 8。
- -l[:]<async_mode>: 设置日志写入模式，0 为异步，1 为同步，默认值为 0。
- -h: 显示帮助信息。

支持多种输入参数格式解析：
- `-p:8080 -l:1 -c:8 -t:8`
- `-p8080 -l1 -c8 -t8`
- `-p8080-l1-c8-t8`

## 缓冲区模块



## 日志模块

**阻塞队列**

使用单消费者-多生产者模式的[阻塞队列](src/log/block_queue.h)，将其作为日志异步写入的缓冲区。

**日志类**

实现了单例模式的[日志类](src/log//log.h)。


## HTTP模块
## 线程池模块
## 定时器模块
## 服务器模块