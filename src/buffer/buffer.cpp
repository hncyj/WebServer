/**
 * @file buffer.cpp
 * @author chenyinjie
 * @date 2024-09-01 
 */

#include <cassert>
#include <string>

#include "buffer.h"

Buffer::Buffer(int init_size): buffer_(init_size), readPos_(0), writePos_(0) {}

size_t Buffer::readableLen() const {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    return writePos_ - readPos_;
}

size_t Buffer::writableLen() const {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    return buffer_.size() - writePos_;
}

const char* Buffer::readPtr() const {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    return bufferPtr() + readPos_;
}

const char* Buffer::writePtr() const {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    return bufferPtr() + writePos_;
}

void Buffer::retrive(size_t len) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    if (len <= readableLen()) {
        readPos_ += len;
    } else {
        retriveAll();
    }
}

void Buffer::retriveAll() {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::retriveAll2Str() {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    std::string str(readPtr(), readableLen());
    retriveAll();
    return str;
}

void Buffer::append(const std::string& str) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    assert(!str.empty());
    append(str.data(), str.size());
}

void Buffer::append(const char* data, size_t len) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    ensureWritable(len);
    std::copy(data, data + len, writePtr());
    writePos_ += len;
}

void Buffer::append(const Buffer& buf) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    append(buf.readPtr(), buf.readableLen());
}

ssize_t Buffer::readFd(int fd, int* saveErrno) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    char buff[65535];
    struct iovec iov[2];
    const size_t write_size = writableLen();
    iov[0].iov_base = bufferPtr() + writePos_;
    iov[0].iov_len = write_size;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        *saveErrno = errno;
    } else if (static_cast<size_t>(len) <= write_size) {
        writePos_ += len;
    } else {
        writePos_ = buffer_.size();
        append(buff, len - write_size);
    }
    return len;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    size_t read_size = readableLen();
    ssize_t len = write(fd, readPtr(), read_size);
    if (len < 0) {
        *saveErrno = errno;
        return len;
    }
    readPos_ += len;
    return len;
}

char* Buffer::bufferPtr() {
    return buffer_.data();
}

const char* Buffer::bufferPtr() const {
    return buffer_.data();
}

void Buffer::expandBuffer(size_t len) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    if (writableLen() + readPos_ < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        std::copy(bufferPtr() + readPos_, bufferPtr() + writePos_, bufferPtr());
        readPos_ = 0;
        writePos_ = readableLen();
    }
}

void Buffer::ensureWritable(size_t len) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    if (writableLen() < len) {
        expandBuffer(len);
    }
}
