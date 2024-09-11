/**
 * @file buffer.cpp
 * @author chenyinjie
 * @date 2024-09-01 
 */

#include <cassert>
#include <string>

#include "buffer.h"

Buffer::Buffer(int init_size): buffer_(init_size), readPos_(0), writePos_(0) {}

void Buffer::Clear() {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    readPos_ = 0;
    writePos_ = 0;
}

size_t Buffer::ReadableLen() const {
  return writePos_ - readPos_;
}

size_t Buffer::WritableLen() const {
    return buffer_.size() - writePos_;
}

char* Buffer::ReadPtr() const {
    return const_cast<char*>(BufferPtr() + readPos_);
}

char* Buffer::WritePtr() const {
    return const_cast<char*>(BufferPtr() + writePos_);
}

std::string Buffer::ReadAllToStr() {
  std::lock_guard<std::mutex> lock(buffer_mtx_);
  size_t readable_len = ReadableLen();
  std::string str(ReadPtr(), readable_len);
  readPos_ += readable_len;

  return str;
}

void Buffer::Append(const char* data, size_t len) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    EnsureWritable(len);
    std::copy(data, data + len, WritePtr());
    writePos_ += len;
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.size());
}

void Buffer::Append(const Buffer& buf) {
    Append(buf.ReadPtr(), buf.ReadableLen());
}

int Buffer::AppendFormatted(const char* format, va_list args) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    va_list args_copy;
    va_copy(args_copy, args);
    int len = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (len <= 0) {
        return len;
    }
    EnsureWritable(len + 1);

    va_copy(args_copy, args);
    len = std::vsnprintf(WritePtr(), WritableLen(), format, args_copy);
    va_end(args_copy);

    if (len > 0) {
        writePos_ += len;
    }

    return len;
}

ssize_t Buffer::ReadFromFd(int fd, int* saveErrno) {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    char buff[65535];
    struct iovec iov[2];
    size_t write_size = WritableLen();
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
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    size_t read_size = ReadableLen();

    ssize_t len = write(fd, ReadPtr(), read_size);
    if (len < 0) {
        *saveErrno = errno;
        return len;
    }
    readPos_ += len;

    return len;
}

char* Buffer::BufferPtr() {
    return buffer_.data();
}

const char* Buffer::BufferPtr() const {
    return buffer_.data();
}

void Buffer::ExpandBuffer(size_t len) {
    if (WritableLen() + readPos_ < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        std::copy(BufferPtr() + readPos_, BufferPtr() + writePos_, BufferPtr());
        readPos_ = 0;
        writePos_ = ReadableLen();
    }
}

void Buffer::EnsureWritable(size_t len) {
    if (WritableLen() < len) {
        ExpandBuffer(len);
    }
}
