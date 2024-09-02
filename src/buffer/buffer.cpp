/**
 * @file buffer.cpp
 * @author chenyinjie
 * @date 2024-09-01 
 */


#include "buffer.h"

char* Buffer::beginWrite() noexcept {
    return buffer_.data() + writePos_;
}

const char* Buffer::beginWrite() const noexcept {
    return buffer_.data() + writePos_;
}

void Buffer::expand(size_t len) {
    if (writable_bytes() + prependable_bytes() < len) {
        buffer_.resize(writePos_ + len);
    } else {
        size_t rb = readable_bytes();
        std::memmove(buffer_.data(), buffer_.data() + readPos_, rb);
        readPos_ = 0;
        writePos_ = rb;
    }
}

Buffer::Buffer(size_t init_size): buffer_(init_size), readPos_(0), writePos_(0) {}

size_t Buffer::writable_bytes() const noexcept {
    return buffer_.size() - writePos_;
}

size_t Buffer::readable_bytes() const noexcept {
    return writePos_ - readPos_;
}

size_t Buffer::prependable_bytes() const noexcept {
    return readPos_;
}

const char* Buffer::read_peek() const noexcept {
    return buffer_.data() + readPos_;
}

void Buffer::check_writable(size_t len) {
    if (writable_bytes() < len) {
        expand(len);
    }
    assert(writable_bytes() >= len);
}

void Buffer::update_write_pos(size_t len) noexcept {
    assert(writable_bytes() >= len);
    writePos_ += len;
}

void Buffer::read_bytes(size_t len) noexcept {
    assert(readable_bytes() >= len);
    readPos_ += len;
}

void Buffer::clear() noexcept {
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::read_and_clear() {
    std::string data(read_peek(), readable_bytes());
    clear();
    return data;
}

void Buffer::append(std::string_view data) {
    check_writable(data.size());
    std::memcpy(beginWrite(), data.data(), data.size());
    update_write_pos(data.size());
}

ssize_t Buffer::read_from_fd(int fd, int* Errno) {
    char extra_buffer[65536];
    // 设置缓冲区和额外缓冲区写入数组
    iovec vec[2];
    const size_t writableBytes = writable_bytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writableBytes;
    vec[1].iov_base = extra_buffer;
    vec[1].iov_len = sizeof(extra_buffer);

    const ssize_t len = ::readv(fd, vec, 2);
    if (len < 0) {
        *Errno = errno;
        return len;   
    } else if (static_cast<size_t>(len) <= writableBytes) {
        update_write_pos(len);
    } else {
        writePos_ = buffer_.size();
        append(std::string_view(extra_buffer, len - writableBytes));
    }

    return len;
}

ssize_t Buffer::write_to_fd(int fd, int* Errno) {
    const ssize_t len = ::write(fd, read_peek(), readable_bytes());
    if (len < 0) {
        *Errno = errno;
        return len;
    }
    read_bytes(len);

    return len;
}
