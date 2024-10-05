# WebServer

Linux环境下C++实现的轻量级WebServer。

- [WebServer](#webserver)
  - [环境配置](#环境配置)
  - [配置模块](#配置模块)
  - [缓冲区模块](#缓冲区模块)
  - [日志模块](#日志模块)
  - [Epoll模块](#epoll模块)
  - [定时器模块](#定时器模块)
  - [线程池\&连接池模块](#线程池连接池模块)
  - [HTTP模块](#http模块)
  - [服务器模块](#服务器模块)
  - [致谢](#致谢)

## 环境配置

- Ubuntu 24.04.1 LTS
- g++ (Ubuntu 13.2.0-23ubuntu4) 13.2.0
- cmake version 3.28.3
- mysql Ver 8.0.39-0ubuntu0.24.04.2 for Linux on x86_64
- GoogleTest 1.14.0

## 配置模块

[配置模块](/src/config/)提供了一个用于配置服务器参数的类 `Configuration`。

该类通过解析命令行参数来设置服务器的关键配置项，包括端口号、日志写入模式、数据库连接池数量以及线程池内线程数量。

解析后的参数作为服务器启动时的初始化参数。

**支持的命令行参数如下：**

- -p: 设置服务器端口号，默认值为 8080。
- -c: 设置数据库连接池的连接数量，默认值为 8。
- -t: 设置线程池的线程数量，默认值为 8。
- -l: 设置日志写入模式，0 为异步，1 为同步，默认值为 0。
- -h: 显示帮助信息。

**支持多种输入参数格式解析：**

- `-p:8080 -l:1 -c:8 -t:8`
- `-p8080 -l1 -c8 -t8`
- `-p8080-l1-c8-t8`

## 缓冲区模块



## 日志模块

基于单例模式设计与阻塞队列的同步/异步日志系统。

**阻塞队列**

- 使用单消费者-多生产者模式的[阻塞队列](src/log/block_queue.h)，将其作为日志异步写入的缓冲区。
- 使用互斥锁和条件变量保证了阻塞队列的线程安全。

<br>

**日志类**

- 整个服务器只有一个[日志单例](/src/log/log.h)。
- 通过阻塞队列在异步模式下存储日志消息，并由负责写入的异步线程完成从阻塞队列消息到日志文件的写入。
- 设置了四种日志级别：`[INFO]`、`[DEBUG]`、`[WARN]`、`[ERROR]`，用于控制日志记录、文件管理和刷新操作。
- 日志系统根据日期清理过期日志文件。
- 使用互斥锁保证了日志文件写入的线程安全。
- 当日期发生变化或当前到达单个日志文件最大行数时，进行日志切换。

## Epoll模块

- [Epoll模块](/src//epoll/epoll.h)封装了 Linux 环境下的`epoll`系统调用，提供了对事件驱动 I/O 的简化管理。
- 该模块提供了添加、修改、删除文件描述符以及等待 I/O 事件的接口。


## 定时器模块

- 小顶堆实现的[定时器模块](/src/timer/time.h)。
- 定时器管理：使用小顶堆存储定时器，确保最早到期的定时器总是在堆顶部。
- 回调函数执行：每个定时器均可绑定一个回调函数，定时器到期时自动执行回调。
- 动态更新定时器：支持根据新的超时时间更新已有定时器。
- 清理过期定时器：定时清理所有到期的定时器，保持小顶堆的有效性。
- 定时器堆的维护：通过上浮和下沉操作维护堆的有序性，确保每次操作都保持时间复杂度为`O(log n)`。

## 线程池&连接池模块

**线程池**

- [线程池](/src/pool/thread_pool.h)管理一组预先创建的线程，用于减少频繁线程创建与销毁的系统开销。
- 支持任务添加超时检测与异常记录。

**数据库连接池**

- 单例模式设计的 MySQL [数据库连接池](/src/pool/db_connect_pool.h)。
- 预先创建一组数据库连接，并在多个线程间共享，以减少频繁创建和销毁连接的开销。
- 通过互斥锁和信号量来确保线程安全，用户可以通过提供的接口获取空闲连接、释放连接以及查询当前空闲连接数量。

**RAII设计**

- [SQLConnectPoolRAII](/src//pool/db_connect_pool_RAII.h)类的设计遵循了 **RAII（Resource Acquisition Is Initialization）** 的原则，用于自动管理 MySQL 数据库连接的获取和释放。
- 该设计防止连接泄露并确保异常情况下的安全性。





## HTTP模块

该模块共由三部分组成：[HTTP请求报文解析模块](/src/http/http_request.h)、[HTTP响应报文生成模块](/src/http/http_response.h)、[HTTP连接模块](/src/http/http_connect.h)。

**HTTP请求报文解析模块**

- `HTTPRequest`类负责解析 HTTP 请求报文。
- 它通过从缓冲区中读取 HTTP 请求，解析请求的各个部分，包括请求行、首部行和请求体，并处理相应的状态与数据。
- 该类支持 `GET` 和 `POST` 请求，并能对 POST 请求中的 URL 编码数据进行解析。

<br>

**HTTP响应报文生成模块**

- `HTTPResponse`类负责生成 HTTP 响应报文。
- `Init()` 方法初始化响应所需的资源路径、请求路径、连接是否保持以及状态码。
- `GenerateResponse()` 方法根据请求的资源生成 HTTP 响应，包括状态行、头部和内容部分，最终写入到一个 `Buffer` 对象中。
- 通过 `mmap` 将请求的文件映射到内存中，并提供 `UnmapFilePtr()` 方法解除映射。
- 当请求的资源不存在或无法访问时，通过 `ErrorContent()` 和 `ErrorHtml()` 方法生成错误页面内容，并返回适当的 HTTP 状态码。

<br>

**HTTP连接模块**





## 服务器模块

## 致谢
