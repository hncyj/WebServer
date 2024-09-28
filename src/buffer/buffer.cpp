/**
 * @file buffer.cpp
 * @author chenyinjie
 * @date 2024-09-01 
 */

#include "buffer.h"

Buffer::Buffer(int init_size): buffer_(init_size), readPos_(0), writePos_(0) {}

size_t Buffer::ReadableLen() const {
    std::shared_lock<std::shared_mutex> locker(buffer_mtx_);
    return writePos_ - readPos_;
}

char* Buffer::ReadPtr() {
    std::shared_lock<std::shared_mutex> locker(buffer_mtx_);
    return BufferPtr() + readPos_;
}

char* Buffer::WritePtr() {
    std::shared_lock<std::shared_mutex> locker(buffer_mtx_);
    return BufferPtr() + writePos_;
}

const char* Buffer::ReadPtr() const {
    std::shared_lock<std::shared_mutex> locker(buffer_mtx_);
    return BufferPtr() + readPos_;
}

const char* Buffer::WritePtr() const {
    std::shared_lock<std::shared_mutex> locker(buffer_mtx_);
    return BufferPtr() + writePos_;
}

void Buffer::ReadLen(size_t len) {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);
    assert(len <= writePos_ - readPos_);
    readPos_ += len;
}

void Buffer::ReadUntil(const char* end) {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);
    assert(buffer_.data() + writePos_ >= end);
    readPos_ += (end - buffer_.data() - readPos_);
}

void Buffer::ReadAll() {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::ReadAllToStr() {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);
    assert(writePos_ >= readPos_);
    size_t readable_len = writePos_ - readPos_;
    std::string str(BufferPtr() + readPos_, readable_len);
    readPos_ += readable_len;

    return str;
}

void Buffer::Append(const char* data, size_t len) {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);
    EnsureWritable(len);
    std::copy(data, data + len, BufferPtr() + writePos_);
    writePos_ += len;
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.size());
}

void Buffer::Append(const Buffer& buffer) {
    Append(buffer.ReadPtr(), buffer.ReadableLen());
}

int Buffer::AppendFormatted(const char* format, va_list args) {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);

    va_list args_copy;
    va_copy(args_copy, args);
    int len = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (len <= 0) {
        return len;
    }
    EnsureWritable(len + 1);

    va_copy(args_copy, args);
    len = std::vsnprintf(buffer_.data() + writePos_, buffer_.size() - writePos_, format, args_copy);
    va_end(args_copy);

    if (len > 0) {
        writePos_ += len;
    }

    return len;
}

ssize_t Buffer::ReadFromFd(int fd, int* saveErrno) {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);
    char buff[65535];
    struct iovec iov[2];

    size_t write_size = buffer_.size() - writePos_;
    iov[0].iov_base = BufferPtr() + writePos_;
    iov[0].iov_len = write_size;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        *saveErrno = errno;
    } else if (static_cast<size_t>(len) <= write_size) {
        writePos_ += len;
    } else {
        writePos_ = buffer_.size();
        size_t extra_len = len - write_size;
        EnsureWritable(extra_len);
        std::copy(buff, buff + extra_len, BufferPtr() + writePos_);
        writePos_ += extra_len;
    }

    return len;
}

ssize_t Buffer::WriteToFd(int fd, int* saveErrno) {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);

    assert(writePos_ >= readPos_);
    size_t read_size = writePos_ - readPos_;

    ssize_t len = write(fd, ReadPtr(), read_size);
    if (len < 0) {
        *saveErrno = errno;
        return len;
    }
    readPos_ += len;

    return len;
}

void Buffer::Clear() {
    std::unique_lock<std::shared_mutex> locker(buffer_mtx_);
    buffer_.clear();
    readPos_ = 0;
    writePos_ = 0;
}

char* Buffer::BufferPtr() {
    return buffer_.data();
}

const char* Buffer::BufferPtr() const {
    return buffer_.data();
}

void Buffer::ExpandBuffer(size_t len) {
    if (buffer_.size() - writePos_ + readPos_ < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        size_t readable = writePos_ - readPos_;
        std::copy(BufferPtr() + readPos_, BufferPtr() + writePos_, BufferPtr());
        readPos_ = 0;
        writePos_ = readable;
    }
}

void Buffer::EnsureWritable(size_t len) {
    if (buffer_.size() - writePos_ >= len) return;
    ExpandBuffer(len);
}
