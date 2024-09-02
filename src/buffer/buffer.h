/**
 * @file buffer.h
 * @author chenyinjie
 * @date 2024-09-01
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include <string_view>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <cassert>

// Buffer 类用于高效管理和操作字节流
class Buffer {
private:
    char* beginWrite() noexcept;                            // 获取指向当前写入位置的指针
    const char* beginWrite() const noexcept;                // 获取指向当前写入位置的常量指针
    void expand(size_t len);                                // 扩展缓冲区以确保有足够的空间写入数据

    std::vector<char> buffer_;                              // 存储实际数据的缓冲区
    size_t readPos_;                                        // 当前缓冲区可读数据起始位置
    size_t writePos_;                                       // 当前缓冲区可写数据起始位置

public:
    explicit Buffer(size_t init_size = 1024);

    size_t writable_bytes() const noexcept;                 // 获取缓冲区中剩余的可写字节数
    size_t readable_bytes() const noexcept;                 // 获取缓冲区中当前可读的字节数
    size_t prependable_bytes() const noexcept;              // 获取缓冲区中可向前扩展的字节数

    const char* read_peek() const noexcept;                 // 获取指向当前可读数据的指针
    void check_writable(size_t len);                        // 确保缓冲区有足够的空间写入指定长度的数据
    void update_write_pos(size_t len) noexcept;             // 更新写入位置，表示已经写入了指定长度的数据
    void read_bytes(size_t len) noexcept;                   // 读取指定长度的数据

    void clear() noexcept;                                  // 清空缓冲区，将读写位置重置
    std::string read_and_clear();                           // 将缓冲区中的所有可读数据转换为字符串，并清空缓冲区
    void append(std::string_view data);                     // 将数据追加到缓冲区中
    ssize_t read_from_fd(int fd, int* Errno);               // 从文件描述符中读取数据到缓冲区
    ssize_t write_to_fd(int fd, int* Errno);                // 将缓冲区中的数据写入到文件描述符
};

#endif // BUFFER_H