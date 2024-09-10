/**
 * @file buffer.h
 * @author chenyinjie
 * @date 2024-09-01
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <vector>
#include <atomic>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <sys/uio.h>
#include <mutex>

class Buffer {
public:
    explicit Buffer(int init_size = 1024);
    ~Buffer() = default;

    size_t ReadableLen() const;                 // 当前可读数据长度
    size_t WritableLen() const;                 // 当前可写数据长度
    const char* ReadPtr() const;                // 可读数据起始指针
    const char* WritePtr() const;               // 可写数据起始指针
    
    std::string ReadAllToStr();                 // 读取缓存区全部可读数据转存为字符串并移动读指针
    
    void Append(const char*, size_t);           // 追加指定长度数据至缓冲区写指针后
    void Append(const std::string&);            // 追加字符串数据至缓冲写指针后
    void Append(const Buffer&);                 // 追加另一缓冲区数据至缓冲区写指针后

    ssize_t ReadFromFd(int, int*);              // 从指定文件描述符中读取数据到缓存区中
    ssize_t WriteToFd(int, int*);               // 将缓存区中的数据写入文件描述符

private:
    std::vector<char> buffer_;                  // 缓存区字符数组
    size_t readPos_;                            // 缓存区读取位置
    size_t writePos_;                           // 缓存区写入位置
    mutable std::mutex buffer_mtx_;             // 缓存区互斥锁
    
    char* BufferPtr();                          // 缓存区指针
    const char* BufferPtr() const;              // 缓存区指针常量
    void ExpandBuffer(size_t);                  // 缓存区扩容
    void EnsureWritable(size_t);                // 缓存区可写长度判断
};

#endif