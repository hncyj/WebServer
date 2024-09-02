/**
 * @file block_queue.h
 * @author chenyinjie
 * @date 2024-09-02
 */

#ifndef BLOCK_QUEUE_H_
#define BLOCK_QUEUE_H_

#include <deque>
#include <chrono>
#include <cassert>
#include <mutex>
#include <condition_variable>

template <typename T>
class BlockDeque {
private:
    std::deque<T> block_deque_;
    bool is_close_;
    size_t deque_capacity_;
    mutable std::mutex deque_mutex_;
    std::condition_variable consumer_con_var_;
    std::condition_variable producer_con_var_;

public:
    explicit BlockDeque(size_t max_capacity = 1024);
    ~BlockDeque();

    void clear();
    void close();
    bool isEmpty() const noexcept;
    bool isFull() const noexcept;
    size_t get_size() const noexcept;
    size_t get_capacity() const noexcept;
    T* get_front_element();
    T* get_last_element();

    void push_front(const T&);
    void push_back(const T&);
    bool pop(T&);
    bool pop(T&, int);  // 超时版本
    
    void flush();
};

template <typename T>
BlockDeque<T>::BlockDeque(size_t max_capacity): deque_capacity_(max_capacity), is_close_(false) {
    assert(deque_capacity_ > 0);
}

template <typename T>
BlockDeque<T>::~BlockDeque() {
    close();
}

template <typename T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> lock(deque_mutex_);
    block_deque_.clear();
}

template <typename T>
void BlockDeque<T>::close() {
    {
        std::unique_lock<std::mutex> lock(deque_mutex_);
        block_deque_.clear();
        is_close_ = true;
    }
    consumer_con_var_.notify_all();
    producer_con_var_.notify_all();
}

template <typename T>
bool BlockDeque<T>::isEmpty() const noexcept {
    std::lock_guard<std::mutex> lock(deque_mutex_);
    return block_deque_.empty();
}

template <typename T>
bool BlockDeque<T>::isFull() const noexcept {
    std::lock_guard<std::mutex> lock(deque_mutex_);
    return block_deque_.size() >= deque_capacity_;
}

template <typename T>
size_t BlockDeque<T>::get_size() const noexcept {
    std::lock_guard<std::mutex> lock(deque_mutex_);
    return block_deque_.size();
}

template <typename T>
size_t BlockDeque<T>::get_capacity() const noexcept {
    std::lock_guard<std::mutex> lock(deque_mutex_);
    return deque_capacity_;
}

template <typename T>
T* BlockDeque<T>::get_front_element() {
    std::lock_guard<std::mutex> lock(deque_mutex_);
    if (block_deque_.empty()) return nullptr;
    return &block_deque_.front();
}

template <typename T>
T* BlockDeque<T>::get_last_element() {
    std::lock_guard<std::mutex> lock(deque_mutex_);
    if (block_deque_.empty()) return nullptr;
    return &block_deque_.back();
}

template <typename T>
void BlockDeque<T>::push_front(const T& element) {
    std::unique_lock<std::mutex> lock(deque_mutex_);
    producer_con_var_.wait(lock, [this] {return block_deque_.size() < deque_capacity_;});
    if (is_close_) return;
    block_deque_.push_front(element);
    consumer_con_var_.notify_one();
}

template <typename T>
void BlockDeque<T>::push_back(const T& element) {
    std::unique_lock<std::mutex> lock(deque_mutex_);
    producer_con_var_.wait(lock, [this] {return block_deque_.size() < deque_capacity_;});
    if (is_close_) return;
    block_deque_.push_back(element);
    consumer_con_var_.notify_one();
}

template <typename T>
bool BlockDeque<T>::pop(T& element) {
    std::unique_lock<std::mutex> lock(deque_mutex_);
    consumer_con_var_.wait(lock, [this] {return !block_deque_.empty() || is_close_;});
    if (is_close_ && block_deque_.empty()) return false;
    element = std::move(block_deque_.front());
    block_deque_.pop_front();
    producer_con_var_.notify_one();
    return true;
}

template <typename T>
bool BlockDeque<T>::pop(T& element, int timeout) {
    std::unique_lock<std::mutex> lock(deque_mutex_);
    if (!consumer_con_var_.wait_for(lock, std::chrono::seconds(timeout), [this] {
        return !block_deque_.empty() || is_close_;
    })) {
        return false;
    };

    if (block_deque_.empty() && is_close_) return false;
    element = std::move(block_deque_.front());
    block_deque_.pop_front();
    producer_con_var_.notify_one();
    return true;
}

template <typename T>
void BlockDeque<T>::flush() {
    consumer_con_var_.notify_all();
}

#endif
