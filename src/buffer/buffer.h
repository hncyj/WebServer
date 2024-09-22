/**
 * @file buffer.h
 * @author chenyinjie
 * @date 2024-09-01
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <sys/uio.h>
#include <stdarg.h>
#include <mutex>

class Buffer {
public:
    explicit Buffer(int init_size = 1024);
    ~Buffer() = default;

    size_t ReadableLen() const;                 // 缓冲区可读数据长度
    size_t WritableLen() const;                 // 缓冲区可写数据长度
    size_t PrependableLen() const;              // 缓冲区可复用长度 

    char* ReadPtr() const;                      // 缓冲区可读数据起始指针
    char* WritePtr() const;                     // 缓冲区可写位置起始指针
    
    void ReadLen(size_t);                       // 读取缓冲区指定长度的数据
    void ReadToPtr(const char*);                // 读取到指定指针位置中的数据
    void ReadAll();                             // 读取缓冲区全部可读数据
    std::string ReadAllToStr();                 // 读取缓存区全部可读数据并转存为字符串
    
    void Append(const char*, size_t);           // 追加指定长度数据至缓冲区写指针后
    void Append(const std::string&);            // 追加字符串数据至缓冲写指针后
    void Append(const Buffer&);                 // 追加另一缓冲区数据至缓冲区写指针后
    int AppendFormatted(const char* format, 
                                va_list args);  // 添加格式化后的数据至缓冲区

    ssize_t ReadFromFd(int, int*);              // 从指定文件描述符中读取数据到缓存区中
    ssize_t WriteToFd(int, int*);               // 将缓存区中的数据写入文件描述符

    void Clear();                               // 清空缓冲区

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