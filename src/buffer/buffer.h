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

    size_t readableLen() const;
    size_t writableLen() const;
    
    const char* readPtr() const;
    const char* writePtr() const;
    
    void retrive(size_t);
    void retriveAll();
    std::string retriveAll2Str();
    
    void append(const std::string&);
    void append(const char*, size_t);
    void append(const Buffer&);

    ssize_t readFd(int, int*);
    ssize_t writeFd(int, int*);

private:
    std::vector<char> buffer_;
    std::atomic<size_t> readPos_;
    std::atomic<size_t> writePos_;
    mutable std::mutex buffer_mtx_;
    
    char* bufferPtr();
    const char* bufferPtr() const;
    void expandBuffer(size_t);
    void ensureWritable(size_t);
};

#endif